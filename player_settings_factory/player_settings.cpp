#include "player_settings.h"
#include <unistd.h>
#include <math.h>

player_settings_factory::player_settings_factory(load_list *list_in, disassembly_factory *disassembly, server_info *serverinfo_in)
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

    count = ceil(float(list->list_count()) / float(cpu_count));
    cout << "单核心处理" << count << "个文件" << endl;

    for (int i = 0; i < cpu_count; i++)
    {
        cpu_settings *cpu_list = new cpu_settings(disassembly, serverinfo_in);
        // int max_file = (i + 1) * count > list->list_count() ? list->list_count() : (i + 1) * count;
        for (int j = i; j < list->list_count(); j += cpu_count)
        {
            cpu_list->cpu_list_add(list->get_file(j));
        }
        cpus.push_back(cpu_list);
    }
}

void cpu_settings::cpu_work()
{
    for (auto filename : cpu_list)
    {
        cv::Mat dstImage;

        string loadfile = get_server_info()->GetLoadPath() + "/" + *filename;
        cv::Mat quad = cv::imread(loadfile, -1);
        if (!quad.data)
        {
            cout << "读取图片错误" << endl;
            abort();
        }
        quad.copyTo(dstImage);
        for (auto i = 0; i < get_disassembly()->get_prim().size(); i++)
        {
            cv::warpPerspective(dstImage, quad, get_disassembly()->get_prim()[i]->get_transmtx(), cv::Size(get_disassembly()->get_prim()[i]->get_quad_pts()[2]));

            string savefile = get_server_info()->GetSavePath() + get_server_info()->GetPrefix()[i] + "/" + get_server_info()->GetPrefix()[i] + *filename;
            imwrite(savefile, quad);
            std::cout << "processing：" << savefile << std::endl;
        }
    }
}