#include "mask.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <string>

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

disassembly_factory::disassembly_factory(server_info *serverinfo, obj_uv_padding *obj_input, obj_basic *obj_output)
{
    direction = serverinfo->get_direction();
    for (int i =0;i<obj_input->get_prim_to_point().size();i++)
    {
        std::vector<cv::Point2f*> temp_input;
        std::vector<std::vector<int>> prim_temp_input = obj_input->get_prim_to_point();
        for (auto j:prim_temp_input[i])
        {
            temp_input.push_back(obj_input->get_uv_point_location()[j]);
        }
        std::vector<cv::Point2f*> temp_output;
        std::vector<std::vector<int>> prim_temp_output = obj_output->get_prim_to_point();
        for (auto j:prim_temp_output[i])
        {
            temp_output.push_back(obj_output->get_uv_point_location()[j]);
        }
        prim.push_back(new disassembly(temp_input,temp_output,cv::Point2f(10000,5000)));

    }
    
}

disassembly::disassembly(std::vector<cv::Point2f *> input_point,std::vector<cv::Point2f *> output_point, cv::Point2f input_screen,cv::Point2f output_screen)
{
    for (auto i:input_point)
    {
        roi_point_approx.push_back(cv::Point2f((i->x*input_screen.x,i->y*input_screen.y)));
    }

    center = GetCenter(roi_point_approx);

    for (auto i:output_point)
    {
        quad_pts.push_back(cv::Point2f((i->x*output_screen.x,i->y*output_screen.y)));
    }

    transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts); //最终矩阵
    // cv::namedWindow("check(按任意按键关闭)1", cv::WINDOW_NORMAL);
    // cv::imshow("check(按任意按键关闭)1", transmtx);
    // cv::waitKey(0);
    // cv::destroyAllWindows();
}
