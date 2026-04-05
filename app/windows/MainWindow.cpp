#include "MainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidget>

#include "RunController.h"
#include "TaskFormValidator.h"

namespace {

QString utf8Text(const std::string &text)
{
    return QString::fromUtf8(text.data(), static_cast<int>(text.size()));
}

int directionIndex(disassemble::core::ProcessingDirection direction)
{
    switch (direction) {
    case disassemble::core::ProcessingDirection::None:
        return 0;
    case disassemble::core::ProcessingDirection::X:
        return 1;
    case disassemble::core::ProcessingDirection::Y:
        return 2;
    }
    return 1;
}

disassemble::core::ProcessingDirection directionFromIndex(int index)
{
    switch (index) {
    case 0:
        return disassemble::core::ProcessingDirection::None;
    case 2:
        return disassemble::core::ProcessingDirection::Y;
    default:
        return disassemble::core::ProcessingDirection::X;
    }
}

int outputPolicyIndex(disassemble::core::OutputConflictPolicy policy)
{
    switch (policy) {
    case disassemble::core::OutputConflictPolicy::ForbidOverwrite:
        return 0;
    case disassemble::core::OutputConflictPolicy::OverwriteExisting:
        return 1;
    case disassemble::core::OutputConflictPolicy::AutoRename:
        return 2;
    }
    return 0;
}

disassemble::core::OutputConflictPolicy outputPolicyFromIndex(int index)
{
    switch (index) {
    case 1:
        return disassemble::core::OutputConflictPolicy::OverwriteExisting;
    case 2:
        return disassemble::core::OutputConflictPolicy::AutoRename;
    default:
        return disassemble::core::OutputConflictPolicy::ForbidOverwrite;
    }
}

} // namespace

