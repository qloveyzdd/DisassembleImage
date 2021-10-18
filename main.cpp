#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef _WINDOWS_

#include <windows.h>

#endif // _WINDOWS_

#include "server_info/server_info.h"
#include "mask/mask.h"
#include "player_settings_factory/player_settings.h"

#include <iostream>
#include <fstream>
// #include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include<sys/wait.h>

using namespace cv;
using namespace std;

// void thread_main(Mat quad, Mat mask_dilate, server_info serverinfo, load_list loadlist, player_settings playerset, cv::Mat transmtx);

// class TestThread
// {
// private:
//     struct ThreadParam
//     {
//         TestThread *myself_;
//     };
//     cpu_settings *cpusetting;

// public:
//     void runThread();
//     void setcpusetting(cpu_settings *cpusetting_in) { cpusetting = cpusetting_in; }

// private:
//     static void *threadFunction(void *threadParam);
//     void function();
// };

// void TestThread::runThread()
// {
//     pthread_t thread;
//     ThreadParam threadParam;

//     threadParam.myself_ = this;
//     pthread_create(&thread, NULL, threadFunction, (ThreadParam *)&threadParam);
// }

// void *TestThread::threadFunction(void *threadParam)
// {
//     ThreadParam *thread = (ThreadParam *)threadParam;
//     thread->myself_->function();
//     return NULL;
// }

// void TestThread::function()
// {
//     cout<<"AAA"<<endl;
// }


pid_t r_wait(int * stat_loc)
{
	int revalue;
	while(((revalue = wait(stat_loc)) == -1) && (errno == EINTR));//如果等待的过程中被一个不可阻塞的信号终断则继续循环等待
	return revalue;
}

int main()
{
    server_info serverinfo;
    load_list loadlist(serverinfo);

    disassembly disassemblyImage(&serverinfo);

    player_settings_factory playerset(&loadlist);
    vector<cpu_settings *> cpus_list = playerset.create(&disassemblyImage, &serverinfo);

    for (int i = 0; i < playerset.get_cpu_count(); i++)
    {
        if(fork()>0)
        {

        }
        else
        {
            cpus_list[i]->cpu_work();
            break;
        }
    }

    while(r_wait(NULL) > 0);//wait for all the subprocess.
    
    // cpus_list[0]->cpu_work();

    // TestThread threadcpu[playerset.get_cpu_count()];

    // for (int i = 0; i < playerset.get_cpu_count(); i++)
    // {
    //     threadcpu[i].setcpusetting(cpus_list[i]);
    //     threadcpu[i].runThread();
    // }

    // TestThread thread1;
    // thread1.setcpusetting(cpus_list[0]);
    // thread1.runThread();

    // pthread_exit(NULL);
    return 0;
}
