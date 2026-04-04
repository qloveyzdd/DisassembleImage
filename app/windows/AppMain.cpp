#include <QApplication>

#include "MainWindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    disassemble::desktop::MainWindow window;
    window.show();

    return app.exec();
}
