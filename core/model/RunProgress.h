#pragma once

#include <string>

#include "ProcessingTask.h"

namespace disassemble::core {

enum class RunStage {
    Validating,
    PreparingOutput,
    CollectingInputs,
    Processing,
    Finished,
    Cancelled
};

struct RunProgress {
    RunStage stage = RunStage::Validating;
    int totalInputs = 0;
    int completedInputs = 0;
    int successCount = 0;
    int failedCount = 0;
    bool cancelRequested = false;
    ProcessingBackend requestedBackend = ProcessingBackend::Auto;
    ProcessingBackend activeBackend = ProcessingBackend::Cpu;
    std::string acceleratorName;
    std::string fallbackReason;
    double elapsedMs = 0.0;
    std::string currentInputPath;
};

} // namespace disassemble::core
