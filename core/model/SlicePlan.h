#pragma once

#include <vector>

#include <opencv2/core.hpp>

#include "ObjModel.h"
#include "ProcessingTask.h"

namespace disassemble::core {

struct SlicePrimitive {
    std::vector<cv::Point2f> quadPoints;
    cv::Point2f maxPoint{0.0F, 0.0F};
    cv::Mat transform;
};

class SlicePlanBuilder {
public:
    static std::vector<SlicePrimitive> buildGrouped(const PaddedUvModel &inputModel,
                                                    const ObjModel &outputModel,
                                                    const ImageSize &inputSize,
                                                    const ImageSize &outputSize,
                                                    ProcessingDirection direction);

    static std::vector<SlicePrimitive> buildPerFace(const PaddedUvModel &inputModel,
                                                    const ImageSize &inputSize,
                                                    const std::vector<ImageSize> &outputSizes);
};

} // namespace disassemble::core
