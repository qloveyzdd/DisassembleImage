#include "CpuDisassemblyRunner.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <unordered_map>
#include <utility>

#include <opencv2/core/ocl.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "../gpu/GpuBackendInfo.h"
#include "../io/ImageCatalog.h"
#include "../model/FacePositionPrefix.h"
#include "../model/ObjModel.h"
#include "../model/PreviewGalleryItem.h"
#include "../model/SlicePlan.h"

namespace fs = std::filesystem;

namespace {

using disassemble::core::GpuBackendInfo;
using disassemble::core::ImageSize;
using disassemble::core::OutputConflictPolicy;
using disassemble::core::ProcessingBackend;
using disassemble::core::ProcessingDirection;
using disassemble::core::ProcessingTask;
using disassemble::core::RunFailure;
using disassemble::core::RunProgress;
using disassemble::core::RunResult;
using disassemble::core::RunStage;
using disassemble::core::SlicePrimitive;

struct ScopedOpenClState {
    explicit ScopedOpenClState(bool enabled)
        : previous_(cv::ocl::useOpenCL())
    {
        cv::ocl::setUseOpenCL(enabled);
    }

    ~ScopedOpenClState()
    {
        cv::ocl::setUseOpenCL(previous_);
    }

    bool previous_ = false;
};

struct ExecutionMode {
    ProcessingBackend requestedBackend = ProcessingBackend::Auto;
    ProcessingBackend activeBackend = ProcessingBackend::Cpu;
    std::string acceleratorName;
    std::string fallbackReason;

    bool useGpu() const
    {
        return activeBackend == ProcessingBackend::Gpu;
    }
};

struct SingleImageMetrics {
    double gpuHotPathMs = 0.0;
};

struct ProcessingContextKey {
    int width = 0;
    int height = 0;

    bool operator==(const ProcessingContextKey &other) const
    {
        return width == other.width && height == other.height;
    }
};

struct ProcessingContextKeyHash {
    size_t operator()(const ProcessingContextKey &key) const
    {
        return (static_cast<size_t>(static_cast<unsigned int>(key.width)) << 32)
            ^ static_cast<size_t>(static_cast<unsigned int>(key.height));
    }
};

struct RemapCacheEntry {
    cv::Mat mapX;
    cv::Mat mapY;
    cv::Size outputSize;
};

RemapCacheEntry buildRemapCacheEntry(const SlicePrimitive &primitive)
{
    const auto quadPoint = primitive.quadPoints[2];
    RemapCacheEntry entry;
    entry.outputSize = cv::Size(static_cast<int>(quadPoint.x), static_cast<int>(quadPoint.y));
    entry.mapX = cv::Mat(entry.outputSize, CV_32FC1);
    entry.mapY = cv::Mat(entry.outputSize, CV_32FC1);

    cv::Mat inverseTransform;
    primitive.transform.convertTo(inverseTransform, CV_64F);
    inverseTransform = inverseTransform.inv();

    for (int y = 0; y < entry.outputSize.height; ++y) {
        float *mapXRow = entry.mapX.ptr<float>(y);
        float *mapYRow = entry.mapY.ptr<float>(y);
        for (int x = 0; x < entry.outputSize.width; ++x) {
            const double homogeneous = inverseTransform.at<double>(2, 0) * x
                + inverseTransform.at<double>(2, 1) * y
                + inverseTransform.at<double>(2, 2);
            mapXRow[x] = static_cast<float>((inverseTransform.at<double>(0, 0) * x
                + inverseTransform.at<double>(0, 1) * y
                + inverseTransform.at<double>(0, 2)) / homogeneous);
            mapYRow[x] = static_cast<float>((inverseTransform.at<double>(1, 0) * x
                + inverseTransform.at<double>(1, 1) * y
                + inverseTransform.at<double>(1, 2)) / homogeneous);
        }
    }

    return entry;
}

struct CachedProcessingContext {
    explicit CachedProcessingContext(const ProcessingTask &task,
                                     int inputWidth,
                                     int inputHeight)
    {
        const ImageSize inputSize{inputWidth, inputHeight};
        inputModel = std::make_unique<disassemble::core::PaddedUvModel>(disassemble::core::PaddedUvModel::load(task.inputObjPath));

        if (task.usesGroupedOutput()) {
            outputModel = std::make_unique<disassemble::core::ObjModel>(disassemble::core::ObjModel::load(task.outputObjPath));
            primitives = disassemble::core::SlicePlanBuilder::buildGrouped(*inputModel,
                                                                           *outputModel,
                                                                           inputSize,
                                                                           task.outputSizes.front(),
                                                                           task.direction);
            prefixes = task.prefixes;
        } else {
            primitives = disassemble::core::SlicePlanBuilder::buildPerFace(*inputModel, inputSize, task.outputSizes);
            prefixes = disassemble::core::buildFacePositionPrefixes(task.inputObjPath);
        }

        remapEntries.reserve(primitives.size());
        for (const auto &primitive : primitives) {
            remapEntries.push_back(buildRemapCacheEntry(primitive));
        }
    }

