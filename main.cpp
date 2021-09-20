#include "opencv2/highgui/highgui.hpp"  
#include "opencv2/imgproc/imgproc.hpp" 
#include <fstream> 
#include <iostream>
#include <string>

#ifdef _WINDOWS_

#include <windows.h>

#endif // _WINDOWS_

using namespace cv;
using namespace std;


int main()
{
	cout << "输入屏幕横向尺寸:" << endl;
	string temp_x;
	cin >> temp_x;
	int xx = atoi(temp_x.c_str());
	cout << "输入屏幕纵向尺寸:" << endl;
	string temp_y;
	cin >> temp_y;
	int yy = atoi(temp_y.c_str());
	cout << "旋转角度: 0:(不旋转)1：(90度)" << endl;
	string xz_string;
	cin >> xz_string;
	int xz = atoi(xz_string.c_str());
	cout << "翻转角度: 0:(不翻转)1：(水平)2：(竖直)3：(二者都)" << endl;
	string fz_string;
	cin >> fz_string;
	int fz = atoi(fz_string.c_str());
	cout << "输入前缀:" << endl;
	string qz_string;
	cin >> qz_string;
	cout << "输入mask:" << endl;
	string mask_string;
	cin >> mask_string;
	cout << "输入存储路径:" << endl;
	string cc_string;
	cin >> cc_string;

	return 0;
}

