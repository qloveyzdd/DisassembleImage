#pragma once

#include <string>
#include <vector>

#include "EnvironmentCheck.h"
#include "TaskFormState.h"

namespace disassemble::desktop {

struct ValidationResult {
    bool ok = false;
    std::vector<std::string> errors;
    std::vector<std::string> warnings;

    std::string summaryText() const;
};

class TaskFormValidator {
public:
    static ValidationResult validate(const TaskFormState &state, const EnvironmentStatus &environment);
};

} // namespace disassemble::desktop
