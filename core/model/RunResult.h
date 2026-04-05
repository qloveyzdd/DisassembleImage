#pragma once

#include <string>
#include <vector>

#include "PreviewGalleryItem.h"

namespace disassemble::core {

struct RunFailure {
    std::string inputPath;
    std::string reason;
};

struct RunResult {
    int successCount = 0;
    int failedCount = 0;
    bool cancelled = false;
    std::string outputRoot;
    std::vector<std::string> outputFiles;
    std::vector<PreviewGalleryItem> previewItems;
    std::vector<std::string> logs;
    std::vector<RunFailure> failures;

    bool ok() const;
    void addLog(std::string message);
};

} // namespace disassemble::core
