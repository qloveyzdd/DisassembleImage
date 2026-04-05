#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "../../core/model/ProcessingTask.h"
#include "../../core/model/ProcessingTypes.h"

namespace disassemble::desktop {

enum class InputMode {
    SingleImage,
    Directory
};

struct TaskFormState {
    InputMode inputMode = InputMode::SingleImage;
    std::filesystem::path inputImagePath;
    std::filesystem::path inputDirectory;
    std::filesystem::path outputRoot;
    bool autoDetectModels = true;
    std::filesystem::path inputObjPath;
    std::filesystem::path outputObjPath;
    disassemble::core::ProcessingDirection direction = disassemble::core::ProcessingDirection::X;
    std::string outputSizesText = "6144*6720";
    std::string prefixesText = "radian";
    disassemble::core::OutputConflictPolicy outputConflictPolicy = disassemble::core::OutputConflictPolicy::ForbidOverwrite;
    bool enableParallel = false;
    unsigned int maxWorkers = 1;

    bool usesSingleImageInput() const;
    bool usesDirectoryInput() const;
    bool usesGroupedOutput() const;
    std::filesystem::path resolvedInputObjPath(const std::filesystem::path &detectedPath) const;
    std::filesystem::path resolvedOutputObjPath(const std::filesystem::path &detectedPath) const;

    static std::vector<disassemble::core::ImageSize> parseOutputSizes(const std::string &text);
    static std::vector<std::string> parsePrefixes(const std::string &text);
    static std::string formatOutputSizes(const std::vector<disassemble::core::ImageSize> &sizes);
    static std::string formatPrefixes(const std::vector<std::string> &prefixes);
};

} // namespace disassemble::desktop
