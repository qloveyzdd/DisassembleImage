#pragma once

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

class obj_basic
{
private:
    std::vector<cv::Point3f *> point_spatial_location; //空间点位置
    std::vector<cv::Point2f *> point_uv_location;      // uv点位置
    std::vector<int[4]> prim;       // 面的点组成
    // cv::Point2f screen;                                //面的原始像素信息
public:
    obj_basic(std::string obj_address);
    inline std::vector<cv::Point2f *> get_uv_point_location() { return point_uv_location; };
    inline std::vector<int[4]> get_prim() { return prim; };
};

class obj_uv_padding : public obj_basic //按照uv的坐标作为边缘拓展
{
private:
    float top;
    float botton;
    float left;
    float right;

public:
    obj_uv_padding(std::string obj_address);
    inline float get_crosswise_mul() { return left + right + 1; };  //横向乘积
    inline float get_lengthways_mul() { return top + botton + 1; }; //纵向乘积
};