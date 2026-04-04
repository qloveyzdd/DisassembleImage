#include "server_info.h"

#include <algorithm>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <sstream>
#include <typeinfo>

namespace fs = std::filesystem;

output_image_info::output_image_info(vector<vector<int>> prim_screen)
{
    for (auto i : prim_screen)
    {
        alone_prim.push_back(image_info(i[0], i[1]));
    }
}

template <class T>
std::vector<T> Stringsplit(std::string str, const char split)
{
    std::vector<T> rst;
    std::istringstream iss(str);
    std::string token;
    while (getline(iss, token, split))
    {
        rst.push_back(atoi(token.c_str()));
    }
    return rst;
}

template <class T>
std::vector<T> Stringsplit_string(std::string str, const char split)
{
    std::vector<T> rst;
    std::istringstream iss(str);
    std::string token;
    while (getline(iss, token, split))
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

    {
        std::vector<int> tempSize = Stringsplit<int>(info[0], '*');
        input_image = new input_image_info({tempSize[0], tempSize[1]});
    }

    {
        vector<vector<int>> input;
        std::vector<string> tempSizes = Stringsplit_string<string>(info[1], ' ');
        for (auto i : tempSizes)
        {
            std::vector<int> tempA = Stringsplit<int>(i, '*');
            input.push_back(tempA);
        }
        output_image_size = new output_image_info(input);
    }

    {
        Prefix = Stringsplit_string<string>(info[2], ' ');
    }

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
    const fs::path inputDir(serverinfo.GetLoadPath());
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir))
    {
        cout << "Can't open " << inputDir.string() << endl;
        abort();
    }

    for (const auto &entry : fs::directory_iterator(inputDir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }

        string suffixStr = entry.path().extension().string();
        transform(suffixStr.begin(), suffixStr.end(), suffixStr.begin(), [](unsigned char ch) {
            return static_cast<char>(tolower(ch));
        });
        if (suffixStr == ".jpg" || suffixStr == ".jpeg" || suffixStr == ".png" || suffixStr == ".tga")
        {
            list.push_back(entry.path().filename().string());
            count++;
        }
    }

    sort(list.begin(), list.end(), [](string a, string b)
         { return a < b; });
    for (auto i : list)
    {
        cout << (inputDir / i).string() << endl;
    }
    cout << "共有" << count << "个文件等待处理" << endl;
}

load_list::load_list(string load_path, string load_name)
{
    const fs::path inputDir(load_path);
    for (const auto &entry : fs::directory_iterator(inputDir))
    {
        if (!entry.is_regular_file())
        {
            continue;
        }
        if (entry.path().filename().string() == load_name)
        {
            list.push_back(load_name);
            return;
        }
    }
}
