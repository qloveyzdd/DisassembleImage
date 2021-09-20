#pragma once
#include <string>
#include <vector>

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
    string load_name; //读取文件名称
    string save_path; //存储路径
public:
    server_info();
    string GetLoadName()const{return load_name;}
};

class load_list
{
private:
    vector<string> list;

public:
    load_list(server_info serverinfo);
    load_list(string load_name);
};