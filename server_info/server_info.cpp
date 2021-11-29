#include "server_info.h"
#include <iostream>
#include <fstream>
#include <unistd.h>
#include <dirent.h>
#include <algorithm>

server_image_dlc1::server_image_dlc1()
{
    string temp;
    cout << "此拆分是否占用整个平面（1.是 2.否）" << endl;
    cin >> temp;
    int system = atoi(temp.c_str());
    switch (system)
    {
    case 1:
    {
        tl[0] = 0.f;
        tl[1] = 0.f;
        tr[0] = 1.f;
        tr[1] = 0.f;
        bl[0] = 0.f;
        bl[1] = 1.f;
        br[0] = 1.f;
        br[1] = 1.f;
        break;
    }
    case 2:
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
            cout << "输入屏幕左上角横向比例(万分比):" << endl;
            cin >> temp;
            tl[0] = atoi(temp.c_str())/10000;

            temp = "";
            cout << "输入屏幕左上角纵向比例(万分比):" << endl;
            cin >> temp;
            tl[1] = atoi(temp.c_str())/10000;
            //////////////////////////////////////////////////////////
            temp = "";
            cout << "输入屏幕右上角横向比例(万分比):" << endl;
            cin >> temp;
            tr[0] = atoi(temp.c_str())/10000;

            temp = "";
            cout << "输入屏幕右上角纵向比例(万分比):" << endl;
            cin >> temp;
            tr[1] = atoi(temp.c_str())/10000;
            ////////////////////////////////////////////////////////////
            temp = "";
            cout << "输入屏幕左下角横向比例(万分比):" << endl;
            cin >> temp;
            bl[0] = atoi(temp.c_str())/10000;

            temp = "";
            cout << "输入屏幕左下角纵向比例(万分比):" << endl;
            cin >> temp;
            bl[1] = atoi(temp.c_str())/10000;
            ///////////////////////////////////////////////////////////
            temp = "";
            cout << "输入屏幕右下角横向比例(万分比):" << endl;
            cin >> temp;
            br[0] = atoi(temp.c_str())/10000;

            temp = "";
            cout << "输入屏幕右下角纵向比例(万分比):" << endl;
            cin >> temp;
            br[1] = atoi(temp.c_str())/10000;

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
            tl[0] = float(atoi(info[0].c_str()))/10000.f;
            tl[1] = float(atoi(info[1].c_str()))/10000.f;
            tr[0] = float(atoi(info[2].c_str()))/10000.f;
            tr[1] = float(atoi(info[3].c_str()))/10000.f;
            bl[0] = float(atoi(info[4].c_str()))/10000.f;
            bl[1] = float(atoi(info[5].c_str()))/10000.f;
            br[0] = float(atoi(info[6].c_str()))/10000.f;
            br[1] = float(atoi(info[7].c_str()))/10000.f;
            break;
        }
        }
    }
    }
}

server_info::server_info()
{
    string temp;
    cout << "选择输入模式（1.手动输入(未完成)2.文本导入）" << endl;
    cin >> temp;
    int system = atoi(temp.c_str());
    switch (system)
    {
    case 1:
    {
        abort();
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

        cout << "输入读取待拆分文件路径:" << endl;
        cin >> load_path;

        // cout << "输入读取待拆分文件目录:" << endl;
        // cin >> load_name;

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
        load_path = info[6];
        // load_name = info[7];
        save_path = info[7];
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

load_list::load_list(string load_path, string load_name)
{
    ifstream inf(load_name);
    string image_name;
    int count;
    while (getline(inf, image_name))
    {
        string temp = load_path + "/" + image_name;
        list.push_back(image_name);
        cout << temp << endl;
        count++;
    }
    //   cout << count << endl;
    cout << "共有" << count << "个文件等待处理" << endl;
}
