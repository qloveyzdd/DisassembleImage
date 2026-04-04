#pragma once

#include "../model/ProcessingTask.h"
#include "../model/RunResult.h"

namespace disassemble::core {

class CpuDisassemblyRunner {
public:
    RunResult run(const ProcessingTask &task) const;
};

} // namespace disassemble::core
