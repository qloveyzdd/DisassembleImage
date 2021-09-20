#pragma once
#include <string>

using namespace std;

class server_info
{
private:
    int x;            //屏幕横向尺寸
    int y;            //屏幕纵向尺寸
    char xz;          //旋转
    char fz;          //翻转
    string Prefix;    //输出前缀
    string mask;      //mask名称
    string save_file; //存储路径
public:
    server_info();
};