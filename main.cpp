#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef _WINDOWS_

#include <windows.h>

#endif // _WINDOWS_

#include "server_info/server_info.h"
#include "mask/mask.h"
#include "player_settings_factory/player_settings.h"
#include "welcome_regulation/welcome_regulation.h"

#include <iostream>
#include <fstream>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <dirent.h>

using namespace cv;
using namespace std;

pid_t r_wait(int *stat_loc)
{
    int revalue;
    while (((revalue = wait(stat_loc)) == -1) && (errno == EINTR))
        ; //如果等待的过程中被一个不可阻塞的信号终断则继续循环等待
    return revalue;
}

int main(int argc, char *argv[])
{
    welcome::welcome_string();
    regulation::regulation_string();

    obj_uv_padding obj_input("poly.obj");
    obj_basic obj_output("poly.obj");

    server_info serverinfo;
    load_list loadlist(serverinfo);
    disassembly_factory disassemblyfactory(&serverinfo, &obj_input,&obj_output);

    if (argc > 1)
    {
        // disassemblyImage.check_mask();
    }

    player_settings_factory playerset(&loadlist);
    // vector<cpu_settings *> cpus_list = playerset.create(&disassemblyImage, &serverinfo);

    // for (int i = 0; i < playerset.get_cpu_count(); i++)
    // {
    //     if (fork() > 0)
    //     {
    //     }
    //     else
    //     {
    //         cpus_list[i]->cpu_work();
    //         break;
    //     }
    // }

    // while (r_wait(NULL) > 0)
    //     ; // wait for all the subprocess.

    return 0;
}
