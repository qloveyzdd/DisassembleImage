#pragma once
#include <string>
#include <vector>

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

using namespace std;

enum group_direction
{
    NONE,
    X,
    Y
};

class image_info
{
public:
    cv::Point2f size_A;
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
    inline vector<image_info> get_prim_screen() { return alone_prim; };
};

class server_info
{
private:
    input_image_info *input_image;
    output_image_info *output_image_size;
    vector<string> Prefix;
    string load_path;
    string save_path;
    group_direction direction;

public:
    server_info();
    input_image_info *Get_input() const { return input_image; }
    output_image_info *Get_output() const { return output_image_size; }
    const string GetLoadPath() const { return load_path; }
    const string GetSavePath() const { return save_path; }
    vector<string> GetPrefix() const { return Prefix; }
    const group_direction get_direction() const { return direction; }
};

class load_list
{
private:
    vector<string> list;

public:
    load_list(server_info serverinfo);
    load_list(string load_path, string load_name);
    const vector<string> *Get_list() { return &list; }
    const int list_count() { return static_cast<int>(list.size()); }
    string *get_file(int count) { return &(list[count]); }
};
