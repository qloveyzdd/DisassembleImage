#pragma once
#include "../server_info/server_info.h"
#include "../mask/mask.h"
#include <iostream>
using namespace std;

class settings
{
private:
    disassembly_factory *disassemblyIm;
    server_info *serverinfo;

public:
    settings(disassembly_factory *disassembly,server_info *serverinfo_in):disassemblyIm(disassembly),serverinfo(serverinfo_in){};
    server_info *get_server_info() { return serverinfo; }
    disassembly_factory *get_disassembly() { return disassemblyIm; }
};

class cpu_settings : public settings //单cpu工作表格
{
private:
    vector<string *> cpu_list;

public:
    cpu_settings(disassembly_factory *disassembly,server_info *serverinfo_in):settings(disassembly,serverinfo_in){};
    void cpu_list_add(string *string_temp) { cpu_list.push_back(string_temp); }
    string *cpu_list_show(int i) { return cpu_list[i]; }
    void cpu_work(); //单线程工作主函数
};

class player_settings_factory //总清单及cpu量
{
private:
    load_list *list;             //总任务清单
    vector<cpu_settings *> cpus; //工作cpu任务集合
    int cpu_count;               // cpu数量

public:
    player_settings_factory(load_list *list, disassembly_factory *disassembly, server_info *serverinfo_in);
    int get_cpu_count() { return cpu_count; }
    vector<cpu_settings *> get_cpus(){return cpus;}
};