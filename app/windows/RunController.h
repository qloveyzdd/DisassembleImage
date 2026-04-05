#pragma once

#include <filesystem>

#include "EnvironmentCheck.h"
#include "TaskFormState.h"
#include "../../core/model/ProcessingTask.h"
#include "../../core/model/RunResult.h"

namespace disassemble::desktop {

class RunController {
public:
    static disassemble::core::ProcessingTask buildTask(const TaskFormState &state,
                                                       const EnvironmentStatus &environment);

    static disassemble::core::RunResult runTask(const TaskFormState &state,
                                                const EnvironmentStatus &environment);

    static disassemble::core::ProcessingTask buildSmokeTask(const std::filesystem::path &inputImage,
                                                            const std::filesystem::path &outputRoot,
                                                            const std::filesystem::path &modelRoot);

    static disassemble::core::RunResult runSmokeTask(const std::filesystem::path &inputImage,
                                                     const std::filesystem::path &outputRoot,
                                                     const std::filesystem::path &modelRoot);
};

} // namespace disassemble::desktop
