#pragma once

#include <string>

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
    std::string currentInputPath;
};

} // namespace disassemble::core
