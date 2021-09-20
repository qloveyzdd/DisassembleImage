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
    server_info serverinfo;
    // load_list loadlist(serverinfo);

    mask = imread("mask.jpg");
    DilationMask(mask,mask_dilate);

    std::vector<cv::Point2f> roi_point_approx;
    roi_point_approx = RoiPointApprox(mask);
    if (roi_point_approx.size()!=4)
	{
		printf("锚点不为4！"); 
		return false;
	}
    cv::Point2f center(0,0);
    center = GetCenter(roi_point_approx);
    sortCorners(roi_point_approx, center);

    quad = cv::Mat::zeros(serverinfo.Get_y(),serverinfo.Get_x(),CV_8UC3);
    std::vector<cv::Point2f>quad_pts;
    Mattopts(quad,quad_pts);
    

    // namedWindow("A");
    // namedWindow("B");
    // imshow("A",mask);
    // imshow("B",mask_dilate);

    waitKey(0);

    return 0;
}
