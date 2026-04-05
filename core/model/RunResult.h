#pragma once

#include <string>
#include <vector>

#include "PreviewGalleryItem.h"
#include "ProcessingTask.h"

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
    ProcessingBackend requestedBackend = ProcessingBackend::Auto;
    ProcessingBackend activeBackend = ProcessingBackend::Cpu;
    std::string acceleratorName;
    std::string fallbackReason;
    std::string consistencySummary = u8"未执行 CPU/GPU 自动对照";
    double totalProcessingMs = 0.0;
    double gpuHotPathMs = 0.0;
    std::vector<std::string> outputFiles;
    std::vector<PreviewGalleryItem> previewItems;
    std::vector<std::string> logs;
    std::vector<RunFailure> failures;

    bool ok() const;
    void addLog(std::string message);
};

} // namespace disassemble::core
