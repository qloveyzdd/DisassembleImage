#include "point_uv.h"
#include <iostream>
#include <fstream>
#include <sstream>

prim_uv_factory::prim_uv_factory(std::string uv_file)
{
    std::ifstream inf(uv_file);
    if (!inf.is_open())
    {
        std::cout << uv_file << "不能读取uv表，请检查！" << std::endl;
    }
    std::string temp = "";

    for (int i = 0; getline(inf, temp); i++)
    {
        if (i % 4 == 0)
        {
            uv_prim.push_back(new prim_uv(i / 4));
        }
        uv_prim[i / 4]->add_point(temp);
    }
}

std::vector<std::string> Stringsplit(std::string str, const char split)
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

point_uv::point_uv(std::string uv_line)
{
    std::vector<std::string> temp = Stringsplit(uv_line, '\/');
    id = atoi(temp[0].c_str());
    point_loc = new uv_location(atof(temp[1].c_str()) / 1000000, 1-(atof(temp[2].c_str()) / 1000000));
}

uv_location prim_uv::centor()
{
    float x = 0, y = 0;
    for (auto i : uv_point)
    {
        x += i->get_location()->x;
        y += i->get_location()->y;
    }
    x /= 4;
    y /= 4;

    return uv_location(x,y);
}