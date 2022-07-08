#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include "../server_info/server_info.h"
#include "../point_uv/point_uv.h"

class disassembly
{
private:
    std::vector<cv::Point2f> roi_point_approx; // mask的顶点的位置
    cv::Point2f center;                        // mask的中心点
    std::vector<cv::Point2f> quad_pts;         //最终图片的顶点位置
    cv::Mat transmtx;                          //变换矩阵
public:
    disassembly(std::vector<cv::Point2f *> input_point,std::vector<cv::Point2f *> output_point, cv::Point2f input_screen,cv::Point2f output_screen);
    cv::Mat get_transmtx() const { return transmtx; } //获取变换矩阵
};

class disassembly_factory
{
private:
    group_direction direction;  //组合方向
    vector<disassembly *> prim; //每个面的变化信息
public:
    disassembly_factory(server_info *serverinfo, obj_uv_padding *obj_input, obj_basic *obj_output);
};
