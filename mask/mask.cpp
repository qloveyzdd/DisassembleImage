#include "mask.h"
#include <stdlib.h>
#include <iostream>

void DilationMask(const cv::Mat &src, cv::Mat &dst) //膨胀mask
{
    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(50, 50));
    cv::dilate(src, dst, element);
}

std::vector<cv::Point2f> RoiPointApprox(const cv::Mat &src) //获取mask的顶点的位置
{
    cv::Mat bw;
    cv::cvtColor(src, bw, CV_BGR2GRAY);
    cv::Canny(bw, bw, 100, 100, 3);
    std::vector<std::vector<cv::Point>> roi_point;
    std::vector<std::vector<cv::Point>> roi_point1;
    cv::findContours(bw, roi_point1, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
    for (int i = 0; i < roi_point1.size(); i++)
    {
        if (roi_point1[i].size() >= 5)
        {
            roi_point.push_back(roi_point1[i]);
        }
    }
    std::vector<cv::Point2f> roi_point_approx;
    // cv::Mat roi_approx(bw.size(), CV_8UC3, cv::Scalar(0, 0, 0));
    auto i = roi_point.begin();
    approxPolyDP(*i, roi_point_approx, 7, 1);
    if (roi_point_approx.size() != 4)
    {
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

        return roi_point_approx_end;
    }
    else
    {
        return roi_point_approx;
    }
}

cv::Point2f GetCenter(const std::vector<cv::Point2f> &point) //获取mask的中心点
{
    cv::Point2f center(0, 0);

    for (int i = 0; i < point.size(); i++)
        center += point[i];
    center *= (1. / point.size());

    return center;
}

void sortCorners(std::vector<cv::Point2f> &corners, const cv::Point2f &center) //对四个点的顺序进行校对
{
    std::vector<cv::Point2f> top_temp, bot_temp, top, bot;

    sort(corners.begin(), corners.end(), [](cv::Point2f a, cv::Point2f b)
         { return a.y > b.y; });

    // for (int i = 0; i < corners.size(); i++)
    // {
    //     if (corners[i].y < center.y)
    //         top_temp.push_back(corners[i]);
    //     else
    //         bot_temp.push_back(corners[i]);
    // }
    top_temp.push_back(corners[2]);
    top_temp.push_back(corners[3]);
    bot_temp.push_back(corners[0]);
    bot_temp.push_back(corners[1]);
    
    corners.clear();

    if (top.size() != 2 || bot.size() != 2)
    {
        sort(top_temp.begin(), top_temp.end(), [](cv::Point2f a, cv::Point2f b)
             { return a.x > b.x; });
        top.push_back(top_temp[0]);
        top.push_back(top_temp[top_temp.size() - 1]);

        sort(bot_temp.begin(), bot_temp.end(), [](cv::Point2f a, cv::Point2f b)
             { return a.x > b.x; });
        bot.push_back(bot_temp[0]);
        bot.push_back(bot_temp[bot_temp.size() - 1]);
    }

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

void Mattopts(std::vector<cv::Point2f> &quad_pts, server_info *serverinfo) //设置最终图片的锚点位置
{
    quad_pts.push_back(cv::Point2f(serverinfo->get_tl(0) * serverinfo->Get_x(), serverinfo->get_tl(1) * serverinfo->Get_y()));
    quad_pts.push_back(cv::Point2f(serverinfo->get_tr(0) * serverinfo->Get_x(), serverinfo->get_tr(1) * serverinfo->Get_y()));
    quad_pts.push_back(cv::Point2f(serverinfo->get_br(0) * serverinfo->Get_x(), serverinfo->get_br(1) * serverinfo->Get_y()));
    quad_pts.push_back(cv::Point2f(serverinfo->get_bl(0) * serverinfo->Get_x(), serverinfo->get_bl(1) * serverinfo->Get_y()));
}

disassembly::disassembly(server_info *serverinfo)
{
    mask = imread(serverinfo->GetMask());

    if (!mask.data)
    {
        cout << "读取文件错误！！！" << endl;
        abort();
    }

    DilationMask(mask, mask_dilate);
    roi_point_approx = RoiPointApprox(mask);
    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    if (roi_point_approx.size() != 4)
    {
        cout << "锚点不为4！" << endl;
        abort();
    }

    Mattopts(quad_pts, serverinfo);
    transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts); //最终矩阵
}

void RoadImageAndSetMask(cv::Mat &dst, const std::string Image, const cv::Mat &mask)
{
    cv::Mat temp = cv::imread(Image, -1);
    if (!temp.data)
    {
        printf("读取文件错误！！！");
        return;
    }
    temp.copyTo(dst, mask);
}