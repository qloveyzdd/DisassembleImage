#pragma once

#include <array>
#include <filesystem>
#include <vector>

#include <opencv2/core.hpp>

namespace disassemble::core {

struct ObjFaceQuad {
    std::array<int, 4> uvIndices{0, 0, 0, 0};
    std::array<int, 4> spatialIndices{0, 0, 0, 0};
};

class ObjModel {
public:
    static ObjModel load(const std::filesystem::path &path);

    const std::vector<cv::Point3f> &spatialPoints() const;
    const std::vector<cv::Point2f> &uvPoints() const;
    const std::vector<ObjFaceQuad> &faces() const;

private:
    std::vector<cv::Point3f> spatialPoints_;
    std::vector<cv::Point2f> uvPoints_;
    std::vector<ObjFaceQuad> faces_;
};

struct PaddedUvModel {
    ObjModel model;
    float top = 0.0F;
    float bottom = 0.0F;
    float left = 0.0F;
    float right = 0.0F;

    static PaddedUvModel load(const std::filesystem::path &path);

    float crosswiseMul() const;
    float lengthwaysMul() const;
};

} // namespace disassemble::core
