#include <filesystem>
#include <iostream>

#include "../../app/windows/EnvironmentCheck.h"
#include "../../app/windows/RunController.h"

namespace fs = std::filesystem;

int main()
{
    const fs::path sourceRoot(DISASSEMBLEIMAGE_SOURCE_ROOT);
    const fs::path sampleImage = sourceRoot / "HD" / "a0000.jpg";
    const fs::path outputRoot = sourceRoot / "build" / "phase1-smoke-output";

    const auto status = disassemble::desktop::EnvironmentCheck::inspect(sourceRoot, outputRoot);
    if (!status.ok)
    {
        std::cerr << "环境检查失败" << std::endl;
        for (const auto &message : status.messages)
        {
            std::cerr << message << std::endl;
        }
        return 1;
    }

    const auto result = disassemble::desktop::RunController::runSmokeTask(sampleImage, outputRoot, status.modelRoot);
    if (!result.ok() || result.outputFiles.empty())
    {
        std::cerr << "Smoke 任务未生成输出" << std::endl;
        return 2;
    }

    for (const auto &output : result.outputFiles)
    {
        if (!fs::exists(output))
        {
            std::cerr << "输出文件不存在: " << output << std::endl;
            return 3;
        }
    }

    return 0;
}