    std::unique_ptr<disassemble::core::PaddedUvModel> inputModel;
    std::unique_ptr<disassemble::core::ObjModel> outputModel;
    std::vector<SlicePrimitive> primitives;
    std::vector<std::string> prefixes;
    std::vector<RemapCacheEntry> remapEntries;
};

class ProcessingContextCache {
public:
    explicit ProcessingContextCache(const ProcessingTask &task)
        : task_(task)
    {
    }

    std::shared_ptr<const CachedProcessingContext> get(int inputWidth, int inputHeight)
    {
        const ProcessingContextKey key{inputWidth, inputHeight};
        std::lock_guard<std::mutex> lock(mutex_);
        const auto it = contexts_.find(key);
        if (it != contexts_.end()) {
            return it->second;
        }

        auto context = std::make_shared<CachedProcessingContext>(task_, inputWidth, inputHeight);
        contexts_.emplace(key, context);
        return context;
    }

private:
    ProcessingTask task_;
    std::mutex mutex_;
    std::unordered_map<ProcessingContextKey,
                       std::shared_ptr<const CachedProcessingContext>,
                       ProcessingContextKeyHash> contexts_;
};

std::string prefixedFileName(const std::string &prefix, const fs::path &inputPath)
{
    return prefix + inputPath.filename().string();
}

std::string displayNameFor(const fs::path &outputPath)
{
    return outputPath.stem().string();
}

double elapsedMilliseconds(const std::chrono::steady_clock::time_point &start,
                           const std::chrono::steady_clock::time_point &end)
{
    return std::chrono::duration<double, std::milli>(end - start).count();
}

fs::path resolveOutputPath(const fs::path &preferredPath, OutputConflictPolicy policy)
{
    if (policy == OutputConflictPolicy::OverwriteExisting || !fs::exists(preferredPath)) {
        return preferredPath;
    }

    if (policy == OutputConflictPolicy::ForbidOverwrite) {
        throw std::runtime_error(std::string(u8"输出文件已存在: ") + preferredPath.string());
    }

    const auto stem = preferredPath.stem().string();
    const auto extension = preferredPath.extension().string();
    for (int index = 1; index <= 9999; ++index) {
        const auto candidate = preferredPath.parent_path() / (stem + "_" + std::to_string(index) + extension);
        if (!fs::exists(candidate)) {
            return candidate;
        }
    }

    throw std::runtime_error(std::string(u8"无法为输出文件生成新的名称: ") + preferredPath.string());
}

void ensureOutputDirectories(const ProcessingTask &task)
{
    const auto prefixes = task.usesGroupedOutput()
        ? task.prefixes
        : disassemble::core::buildFacePositionPrefixes(task.inputObjPath);
    for (const auto &prefix : prefixes) {
        fs::create_directories(fs::path(task.outputRoot) / prefix);
    }
}

void validateTask(const ProcessingTask &task)
{
    if (task.outputRoot.empty()) {
        throw std::runtime_error(u8"未设置输出目录");
    }
    if (task.inputObjPath.empty() || !fs::exists(task.inputObjPath)) {
        throw std::runtime_error(u8"input.obj 不存在");
    }
    if (task.usesGroupedOutput()) {
        if (task.outputObjPath.empty() || !fs::exists(task.outputObjPath)) {
            throw std::runtime_error(u8"分组模式需要 output.obj");
        }
        if (task.outputSizes.empty()) {
            throw std::runtime_error(u8"分组模式至少需要一个输出尺寸");
        }
        if (task.prefixes.empty()) {
            throw std::runtime_error(u8"分组模式至少需要一个输出前缀");
        }
        return;
    }
    if (task.outputSizes.empty()) {
        throw std::runtime_error(u8"一对一模式需要输出尺寸");
    }
}

ExecutionMode buildExecutionMode(const ProcessingTask &task, const GpuBackendInfo &backendInfo)
{
    ExecutionMode mode;
    mode.requestedBackend = task.processingBackend;
    mode.activeBackend = disassemble::core::resolveActiveBackend(task.processingBackend,
                                                                 backendInfo,
                                                                 mode.fallbackReason);
    if (backendInfo.canUseGpu()) {
        mode.acceleratorName = backendInfo.deviceName;
    }
    return mode;
}

cv::Mat warpPiece(const cv::Mat &inputImage,
                  const SlicePrimitive &primitive,
                  const RemapCacheEntry *remapEntry,
                  bool useGpu,
                  double &gpuHotPathMs)
{
    const auto quadPoint = primitive.quadPoints[2];
    const cv::Size outputSize(static_cast<int>(quadPoint.x), static_cast<int>(quadPoint.y));

    if (!useGpu) {
        cv::Mat output;
        if (remapEntry != nullptr) {
            cv::remap(inputImage,
                      output,
                      remapEntry->mapX,
                      remapEntry->mapY,
                      cv::INTER_LINEAR,
                      cv::BORDER_CONSTANT);
        } else {
            cv::warpPerspective(inputImage, output, primitive.transform, outputSize);
        }
        return output;
    }

    ScopedOpenClState openClGuard(true);
    cv::UMat gpuInput;
    inputImage.copyTo(gpuInput);
    cv::UMat gpuOutput;
    const auto start = std::chrono::steady_clock::now();
    cv::warpPerspective(gpuInput, gpuOutput, primitive.transform, outputSize);
    const auto end = std::chrono::steady_clock::now();
    gpuHotPathMs += elapsedMilliseconds(start, end);

    cv::Mat output;
    gpuOutput.copyTo(output);
    return output;
}

cv::Mat resizeMergedImage(const cv::Mat &merged,
                          const ImageSize &size,
                          bool useGpu,
                          double &gpuHotPathMs)
{
    if (!useGpu) {
        cv::Mat resized;
        cv::resize(merged, resized, cv::Size(size.width, size.height));
        return resized;
    }

    ScopedOpenClState openClGuard(true);
    cv::UMat gpuMerged;
    merged.copyTo(gpuMerged);
    cv::UMat gpuResized;
    const auto start = std::chrono::steady_clock::now();
    cv::resize(gpuMerged, gpuResized, cv::Size(size.width, size.height));
    const auto end = std::chrono::steady_clock::now();
    gpuHotPathMs += elapsedMilliseconds(start, end);

    cv::Mat resized;
    gpuResized.copyTo(resized);
    return resized;
}

void appendFailure(RunResult &result, const fs::path &inputPath, const std::exception &error)
{
    ++result.failedCount;
    result.failures.push_back(RunFailure{inputPath.string(), error.what()});
    result.logs.push_back(std::string(u8"处理失败: ") + inputPath.string() + " - " + error.what());
}

void runSingleImage(const ProcessingTask &task,
                    const fs::path &inputPath,
                    bool useGpu,
                    ProcessingContextCache &contextCache,
                    RunResult &result,
                    std::mutex &resultMutex)
{
    SingleImageMetrics metrics;
    cv::Mat inputImage = cv::imread(inputPath.string(), cv::IMREAD_UNCHANGED);
    if (inputImage.empty()) {
        throw std::runtime_error(std::string(u8"读取图片失败: ") + inputPath.string());
    }

    const auto context = contextCache.get(inputImage.cols, inputImage.rows);

    if (!task.usesGroupedOutput() && task.outputSizes.size() != context->primitives.size()) {
        throw std::runtime_error(u8"一对一模式下输出尺寸数量必须与 input.obj 面数一致");
    }

    std::vector<std::string> generatedFiles;
    std::vector<disassemble::core::PreviewGalleryItem> generatedPreviewItems;

    if (!task.usesGroupedOutput()) {
        if (context->prefixes.size() != context->primitives.size()) {
            throw std::runtime_error(u8"一对一模式下无法根据面位置生成完整前缀");
        }

        for (size_t index = 0; index < context->primitives.size(); ++index) {
            cv::Mat quad = warpPiece(inputImage,
                                     context->primitives[index],
                                     &context->remapEntries[index],
                                     useGpu,
                                     metrics.gpuHotPathMs);
            const fs::path outputDir = fs::path(task.outputRoot) / context->prefixes[index];
            const fs::path outputPath = resolveOutputPath(outputDir / prefixedFileName(context->prefixes[index], inputPath),
                                                          task.outputConflictPolicy);
            if (!cv::imwrite(outputPath.string(), quad)) {
                throw std::runtime_error(std::string(u8"写出图片失败: ") + outputPath.string());
            }

            generatedFiles.push_back(outputPath.string());
            disassemble::core::PreviewGalleryItem item;
            item.inputImagePath = inputPath.string();
            item.outputImagePath = outputPath.string();
            item.displayName = displayNameFor(outputPath);
            item.faceIndex = static_cast<int>(index);
            generatedPreviewItems.push_back(item);
        }
    } else {
        std::vector<cv::Mat> pieces;
        for (size_t index = 0; index < context->primitives.size(); ++index) {
            pieces.push_back(warpPiece(inputImage,
                                       context->primitives[index],
                                       &context->remapEntries[index],
                                       useGpu,
                                       metrics.gpuHotPathMs));
        }

        cv::Mat merged;
        if (task.direction == ProcessingDirection::X) {
            cv::hconcat(pieces, merged);
        } else {
            cv::vconcat(pieces, merged);
        }
        merged = resizeMergedImage(merged, task.outputSizes.front(), useGpu, metrics.gpuHotPathMs);

        const fs::path outputDir = fs::path(task.outputRoot) / task.prefixes.front();
        const fs::path outputPath = resolveOutputPath(outputDir / prefixedFileName(task.prefixes.front(), inputPath),
                                                      task.outputConflictPolicy);
        if (!cv::imwrite(outputPath.string(), merged)) {
            throw std::runtime_error(std::string(u8"写出图片失败: ") + outputPath.string());
        }

        generatedFiles.push_back(outputPath.string());
        disassemble::core::PreviewGalleryItem item;
        item.inputImagePath = inputPath.string();
        item.outputImagePath = outputPath.string();
        item.displayName = displayNameFor(outputPath);
        generatedPreviewItems.push_back(item);
    }

    std::lock_guard<std::mutex> lock(resultMutex);
    ++result.successCount;
    result.gpuHotPathMs += metrics.gpuHotPathMs;
    result.logs.push_back(std::string(u8"处理完成: ") + inputPath.string());
    result.outputFiles.insert(result.outputFiles.end(), generatedFiles.begin(), generatedFiles.end());
    result.previewItems.insert(result.previewItems.end(), generatedPreviewItems.begin(), generatedPreviewItems.end());
}

} // namespace

