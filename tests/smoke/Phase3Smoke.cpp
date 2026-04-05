#include <atomic>
#include <filesystem>
#include <iostream>
#include <vector>

#include <QCoreApplication>

#include "../../app/windows/RunController.h"
#include "../../core/model/RunProgress.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const fs::path sourceRoot(DISASSEMBLEIMAGE_SOURCE_ROOT);
    const fs::path tempRoot = sourceRoot / "build" / "phase3-smoke-output";
    const fs::path inputDir = tempRoot / "inputs";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(inputDir, ec);

    const fs::path sourceImage = sourceRoot / "HD" / "a0000.jpg";
    fs::copy_file(sourceImage, inputDir / "a0000.jpg", fs::copy_options::overwrite_existing, ec);
    fs::copy_file(sourceImage, inputDir / "a0001.jpg", fs::copy_options::overwrite_existing, ec);

    auto task = disassemble::desktop::RunController::buildSmokeTask(sourceImage, tempRoot / "single-run", sourceRoot);
    task.outputSizes = {{640, 480}};

    int progressEvents = 0;
    bool sawProcessing = false;
    const auto result = disassemble::desktop::RunController::runTask(
        task,
        [&](const disassemble::core::RunProgress &progress) {
            ++progressEvents;
            if (progress.stage == disassemble::core::RunStage::Processing) {
                sawProcessing = true;
            }
        });

    if (!result.ok() || result.outputFiles.empty() || progressEvents == 0 || !sawProcessing) {
        std::cerr << "Phase 3 基础运行链路未通过" << std::endl;
        return 1;
    }

    for (const auto &output : result.outputFiles) {
        if (!fs::exists(output)) {
            std::cerr << "输出文件不存在: " << output << std::endl;
            return 2;
        }
    }

    std::vector<std::string> summaryLogs = {
        u8"开始处理测试任务",
        u8"处理完成"
    };

    const auto summaryPath = disassemble::desktop::RunController::exportLogSummary(result, summaryLogs);
    if (!fs::exists(summaryPath)) {
        std::cerr << "日志摘要导出失败" << std::endl;
        return 3;
    }

    const auto bundlePath = disassemble::desktop::RunController::exportResultBundle(result, summaryLogs);
    if (!fs::exists(bundlePath)) {
        std::cerr << "结果 zip 导出失败" << std::endl;
        return 4;
    }

    auto cancelTask = disassemble::desktop::RunController::buildSmokeTask(sourceImage, tempRoot / "cancel-run", sourceRoot);
    cancelTask.inputImagePath.clear();
    cancelTask.inputDirectory = inputDir.string();
    cancelTask.outputSizes = {{640, 480}};
    cancelTask.enableParallel = false;
    cancelTask.maxWorkers = 1;

    std::atomic_bool cancelRequested{false};
    const auto cancelledResult = disassemble::desktop::RunController::runTask(
        cancelTask,
        [&](const disassemble::core::RunProgress &progress) {
            if (progress.stage == disassemble::core::RunStage::Processing && progress.completedInputs >= 1) {
                cancelRequested.store(true);
            }
        },
        [&]() {
            return cancelRequested.load();
        });

    if (!cancelledResult.cancelled || cancelledResult.successCount < 1 || cancelledResult.successCount >= 2) {
        std::cerr << "安全取消路径未按预期工作" << std::endl;
        return 5;
    }

    return 0;
}
