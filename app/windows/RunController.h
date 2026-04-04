#pragma once

#include <filesystem>

#include "../../core/model/ProcessingTask.h"
#include "../../core/model/RunResult.h"

namespace disassemble::desktop {

class RunController {
public:
    static disassemble::core::ProcessingTask buildSmokeTask(const std::filesystem::path &inputImage,
                                                            const std::filesystem::path &outputRoot,
                                                            const std::filesystem::path &modelRoot);

    static disassemble::core::RunResult runSmokeTask(const std::filesystem::path &inputImage,
                                                     const std::filesystem::path &outputRoot,
                                                     const std::filesystem::path &modelRoot);
};

} // namespace disassemble::desktop
