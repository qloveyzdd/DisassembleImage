#pragma once

#include <filesystem>
#include <vector>

#include "../model/ProcessingTask.h"

namespace disassemble::core {

class ImageCatalog {
public:
    static std::vector<std::filesystem::path> collect(const ProcessingTask &task);
};

} // namespace disassemble::core
