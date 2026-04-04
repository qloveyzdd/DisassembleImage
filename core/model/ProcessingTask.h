#pragma once

#include <string>
#include <vector>

#include "ProcessingTypes.h"

namespace disassemble::core {

struct ProcessingTask {
    std::string inputImagePath;
    std::string inputDirectory;
    std::string outputRoot;
    std::string inputObjPath;
    std::string outputObjPath;
    ProcessingDirection direction = ProcessingDirection::None;
    std::vector<ImageSize> outputSizes;
    std::vector<std::string> prefixes;
    bool enableParallel = false;
    unsigned int maxWorkers = 1;

    bool hasSingleImage() const;
    bool hasInputDirectory() const;
    bool usesGroupedOutput() const;
};

} // namespace disassemble::core
