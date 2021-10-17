#pragma once
#include "../server_info/server_info.h"
#include <iostream>
using namespace std;

class settings
{
};

class cpu_settings : public settings //单cpu工作表格
{
private:
    vector<string*> cpu_list;

public:
    void cpu_list_add(string* string_temp){cpu_list.push_back(string_temp);}
    void cpu_work(); //单线程工作主函数
};

class player_settings_factory //总清单及cpu量
{
private:
    load_list *list;
    int cpu_count;

public:
    player_settings_factory(load_list *list);
    vector<cpu_settings *> create();
};