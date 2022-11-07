#include "point_uv.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

std::vector<std::string> Stringsplit(std::string str, const char split) // string拆分
{
    std::vector<std::string> rst;
    std::istringstream iss(str);       // 输入流
    std::string token;                 // 接收缓冲区
    while (getline(iss, token, split)) // 以split为分隔符
    {
        rst.push_back(token); // 输出
    }
    return rst;
}

obj_basic::obj_basic(std::string obj_address) //导入obj文件，并格式化
{
    std::ifstream inf(obj_address);
    if (!inf.is_open())
    {
        std::cout << obj_address << "不能读取obj文件，请检查！" << std::endl;
    }
    std::string temp = {};
    for (; getline(inf, temp);)
    {
        if (temp[0] != '#' && temp[0] != '\t')  //不判断#备注及空白行
        {
            temp.pop_back();
            std::vector<std::string> top = {};
            if (temp.substr(0, 2) == "v ")  //保留3d位置信息
            {
                top = Stringsplit(temp, ' ');
                point_spatial_location.push_back(new cv::Point3f(atof(top[1].c_str()), atof(top[2].c_str()), atof(top[3].c_str())));
            }
            else if (temp.substr(0, 2) == "vt") //保留uv信息
            {
                top = Stringsplit(temp, ' ');
                point_uv_location.push_back(new cv::Point2f(atof(top[1].c_str()), 1 - atof(top[2].c_str())));
            }
            else if (temp.substr(0, 2) == "f ") //保留面组成信息
            {
                top = Stringsplit(temp, ' ');
                if (top.size() == 5)    //判断组成面是否为四边面
                {
                    prim.push_back(new int[4]());
                    for (int i = 1; i < top.size(); i++)
                    {
                        std::vector<std::string> tt_prim = Stringsplit(top[i], '/');
                        int i_point = atoi(tt_prim[1].c_str()) - 1;
                        prim.back()[i-1] = i_point; //将面的点组成保留成组
                        // std::cout<<prim.back()[i]<<std::endl;
                    }
                }
                else
                {
                    std::cout << "obj文件有非四边面!!!!!" << std::endl;
                    abort();
                }
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
