#include "server_info.h"
#include <iostream>

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
    
    cout << "输入存储路径:" << endl;
    cin >> save_file;
}