#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <string>

#include "../server_info/server_info.h"

using namespace cv;

class disassembly
{
private:
    bool check;                                //是否检查
    Mat mask;                                  //导入的mask图片；
    Mat mask_dilate;                           // mask图片膨胀；
    std::vector<cv::Point2f> roi_point_approx; // mask的顶点的位置
    cv::Point2f center;                        // mask的中心点
    std::vector<cv::Point2f> quad_pts;         //最终图片的顶点位置
    cv::Mat transmtx;                          //变换矩阵
public:
    disassembly(server_info *serverinfo);
    void check_mask();                                  //添加检查
    cv::Mat get_transmtx() const { return transmtx; }   //获取变换矩阵
    Mat get_mask_dilate() const { return mask_dilate; } //获取膨胀mask
    Mat get_mask() const { return mask; }               //获取膨胀mask
};

void RoadImageAndSetMask(cv::Mat &src, const std::string Image, const cv::Mat &mask); //读取待处理图片并设置边缘mask