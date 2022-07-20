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
std::vector<T> Stringsplit(std::string str, const char split, int type = 1) // string拆分
{
    std::vector<T> rst;

    std::istringstream iss(str);       // 输入流
    std::string token;                 // 接收缓冲区
    while (getline(iss, token, split)) // 以split为分隔符
    {
        switch (type)
        {
        case 0:
            rst.push_back(atoi(token.c_str()));
            break;
        case 1:
            rst.push_back(token);
            break;
        default:
            break;
        }
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

    {
        std::vector<int> temp = Stringsplit<int>(info[0], '*');
        input_image = new input_image_info({temp[0], temp[1]});
    }

    {
        vector<vector<int>> input;
        std::vector<string> temp = Stringsplit<string>(info[1], ' ',1);
        for (auto i : temp)
        {
            std::vector<int> temp_A = Stringsplit<int>(i, '*',0);
            input.push_back(temp_A);
        }
        output_image_size = new output_image_info(input);
    }

    Prefix = info[2];
    load_path = info[3];
    save_path = info[4];

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
            if (suffixStr.compare("jpg") == 0 || suffixStr.compare("png") == 0 || suffixStr.compare("tga") == 0)
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
