#pragma once

#include <array>
#include <string>
#include <vector>

#include "PreviewGalleryItem.h"
#include "ProcessingTask.h"

namespace disassemble::core {

struct RunResult;

struct PreviewVertex {
    float x = 0.0F;
    float y = 0.0F;
    float z = 0.0F;
    float u = 0.0F;
    float v = 0.0F;
};

struct PreviewFaceQuad {
    std::array<int, 4> vertexIndices{0, 0, 0, 0};
    int textureAssignmentIndex = -1;
};

struct FaceTextureAssignment {
    std::string inputImagePath;
    std::string outputImagePath;
    std::string displayName;
};

struct PreviewMesh {
    std::vector<PreviewVertex> vertices;
    std::vector<PreviewFaceQuad> faces;
    std::vector<FaceTextureAssignment> faceTextureAssignments;

    bool empty() const;
};

std::vector<PreviewGalleryItem> buildPreviewGalleryItems(const RunResult &result);
std::vector<PreviewGalleryItem> filterPreviewGalleryItemsByInput(const std::vector<PreviewGalleryItem> &items,
                                                                 const std::string &inputImagePath);
PreviewMesh buildPreviewMesh(const ProcessingTask &task,
                             const std::vector<PreviewGalleryItem> &itemsForInput);

} // namespace disassemble::core
