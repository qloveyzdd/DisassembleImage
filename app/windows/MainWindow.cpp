#include "MainWindow.h"

#include <QCoreApplication>
#include <QFileDialog>
#include <QHBoxLayout>
#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QStringList>
#include <QVBoxLayout>
#include <QWidget>

#include "EnvironmentCheck.h"
#include "RunController.h"

namespace disassemble::desktop {

MainWindow::MainWindow()
    : environmentLabel_(new QLabel(this)),
      inputLabel_(new QLabel(QStringLiteral("未选择输入图片"), this)),
      outputLabel_(new QLabel(QStringLiteral("未选择输出目录"), this)),
      runButton_(new QPushButton(QStringLiteral("开始处理"), this)),
      logOutput_(new QPlainTextEdit(this))
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);
    auto *inputLayout = new QHBoxLayout();
    auto *outputLayout = new QHBoxLayout();
    auto *actionLayout = new QHBoxLayout();

    auto *chooseInputButton = new QPushButton(QStringLiteral("选择输入图片"), this);
    auto *chooseOutputButton = new QPushButton(QStringLiteral("选择输出目录"), this);
    auto *refreshButton = new QPushButton(QStringLiteral("环境检查"), this);

    logOutput_->setReadOnly(true);

    inputLayout->addWidget(chooseInputButton);
    inputLayout->addWidget(inputLabel_);

    outputLayout->addWidget(chooseOutputButton);
    outputLayout->addWidget(outputLabel_);

    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(runButton_);

    mainLayout->addWidget(environmentLabel_);
    mainLayout->addLayout(inputLayout);
    mainLayout->addLayout(outputLayout);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(logOutput_);

    setCentralWidget(central);
    setWindowTitle(QStringLiteral("DisassembleImage"));
    resize(1080, 720);

    connect(chooseInputButton, &QPushButton::clicked, this, &MainWindow::chooseInputImage);
    connect(chooseOutputButton, &QPushButton::clicked, this, &MainWindow::chooseOutputDirectory);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshEnvironmentStatus);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::runSmokeTask);

    refreshEnvironmentStatus();
}

void MainWindow::refreshEnvironmentStatus()
{
    const auto appDir = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
    const auto status = EnvironmentCheck::inspect(appDir, outputDirectory_);

    QStringList lines;
    for (const auto &message : status.messages) {
        lines << QString::fromStdString(message);
    }
    environmentLabel_->setText(lines.join(QStringLiteral(" | ")));
    runButton_->setEnabled(status.ok && !inputImagePath_.empty());
}

void MainWindow::chooseInputImage()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择输入图片"), QString(), QStringLiteral("Images (*.jpg *.jpeg *.png *.tga)"));
    if (selected.isEmpty()) {
        return;
    }

    inputImagePath_ = std::filesystem::path(selected.toStdString());
    inputLabel_->setText(selected);
    appendLog(QStringLiteral("已选择输入图片: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseOutputDirectory()
{
    const auto selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择输出目录"));
    if (selected.isEmpty()) {
        return;
    }

    outputDirectory_ = std::filesystem::path(selected.toStdString());
    outputLabel_->setText(selected);
    appendLog(QStringLiteral("已选择输出目录: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::runSmokeTask()
{
    const auto appDir = std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
    const auto status = EnvironmentCheck::inspect(appDir, outputDirectory_);
    if (!status.ok) {
        appendLog(QStringLiteral("环境检查未通过，已阻止执行。"));
        refreshEnvironmentStatus();
        return;
    }

    try {
        appendLog(QStringLiteral("开始处理: %1").arg(QString::fromStdString(inputImagePath_.string())));
        const auto result = RunController::runSmokeTask(inputImagePath_, outputDirectory_, status.modelRoot);
        for (const auto &message : result.logs) {
            appendLog(QString::fromStdString(message));
        }
        appendLog(QStringLiteral("输出目录: %1").arg(QString::fromStdString(outputDirectory_.string())));
    } catch (const std::exception &error) {
        appendLog(QStringLiteral("处理失败: %1").arg(QString::fromUtf8(error.what())));
    }
}

void MainWindow::appendLog(const QString &message)
{
    logOutput_->appendPlainText(message);
}

} // namespace disassemble::desktop
