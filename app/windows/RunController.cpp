#include "RunController.h"

#include <stdexcept>

#include "../../core/engine/CpuDisassemblyRunner.h"

namespace fs = std::filesystem;

namespace disassemble::desktop {

using disassemble::core::ImageSize;
using disassemble::core::CpuDisassemblyRunner;
using disassemble::core::OutputConflictPolicy;
using disassemble::core::ProcessingDirection;
using disassemble::core::ProcessingTask;
using disassemble::core::RunResult;

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
    task.enableParallel = state.enableParallel;
    task.maxWorkers = state.maxWorkers;
    return task;
}

RunResult RunController::runTask(const TaskFormState &state,
                                 const EnvironmentStatus &environment)
{
    CpuDisassemblyRunner runner;
    return runner.run(buildTask(state, environment));
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

} // namespace disassemble::desktop
