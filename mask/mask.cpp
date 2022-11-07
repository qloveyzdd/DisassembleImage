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
    // std::vector<cv::Point2f> top_temp, bot_temp, top, bot;

    sort(corners.begin(), corners.end(), [](cv::Point2f a, cv::Point2f b)
         { return a.y < b.y; });

    if (corners[0].x > corners[1].x)
    {
        swap(corners[0], corners[1]);
    }
    if (corners[3].x > corners[2].x)
    {
        swap(corners[3], corners[2]);
    }

    // top_temp.push_back(corners[2]);
    // top_temp.push_back(corners[3]);
    // bot_temp.push_back(corners[0]);
    // bot_temp.push_back(corners[1]);

    // corners.clear();

    // if (top.size() != 2 || bot.size() != 2)
    // {
    //     sort(top_temp.begin(), top_temp.end(), [](cv::Point2f a, cv::Point2f b)
    //          { return a.x > b.x; });
    //     top.push_back(top_temp[0]);
    //     top.push_back(top_temp[top_temp.size() - 1]);

    //     sort(bot_temp.begin(), bot_temp.end(), [](cv::Point2f a, cv::Point2f b)
    //          { return a.x > b.x; });
    //     bot.push_back(bot_temp[0]);
    //     bot.push_back(bot_temp[bot_temp.size() - 1]);
    // }

    // if (top_temp.size() == 2 && bot_temp.size() == 2)
    // {
    //     cv::Point2f tl = top_temp[0].x > top_temp[1].x ? top_temp[1] : top_temp[0];
    //     cv::Point2f tr = top_temp[0].x > top_temp[1].x ? top_temp[0] : top_temp[1];
    //     cv::Point2f bl = bot_temp[0].x > bot_temp[1].x ? bot_temp[1] : bot_temp[0];
    //     cv::Point2f br = bot_temp[0].x > bot_temp[1].x ? bot_temp[0] : bot_temp[1];

    //     corners.push_back(tl);
    //     corners.push_back(tr);
    //     corners.push_back(br);
    //     corners.push_back(bl);
    // }
}

cv::Point2f *point_mul_screen(cv::Point2f a, cv::Point2f b) //将uv点坐标转化为图片的像素坐标
{
    return new cv::Point2f(a.x * b.x, a.y * b.y);
}

disassembly_factory::disassembly_factory(obj_uv_padding *obj_input, obj_basic *obj_output, input_image_info *input_message, output_image_info *output_message, group_direction direction_in)
{
    if (obj_input->get_prim().size() == obj_output->get_prim().size())
    {
        for (int i = 0; i < obj_input->get_prim().size(); i++)
        {
            std::vector<std::vector<cv::Point2f *>> temp_input_And_output(4);
            std::vector<cv::Point2f *> temp_input;
            // std::vector<cv::Point2f *> temp_output;
            // std::vector<std::vector<int>> prim_temp_input = obj_input->get_prim();
            for (int j = 0; j < 4; j++) //将输入输出对应点从0-1映射成实际像素
            {
                // cv::Point2f temp = {obj_input->get_uv_point_location()[j]->x * input_message->size_A[0], obj_input->get_uv_point_location()[j]->y * input_message->size_A[1]};
                temp_input_And_output[j].push_back(point_mul_screen(*(obj_input->get_uv_point_location()[obj_input->get_prim()[i][j]]), input_message->size_A));
                temp_input_And_output[j].push_back(point_mul_screen(*(obj_output->get_uv_point_location()[obj_output->get_prim()[i][j]]), output_message->get_prim_screen()[0].size_A));
                // temp_input_And_output[j].push_back(new cv::Point2f(output_message->get_prim_screen()[0].size_A.x/63,output_message->get_prim_screen()[0].size_A.y));
            }

            // std::vector<std::vector<int>> prim_temp_input = obj_input->get_prim();
            // for (int j = 0; j < 4; j++) //将输出对应点从0-1映射成实际像素
            // {
            //     // cv::Point2f temp = {obj_input->get_uv_point_location()[j]->x * input_message->size_A[0], obj_input->get_uv_point_location()[j]->y * input_message->size_A[1]};

            // }

            cv::Point2f output_sceen_temp; //判断排列的长宽
            cv::Point2f max;               //排列顺序的参考值
            sort(temp_input_And_output.begin(), temp_input_And_output.end(), [](vector<cv::Point2f *> a, vector<cv::Point2f *> b)
                 { return a[1]->x < b[1]->x; });
            max.x = temp_input_And_output.back().back()->x;
            output_sceen_temp.x = max.x - temp_input_And_output.front().back()->x;

            sort(temp_input_And_output.begin(), temp_input_And_output.end(), [](vector<cv::Point2f *> a, vector<cv::Point2f *> b)
                 { return a[1]->y < b[1]->y; });
            max.y = temp_input_And_output.back().back()->y;
            output_sceen_temp.y = max.y - temp_input_And_output.front().back()->y;

            // if (temp_input_And_output[2][0]->x < temp_input_And_output[3][0]->x)
            // {
            //     swap(temp_input_And_output[2], temp_input_And_output[3]);
            // }

            // if (temp_input_And_output[1][0]->x < temp_input_And_output[0][0]->x)
            // {
            //     swap(temp_input_And_output[0], temp_input_And_output[1]);
            // }

            for (int i = 0; i < 4; i++)
            {
                temp_input.push_back(temp_input_And_output[i][0]);
            }

            // output_sceen_temp = cv::Point2f(97.5238, 6720);

            prim.push_back(new disassembly(temp_input, &output_sceen_temp, max));
        }
    }
    else
    {
        std::cout << "输入obj面数量与输出obj面数量不一致，请检查！！！" << std::endl;
        abort();
    }
    if (direction_in == group_direction::X)
    {
        sort(prim.begin(), prim.end(), [](disassembly *a, disassembly *b)
             { return a->get_max_point().x < b->get_max_point().x; });
    }
    else if (direction_in == group_direction::Y)
    {
        sort(prim.begin(), prim.end(), [](disassembly *a, disassembly *b)
             { return a->get_max_point().y < b->get_max_point().y; });
    }
}

