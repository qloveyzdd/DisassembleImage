#pragma once

#include <filesystem>
#include <functional>
#include <vector>

#include "EnvironmentCheck.h"
#include "TaskFormState.h"
#include "../../core/gpu/GpuBackendInfo.h"
#include "../../core/model/RunProgress.h"
#include "../../core/model/ProcessingTask.h"
#include "../../core/model/RunResult.h"

namespace disassemble::desktop {

class RunController {
public:
    using ProgressCallback = std::function<void(const disassemble::core::RunProgress &)>;
    using CancelCheck = std::function<bool()>;

    static disassemble::core::GpuBackendInfo probeGpuBackend();

    static disassemble::core::ProcessingTask buildTask(const TaskFormState &state,
                                                       const EnvironmentStatus &environment);

    static disassemble::core::RunResult runTask(const TaskFormState &state,
                                                const EnvironmentStatus &environment,
                                                const ProgressCallback &onProgress = {},
                                                const CancelCheck &isCancelRequested = {});

    static disassemble::core::RunResult runTask(const disassemble::core::ProcessingTask &task,
                                                const ProgressCallback &onProgress = {},
                                                const CancelCheck &isCancelRequested = {});

    static disassemble::core::ProcessingTask buildSmokeTask(const std::filesystem::path &inputImage,
                                                            const std::filesystem::path &outputRoot,
                                                            const std::filesystem::path &modelRoot);

    static disassemble::core::RunResult runSmokeTask(const std::filesystem::path &inputImage,
                                                     const std::filesystem::path &outputRoot,
                                                     const std::filesystem::path &modelRoot);

    static std::filesystem::path exportLogSummary(const disassemble::core::RunResult &result,
                                                  const std::vector<std::string> &summaryLogs);

    static std::filesystem::path exportResultBundle(const disassemble::core::RunResult &result,
                                                    const std::vector<std::string> &summaryLogs);
};

} // namespace disassemble::desktop
