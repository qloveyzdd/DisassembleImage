#include "EnvironmentCheck.h"

#include <fstream>

#include <opencv2/core/version.hpp>

namespace fs = std::filesystem;

namespace {

bool isWritableDirectory(const fs::path &path)
{
    std::error_code ec;
    fs::create_directories(path, ec);
    if (ec) {
        return false;
    }

    const fs::path probe = path / ".write-test";
    std::ofstream output(probe.string(), std::ios::out | std::ios::trunc);
    if (!output.is_open()) {
        return false;
    }
    output << "ok";
    output.close();
    fs::remove(probe, ec);
    return true;
}

std::vector<fs::path> candidateRoots(const fs::path &appDir)
{
    return {
        fs::absolute(appDir),
        fs::absolute(appDir / ".."),
        fs::absolute(appDir / "../.."),
        fs::current_path()
    };
}

} // namespace

namespace disassemble::desktop {

EnvironmentStatus EnvironmentCheck::inspect(const fs::path &appDir, const fs::path &outputRoot)
{
    EnvironmentStatus status;
    status.messages.push_back("OpenCV 可用，版本: " CV_VERSION);

    for (const auto &root : candidateRoots(appDir)) {
        if (fs::exists(root / "input.obj") && fs::exists(root / "output.obj")) {
            status.modelRoot = root;
            status.messages.push_back("已找到模型资源目录: " + root.string());
            break;
        }
    }

    if (status.modelRoot.empty()) {
        status.messages.push_back("未找到 input.obj / output.obj，请将模型文件放在程序目录或仓库根目录。" );
    }

    if (!outputRoot.empty()) {
        if (isWritableDirectory(outputRoot)) {
            status.messages.push_back("输出目录可写: " + outputRoot.string());
        } else {
            status.messages.push_back("输出目录不可写: " + outputRoot.string());
        }
    } else {
        status.messages.push_back("尚未选择输出目录。" );
    }

    status.ok = !status.modelRoot.empty() && (!outputRoot.empty() ? isWritableDirectory(outputRoot) : false);
    return status;
}

} // namespace disassemble::desktop
