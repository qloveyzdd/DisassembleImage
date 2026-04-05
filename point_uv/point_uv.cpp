#include "point_uv.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> Stringsplit(std::string str, const char split)
{
    std::vector<std::string> rst;
    std::istringstream iss(str);
    std::string token;
    while (getline(iss, token, split))
    {
        rst.push_back(token);
    }
    return rst;
}

obj_basic::obj_basic(std::string obj_address)
{
    std::ifstream inf(obj_address);
    if (!inf.is_open())
    {
        std::cout << obj_address << "不能读取 obj 文件，请检查！" << std::endl;
    }

    std::string temp = {};
    for (; getline(inf, temp);)
    {
        if (temp.empty())
        {
            continue;
        }

        if (temp.back() == '\r')
        {
            temp.pop_back();
        }

        if (temp.empty() || temp[0] == '#' || temp[0] == '\t')
        {
            continue;
        }

        std::vector<std::string> top = {};
        if (temp.substr(0, 2) == "v ")
        {
            top = Stringsplit(temp, ' ');
            point_spatial_location.push_back(new cv::Point3f(atof(top[1].c_str()), atof(top[2].c_str()), atof(top[3].c_str())));
        }
        else if (temp.substr(0, 2) == "vt")
        {
            top = Stringsplit(temp, ' ');
            point_uv_location.push_back(new cv::Point2f(atof(top[1].c_str()), 1 - atof(top[2].c_str())));
        }
        else if (temp.substr(0, 2) == "f ")
        {
            top = Stringsplit(temp, ' ');
            if (top.size() == 5)
            {
                prim.push_back(new int[4]());
                prim_spatial.push_back(new int[4]());
                for (int i = 1; i < static_cast<int>(top.size()); i++)
                {
                    std::vector<std::string> tt_prim = Stringsplit(top[i], '/');
                    int spatial_point = atoi(tt_prim[0].c_str()) - 1;
                    int uv_point = atoi(tt_prim[1].c_str()) - 1;
                    prim.back()[i - 1] = uv_point;
                    prim_spatial.back()[i - 1] = spatial_point;
                }
            }
            else
            {
                std::cout << "obj 文件存在非四边面!!!!" << std::endl;
                abort();
            }
        }
    }
}

obj_uv_padding::obj_uv_padding(std::string obj_address) : obj_basic(obj_address)
{
    std::vector<cv::Point2f *> temp = get_uv_point_location();
    sort(temp.begin(), temp.end(), [](cv::Point2f *a, cv::Point2f *b)
         { return a->x < b->x; });
    if (temp[0]->x < 0)
    {
        left = temp[0]->x * -1;
    }
    else
    {
        left = 0;
    }
    if (temp[temp.size() - 1]->x > 1)
    {
        right = temp[temp.size() - 1]->x - 1;
    }
    else
    {
        right = 0;
    }

    sort(temp.begin(), temp.end(), [](cv::Point2f *a, cv::Point2f *b)
         { return a->y < b->y; });
    if (temp[0]->y < 0)
    {
        botton = temp[0]->y * -1;
    }
    else
    {
        botton = 0;
    }
    if (temp[temp.size() - 1]->y > 1)
    {
        top = temp[temp.size() - 1]->y - 1;
    }
    else
    {
        top = 0;
    }
}
