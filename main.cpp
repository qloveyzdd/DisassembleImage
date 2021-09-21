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

int main()
{
    // load_list loadlist("/mnt/54a8f0a8-ae4e-40cd-9886-14ce13e6f03d/DisassembleImage", "yaleok.txt");
    // return 0;

    server_info serverinfo;
    load_list loadlist(serverinfo);
    player_settings playerset(loadlist);

    mask = imread("mask.jpg");
    DilationMask(mask, mask_dilate);

    std::vector<cv::Point2f> roi_point_approx;
    roi_point_approx = RoiPointApprox(mask);
    if (roi_point_approx.size() != 4)
    {
        printf("锚点不为4！");
        return false;
    }
    cv::Point2f center(0, 0);
    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    quad = cv::Mat::zeros(serverinfo.Get_y(), serverinfo.Get_x(), CV_8UC3);
    std::vector<cv::Point2f> quad_pts;
    Mattopts(quad, quad_pts);

    cv::Mat transmtx = cv::getPerspectiveTransform(roi_point_approx, quad_pts);

    cv::Mat dstImage;
    int count = 0;
    for (int i = playerset.get_begin() - 1; i < playerset.get_begin() + playerset.get_count() - 1; i++)
    {
        std::cout << "processing：" << loadlist.file_name(i) << endl;
        string savefile = loadlist.file_name(i);

        quad = imread(loadlist.file_name(i), -1);
        if (!quad.data)
        {
            printf("读取图片错误");
            return false;
        }

        quad.copyTo(dstImage, mask_dilate);

        cv::warpPerspective(dstImage, quad, transmtx, quad.size());

        cv::Mat image_fliped_temp;
        switch (serverinfo.Get_xz())
        {
        case 0:
            dstImage = quad;
            break;
        case 1:
            transpose(quad, image_fliped_temp);
            flip(image_fliped_temp, dstImage, 0);
            break;
        default:
            break;
        }

        switch (serverinfo.Get_fz())
        {
        case 0:
            quad = image_fliped_temp;
            break;
        case 1:
            flip(image_fliped_temp, quad, 1);
            break;
        case 2:
            flip(image_fliped_temp, quad, 0);
            break;
        case 3:
            flip(image_fliped_temp, quad, -1);
            break;
        default:
            break;
        }
        imwrite(savefile, quad);
        count++;
    }

    std::cout << "一共处理的图像数目" << count << endl;
    waitKey(0);

    return 0;
}
