#include <filesystem>
#include <vector>

#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

#include "../../app/windows/RunController.h"
#include "../../core/gpu/GpuBackendInfo.h"

namespace fs = std::filesystem;

namespace {

using disassemble::core::ProcessingBackend;
using disassemble::desktop::RunController;

double meanPixelDifference(const fs::path &leftPath, const fs::path &rightPath)
{
    const cv::Mat left = cv::imread(leftPath.string(), cv::IMREAD_UNCHANGED);
    const cv::Mat right = cv::imread(rightPath.string(), cv::IMREAD_UNCHANGED);
    if (left.empty() || right.empty()) {
        return 9999.0;
    }
    if (left.size() != right.size() || left.type() != right.type()) {
        return 9999.0;
    }

    cv::Mat diff;
    cv::absdiff(left, right, diff);
    const cv::Scalar mean = cv::mean(diff);

    double total = 0.0;
    for (int index = 0; index < 4; ++index) {
        total += mean[index];
    }
    return total / 4.0;
}

} // namespace

int main()
{
    const fs::path sourceRoot(DISASSEMBLEIMAGE_SOURCE_ROOT);
    const fs::path tempRoot = sourceRoot / "build" / "phase5-smoke-output";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);

    const fs::path sourceImage = sourceRoot / "HD" / "a0000.jpg";
    const auto backendInfo = RunController::probeGpuBackend();

    auto cpuTask = RunController::buildSmokeTask(sourceImage, tempRoot / "cpu-run", sourceRoot);
    cpuTask.outputSizes = {{640, 480}};
    cpuTask.processingBackend = ProcessingBackend::Cpu;

    auto autoTask = RunController::buildSmokeTask(sourceImage, tempRoot / "auto-run", sourceRoot);
    autoTask.outputSizes = {{640, 480}};
    autoTask.processingBackend = ProcessingBackend::Auto;

    auto gpuTask = RunController::buildSmokeTask(sourceImage, tempRoot / "gpu-run", sourceRoot);
    gpuTask.outputSizes = {{640, 480}};
    gpuTask.processingBackend = ProcessingBackend::Gpu;

    const auto cpuResult = RunController::runTask(cpuTask);
    if (!cpuResult.ok() || cpuResult.outputFiles.empty()) {
        return 1;
    }
    if (cpuResult.requestedBackend != ProcessingBackend::Cpu || cpuResult.activeBackend != ProcessingBackend::Cpu) {
        return 2;
    }
    if (cpuResult.totalProcessingMs <= 0.0) {
        return 3;
    }

    const auto autoResult = RunController::runTask(autoTask);
    if (!autoResult.ok() || autoResult.outputFiles.empty()) {
        return 4;
    }

    const auto gpuResult = RunController::runTask(gpuTask);
    if (!gpuResult.ok() || gpuResult.outputFiles.empty()) {
        return 5;
    }

    if (backendInfo.canUseGpu()) {
        if (autoResult.activeBackend != ProcessingBackend::Gpu) {
            return 6;
        }
        if (gpuResult.activeBackend != ProcessingBackend::Gpu) {
            return 7;
        }
        if (gpuResult.gpuHotPathMs <= 0.0) {
            return 8;
        }
        if (cpuResult.outputFiles.size() != gpuResult.outputFiles.size()) {
            return 9;
        }

        const fs::path cpuOutput(cpuResult.outputFiles.front());
        const fs::path gpuOutput(gpuResult.outputFiles.front());
        const cv::Mat cpuImage = cv::imread(cpuOutput.string(), cv::IMREAD_UNCHANGED);
        const cv::Mat gpuImage = cv::imread(gpuOutput.string(), cv::IMREAD_UNCHANGED);
        if (cpuImage.empty() || gpuImage.empty()) {
            return 10;
        }
        if (cpuImage.size() != gpuImage.size()) {
            return 11;
        }
        if (meanPixelDifference(cpuOutput, gpuOutput) > 2.0) {
            return 12;
        }
    } else {
        if (autoResult.activeBackend != ProcessingBackend::Cpu || autoResult.fallbackReason.empty()) {
            return 13;
        }
        if (gpuResult.activeBackend != ProcessingBackend::Cpu || gpuResult.fallbackReason.empty()) {
            return 14;
        }
    }

    return 0;
}
