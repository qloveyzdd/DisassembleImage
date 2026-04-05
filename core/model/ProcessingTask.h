#pragma once

#include <string>
#include <vector>

#include "ProcessingTypes.h"

namespace disassemble::core {

enum class OutputConflictPolicy {
    ForbidOverwrite,
    OverwriteExisting,
    AutoRename
};

enum class ProcessingBackend {
    Auto,
    Cpu,
    Gpu
};

struct ProcessingTask {
    std::string inputImagePath;
    std::string inputDirectory;
    std::string outputRoot;
    std::string inputObjPath;
    std::string outputObjPath;
    ProcessingDirection direction = ProcessingDirection::None;
    std::vector<ImageSize> outputSizes;
    std::vector<std::string> prefixes;
    OutputConflictPolicy outputConflictPolicy = OutputConflictPolicy::ForbidOverwrite;
    ProcessingBackend processingBackend = ProcessingBackend::Auto;
    bool enableParallel = false;
    unsigned int maxWorkers = 1;

    bool hasSingleImage() const;
    bool hasInputDirectory() const;
    bool usesGroupedOutput() const;
};

} // namespace disassemble::core
