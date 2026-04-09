#include "SlicePlan.h"

#include <algorithm>
#include <stdexcept>

#include <opencv2/imgproc.hpp>

namespace {

using disassemble::core::ImageSize;
using disassemble::core::ObjFaceQuad;
using disassemble::core::PaddedUvModel;
using disassemble::core::ProcessingDirection;
using disassemble::core::SlicePrimitive;

void sortCorners(std::vector<cv::Point2f> &corners)
{
    std::sort(corners.begin(), corners.end(), [](const cv::Point2f &left, const cv::Point2f &right) {
        return left.y < right.y;
    });

    if (corners[0].x > corners[1].x) {
        std::swap(corners[0], corners[1]);
    }
    if (corners[3].x > corners[2].x) {
        std::swap(corners[3], corners[2]);
    }
}

cv::Point2f scaleUvPoint(const cv::Point2f &uv, const ImageSize &size)
{
    return cv::Point2f(uv.x * static_cast<float>(size.width), uv.y * static_cast<float>(size.height));
}

std::vector<cv::Point2f> collectScaledInputQuad(const PaddedUvModel &inputModel,
                                                const ObjFaceQuad &face,
                                                const ImageSize &inputSize)
{
    std::vector<cv::Point2f> quad;
    quad.reserve(4);
    for (int uvIndex : face.uvIndices) {
        quad.push_back(scaleUvPoint(inputModel.model.uvPoints()[uvIndex], inputSize));
    }
    return quad;
}

std::vector<cv::Point2f> makeOutputQuad(const cv::Point2f &outputPoint)
{
    return {
        cv::Point2f(0.0F, 0.0F),
        cv::Point2f(outputPoint.x, 0.0F),
        outputPoint,
        cv::Point2f(0.0F, outputPoint.y)
    };
}

SlicePrimitive buildPrimitive(const std::vector<cv::Point2f> &inputQuad,
                              const cv::Point2f &outputPoint,
                              const cv::Point2f &maxPoint = cv::Point2f(0.0F, 0.0F))
{
    SlicePrimitive primitive;
    primitive.quadPoints = makeOutputQuad(outputPoint);
    primitive.maxPoint = maxPoint;

    auto orderedInput = inputQuad;
    sortCorners(orderedInput);
    primitive.transform = cv::getPerspectiveTransform(orderedInput, primitive.quadPoints);
    return primitive;
}

cv::Point2f scaleOutputUv(const cv::Point2f &uv, const ImageSize &size)
{
    return cv::Point2f(uv.x * static_cast<float>(size.width), uv.y * static_cast<float>(size.height));
}

} // namespace

namespace disassemble::core {

std::vector<SlicePrimitive> SlicePlanBuilder::buildGrouped(const PaddedUvModel &inputModel,
                                                           const ObjModel &outputModel,
                                                           const ImageSize &inputSize,
                                                           const ImageSize &outputSize,
                                                           ProcessingDirection direction)
{
    if (inputModel.model.faces().size() != outputModel.faces().size()) {
        throw std::runtime_error("input.obj face count does not match output.obj");
    }

    std::vector<SlicePrimitive> primitives;
    primitives.reserve(inputModel.model.faces().size());

    for (size_t faceIndex = 0; faceIndex < inputModel.model.faces().size(); ++faceIndex) {
        const auto &inputFace = inputModel.model.faces()[faceIndex];
        const auto &outputFace = outputModel.faces()[faceIndex];

        std::vector<cv::Point2f> inputQuad = collectScaledInputQuad(inputModel, inputFace, inputSize);

        std::vector<cv::Point2f> outputQuad;
        outputQuad.reserve(4);
        for (int uvIndex : outputFace.uvIndices) {
            outputQuad.push_back(scaleOutputUv(outputModel.uvPoints()[uvIndex], outputSize));
        }

        std::sort(outputQuad.begin(), outputQuad.end(), [](const cv::Point2f &left, const cv::Point2f &right) {
            return left.x < right.x;
        });
        const float maxX = outputQuad.back().x;
        const float minX = outputQuad.front().x;
        const float width = maxX - minX;

        std::sort(outputQuad.begin(), outputQuad.end(), [](const cv::Point2f &left, const cv::Point2f &right) {
            return left.y < right.y;
        });
        const float maxY = outputQuad.back().y;
        const float minY = outputQuad.front().y;
        const float height = maxY - minY;

        primitives.push_back(buildPrimitive(inputQuad, cv::Point2f(width, height), cv::Point2f(maxX, maxY)));
    }

    if (direction == ProcessingDirection::X) {
        std::sort(primitives.begin(), primitives.end(), [](const SlicePrimitive &left, const SlicePrimitive &right) {
            return left.maxPoint.x < right.maxPoint.x;
        });
    } else if (direction == ProcessingDirection::Y) {
        std::sort(primitives.begin(), primitives.end(), [](const SlicePrimitive &left, const SlicePrimitive &right) {
            return left.maxPoint.y < right.maxPoint.y;
        });
    }

    return primitives;
}

std::vector<SlicePrimitive> SlicePlanBuilder::buildPerFace(const PaddedUvModel &inputModel,
                                                           const ImageSize &inputSize,
                                                           const std::vector<ImageSize> &outputSizes)
{
    if (outputSizes.size() != inputModel.model.faces().size()) {
        throw std::runtime_error("output size count does not match input.obj face count");
    }

    std::vector<SlicePrimitive> primitives;
    primitives.reserve(inputModel.model.faces().size());

    for (size_t faceIndex = 0; faceIndex < inputModel.model.faces().size(); ++faceIndex) {
        const auto &inputFace = inputModel.model.faces()[faceIndex];
        const auto &outputSize = outputSizes[faceIndex];
        std::vector<cv::Point2f> inputQuad = collectScaledInputQuad(inputModel, inputFace, inputSize);
        primitives.push_back(buildPrimitive(inputQuad,
                                            cv::Point2f(static_cast<float>(outputSize.width),
                                                        static_cast<float>(outputSize.height))));
    }

    return primitives;
}

} // namespace disassemble::core
