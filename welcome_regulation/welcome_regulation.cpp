#include "welcome_regulation.h"
#include <iostream>

void welcome::welcome_string()
{
    std::cout << "欢迎使用拆分系统！！" << std::endl;
    std::cout << "软件功能是将单一平面画面拆分成为多个画面，并根据实际空间需求进行变形" << std::endl;
    std::cout << "此系统需配置固定文件夹格式及相关配置文件，请确保总体完整" << std::endl;
    std::cout << "dlc1：可使用边缘不完整的四边形尺寸" << std::endl;
}

void regulation::regulation_string()
{
    std::cout << "主体程序：" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "1.屏幕横向尺寸" << std::endl;
    std::cout << "2.屏幕纵向尺寸" << std::endl;
    std::cout << "3.旋转角度: 0:(不旋转)1：(90度)" << std::endl;
    std::cout << "4.翻转角度: 0:(不翻转)1：(水平)2：(竖直)3：(二者都)" << std::endl;
    std::cout << "5.输入前缀" << std::endl;
    std::cout << "6.输入mask" << std::endl;
    std::cout << "7.读取待拆分文件路径" << std::endl;
    std::cout << "8.存储路径" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "DLC1：" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "屏幕左上角横向比例(万分比)" << std::endl;
    std::cout << "屏幕左上角纵向比例(万分比)" << std::endl;
    std::cout << "屏幕右上角横向比例(万分比)" << std::endl;
    std::cout << "屏幕右上角纵向比例(万分比)" << std::endl;
    std::cout << "屏幕左下角横向比例(万分比)" << std::endl;
    std::cout << "屏幕左下角纵向比例(万分比)" << std::endl;
    std::cout << "屏幕右下角横向比例(万分比)" << std::endl;
    std::cout << "屏幕右下角纵向比例(万分比)" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}