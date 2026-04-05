#pragma once

#include <string>

namespace disassemble::core {

struct PreviewGalleryItem {
    std::string inputImagePath;
    std::string outputImagePath;
    std::string displayName;
    bool selected = false;
    int faceIndex = -1;

    bool isValid() const;
};

} // namespace disassemble::core
