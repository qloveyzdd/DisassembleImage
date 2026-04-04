#pragma once

#include <QMainWindow>
#include <QString>

#include <filesystem>

class QLabel;
class QPlainTextEdit;
class QPushButton;

namespace disassemble::desktop {
class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    void refreshEnvironmentStatus();
    void chooseInputImage();
    void chooseOutputDirectory();
    void runSmokeTask();
    void appendLog(const QString &message);

    std::filesystem::path inputImagePath_;
    std::filesystem::path outputDirectory_;

    QLabel *environmentLabel_;
    QLabel *inputLabel_;
    QLabel *outputLabel_;
    QPushButton *runButton_;
    QPlainTextEdit *logOutput_;
};
} // namespace disassemble::desktop
