#pragma once
#include <string>
#include <vector>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

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
    cv::Point2f size_A; //图片尺寸
public:
    image_info(int x, int y) : size_A(x, y){};
};

class input_image_info : public image_info
{
public:
    input_image_info(vector<int> prim_screen) : image_info(prim_screen[0], prim_screen[1]){};
};

class output_image_info
{
private:
    vector<image_info> alone_prim;

public:
    output_image_info(vector<vector<int>> prim_screen);
    inline vector<image_info> get_prim_screen() { return alone_prim; }; //获取导出的所有面的信息
};

class server_info
{
private:
    input_image_info *input_image;        //输入屏幕尺寸
    output_image_info *output_image_size; //输出屏幕尺寸数组
    vector<string> Prefix;                //输出前缀
    string load_path;                     //读取文件路径
    string save_path;                     //存储文件路径
    group_direction direction;            //组合方式

public:
    server_info();
    input_image_info *Get_input() const { return input_image; }
    output_image_info *Get_output() const { return output_image_size; }
    const string GetLoadPath() const { return load_path; }
    const string GetSavePath() const { return save_path; }
    vector<string> GetPrefix() const { return Prefix; }
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
