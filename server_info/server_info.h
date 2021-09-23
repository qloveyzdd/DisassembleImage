#pragma once
#include <string>
#include <vector>

using namespace std;

class server_info
{
private:
    int x;            //屏幕横向尺寸
    int y;            //屏幕纵向尺寸
    int xz;          //旋转
    int fz;          //翻转
    string Prefix;    //输出前缀
    string mask;      //mask名称
    string load_path; //读取文件路径
    string load_name; //读取文件名称
    string save_path; //存储路径
public:
    server_info();
    string GetLoadName() const { return load_name; }
    string GetLoadPath() const { return load_path; }
    string GetSavePath() const { return save_path; }
    int Get_xz() const { return xz; }
    int Get_fz() const { return fz; }
    string GetPrefix() const { return Prefix; }
    string GetMask() const { return mask; }
    int Get_x() const { return x; }
    int Get_y() const { return y; }
};

class load_list
{
private:
    vector<string> list;

public:
    load_list(server_info serverinfo);
    load_list(string load_path, string load_name);
    int get_size() { return list.size(); }
    string file_name(int a) const { return list[a];}
};

class player_settings
{
private:
    int begin; //起始点
    int count; //数量
public:
    player_settings(load_list loadlist_A);
    int get_begin() { return begin; }
    int get_count() { return count; }
};