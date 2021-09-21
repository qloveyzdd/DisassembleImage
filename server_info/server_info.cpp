#include "server_info.h"
#include <iostream>
#include <fstream>

server_info::server_info()
{
    string temp;
    cout << "输入屏幕横向尺寸:" << endl;
    cin >> temp;
    x = atoi(temp.c_str());

    temp = "";
    cout << "输入屏幕纵向尺寸:" << endl;
    cin >> temp;
    y = atoi(temp.c_str());

    temp = "";
    cout << "旋转角度: 0:(不旋转)1：(90度)" << endl;
    cin >> temp;
    xz = atoi(temp.c_str());

    temp = "";
    cout << "翻转角度: 0:(不翻转)1：(水平)2：(竖直)3：(二者都)" << endl;
    cin >> temp;
    fz = atoi(temp.c_str());

    cout << "输入前缀:" << endl;
    cin >> Prefix;

    cout << "输入mask:" << endl;
    cin >> mask;

    cout << "输入读取待拆分文件目录:" << endl;
    cin >> load_name;

    cout << "输入存储路径:" << endl;
    cin >> save_path;
}

load_list::load_list(server_info serverinfo)
{
    ifstream inf(serverinfo.GetLoadName());
    string image_name;
    int count;
    while (getline(inf, image_name))
    {
        list.push_back(image_name);
        cout << image_name << endl;
        count++;
    }
    cout << count << endl;
}

load_list::load_list(string load_path, string load_name)
{
    ifstream inf(load_name);
    string image_name;
    int count;
    while (getline(inf, image_name))
    {
        string temp = load_path + "/" + image_name;
        list.push_back(temp);
        cout << temp << endl;
        count++;
    }
    cout << count << endl;
}

player_settings::player_settings(load_list loadlist)
{
    string temp;
    cout << "输入起始位置（设置为小于等于0则为列表起始位置,超过最大值则为最大值）:" << endl;
    cin >> temp;
    begin = atoi(temp.c_str());
    if (begin <= 0)
        begin = 1;
    else if (begin > loadlist.get_size())
        begin = loadlist.get_size();

    temp = "";
    cout << "输入计算数量（设置为小于等于0则为从开始端口到列表末尾位置,超过最大数量则计算到末尾）:" << endl;
    cin >> temp;
    count = atoi(temp.c_str());
    if (count <= 0)
        count = loadlist.get_size() - begin;
    else
        count + begin > loadlist.get_size() ? loadlist.get_size() - begin : count;
}