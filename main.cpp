#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#ifdef _WINDOWS_

#include <windows.h>

#endif // _WINDOWS_

#include "server_info/server_info.h"
#include "mask/mask.h"

#include <iostream>
#include <fstream>

using namespace cv;
using namespace std;

Mat mask, mask_dilate;
Mat quad;

void thread_main(Mat quad, Mat mask_dilate, server_info serverinfo, load_list loadlist, player_settings playerset, cv::Mat transmtx);

int main()
{
    // load_list loadlist("/mnt/54a8f0a8-ae4e-40cd-9886-14ce13e6f03d/DisassembleImage", "yaleok.txt");
    // return 0;

    server_info serverinfo;
    load_list loadlist(serverinfo);
    player_settings playerset(loadlist);

    mask = imread(serverinfo.GetMask());
    DilationMask(mask, mask_dilate);

    std::vector<cv::Point2f> roi_point_approx;
    roi_point_approx = RoiPointApprox(mask);
    
    cv::Point2f center(0, 0);
    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    if (roi_point_approx.size() != 4)
    {
        printf("锚点不为4！");
        return false;
    }

    quad = cv::Mat::zeros(serverinfo.Get_y(), serverinfo.Get_x(), CV_8UC3);
    std::vector<cv::Point2f> quad_pts;
    Mattopts(quad, quad_pts);

    cv::Mat transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts);

<<<<<<< HEAD
    
    int count = 0;
    for (int i = playerset.get_begin() - 1; i < playerset.get_begin() + playerset.get_count(); i++)
=======
    loadlist.cpu_thread_list_settings(playerset);

    thread_main(quad,mask_dilate,serverinfo,loadlist,playerset,transmtx);

    waitKey(0);

    return 0;
}

void thread_main(Mat quad, Mat mask_dilate, server_info serverinfo, load_list loadlist, player_settings playerset, cv::Mat transmtx)
{
    cv::Mat dstImage;
    for (int i = playerset.get_begin() - 1; i < playerset.get_begin() + playerset.get_count() - 1; i++)
>>>>>>> main
    {
        cv::Mat quad_copy = quad;
        cv::Mat dstImage;
        std::cout << "processing：" << loadlist.file_name(i) << endl;
        string savefile = serverinfo.GetSavePath() + "/" + serverinfo.GetPrefix() + loadlist.file_name(i);
        string qaz = serverinfo.GetLoadPath() + "/" + loadlist.file_name(i);
        Mat quad1 = imread(serverinfo.GetLoadPath() + "/" + loadlist.file_name(i), -1);
        if (!quad1.data)
        {
            printf("读取图片错误");
            return;
        }

        quad1.copyTo(dstImage, mask_dilate);

        cv::warpPerspective(dstImage, quad_copy, transmtx, quad_copy.size());

        cv::Mat image_fliped_temp;
        switch (serverinfo.Get_xz())
        {
        case 0:
            dstImage = quad_copy;
            break;
        case 1:
            transpose(quad_copy, image_fliped_temp);
            flip(image_fliped_temp, dstImage, 0);
            break;
        default:
            break;
        }

        switch (serverinfo.Get_fz())
        {
        case 0:
            quad_copy = dstImage;
            break;
        case 1:
            flip(dstImage, quad_copy, 1);
            break;
        case 2:
            flip(dstImage, quad_copy, 0);
            break;
        case 3:
            flip(dstImage, quad_copy, -1);
            break;
        default:
            break;
        }
        // namedWindow("A",CV_WINDOW_NORMAL);
        // imshow("A",quad);
<<<<<<< HEAD
        imwrite(savefile, quad_copy);
        count++;
=======
        imwrite(savefile, quad);
>>>>>>> main
    }
}