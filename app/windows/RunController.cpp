#include "RunController.h"

#include <stdexcept>

#include "../../core/engine/CpuDisassemblyRunner.h"

namespace fs = std::filesystem;

namespace disassemble::desktop {

using disassemble::core::ImageSize;
using disassemble::core::ProcessingDirection;
using disassemble::core::ProcessingTask;
using disassemble::core::RunResult;

ProcessingTask RunController::buildSmokeTask(const fs::path &inputImage,
                                             const fs::path &outputRoot,
                                             const fs::path &modelRoot)
{
    if (inputImage.empty()) {
        throw std::runtime_error("未选择输入图片");
    }
    if (outputRoot.empty()) {
        throw std::runtime_error("未选择输出目录");
    }
    if (modelRoot.empty()) {
        throw std::runtime_error("未找到模型资源目录");
    }

    ProcessingTask task;
    task.inputImagePath = fs::absolute(inputImage).string();
    task.outputRoot = fs::absolute(outputRoot).string();
    task.inputObjPath = (modelRoot / "input.obj").string();
    task.outputObjPath = (modelRoot / "output.obj").string();
    task.direction = ProcessingDirection::X;
    task.outputSizes = {ImageSize{6144, 6720}};
    task.prefixes = {"radian"};
    task.enableParallel = false;
    task.maxWorkers = 1;
    return task;
}

RunResult RunController::runSmokeTask(const fs::path &inputImage,
                                      const fs::path &outputRoot,
                                      const fs::path &modelRoot)
{
    CpuDisassemblyRunner runner;
    return runner.run(buildSmokeTask(inputImage, outputRoot, modelRoot));
}

} // namespace disassemble::desktop