namespace disassemble::core {

bool ProcessingTask::hasSingleImage() const
{
    return !inputImagePath.empty();
}

bool ProcessingTask::hasInputDirectory() const
{
    return !inputDirectory.empty();
}

bool ProcessingTask::usesGroupedOutput() const
{
    return direction != ProcessingDirection::None;
}

bool RunResult::ok() const
{
    return failedCount == 0;
}

void RunResult::addLog(std::string message)
{
    logs.push_back(std::move(message));
}

RunResult CpuDisassemblyRunner::run(const ProcessingTask &task,
                                    const GpuBackendInfo &backendInfo,
                                    ProgressCallback onProgress,
                                    CancelCheck isCancelRequested) const
{
    const auto overallStart = std::chrono::steady_clock::now();
    auto mode = buildExecutionMode(task, backendInfo);

    auto emitProgress = [&](RunStage stage,
                            int totalInputs,
                            int completedInputs,
                            int successCount,
                            int failedCount,
                            bool cancelRequested,
                            std::string currentInputPath = std::string()) {
        if (!onProgress) {
            return;
        }

        RunProgress progress;
        progress.stage = stage;
        progress.totalInputs = totalInputs;
        progress.completedInputs = completedInputs;
        progress.successCount = successCount;
        progress.failedCount = failedCount;
        progress.cancelRequested = cancelRequested;
        progress.requestedBackend = mode.requestedBackend;
        progress.activeBackend = mode.activeBackend;
        progress.acceleratorName = mode.acceleratorName;
        progress.fallbackReason = mode.fallbackReason;
        progress.elapsedMs = elapsedMilliseconds(overallStart, std::chrono::steady_clock::now());
        progress.currentInputPath = std::move(currentInputPath);
        onProgress(progress);
    };

    emitProgress(RunStage::Validating, 0, 0, 0, 0, false);
    validateTask(task);
    emitProgress(RunStage::PreparingOutput, 0, 0, 0, 0, false);
    ensureOutputDirectories(task);

    emitProgress(RunStage::CollectingInputs, 0, 0, 0, 0, false);
    auto inputs = ImageCatalog::collect(task);
    if (inputs.empty()) {
        throw std::runtime_error(u8"没有找到可处理的输入图片");
    }

    RunResult result;
    result.outputRoot = task.outputRoot;
    result.requestedBackend = mode.requestedBackend;
    result.activeBackend = mode.activeBackend;
    result.acceleratorName = mode.acceleratorName;
    result.fallbackReason = mode.fallbackReason;
    result.logs.push_back(std::string(u8"开始处理 ") + std::to_string(inputs.size()) + u8" 个输入文件");
    if (mode.useGpu()) {
        result.logs.push_back(std::string(u8"已启用 GPU 后端: ") + mode.acceleratorName);
    } else if (!mode.fallbackReason.empty()) {
        result.logs.push_back(mode.fallbackReason);
    }

    std::mutex resultMutex;
    ProcessingContextCache contextCache(task);
    std::atomic_size_t cursor{0};
    std::atomic_bool stopRequested{false};
    const unsigned int workerCount = mode.useGpu()
        ? 1U
        : (task.enableParallel && task.maxWorkers > 1
            ? std::min<unsigned int>(task.maxWorkers, static_cast<unsigned int>(inputs.size()))
            : 1U);

    if (mode.useGpu()) {
        result.logs.push_back(u8"GPU 路径当前使用单线程执行，以保证稳定回退到 CPU。");
    } else {
        result.logs.push_back(std::string(u8"CPU 路径并行线程数: ") + std::to_string(workerCount));
    }

    emitProgress(RunStage::Processing, static_cast<int>(inputs.size()), 0, 0, 0, false);

    auto worker = [&]() {
        while (true) {
            if (isCancelRequested && isCancelRequested()) {
                stopRequested.store(true);
            }
            if (stopRequested.load()) {
                return;
            }

            const size_t index = cursor.fetch_add(1);
            if (index >= inputs.size()) {
                return;
            }

            int successBefore = 0;
            int failedBefore = 0;
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                successBefore = result.successCount;
                failedBefore = result.failedCount;
            }
            emitProgress(RunStage::Processing,
                         static_cast<int>(inputs.size()),
                         successBefore + failedBefore,
                         successBefore,
                         failedBefore,
                         stopRequested.load(),
                         inputs[index].string());

            try {
                runSingleImage(task, inputs[index], mode.useGpu(), contextCache, result, resultMutex);
            } catch (const std::exception &gpuError) {
                if (mode.useGpu()) {
                    mode.activeBackend = ProcessingBackend::Cpu;
                    mode.fallbackReason = std::string(u8"GPU 路径运行失败，已回退到 CPU：") + gpuError.what();
                    {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        result.activeBackend = ProcessingBackend::Cpu;
                        result.fallbackReason = mode.fallbackReason;
                        result.logs.push_back(mode.fallbackReason);
                    }

                    try {
                        runSingleImage(task, inputs[index], false, contextCache, result, resultMutex);
                    } catch (const std::exception &cpuError) {
                        std::lock_guard<std::mutex> lock(resultMutex);
                        appendFailure(result, inputs[index], cpuError);
                    }
                } else {
                    std::lock_guard<std::mutex> lock(resultMutex);
                    appendFailure(result, inputs[index], gpuError);
                }
            }

            int successAfter = 0;
            int failedAfter = 0;
            {
                std::lock_guard<std::mutex> lock(resultMutex);
                successAfter = result.successCount;
                failedAfter = result.failedCount;
            }
            if (isCancelRequested && isCancelRequested()) {
                stopRequested.store(true);
            }

            emitProgress(RunStage::Processing,
                         static_cast<int>(inputs.size()),
                         successAfter + failedAfter,
                         successAfter,
                         failedAfter,
                         stopRequested.load(),
                         inputs[index].string());
        }
    };

