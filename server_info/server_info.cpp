#include "server_info.h"
#include <iostream>
#include <fstream>

server_info::server_info()
{
    string temp;
    cout << "选择输入模式（1.手动输入2.文本导入）" << endl;
    cin >> temp;
    int system = atoi(temp.c_str());
    switch (system)
    {
    case 1:
    {
        temp = "";
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
        break;
    }

    case 2:
    {
        vector<string> info;
        temp = "";
        cout << "请输入文本文件名：" << endl;
        cin >> temp;
        ifstream inf(temp);
        temp = "";
        while (getline(inf, temp))
        {
            info.push_back(temp);
        }
        x = atoi(info[0].c_str());
        y = atoi(info[1].c_str());
        xz = atoi(info[2].c_str());
        fz = atoi(info[3].c_str());
        Prefix = info[4];
        mask = info[5];
        load_name = info[6];
        save_path = info[7];
        break;
    }

    default:
        break;
    }
}

load_list::load_list(server_info serverinfo)
{
    ifstream inf(serverinfo.GetLoadName());
    string image_name;
    int count = 0;
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