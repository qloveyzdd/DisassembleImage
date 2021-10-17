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

using namespace cv;
using namespace std;

// void thread_main(Mat quad, Mat mask_dilate, server_info serverinfo, load_list loadlist, player_settings playerset, cv::Mat transmtx);

int main()
{
    server_info serverinfo;
    load_list loadlist(serverinfo);

    disassembly disassemblyImage(&serverinfo);

    player_settings_factory playerset(&loadlist);
    vector<cpu_settings *> cpus_list = playerset.create(&disassemblyImage,&serverinfo);

    cpus_list[0]->cpu_work();

    return 0;
}