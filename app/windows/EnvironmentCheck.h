#pragma once

#include <filesystem>
#include <string>
#include <vector>

namespace disassemble::desktop {

struct EnvironmentStatus {
    bool ok = false;
    bool outputDirectoryWritable = false;
    std::vector<std::string> messages;
    std::filesystem::path modelRoot;
    std::filesystem::path detectedInputObjPath;
    std::filesystem::path detectedOutputObjPath;
};

class EnvironmentCheck {
public:
    static EnvironmentStatus inspect(const std::filesystem::path &appDir,
                                     const std::filesystem::path &outputRoot = {});
};

} // namespace disassemble::desktop