namespace disassemble::desktop {

MainWindow::MainWindow()
    : presetStore_(),
      environmentLabel_(new QLabel(this)),
      validationLabel_(new QLabel(this)),
      singleModeButton_(new QRadioButton(QStringLiteral("单张图片"), this)),
      directoryModeButton_(new QRadioButton(QStringLiteral("图片目录"), this)),
      singleInputWidget_(new QWidget(this)),
      directoryInputWidget_(new QWidget(this)),
      inputImageEdit_(new QLineEdit(this)),
      inputDirectoryEdit_(new QLineEdit(this)),
      outputDirectoryEdit_(new QLineEdit(this)),
      autoDetectModelsCheck_(new QCheckBox(QStringLiteral("自动探测 input.obj / output.obj"), this)),
      inputObjEdit_(new QLineEdit(this)),
      outputObjEdit_(new QLineEdit(this)),
      inputObjBrowseButton_(new QPushButton(QStringLiteral("浏览"), this)),
      outputObjBrowseButton_(new QPushButton(QStringLiteral("浏览"), this)),
      sizeEdit_(new QLineEdit(this)),
      prefixEdit_(new QLineEdit(this)),
      directionCombo_(new QComboBox(this)),
      outputPolicyCombo_(new QComboBox(this)),
      advancedToggleButton_(new QToolButton(this)),
      advancedWidget_(new QWidget(this)),
      parallelCheck_(new QCheckBox(QStringLiteral("启用并行"), this)),
      workerSpin_(new QSpinBox(this)),
      runButton_(new QPushButton(QStringLiteral("开始处理"), this)),
      logOutput_(new QPlainTextEdit(this))
{
    auto *central = new QWidget(this);
    auto *mainLayout = new QVBoxLayout(central);

    auto *presetLayout = new QHBoxLayout();
    auto *savePresetButton = new QPushButton(QStringLiteral("保存预设"), this);
    auto *loadPresetButton = new QPushButton(QStringLiteral("加载预设"), this);
    presetLayout->addWidget(savePresetButton);
    presetLayout->addWidget(loadPresetButton);
    presetLayout->addStretch();

    auto *inputModeGroup = new QGroupBox(QStringLiteral("任务输入"), this);
    auto *inputModeLayout = new QVBoxLayout(inputModeGroup);
    auto *modeLayout = new QHBoxLayout();
    auto *singleInputLayout = new QHBoxLayout(singleInputWidget_);
    auto *directoryInputLayout = new QHBoxLayout(directoryInputWidget_);
    auto *singleBrowseButton = new QPushButton(QStringLiteral("选择输入图片"), this);
    auto *directoryBrowseButton = new QPushButton(QStringLiteral("选择输入目录"), this);
    modeLayout->addWidget(singleModeButton_);
    modeLayout->addWidget(directoryModeButton_);
    modeLayout->addStretch();
    singleInputLayout->setContentsMargins(0, 0, 0, 0);
    singleInputLayout->addWidget(inputImageEdit_);
    singleInputLayout->addWidget(singleBrowseButton);
    directoryInputLayout->setContentsMargins(0, 0, 0, 0);
    directoryInputLayout->addWidget(inputDirectoryEdit_);
    directoryInputLayout->addWidget(directoryBrowseButton);
    inputModeLayout->addLayout(modeLayout);
    inputModeLayout->addWidget(singleInputWidget_);
    inputModeLayout->addWidget(directoryInputWidget_);

    auto *resourceGroup = new QGroupBox(QStringLiteral("资源与输出"), this);
    auto *resourceLayout = new QFormLayout(resourceGroup);
    auto *outputLayout = new QHBoxLayout();
    auto *outputBrowseButton = new QPushButton(QStringLiteral("浏览"), this);
    outputLayout->addWidget(outputDirectoryEdit_);
    outputLayout->addWidget(outputBrowseButton);

    auto *inputObjLayout = new QHBoxLayout();
    inputObjLayout->addWidget(inputObjEdit_);
    inputObjLayout->addWidget(inputObjBrowseButton_);

    auto *outputObjLayout = new QHBoxLayout();
    outputObjLayout->addWidget(outputObjEdit_);
    outputObjLayout->addWidget(outputObjBrowseButton_);

    resourceLayout->addRow(autoDetectModelsCheck_);
    resourceLayout->addRow(QStringLiteral("input.obj"), inputObjLayout);
    resourceLayout->addRow(QStringLiteral("output.obj"), outputObjLayout);
    resourceLayout->addRow(QStringLiteral("输出目录"), outputLayout);

    auto *parameterGroup = new QGroupBox(QStringLiteral("核心参数"), this);
    auto *parameterLayout = new QFormLayout(parameterGroup);
    directionCombo_->addItems({QStringLiteral("逐面输出"), QStringLiteral("横向拼接"), QStringLiteral("纵向拼接")});
    outputPolicyCombo_->addItems({QStringLiteral("禁止覆盖"), QStringLiteral("覆盖同名"), QStringLiteral("自动重命名")});
    sizeEdit_->setPlaceholderText(QStringLiteral("例如: 6144*6720，多个用空格或逗号分隔"));
    prefixEdit_->setPlaceholderText(QStringLiteral("逐面输出会自动按位置命名；拼接模式填写一个前缀"));
    parameterLayout->addRow(QStringLiteral("输出尺寸"), sizeEdit_);
    parameterLayout->addRow(QStringLiteral("输出前缀"), prefixEdit_);
    parameterLayout->addRow(QStringLiteral("拼接方式"), directionCombo_);
    parameterLayout->addRow(QStringLiteral("输出策略"), outputPolicyCombo_);

    advancedToggleButton_->setText(QStringLiteral("高级设置"));
    advancedToggleButton_->setCheckable(true);
    auto *advancedLayout = new QFormLayout(advancedWidget_);
    workerSpin_->setMinimum(1);
    workerSpin_->setMaximum(128);
    advancedLayout->addRow(parallelCheck_);
    advancedLayout->addRow(QStringLiteral("并行线程数"), workerSpin_);
    advancedWidget_->setVisible(false);

    auto *actionLayout = new QHBoxLayout();
    auto *refreshButton = new QPushButton(QStringLiteral("刷新校验"), this);
    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(runButton_);
    actionLayout->addStretch();

    logOutput_->setReadOnly(true);
    environmentLabel_->setWordWrap(true);
    validationLabel_->setWordWrap(true);
    validationLabel_->setStyleSheet(QStringLiteral("color: #b42318;"));

    mainLayout->addLayout(presetLayout);
    mainLayout->addWidget(environmentLabel_);
    mainLayout->addWidget(validationLabel_);
    mainLayout->addWidget(inputModeGroup);
    mainLayout->addWidget(resourceGroup);
    mainLayout->addWidget(parameterGroup);
    mainLayout->addWidget(advancedToggleButton_);
    mainLayout->addWidget(advancedWidget_);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(logOutput_);

    setCentralWidget(central);
    setWindowTitle(QStringLiteral("DisassembleImage"));
    resize(1180, 840);

    connect(singleBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseInputImage);
    connect(directoryBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseInputDirectory);
    connect(outputBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseOutputDirectory);
    connect(inputObjBrowseButton_, &QPushButton::clicked, this, &MainWindow::chooseInputObjPath);
    connect(outputObjBrowseButton_, &QPushButton::clicked, this, &MainWindow::chooseOutputObjPath);
    connect(savePresetButton, &QPushButton::clicked, this, &MainWindow::savePreset);
    connect(loadPresetButton, &QPushButton::clicked, this, &MainWindow::loadPreset);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshEnvironmentStatus);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::runConfiguredTask);
    connect(advancedToggleButton_, &QToolButton::clicked, this, &MainWindow::toggleAdvancedSection);
    connect(singleModeButton_, &QRadioButton::toggled, this, &MainWindow::handleFormEdited);
    connect(directoryModeButton_, &QRadioButton::toggled, this, &MainWindow::handleFormEdited);
    connect(autoDetectModelsCheck_, &QCheckBox::toggled, this, &MainWindow::handleFormEdited);
    connect(parallelCheck_, &QCheckBox::toggled, this, &MainWindow::handleFormEdited);
    connect(workerSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::handleFormEdited);
    connect(directionCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFormEdited);
    connect(outputPolicyCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFormEdited);
    connect(inputImageEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(inputDirectoryEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(outputDirectoryEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(inputObjEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(outputObjEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(sizeEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(prefixEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);

    if (const auto restored = presetStore_.loadLastSession()) {
        formState_ = *restored;
        appendLog(QStringLiteral("已恢复上次配置。"));
    }

    applyFormToUi();
    refreshEnvironmentStatus();
}

void MainWindow::refreshEnvironmentStatus()
{
    captureFormFromUi();
    environmentStatus_ = EnvironmentCheck::inspect(applicationDir(), formState_.outputRoot);

    {
        QSignalBlocker inputObjBlocker(inputObjEdit_);
        QSignalBlocker outputObjBlocker(outputObjEdit_);
        if (formState_.autoDetectModels) {
            inputObjEdit_->setText(QString::fromStdString(environmentStatus_.detectedInputObjPath.string()));
            outputObjEdit_->setText(QString::fromStdString(environmentStatus_.detectedOutputObjPath.string()));
        } else {
            inputObjEdit_->setText(QString::fromStdString(formState_.inputObjPath.string()));
            outputObjEdit_->setText(QString::fromStdString(formState_.outputObjPath.string()));
        }
    }

    singleInputWidget_->setVisible(singleModeButton_->isChecked());
    directoryInputWidget_->setVisible(directoryModeButton_->isChecked());
    inputObjEdit_->setEnabled(!formState_.autoDetectModels);
    outputObjEdit_->setEnabled(!formState_.autoDetectModels);
    inputObjBrowseButton_->setEnabled(!formState_.autoDetectModels);
    outputObjBrowseButton_->setEnabled(!formState_.autoDetectModels);
    workerSpin_->setEnabled(parallelCheck_->isChecked());

    QStringList lines;
    for (const auto &message : environmentStatus_.messages) {
        lines << utf8Text(message);
    }
    environmentLabel_->setText(lines.join(QStringLiteral("\n")));

    const auto validation = TaskFormValidator::validate(formState_, environmentStatus_);
    validationLabel_->setText(utf8Text(validation.summaryText()));
    validationLabel_->setStyleSheet(validation.ok
        ? QStringLiteral("color: #067647;")
        : QStringLiteral("color: #b42318;"));
    runButton_->setEnabled(validation.ok);

    try {
        presetStore_.saveLastSession(formState_);
    } catch (const std::exception &error) {
        appendLog(QStringLiteral("保存上次配置失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::captureFormFromUi()
{
    formState_.inputMode = singleModeButton_->isChecked() ? InputMode::SingleImage : InputMode::Directory;
    formState_.inputImagePath = std::filesystem::path(inputImageEdit_->text().toStdString());
    formState_.inputDirectory = std::filesystem::path(inputDirectoryEdit_->text().toStdString());
    formState_.outputRoot = std::filesystem::path(outputDirectoryEdit_->text().toStdString());
    formState_.autoDetectModels = autoDetectModelsCheck_->isChecked();
    if (formState_.autoDetectModels) {
        formState_.inputObjPath.clear();
        formState_.outputObjPath.clear();
    } else {
        formState_.inputObjPath = std::filesystem::path(inputObjEdit_->text().toStdString());
        formState_.outputObjPath = std::filesystem::path(outputObjEdit_->text().toStdString());
    }
    formState_.outputSizesText = sizeEdit_->text().toStdString();
    formState_.prefixesText = prefixEdit_->text().toStdString();
    formState_.direction = directionFromIndex(directionCombo_->currentIndex());
    formState_.outputConflictPolicy = outputPolicyFromIndex(outputPolicyCombo_->currentIndex());
    formState_.enableParallel = parallelCheck_->isChecked();
    formState_.maxWorkers = static_cast<unsigned int>(workerSpin_->value());
}

void MainWindow::applyFormToUi()
{
    QSignalBlocker singleBlocker(singleModeButton_);
    QSignalBlocker directoryBlocker(directoryModeButton_);
    QSignalBlocker imageBlocker(inputImageEdit_);
    QSignalBlocker dirBlocker(inputDirectoryEdit_);
    QSignalBlocker outputBlocker(outputDirectoryEdit_);
    QSignalBlocker autoBlocker(autoDetectModelsCheck_);
    QSignalBlocker inputObjBlocker(inputObjEdit_);
    QSignalBlocker outputObjBlocker(outputObjEdit_);
    QSignalBlocker sizeBlocker(sizeEdit_);
    QSignalBlocker prefixBlocker(prefixEdit_);
    QSignalBlocker directionBlocker(directionCombo_);
    QSignalBlocker policyBlocker(outputPolicyCombo_);
    QSignalBlocker parallelBlocker(parallelCheck_);
    QSignalBlocker workerBlocker(workerSpin_);

    singleModeButton_->setChecked(formState_.inputMode == InputMode::SingleImage);
    directoryModeButton_->setChecked(formState_.inputMode == InputMode::Directory);
    inputImageEdit_->setText(QString::fromStdString(formState_.inputImagePath.string()));
    inputDirectoryEdit_->setText(QString::fromStdString(formState_.inputDirectory.string()));
    outputDirectoryEdit_->setText(QString::fromStdString(formState_.outputRoot.string()));
    autoDetectModelsCheck_->setChecked(formState_.autoDetectModels);
    inputObjEdit_->setText(QString::fromStdString(formState_.inputObjPath.string()));
    outputObjEdit_->setText(QString::fromStdString(formState_.outputObjPath.string()));
    sizeEdit_->setText(QString::fromStdString(formState_.outputSizesText));
    prefixEdit_->setText(QString::fromStdString(formState_.prefixesText));
    directionCombo_->setCurrentIndex(directionIndex(formState_.direction));
    outputPolicyCombo_->setCurrentIndex(outputPolicyIndex(formState_.outputConflictPolicy));
    parallelCheck_->setChecked(formState_.enableParallel);
    workerSpin_->setValue(static_cast<int>(formState_.maxWorkers));
}

void MainWindow::chooseInputImage()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择输入图片"), QString(), QStringLiteral("Images (*.jpg *.jpeg *.png *.tga)"));
    if (selected.isEmpty()) {
        return;
    }

    inputImageEdit_->setText(selected);
    singleModeButton_->setChecked(true);
    appendLog(QStringLiteral("已选择输入图片: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseInputDirectory()
{
    const auto selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择输入目录"));
    if (selected.isEmpty()) {
        return;
    }

    inputDirectoryEdit_->setText(selected);
    directoryModeButton_->setChecked(true);
    appendLog(QStringLiteral("已选择输入目录: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseOutputDirectory()
{
    const auto selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择输出目录"));
    if (selected.isEmpty()) {
        return;
    }

    outputDirectoryEdit_->setText(selected);
    appendLog(QStringLiteral("已选择输出目录: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseInputObjPath()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择 input.obj"), QString(), QStringLiteral("OBJ (*.obj)"));
    if (selected.isEmpty()) {
        return;
    }

    inputObjEdit_->setText(selected);
    appendLog(QStringLiteral("已设置 input.obj: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseOutputObjPath()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择 output.obj"), QString(), QStringLiteral("OBJ (*.obj)"));
    if (selected.isEmpty()) {
        return;
    }

    outputObjEdit_->setText(selected);
    appendLog(QStringLiteral("已设置 output.obj: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::savePreset()
{
    captureFormFromUi();
    bool accepted = false;
    const auto name = QInputDialog::getText(this,
                                            QStringLiteral("保存预设"),
                                            QStringLiteral("预设名称"),
                                            QLineEdit::Normal,
                                            QString(),
                                            &accepted);
    if (!accepted || name.trimmed().isEmpty()) {
        return;
    }

    try {
        presetStore_.saveNamedPreset(name.toStdString(), formState_);
        appendLog(QStringLiteral("已保存预设: %1").arg(name));
    } catch (const std::exception &error) {
        appendLog(QStringLiteral("保存预设失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::loadPreset()
{
    const auto presetNames = presetStore_.listPresetNames();
    if (presetNames.empty()) {
        QMessageBox::information(this, QStringLiteral("加载预设"), QStringLiteral("当前还没有可用预设。"));
        return;
    }

    QStringList items;
    for (const auto &name : presetNames) {
        items << QString::fromStdString(name);
    }

    bool accepted = false;
    const auto selected = QInputDialog::getItem(this,
                                                QStringLiteral("加载预设"),
                                                QStringLiteral("选择预设"),
                                                items,
                                                0,
                                                false,
                                                &accepted);
    if (!accepted || selected.isEmpty()) {
        return;
    }

    const auto reply = QMessageBox::question(this,
                                             QStringLiteral("加载预设"),
                                             QStringLiteral("加载预设会覆盖当前表单内容，是否继续？"));
    if (reply != QMessageBox::Yes) {
        return;
    }

    try {
        const auto loaded = presetStore_.loadNamedPreset(selected.toStdString());
        if (!loaded) {
            appendLog(QStringLiteral("未找到预设: %1").arg(selected));
            return;
        }

        formState_ = *loaded;
        applyFormToUi();
        appendLog(QStringLiteral("已加载预设: %1").arg(selected));
        refreshEnvironmentStatus();
    } catch (const std::exception &error) {
        appendLog(QStringLiteral("加载预设失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::toggleAdvancedSection()
{
    advancedWidget_->setVisible(advancedToggleButton_->isChecked());
}

void MainWindow::handleFormEdited()
{
    refreshEnvironmentStatus();
}

void MainWindow::runConfiguredTask()
{
    captureFormFromUi();
    refreshEnvironmentStatus();
    const auto validation = TaskFormValidator::validate(formState_, environmentStatus_);
    if (!validation.ok) {
        appendLog(QStringLiteral("当前配置未通过校验，已阻止执行。"));
        return;
    }

    try {
        appendLog(formState_.usesSingleImageInput()
            ? QStringLiteral("开始处理图片: %1").arg(utf8Text(formState_.inputImagePath.string()))
            : QStringLiteral("开始处理目录: %1").arg(utf8Text(formState_.inputDirectory.string())));
        const auto result = RunController::runTask(formState_, environmentStatus_);
        for (const auto &message : result.logs) {
            appendLog(utf8Text(message));
        }
        appendLog(QStringLiteral("成功 %1 个，失败 %2 个。").arg(result.successCount).arg(result.failedCount));
    } catch (const std::exception &error) {
        appendLog(QStringLiteral("处理失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::appendLog(const QString &message)
{
    logOutput_->appendPlainText(message);
}

std::filesystem::path MainWindow::applicationDir() const
{
    return std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
}

} // namespace disassemble::desktop
