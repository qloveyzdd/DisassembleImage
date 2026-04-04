#pragma once

namespace disassemble::core {

enum class ProcessingDirection {
    None,
    X,
    Y
};

struct ImageSize {
    int width = 0;
    int height = 0;
};

} // namespace disassemble::core
