#pragma once
#include <string>
#include <vector>

using namespace std;

class server_info;
class load_list;
class server_image_dlc1;

class image_info
{
private:
    int x;         //屏幕横向尺寸
    int y;         //屏幕纵向尺寸
    int xz;        //旋转
    int fz;        //翻转
    string Prefix; //输出前缀
    string mask;   //mask名称
};

class server_image_dlc1 //对于非完全画面覆盖定制覆盖面锚点
{
private:
    float tl[2]; //上左
    float tr[2]; //上右
    float br[2]; //下右
    float bl[2]; //下左
public:
    server_image_dlc1();
    const float get_tl(int i) { return tl[i]; }
    const float get_tr(int i) { return tr[i]; }
    const float get_br(int i) { return br[i]; }
    const float get_bl(int i) { return bl[i]; }
};

class server_info : public server_image_dlc1
{
private:
    int x;            //屏幕横向尺寸
    int y;            //屏幕纵向尺寸
    int xz;           //旋转
    int fz;           //翻转
    string Prefix;    //输出前缀
    string mask;      //mask名称
    string load_path; //读取文件路径
    // string load_name; //读取文件名称
    string save_path; //存储路径
public:
    server_info();
    // string GetLoadName() const { return load_name; }
    string GetLoadPath() const { return load_path; }
    string GetSavePath() const { return save_path; }
    int Get_xz() const { return xz; }
    int Get_fz() const { return fz; }
    string GetPrefix() const { return Prefix; }
    string GetMask() const { return mask; }
    int Get_x() const { return x; }
    int Get_y() const { return y; }
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