    std::vector<std::thread> workers;
    workers.reserve(workerCount);
    for (unsigned int i = 0; i < workerCount; ++i) {
        workers.emplace_back(worker);
    }

    for (auto &thread : workers) {
        thread.join();
    }

    result.cancelled = stopRequested.load();
    result.totalProcessingMs = elapsedMilliseconds(overallStart, std::chrono::steady_clock::now());
    if (result.cancelled) {
        result.logs.push_back(std::string(u8"处理已安全取消，成功 ")
            + std::to_string(result.successCount) + u8"，失败 " + std::to_string(result.failedCount));
        emitProgress(RunStage::Cancelled,
                     static_cast<int>(inputs.size()),
                     result.successCount + result.failedCount,
                     result.successCount,
                     result.failedCount,
                     true);
    } else {
        result.logs.push_back(std::string(u8"处理结束，成功 ")
            + std::to_string(result.successCount) + u8"，失败 " + std::to_string(result.failedCount));
        emitProgress(RunStage::Finished,
                     static_cast<int>(inputs.size()),
                     result.successCount + result.failedCount,
                     result.successCount,
                     result.failedCount,
                     false);
    }

    if (result.activeBackend == ProcessingBackend::Gpu) {
        result.consistencySummary = u8"GPU 路径已执行；请结合 smoke 和目标机器验证 CPU/GPU 输出差异。";
    } else if (!result.fallbackReason.empty()) {
        result.consistencySummary = u8"本次任务已回退到 CPU，当前结果以 CPU 路径为准。";
    } else {
        result.consistencySummary = u8"本次任务使用 CPU 路径完成。";
    }
    return result;
}

RunResult CpuDisassemblyRunner::run(const ProcessingTask &task,
                                    ProgressCallback onProgress,
                                    CancelCheck isCancelRequested) const
{
    return run(task, detectGpuBackend(), std::move(onProgress), std::move(isCancelRequested));
}

} // namespace disassemble::core
