#pragma once

#include <filesystem>

#include "../model/ProcessingTask.h"

namespace disassemble::core {

class LegacyTextConfigLoader {
public:
    static ProcessingTask load(const std::filesystem::path &configPath);
};

} // namespace disassemble::core
