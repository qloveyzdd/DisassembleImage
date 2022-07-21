#include "server_info.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <typeinfo>

output_image_info::output_image_info(vector<vector<int>> prim_screen)
{
    for (auto i : prim_screen)
    {
        alone_prim.push_back(image_info(i[0], i[1]));
    }
}

template <class T>
std::vector<T> Stringsplit(std::string str, const char split) // string拆分
{
    std::vector<T> rst;

    std::istringstream iss(str);       // 输入流
    std::string token;                 // 接收缓冲区
    while (getline(iss, token, split)) // 以split为分隔符
    {
        rst.push_back(atoi(token.c_str()));
    }
    return rst;
}

template <class T>
std::vector<T> Stringsplit_string(std::string str, const char split) // string拆分
{
    std::vector<T> rst;

    std::istringstream iss(str);       // 输入流
    std::string token;                 // 接收缓冲区
    while (getline(iss, token, split)) // 以split为分隔符
    {
        rst.push_back(token);
    }
    return rst;
}

server_info::server_info()
{
    string temp = "";
    vector<string> info;
    cout << "请输入拆分基础文本文件名：" << endl;
    cin >> temp;
    ifstream inf(temp);
    if (!inf.is_open())
    {
        cout << temp << "不能读取拆分基础文本，请检查！" << endl;
        abort();
    }
    temp.clear();
    while (getline(inf, temp))
    {
        info.push_back(temp);
    }
    inf.close();

    //读入第一行数据，为待拆分图片尺寸
    {
        std::vector<int> temp = Stringsplit<int>(info[0], '*');
        input_image = new input_image_info({temp[0], temp[1]});
    }

    //读入第二行数据，为结果图片尺寸，应要求不同有一个或多个
    {
        vector<vector<int>> input;
        std::vector<string> temp = Stringsplit_string<string>(info[1], ' ');
        for (auto i : temp)
        {
            std::vector<int> temp_A = Stringsplit<int>(i, '*');
            input.push_back(temp_A);
        }
        output_image_size = new output_image_info(input);
    }

    //读入第三行数据，为结果文件前缀名，应要求不同有一个或多个
    {
        Prefix = Stringsplit_string<string>(info[2], ' ');
    }

    load_path = info[3];

    if (info[5] == "NONE")
    {
        direction = group_direction::NONE;
    }
    else if (info[5] == "X")
    {
        direction = group_direction::X;
    }
    else if (info[5] == "Y")
    {
        direction = group_direction::Y;
    }
}

load_list::load_list(server_info serverinfo)
{
    int count = 0;
    string dirname;
    DIR *dp;
    struct dirent *dirp;
    dirname = serverinfo.GetLoadPath();
    if ((dp = opendir(dirname.c_str())) == NULL)
    {
        cout << "Can't open " << dirname << endl;
        abort();
    }
    while ((dirp = readdir(dp)) != NULL)
    {
        if (dirp->d_type == 8)
        {
            string sFilename(dirp->d_name);
            string suffixStr = sFilename.substr(sFilename.find_last_of('.') + 1);
            if (suffixStr.compare("jpg") == 0 || suffixStr.compare("jpeg") == 0 || suffixStr.compare("png") == 0 || suffixStr.compare("tga") == 0)
            {
                list.push_back(dirp->d_name);
                count++;
            }
        }
    }
    sort(list.begin(), list.end(), [](string a, string b)
         { return a < b; });
    for (auto i : list)
    {
        cout << serverinfo.GetLoadPath() + "/" + i << endl;
    }
    cout << "共有" << count << "个文件等待处理" << endl;

    closedir(dp);
}
