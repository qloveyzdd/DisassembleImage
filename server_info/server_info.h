#pragma once
#include <string>
#include <vector>

using namespace std;

enum group_direction
{
    NONE, //不组合
    X,    // X方向
    Y     // Y方向
};

class image_info
{
public:
    int x; //图片横向尺寸
    int y; //图片纵向尺寸
public:
    image_info(int x, int y):x(x),y(y){};
};

class input_image_info:public image_info
{
public:
    input_image_info(int x, int y):image_info(x,y){};
};

class output_image_info
{
private:
vector<image_info> alone_prim;
public:
    output_image_info(vector<int[2]> prim_screen);
    inline image_info& operator[](int i){return alone_prim[i];};
};

class server_info
{
private:
    input_image_info *input_image_size; //输入屏幕尺寸
    image_info *output_image_size;      //输出屏幕尺寸数组
    string Prefix;                      //输出前缀
    string load_path;                   //读取文件路径
    string save_path;                   //存储路径
    group_direction direction;          //组合方式

public:
    server_info();
    const input_image_info *Get_input() const { return input_image_size; }
    const image_info *Get_output() const { return output_image_size; }
    const string GetLoadPath() const { return load_path; }
    const string GetSavePath() const { return save_path; }
    const string GetPrefix() const { return Prefix; }
    const group_direction get_direction() const { return direction; }
};

class load_list //待处理文件总目录
{
private:
    vector<string> list;

public:
    load_list(server_info serverinfo);
    load_list(string load_path, string load_name);
    const vector<string> *Get_list() { return &list; }
    const int list_count() { return list.size(); }
    string *get_file(int count) { return &(list[count]); }
};
