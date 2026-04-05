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

fs::path findFirstFile(const std::vector<fs::path> &roots, const fs::path &name)
{
    for (const auto &root : roots) {
        const auto candidate = fs::absolute(root / name);
        if (fs::exists(candidate)) {
            return candidate;
        }
    }
    return {};
}

} // namespace

namespace disassemble::desktop {

EnvironmentStatus EnvironmentCheck::inspect(const fs::path &appDir, const fs::path &outputRoot)
{
    EnvironmentStatus status;
    const auto roots = candidateRoots(appDir);
    status.messages.push_back(std::string(u8"OpenCV 可用，版本: ") + CV_VERSION);

    status.detectedInputObjPath = findFirstFile(roots, "input.obj");
    status.detectedOutputObjPath = findFirstFile(roots, "output.obj");

    if (!status.detectedInputObjPath.empty() && !status.detectedOutputObjPath.empty()
        && status.detectedInputObjPath.parent_path() == status.detectedOutputObjPath.parent_path()) {
        status.modelRoot = status.detectedInputObjPath.parent_path();
        status.messages.push_back(std::string(u8"已自动找到模型资源目录: ") + status.modelRoot.string());
    }

    if (status.detectedInputObjPath.empty()) {
        status.messages.push_back(u8"未自动找到 input.obj。");
    }
    if (status.detectedOutputObjPath.empty()) {
        status.messages.push_back(u8"未自动找到 output.obj。");
    }

    if (!outputRoot.empty()) {
        status.outputDirectoryWritable = isWritableDirectory(outputRoot);
        if (status.outputDirectoryWritable) {
            status.messages.push_back(std::string(u8"输出目录可写: ") + outputRoot.string());
        } else {
            status.messages.push_back(std::string(u8"输出目录不可写: ") + outputRoot.string());
        }
    } else {
        status.messages.push_back(u8"尚未选择输出目录。");
    }

    status.ok = !status.detectedInputObjPath.empty()
        && !status.detectedOutputObjPath.empty()
        && (outputRoot.empty() || status.outputDirectoryWritable);
    return status;
}

} // namespace disassemble::desktop
