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

vector<cpu_settings *> player_settings_factory::create()
{
    int count = ceil(float(list->list_count()) / float(cpu_count));
    cout << "单核心处理" << count << "个文件" << endl;
    vector<cpu_settings *> cpus;
    for (int i = 0; i < cpu_count; i++)
    {
        cpu_settings *cpu_list = new cpu_settings;
        int max_file = (i + 1) * count > list->list_count() ? list->list_count() : (i + 1) * count;
        for (int j = i * count; j < (i + 1) * count; j++)
        {
            cpu_list->cpu_list_add(list->get_file(j));
        }
        cpus.push_back(cpu_list);
    }
    return cpus;
}