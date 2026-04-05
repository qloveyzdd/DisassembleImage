#pragma once

#include "opencv2/opencv.hpp"

class obj_basic
{
private:
    std::vector<cv::Point3f *> point_spatial_location; // 空间点位置
    std::vector<cv::Point2f *> point_uv_location;      // uv 点位置
    std::vector<int *> prim;                           // 面的 uv 点组成
    std::vector<int *> prim_spatial;                   // 面的空间点组成

public:
    obj_basic(std::string obj_address);
    inline std::vector<cv::Point2f *> get_uv_point_location() { return point_uv_location; };
    inline std::vector<int *> get_prim() { return prim; };
    inline std::vector<cv::Point3f *> get_spatial_point_location() { return point_spatial_location; };
    inline std::vector<int *> get_spatial_prim() { return prim_spatial; };
};

class obj_uv_padding : public obj_basic // 按照 uv 的坐标作为边缘拓展
{
private:
    float top;
    float botton;
    float left;
    float right;

public:
    obj_uv_padding(std::string obj_address);
    inline float get_crosswise_mul() { return left + right + 1; };
    inline float get_lengthways_mul() { return top + botton + 1; };
};