disassembly_factory::disassembly_factory(obj_uv_padding *obj_input, input_image_info *input_message, output_image_info *output_message)
{
    if (output_message->get_prim_screen().size() == obj_input->get_prim().size())
    {
        for (int i = 0; i < obj_input->get_prim().size(); i++)
        {
            std::vector<cv::Point2f *> temp_input;
            // std::vector<std::vector<int>> prim_temp_input = obj_input->get_prim();
            for (int j = 0; j < 4; j++)
            {
                // cv::Point2f temp = {obj_input->get_uv_point_location()[j]->x * input_message->size_A[0], obj_input->get_uv_point_location()[j]->y * input_message->size_A[1]};
                temp_input.push_back(point_mul_screen(*(obj_input->get_uv_point_location()[obj_input->get_prim()[i][j]]), input_message->size_A));
            }

            prim.push_back(new disassembly(temp_input, &(output_message->get_prim_screen()[i].size_A)));
        }
    }
    else
    {
        std::cout << "输出尺寸数量与obj模型面数不一致，请检查！！！" << std::endl;
        abort();
    }
}

disassembly::disassembly(std::vector<cv::Point2f *> input_point, cv::Point2f *output_point)
{
    for (auto i : input_point)
    {
        roi_point_approx.push_back(*i);
    }

    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    // for (auto i : output_point)
    // {
    //     quad_pts.push_back(cv::Point2f((i->x * output_screen[0], i->y * output_screen[1])));
    // }

    quad_pts.push_back(cv::Point2f(0, 0));
    quad_pts.push_back(cv::Point2f(output_point->x, 0));
    quad_pts.push_back(*output_point);
    quad_pts.push_back(cv::Point2f(0, output_point->y));

    transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts); //最终矩阵
    // cv::namedWindow("check(按任意按键关闭)1", cv::WINDOW_NORMAL);
    // cv::imshow("check(按任意按键关闭)1", transmtx);
    // cv::waitKey(0);
    // cv::destroyAllWindows();
}

disassembly::disassembly(std::vector<cv::Point2f *> input_point, cv::Point2f *output_point, cv::Point2f max) : max_point(max)
{
    for (auto i : input_point)
    {
        roi_point_approx.push_back(*i);
    }

    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    // for (auto i : output_point)
    // {
    //     quad_pts.push_back(cv::Point2f((i->x * output_screen[0], i->y * output_screen[1])));
    // }

    quad_pts.push_back(cv::Point2f(0, 0));
    quad_pts.push_back(cv::Point2f(output_point->x, 0));
    quad_pts.push_back(*output_point);
    quad_pts.push_back(cv::Point2f(0, output_point->y));

    transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts); //最终矩阵
    // cv::namedWindow("check(按任意按键关闭)1", cv::WINDOW_NORMAL);
    // cv::imshow("check(按任意按键关闭)1", transmtx);
    // cv::waitKey(0);
    // cv::destroyAllWindows();
}