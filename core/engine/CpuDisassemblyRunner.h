#pragma once

#include <functional>

#include "../gpu/GpuBackendInfo.h"
#include "../model/ProcessingTask.h"
#include "../model/RunProgress.h"
#include "../model/RunResult.h"

namespace disassemble::core {

class CpuDisassemblyRunner {
public:
    using ProgressCallback = std::function<void(const RunProgress &)>;
    using CancelCheck = std::function<bool()>;

    RunResult run(const ProcessingTask &task,
                  const GpuBackendInfo &backendInfo,
                  ProgressCallback onProgress = {},
                  CancelCheck isCancelRequested = {}) const;

    RunResult run(const ProcessingTask &task,
                  ProgressCallback onProgress = {},
                  CancelCheck isCancelRequested = {}) const;
};

} // namespace disassemble::core
