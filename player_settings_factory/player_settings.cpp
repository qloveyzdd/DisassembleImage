#include "player_settings.h"
#include <unistd.h>
#include <math.h>

player_settings_factory::player_settings_factory(load_list *list_in)
{
    list = list_in;
    string temp = "";
    cout << "是否采用多线程？0：否，1：是" << endl;
    cin >> temp;
    int count = atoi(temp.c_str());
    switch (count)
    {
    case 0:
    {
        cpu_count = 1;
        break;
    }
    case 1:
    {
        cpu_count = sysconf(_SC_NPROCESSORS_ONLN);
        break;
    }
    default:
        break;
    }
    cout << "处理核心数量为" << cpu_count << "个" << endl;
}

vector<cpu_settings *> player_settings_factory::create(disassembly *disassembly, server_info *serverinfo_in)
{
    int count = ceil(float(list->list_count()) / float(cpu_count));
    cout << "单核心处理" << count << "个文件" << endl;
    vector<cpu_settings *> cpus;
    for (int i = 0; i < cpu_count; i++)
    {
        cpu_settings *cpu_list = new cpu_settings;
        cpu_list->set_disassemblyIm(disassembly);
        cpu_list->set_server_info(serverinfo_in);
        int max_file = (i + 1) * count > list->list_count() ? list->list_count() : (i + 1) * count;
        for (int j = i * count; j < max_file; j++)
        {
            cpu_list->cpu_list_add(list->get_file(j));
        }
        cpus.push_back(cpu_list);
    }
    return cpus;
}

void cpu_settings::cpu_work()
{
    int count = 0;
    for (auto filename : cpu_list)
    {
        Mat dstImage;
        std::cout << "processing：" << get_server_info()->GetPrefix() << *filename << endl;
        string savefile = get_server_info()->GetSavePath() + "/" + get_server_info()->GetPrefix() + *filename;
        string loadfile = get_server_info()->GetLoadPath() + "/" + *filename;
        Mat quad = imread(loadfile, -1);
        if (!quad.data)
        {
            printf("读取图片错误");
            abort();
        }
        quad.copyTo(dstImage, get_disassembly()->get_mask_dilate());
        cv::warpPerspective(dstImage, quad, get_disassembly()->get_transmtx(), quad.size());
        switch (get_server_info()->Get_xz())
        {
        case 1:
            transpose(quad, quad);
            flip(quad, quad, 0);
            break;
        default:
            break;
        }
        switch (get_server_info()->Get_fz())
        {
        case 1:
            flip(quad, quad, 1);
            break;
        case 2:
            flip(quad, quad, 0);
            break;
        case 3:
            flip(quad, quad, -1);
            break;
        default:
            break;
        }
        imwrite(savefile, quad);
    }
}