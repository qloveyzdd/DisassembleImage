#pragma once
#include "../server_info/server_info.h"
#include "../mask/mask.h"
#include <iostream>
using namespace std;

class settings
{
private:
    disassembly *disassemblyIm;
    server_info *serverinfo;

public:
    void set_disassemblyIm(disassembly *disassembly){disassemblyIm = disassembly;}
    void set_server_info(server_info *serverinfo_in){serverinfo = serverinfo_in;}
    server_info *get_server_info(){return serverinfo;}
    disassembly *get_disassembly(){return disassemblyIm;}
};

class cpu_settings : public settings //单cpu工作表格
{
private:
    vector<string *> cpu_list;

public:
    void cpu_list_add(string *string_temp) { cpu_list.push_back(string_temp); }
    string *cpu_list_show(int i) { return cpu_list[i]; }
    void cpu_work(); //单线程工作主函数
};

class player_settings_factory //总清单及cpu量
{
private:
    load_list *list;
    int cpu_count;

public:
    player_settings_factory(load_list *list);
    vector<cpu_settings *> create(disassembly *disassembly,server_info *serverinfo_in);
    int get_cpu_count(){return cpu_count; }
};