#include "mask.h"

std::vector<cv::Point2f> RoiPointApprox(cv::Mat src)
{
    cv::Mat bw;
    cv::cvtColor(src, bw, CV_BGR2GRAY);
    cv::Canny(bw, bw, 100, 100, 3);
    std::vector<std::vector<cv::Point>> roi_point;
    cv::findContours(bw, roi_point, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    std::vector<cv::Point2f> roi_point_approx;
    cv::Mat roi_approx(bw.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    auto i = roi_point.begin();
    approxPolyDP(*i, roi_point_approx, 7, 1);

    sort(roi_point_approx.begin(), roi_point_approx.end(), [](cv::Point2f a, cv::Point2f b)
         { return sqrt(pow(a.x, 2) + pow(a.y, 2)) > sqrt(pow(b.x, 2) + pow(b.y, 2)); });
    std::vector<cv::Point2f> roi_point_approx_end;
    for (int i = 0; i < roi_point_approx.size() - 1; i++)
    {
        if (abs(sqrt(pow(roi_point_approx[i].x, 2) + pow(roi_point_approx[i].y, 2)) - sqrt(pow(roi_point_approx[i + 1].x, 2) + pow(roi_point_approx[i + 1].y, 2))) > 10)
        {
            roi_point_approx_end.push_back(roi_point_approx[i]);
        }
    }
    roi_point_approx_end.push_back(roi_point_approx.back());

    return roi_point_approx;
}

cv::Point2f GetCenter(std::vector<cv::Point2f> point)
{
    cv::Point2f center(0, 0);

    for (int i = 0; i < point.size(); i++)
        center += point[i];
    center *= (1. / point.size());

    return center;
}

void sortCorners(std::vector<cv::Point2f> &corners, cv::Point2f center)
{
    std::vector<cv::Point2f> top, bot;

    for (int i = 0; i < corners.size(); i++)
    {
        if (corners[i].y < center.y)
            top.push_back(corners[i]);
        else
            bot.push_back(corners[i]);
    }
    corners.clear();

    if (top.size() == 2 && bot.size() == 2)
    {
        cv::Point2f tl = top[0].x > top[1].x ? top[1] : top[0];
        cv::Point2f tr = top[0].x > top[1].x ? top[0] : top[1];
        cv::Point2f bl = bot[0].x > bot[1].x ? bot[1] : bot[0];
        cv::Point2f br = bot[0].x > bot[1].x ? bot[0] : bot[1];

        corners.push_back(tl);
        corners.push_back(tr);
        corners.push_back(br);
        corners.push_back(bl);
    }
}

void DilationMask(cv::Mat& src,cv::Mat& dst)
{
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT,cv::Size(50,50));
    cv::dilate(src,dst,element);
}