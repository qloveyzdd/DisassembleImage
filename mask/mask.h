#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

std::vector<cv::Point2f> RoiPointApprox(cv::Mat src);   //获取mask的顶点的位置

cv::Point2f GetCenter(std::vector<cv::Point2f> point);  //获取mask的中心点

void sortCorners(std::vector<cv::Point2f>& corners,cv::Point2f center); //对四个点的顺序进行校对

void DilationMask(cv::Mat& src,cv::Mat& dst); //将mask膨胀作为遮照保留图片信息