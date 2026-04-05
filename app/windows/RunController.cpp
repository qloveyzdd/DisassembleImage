#include "RunController.h"

#include <stdexcept>

#include <QDateTime>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QStringList>

#include "../../core/engine/CpuDisassemblyRunner.h"
#include "../../core/gpu/GpuBackendInfo.h"

namespace fs = std::filesystem;

namespace disassemble::desktop {

using disassemble::core::ImageSize;
using disassemble::core::CpuDisassemblyRunner;
using disassemble::core::OutputConflictPolicy;
using disassemble::core::ProcessingBackend;
using disassemble::core::ProcessingDirection;
using disassemble::core::ProcessingTask;
using disassemble::core::RunResult;

namespace {

std::string timestampLabel()
{
    return QDateTime::currentDateTime().toString("yyyyMMdd-HHmmss").toStdString();
}

std::string buildSummaryText(const RunResult &result, const std::vector<std::string> &summaryLogs)
{
    std::string text;
    text += u8"DisassembleImage 运行摘要\n";
    text += std::string(u8"输出目录: ") + result.outputRoot + "\n";
    text += std::string(u8"状态: ") + (result.cancelled ? u8"已取消" : u8"已完成") + "\n";
    text += std::string(u8"请求后端: ") + disassemble::core::processingBackendLabel(result.requestedBackend) + "\n";
    text += std::string(u8"实际后端: ") + disassemble::core::processingBackendLabel(result.activeBackend) + "\n";
    if (!result.acceleratorName.empty()) {
        text += std::string(u8"GPU 设备: ") + result.acceleratorName + "\n";
    }
    if (!result.fallbackReason.empty()) {
        text += std::string(u8"回退说明: ") + result.fallbackReason + "\n";
    }
    text += std::string(u8"成功数量: ") + std::to_string(result.successCount) + "\n";
    text += std::string(u8"失败数量: ") + std::to_string(result.failedCount) + "\n";
    text += std::string(u8"总耗时(ms): ") + std::to_string(result.totalProcessingMs) + "\n";
    text += std::string(u8"GPU 热点耗时(ms): ") + std::to_string(result.gpuHotPathMs) + "\n";
    text += std::string(u8"一致性摘要: ") + result.consistencySummary + "\n";
    if (!result.failures.empty()) {
        text += u8"失败摘要:\n";
        for (const auto &failure : result.failures) {
            text += " - " + failure.inputPath + " : " + failure.reason + "\n";
        }
    }
    if (!summaryLogs.empty()) {
        text += u8"\n摘要日志:\n";
        for (const auto &line : summaryLogs) {
            text += " - " + line + "\n";
        }
    }
    return text;
}

std::filesystem::path writeSummaryFile(const RunResult &result, const std::vector<std::string> &summaryLogs)
{
    if (result.outputRoot.empty()) {
        throw std::runtime_error(u8"当前结果没有输出目录，无法导出摘要");
    }

    const auto outputRoot = fs::path(result.outputRoot);
    fs::create_directories(outputRoot);
    const auto summaryPath = outputRoot / ("run-summary-" + timestampLabel() + ".txt");

    QSaveFile file(QString::fromStdString(summaryPath.string()));
    if (!file.open(QIODevice::WriteOnly)) {
        throw std::runtime_error(std::string(u8"无法写入日志摘要: ") + summaryPath.string());
    }

    file.write(QByteArray::fromStdString(buildSummaryText(result, summaryLogs)));
    if (!file.commit()) {
        throw std::runtime_error(std::string(u8"提交日志摘要文件失败: ") + summaryPath.string());
    }

    return summaryPath;
}

} // namespace

disassemble::core::GpuBackendInfo RunController::probeGpuBackend()
{
    return disassemble::core::detectGpuBackend();
}

ProcessingTask RunController::buildTask(const TaskFormState &state,
                                        const EnvironmentStatus &environment)
{
    const auto inputObjPath = state.resolvedInputObjPath(environment.detectedInputObjPath);
    const auto outputObjPath = state.resolvedOutputObjPath(environment.detectedOutputObjPath);

    if (state.usesSingleImageInput() && state.inputImagePath.empty()) {
        throw std::runtime_error("未选择输入图片");
    }
    if (state.usesDirectoryInput() && state.inputDirectory.empty()) {
        throw std::runtime_error("未选择输入目录");
    }
    if (state.outputRoot.empty()) {
        throw std::runtime_error("未选择输出目录");
    }
    if (inputObjPath.empty()) {
        throw std::runtime_error("未找到 input.obj");
    }
    if (state.usesGroupedOutput() && outputObjPath.empty()) {
        throw std::runtime_error("当前模式需要 output.obj");
    }

    ProcessingTask task;
    if (state.usesSingleImageInput()) {
        task.inputImagePath = fs::absolute(state.inputImagePath).string();
    } else {
        task.inputDirectory = fs::absolute(state.inputDirectory).string();
    }
    task.outputRoot = fs::absolute(state.outputRoot).string();
    task.inputObjPath = fs::absolute(inputObjPath).string();
    task.outputObjPath = outputObjPath.empty() ? std::string() : fs::absolute(outputObjPath).string();
    task.direction = state.direction;
    task.outputSizes = TaskFormState::parseOutputSizes(state.outputSizesText);
    if (state.usesGroupedOutput()) {
        task.prefixes = TaskFormState::parsePrefixes(state.prefixesText);
    }
    task.outputConflictPolicy = state.outputConflictPolicy;
    task.processingBackend = state.processingBackend;
    task.enableParallel = state.enableParallel;
    task.maxWorkers = state.maxWorkers;
    return task;
}

RunResult RunController::runTask(const TaskFormState &state,
                                 const EnvironmentStatus &environment,
                                 const ProgressCallback &onProgress,
                                 const CancelCheck &isCancelRequested)
{
    return runTask(buildTask(state, environment), onProgress, isCancelRequested);
}

RunResult RunController::runTask(const ProcessingTask &task,
                                 const ProgressCallback &onProgress,
                                 const CancelCheck &isCancelRequested)
{
    CpuDisassemblyRunner runner;
    const auto backendInfo = probeGpuBackend();
    return runner.run(task, backendInfo, onProgress, isCancelRequested);
}

ProcessingTask RunController::buildSmokeTask(const fs::path &inputImage,
                                             const fs::path &outputRoot,
                                             const fs::path &modelRoot)
{
    TaskFormState state;
    state.inputMode = InputMode::SingleImage;
    state.inputImagePath = inputImage;
    state.outputRoot = outputRoot;
    state.autoDetectModels = false;
    state.inputObjPath = modelRoot / "input.obj";
    state.outputObjPath = modelRoot / "output.obj";
    state.direction = ProcessingDirection::X;
    state.outputSizesText = TaskFormState::formatOutputSizes({ImageSize{6144, 6720}});
    state.prefixesText = "radian";
    state.outputConflictPolicy = OutputConflictPolicy::OverwriteExisting;
    state.processingBackend = ProcessingBackend::Cpu;
    state.enableParallel = false;
    state.maxWorkers = 1;

    EnvironmentStatus environment;
    environment.detectedInputObjPath = state.inputObjPath;
    environment.detectedOutputObjPath = state.outputObjPath;
    environment.outputDirectoryWritable = true;
    return buildTask(state, environment);
}

RunResult RunController::runSmokeTask(const fs::path &inputImage,
                                      const fs::path &outputRoot,
                                      const fs::path &modelRoot)
{
    CpuDisassemblyRunner runner;
    return runner.run(buildSmokeTask(inputImage, outputRoot, modelRoot));
}

std::filesystem::path RunController::exportLogSummary(const RunResult &result,
                                                      const std::vector<std::string> &summaryLogs)
{
    return writeSummaryFile(result, summaryLogs);
}

std::filesystem::path RunController::exportResultBundle(const RunResult &result,
                                                        const std::vector<std::string> &summaryLogs)
{
    if (result.outputRoot.empty()) {
        throw std::runtime_error(u8"当前结果没有输出目录，无法导出 zip");
    }

    const auto outputRoot = fs::path(result.outputRoot);
    fs::create_directories(outputRoot);
    const auto summaryPath = writeSummaryFile(result, summaryLogs);
    const auto bundlePath = outputRoot / ("result-bundle-" + timestampLabel() + ".zip");

    QString tarExecutable = QStandardPaths::findExecutable(QStringLiteral("tar"));
    if (tarExecutable.isEmpty()) {
        tarExecutable = QStringLiteral("tar");
    }

    QStringList arguments;
    arguments << QStringLiteral("-a")
              << QStringLiteral("-cf")
              << QString::fromStdString(bundlePath.string());
    for (const auto &outputFile : result.outputFiles) {
        const auto relativePath = fs::relative(fs::path(outputFile), outputRoot);
        arguments << QString::fromStdString(relativePath.generic_string());
    }
    arguments << QString::fromStdString(fs::relative(summaryPath, outputRoot).generic_string());

    QProcess process;
    process.setWorkingDirectory(QString::fromStdString(outputRoot.string()));
    process.start(tarExecutable, arguments);
    if (!process.waitForFinished(-1) || process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        throw std::runtime_error(std::string(u8"导出 zip 失败: ") + process.readAllStandardError().toStdString());
    }

    return bundlePath;
}

} // namespace disassemble::desktop
