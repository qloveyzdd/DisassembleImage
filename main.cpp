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

    for (int i = playerset.get_begin()-1; i < playerset.get_begin()+playerset.get_count(); i++)
    {
        std::cout << "processing：" << loadlist.file_name(i) << endl;
    }
    
    // cv::warpPerspective(dstImage, quad, transmtx, quad.size());
    // namedWindow("A");
    // namedWindow("B");
    // imshow("A",mask);
    // imshow("B",mask_dilate);

    waitKey(0);

    return 0;
}
