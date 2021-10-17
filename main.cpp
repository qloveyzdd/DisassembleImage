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
    player_settings_factory playerset(&loadlist);
    vector<cpu_settings *> cpus_list = playerset.create();
    disassembly disassemblyImage(&serverinfo);

    return 0;

    // int count = 0;
    // for (int i = playerset.get_begin() - 1; i < playerset.get_begin() + playerset.get_count(); i++)
    // {
    //     cv::Mat quad_copy = quad;
    //     cv::Mat dstImage;
    //     std::cout << "processing：" << loadlist.file_name(i) << endl;
    //     string savefile = serverinfo.GetSavePath() + "/" + serverinfo.GetPrefix() + loadlist.file_name(i);
    //     string qaz = serverinfo.GetLoadPath() + "/" + loadlist.file_name(i);
    //     Mat quad1 = imread(serverinfo.GetLoadPath() + "/" + loadlist.file_name(i), -1);
    //     if (!quad1.data)
    //     {
    //         printf("读取图片错误");
    //         return 0;
    //     }

    //     quad1.copyTo(dstImage, mask_dilate);

    //     cv::warpPerspective(dstImage, quad_copy, transmtx, quad_copy.size());

    //     cv::Mat image_fliped_temp;
    //     switch (serverinfo.Get_xz())
    //     {
    //     case 0:
    //         dstImage = quad_copy;
    //         break;
    //     case 1:
    //         transpose(quad_copy, image_fliped_temp);
    //         flip(image_fliped_temp, dstImage, 0);
    //         break;
    //     default:
    //         break;
    //     }

    //     switch (serverinfo.Get_fz())
    //     {
    //     case 0:
    //         quad_copy = dstImage;
    //         break;
    //     case 1:
    //         flip(dstImage, quad_copy, 1);
    //         break;
    //     case 2:
    //         flip(dstImage, quad_copy, 0);
    //         break;
    //     case 3:
    //         flip(dstImage, quad_copy, -1);
    //         break;
    //     default:
    //         break;
    //     }
    //     // namedWindow("A",CV_WINDOW_NORMAL);
    //     // imshow("A",quad);
    //     imwrite(savefile, quad_copy);
    //     count++;
    // }
}