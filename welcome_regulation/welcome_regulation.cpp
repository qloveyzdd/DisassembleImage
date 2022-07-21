#include "welcome_regulation.h"
#include <iostream>

void welcome::welcome_string()
{
    std::cout << "欢迎使用拆分系统！！" << std::endl;
    std::cout << "软件功能是将obj文件中的面片拆分成为多个画面或者合并为一个画面，并根据实际空间需求进行变形" << std::endl;
    std::cout << "此系统需配置固定文件夹格式及相关配置文件，请确保总体完整" << std::endl;
    std::cout << "obj文件为（input.obj）必须，（output.obj）非必须" << std::endl;
    std::cout << "可使用边缘不完整的四边形尺寸" << std::endl;
}

void regulation::regulation_string()
{
    std::cout << "主体程序：" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "1.输入屏幕尺寸，通过*来分别（长*宽），保持尺寸为唯一" << std::endl;
    std::cout << "2.输出屏幕尺寸数组，通过（空格）来分别，通过*来分别（长*宽）" << std::endl;
    std::cout << "3.输入前缀数组，通过（空格）来分别，如是一对一拆分，需保持与2项数量相同" << std::endl;
    std::cout << "4.读取待拆分文件路径" << std::endl;
    std::cout << "5.存储路径" << std::endl;
    std::cout << "6.组合方式：NONE，X，Y" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "注意事项：" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
    std::cout << "查看配置文件中最后一项为拆分模式" << std::endl;
    std::cout << "文件系统中只有input.obj，则为一对一拆分，保持2.3项数目相同" << std::endl;
    std::cout << "多对一拆分中，2.3项只有第一项有效" << std::endl;
    std::cout << "存储文件需保证有相对应的文件夹" << std::endl;
    std::cout << "--------------------------------------------" << std::endl;
}