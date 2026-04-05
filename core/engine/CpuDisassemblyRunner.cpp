#include "CpuDisassemblyRunner.h"

#include <algorithm>
#include <atomic>
#include <filesystem>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <thread>
#include <utility>

#include <opencv2/imgcodecs.hpp>
#include <opencv2/imgproc.hpp>

#include "../../mask/mask.h"
#include "../../server_info/server_info.h"
#include "../io/ImageCatalog.h"

namespace fs = std::filesystem;

namespace {

group_direction toLegacyDirection(disassemble::core::ProcessingDirection direction)
{
    switch (direction) {
    case disassemble::core::ProcessingDirection::None:
        return group_direction::NONE;
    case disassemble::core::ProcessingDirection::X:
        return group_direction::X;
    case disassemble::core::ProcessingDirection::Y:
        return group_direction::Y;
    }
    return group_direction::NONE;
}

std::vector<std::vector<int>> toLegacySizes(const std::vector<disassemble::core::ImageSize> &sizes)
{
    std::vector<std::vector<int>> converted;
    converted.reserve(sizes.size());
    for (const auto &size : sizes) {
        converted.push_back({size.width, size.height});
    }
    return converted;
}

std::string prefixedFileName(const std::string &prefix, const fs::path &inputPath)
{
    return prefix + inputPath.filename().string();
}

fs::path resolveOutputPath(const fs::path &preferredPath, disassemble::core::OutputConflictPolicy policy)
{
    if (policy == disassemble::core::OutputConflictPolicy::OverwriteExisting || !fs::exists(preferredPath)) {
        return preferredPath;
    }

    if (policy == disassemble::core::OutputConflictPolicy::ForbidOverwrite) {
        throw std::runtime_error("输出文件已存在: " + preferredPath.string());
    }

    const auto stem = preferredPath.stem().string();
    const auto extension = preferredPath.extension().string();
    for (int index = 1; index <= 9999; ++index) {
        const auto candidate = preferredPath.parent_path() / (stem + "_" + std::to_string(index) + extension);
        if (!fs::exists(candidate)) {
            return candidate;
        }
    }

    throw std::runtime_error("无法为输出文件生成新的名称: " + preferredPath.string());
}

void ensureOutputDirectories(const disassemble::core::ProcessingTask &task)
{
    for (const auto &prefix : task.prefixes) {
        fs::create_directories(fs::path(task.outputRoot) / prefix);
    }
}

void validateTask(const disassemble::core::ProcessingTask &task)
{
    if (task.outputRoot.empty()) {
        throw std::runtime_error("未设置输出目录");
    }
    if (task.inputObjPath.empty() || !fs::exists(task.inputObjPath)) {
        throw std::runtime_error("input.obj 不存在");
    }
    if (task.usesGroupedOutput()) {
        if (task.outputObjPath.empty() || !fs::exists(task.outputObjPath)) {
            throw std::runtime_error("分组模式需要 output.obj");
        }
        if (task.outputSizes.empty()) {
            throw std::runtime_error("分组模式至少需要一个输出尺寸");
        }
        if (task.prefixes.empty()) {
            throw std::runtime_error("分组模式至少需要一个输出前缀");
        }
        return;
    }
    if (task.outputSizes.empty() || task.prefixes.empty()) {
        throw std::runtime_error("一对一模式需要输出尺寸和输出前缀");
    }
}

void runSingleImage(const disassemble::core::ProcessingTask &task,
                    const fs::path &inputPath,
                    disassemble::core::RunResult &result,
                    std::mutex &resultMutex)
{
    cv::Mat inputImage = cv::imread(inputPath.string(), cv::IMREAD_UNCHANGED);
    if (inputImage.empty()) {
        throw std::runtime_error("读取图片失败: " + inputPath.string());
    }

    input_image_info inputInfo({inputImage.cols, inputImage.rows});
    output_image_info outputInfo(toLegacySizes(task.outputSizes));
    obj_uv_padding inputObj(task.inputObjPath);

    if (!task.usesGroupedOutput()) {
        if (task.outputSizes.size() != inputObj.get_prim().size()) {
            throw std::runtime_error("一对一模式下输出尺寸数量必须与 input.obj 面数一致");
        }
        if (task.prefixes.size() != inputObj.get_prim().size()) {
            throw std::runtime_error("一对一模式下输出前缀数量必须与 input.obj 面数一致");
        }
    }

    std::unique_ptr<obj_basic> outputObj;
    std::unique_ptr<disassembly_factory> factory;
    if (task.usesGroupedOutput()) {
        outputObj = std::make_unique<obj_basic>(task.outputObjPath);
        factory = std::make_unique<disassembly_factory>(&inputObj, outputObj.get(), &inputInfo, &outputInfo, toLegacyDirection(task.direction));
    } else {
        factory = std::make_unique<disassembly_factory>(&inputObj, &inputInfo, &outputInfo);
    }

    std::vector<std::string> generatedFiles;
    cv::Mat quad;

    if (!task.usesGroupedOutput()) {
        const auto prims = factory->get_prim();
        for (size_t index = 0; index < prims.size(); ++index) {
            const auto quadPoint = prims[index]->get_quad_pts()[2];
            cv::warpPerspective(inputImage, quad, prims[index]->get_transmtx(), cv::Size(static_cast<int>(quadPoint.x), static_cast<int>(quadPoint.y)));
            const fs::path outputDir = fs::path(task.outputRoot) / task.prefixes[index];
            const fs::path outputPath = resolveOutputPath(outputDir / prefixedFileName(task.prefixes[index], inputPath), task.outputConflictPolicy);
            if (!cv::imwrite(outputPath.string(), quad)) {
                throw std::runtime_error("写出图片失败: " + outputPath.string());
            }
            generatedFiles.push_back(outputPath.string());
        }
    } else {
        std::vector<cv::Mat> pieces;
        const auto prims = factory->get_prim();
        for (const auto *prim : prims) {
            const auto quadPoint = prim->get_quad_pts()[2];
            cv::warpPerspective(inputImage, quad, prim->get_transmtx(), cv::Size(static_cast<int>(quadPoint.x), static_cast<int>(quadPoint.y)));
            pieces.push_back(quad.clone());
        }

        cv::Mat merged;
        if (task.direction == disassemble::core::ProcessingDirection::X) {
            cv::hconcat(pieces, merged);
        } else {
            cv::vconcat(pieces, merged);
        }
        cv::resize(merged, merged, cv::Size(task.outputSizes.front().width, task.outputSizes.front().height));

        const fs::path outputDir = fs::path(task.outputRoot) / task.prefixes.front();
        const fs::path outputPath = resolveOutputPath(outputDir / prefixedFileName(task.prefixes.front(), inputPath), task.outputConflictPolicy);
        if (!cv::imwrite(outputPath.string(), merged)) {
            throw std::runtime_error("写出图片失败: " + outputPath.string());
        }
        generatedFiles.push_back(outputPath.string());
    }

    std::lock_guard<std::mutex> lock(resultMutex);
    ++result.successCount;
    result.logs.push_back("处理完成: " + inputPath.string());
    result.outputFiles.insert(result.outputFiles.end(), generatedFiles.begin(), generatedFiles.end());
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

RunResult CpuDisassemblyRunner::run(const ProcessingTask &task) const
{
    validateTask(task);
    ensureOutputDirectories(task);

    auto inputs = ImageCatalog::collect(task);
    if (inputs.empty()) {
        throw std::runtime_error("没有找到可处理的输入图片");
    }

    RunResult result;
    result.logs.push_back("开始处理 " + std::to_string(inputs.size()) + " 个输入文件");

    std::mutex resultMutex;
    std::atomic_size_t cursor{0};
    const unsigned int workerCount = task.enableParallel && task.maxWorkers > 1
        ? std::min<unsigned int>(task.maxWorkers, static_cast<unsigned int>(inputs.size()))
        : 1;

    auto worker = [&]() {
        while (true) {
            const size_t index = cursor.fetch_add(1);
            if (index >= inputs.size()) {
                return;
            }

            try {
                runSingleImage(task, inputs[index], result, resultMutex);
            } catch (const std::exception &error) {
                std::lock_guard<std::mutex> lock(resultMutex);
                ++result.failedCount;
                result.failures.push_back({inputs[index].string(), error.what()});
                result.logs.push_back("处理失败: " + inputs[index].string() + " - " + error.what());
            }
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

    result.logs.push_back("处理结束，成功 " + std::to_string(result.successCount) + "，失败 " + std::to_string(result.failedCount));
    return result;
}

} // namespace disassemble::core
