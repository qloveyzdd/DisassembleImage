#include "server_info.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

image_info::image_info(int x, int y, int xz, int fz) : x(x), y(y), xz(xz), fz(fz)
{
}

server_image_dlc1::server_image_dlc1()
{
    tl[0] = 0.f;
    tl[1] = 0.f;
    tr[0] = 1.f;
    tr[1] = 0.f;
    bl[0] = 0.f;
    bl[1] = 1.f;
    br[0] = 1.f;
    br[1] = 1.f;
}

server_image_dlc1::server_image_dlc1(server_info info_server)
{
    string temp = "";
    vector<float> info;
    temp.append(info_server.GetPrefix()).append("_scaleplate.txt");
    ifstream inf(temp);
    if (!inf.is_open())
    {
        cout << temp << "不能读取，请检查！" << endl;
    }
    temp.clear();
    while (getline(inf, temp))
    {
        info.push_back(float(atoi(temp.c_str())) / 10000.f);
    }
    tl[0] = info[0];
    tl[1] = info[1];
    tr[0] = info[2];
    tr[1] = info[3];
    bl[0] = info[4];
    bl[1] = info[5];
    br[0] = info[6];
    br[1] = info[7];
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
        cout << temp << "不能读取，请检查！" << endl;
    }
    temp.clear();
    while (getline(inf, temp))
    {
        info.push_back(temp);
    }
    image_size = new image_info(atoi(info[0].c_str()), atoi(info[1].c_str()), atoi(info[2].c_str()), atoi(info[3].c_str()));
    Prefix = info[4];
    mask = info[5];
    load_path = info[6];
    save_path = info[7];
    temp.clear();
    cout << "此拆分是否占用整个平面（1.是 2.否）" << endl;
    cin >> temp;
    int system = atoi(temp.c_str());
    temp = "";
    switch (system)
    {
    case 1:
    {

        image_screen_size = new server_image_dlc1();
        break;
    }
    case 2:
    {
        image_screen_size = new server_image_dlc1(*this);
        break;
    }
    default:
        break;
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
                // cout << serverinfo.GetLoadPath() + "/" + dirp->d_name << endl;
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

// load_list::load_list(string load_path, string load_name)
// {
//     ifstream inf(load_name);
//     string image_name;
//     int count;
//     while (getline(inf, image_name))
//     {
//         string temp = load_path + "/" + image_name;
//         list.push_back(image_name);
//         cout << temp << endl;
//         count++;
//     }
//     //   cout << count << endl;
//     cout << "共有" << count << "个文件等待处理" << endl;
// }
