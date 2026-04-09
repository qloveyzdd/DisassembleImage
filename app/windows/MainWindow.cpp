#include "MainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QDesktopServices>
#include <QFileDialog>
#include <QFormLayout>
#include <QGuiApplication>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <QProgressBar>
#include <QPushButton>
#include <QRadioButton>
#include <QScrollArea>
#include <QScreen>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QStringList>
#include <QToolButton>
#include <QUrl>
#include <QVBoxLayout>
#include <QWidget>

#include "RunController.h"
#include "TaskRunSession.h"
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

int backendIndex(disassemble::core::ProcessingBackend backend)
{
    switch (backend) {
    case disassemble::core::ProcessingBackend::Cpu:
        return 1;
    case disassemble::core::ProcessingBackend::Gpu:
        return 2;
    case disassemble::core::ProcessingBackend::Auto:
    default:
        return 0;
    }
}

disassemble::core::ProcessingBackend backendFromIndex(int index)
{
    switch (index) {
    case 1:
        return disassemble::core::ProcessingBackend::Cpu;
    case 2:
        return disassemble::core::ProcessingBackend::Gpu;
    default:
        return disassemble::core::ProcessingBackend::Auto;
    }
}

QString backendText(disassemble::core::ProcessingBackend backend)
{
    return utf8Text(disassemble::core::processingBackendLabel(backend));
}

QString stageText(disassemble::core::RunStage stage)
{
    switch (stage) {
    case disassemble::core::RunStage::Validating:
        return QStringLiteral("正在校验任务配置");
    case disassemble::core::RunStage::PreparingOutput:
        return QStringLiteral("正在准备输出目录");
    case disassemble::core::RunStage::CollectingInputs:
        return QStringLiteral("正在扫描输入文件");
    case disassemble::core::RunStage::Processing:
        return QStringLiteral("正在处理图片");
    case disassemble::core::RunStage::Finished:
        return QStringLiteral("处理完成");
    case disassemble::core::RunStage::Cancelled:
        return QStringLiteral("已安全取消");
    }
    return QStringLiteral("处理中");
}

QString fileNameText(const std::string &path)
{
    if (path.empty()) {
        return QStringLiteral("当前文件: 暂无");
    }
    return QStringLiteral("当前文件: %1")
        .arg(QString::fromStdString(std::filesystem::path(path).filename().string()));
}

QString displayText(const std::string &value, const QString &fallback = QStringLiteral("未提供"))
{
    return value.empty() ? fallback : utf8Text(value);
}

constexpr size_t kMaxPreview3dItems = 96;

} // namespace

namespace disassemble::desktop {

MainWindow::MainWindow()
    : presetStore_(),
      runSession_(std::make_unique<TaskRunSession>(this)),
      formSectionWidget_(new QWidget()),
      environmentLabel_(new QLabel(this)),
      gpuDiagnosticLabel_(new QLabel(this)),
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
      backendCombo_(new QComboBox(this)),
      advancedToggleButton_(new QToolButton(this)),
      advancedWidget_(new QWidget(this)),
      parallelCheck_(new QCheckBox(QStringLiteral("启用并行"), this)),
      workerSpin_(new QSpinBox(this)),
      refreshDiagnosticButton_(new QPushButton(QStringLiteral("刷新诊断"), this)),
      runButton_(new QPushButton(QStringLiteral("开始处理"), this)),
      cancelButton_(new QPushButton(QStringLiteral("安全取消"), this)),
      progressStageLabel_(new QLabel(QStringLiteral("阶段: 待开始"), this)),
      progressFileLabel_(new QLabel(QStringLiteral("当前文件: 暂无"), this)),
      progressStatsLabel_(new QLabel(QStringLiteral("成功 0，失败 0"), this)),
      progressBar_(new QProgressBar(this)),
      logOutput_(new QPlainTextEdit(this)),
      technicalLogToggleButton_(new QToolButton(this)),
      technicalLogOutput_(new QPlainTextEdit(this)),
      technicalLogWidget_(new QWidget(this)),
      resultWidget_(new QGroupBox(QStringLiteral("本次结果"), this)),
      resultSummaryLabel_(new QLabel(this)),
      resultFailuresLabel_(new QLabel(this)),
      resultOutputLabel_(new QLabel(this)),
      openOutputButton_(new QPushButton(QStringLiteral("打开结果目录"), this)),
      exportSummaryButton_(new QPushButton(QStringLiteral("导出日志摘要"), this)),
      exportZipButton_(new QPushButton(QStringLiteral("导出 zip"), this)),
      sourcePreviewPane_(new PreviewImagePane(QStringLiteral("原图"), this)),
      previewGalleryWidget_(new PreviewGalleryWidget(this)),
      outputPreviewPane_(new PreviewImagePane(QStringLiteral("切片预览"), this)),
      preview3dWidget_(new Preview3DWidget(this)),
      load3dPreviewButton_(new QPushButton(QStringLiteral("加载 3D 预览"), this)),
      reset3dViewButton_(new QPushButton(QStringLiteral("重置 3D 视角"), this))
{
    auto *scrollArea = new QScrollArea(this);
    auto *central = new QWidget(scrollArea);
    auto *mainLayout = new QVBoxLayout(central);
    auto *formLayout = new QVBoxLayout(formSectionWidget_);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(12);

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
    backendCombo_->addItems({QStringLiteral("自动（优先 GPU）"), QStringLiteral("仅 CPU"), QStringLiteral("优先 GPU")});
    workerSpin_->setMinimum(1);
    workerSpin_->setMaximum(128);
    advancedLayout->addRow(QStringLiteral("计算后端"), backendCombo_);
    advancedLayout->addRow(parallelCheck_);
    advancedLayout->addRow(QStringLiteral("并行线程数"), workerSpin_);
    advancedWidget_->setVisible(false);

    auto *actionLayout = new QHBoxLayout();
    auto *refreshButton = new QPushButton(QStringLiteral("刷新环境"), this);
    actionLayout->addWidget(refreshButton);
    actionLayout->addWidget(refreshDiagnosticButton_);
    actionLayout->addWidget(runButton_);
    actionLayout->addWidget(cancelButton_);
    actionLayout->addStretch();

    auto *diagnosticGroup = new QGroupBox(QStringLiteral("OpenCL 诊断"), this);
    auto *diagnosticLayout = new QVBoxLayout(diagnosticGroup);
    gpuDiagnosticLabel_->setWordWrap(true);
    diagnosticLayout->addWidget(gpuDiagnosticLabel_);

    auto *progressGroup = new QGroupBox(QStringLiteral("运行状态"), this);
    auto *progressLayout = new QVBoxLayout(progressGroup);
    progressBar_->setMinimum(0);
    progressBar_->setMaximum(1);
    progressBar_->setValue(0);
    progressLayout->addWidget(progressStageLabel_);
    progressLayout->addWidget(progressFileLabel_);
    progressLayout->addWidget(progressStatsLabel_);
    progressLayout->addWidget(progressBar_);

    logOutput_->setReadOnly(true);
    logOutput_->setPlaceholderText(QStringLiteral("这里显示面向普通同事的摘要日志。"));
    logOutput_->setMinimumHeight(120);
    technicalLogToggleButton_->setText(QStringLiteral("技术日志"));
    technicalLogToggleButton_->setCheckable(true);
    technicalLogOutput_->setReadOnly(true);
    technicalLogOutput_->setMinimumHeight(140);
    technicalLogWidget_->setVisible(false);
    auto *technicalLayout = new QVBoxLayout(technicalLogWidget_);
    technicalLayout->setContentsMargins(0, 0, 0, 0);
    technicalLayout->addWidget(technicalLogOutput_);

    resultSummaryLabel_->setWordWrap(true);
    resultFailuresLabel_->setWordWrap(true);
    resultOutputLabel_->setWordWrap(true);
    auto *resultLayout = new QVBoxLayout(resultWidget_);
    auto *resultButtonsLayout = new QHBoxLayout();
    auto *previewLayout = new QHBoxLayout();
    auto *galleryPane = new QWidget(this);
    auto *galleryLayout = new QVBoxLayout(galleryPane);
    auto *galleryTitle = new QLabel(QStringLiteral("切片缩略图"), this);
    auto *preview3dPane = new QWidget(this);
    auto *preview3dLayout = new QVBoxLayout(preview3dPane);
    auto *preview3dButtonsLayout = new QHBoxLayout();
    auto *preview3dTitle = new QLabel(QStringLiteral("3D 对照"), this);
    resultButtonsLayout->addWidget(openOutputButton_);
    resultButtonsLayout->addWidget(exportSummaryButton_);
    resultButtonsLayout->addWidget(exportZipButton_);
    resultButtonsLayout->addStretch();
    galleryLayout->setContentsMargins(0, 0, 0, 0);
    galleryLayout->addWidget(galleryTitle);
    galleryLayout->addWidget(previewGalleryWidget_);
    galleryLayout->addWidget(outputPreviewPane_);
    preview3dLayout->setContentsMargins(0, 0, 0, 0);
    preview3dLayout->addWidget(preview3dTitle);
    preview3dButtonsLayout->addWidget(load3dPreviewButton_);
    preview3dButtonsLayout->addWidget(reset3dViewButton_);
    preview3dButtonsLayout->addStretch();
    preview3dLayout->addLayout(preview3dButtonsLayout);
    preview3dLayout->addWidget(preview3dWidget_, 1);
    previewLayout->addWidget(sourcePreviewPane_, 1);
    previewLayout->addWidget(galleryPane, 1);
    previewLayout->addWidget(preview3dPane, 1);
    resultLayout->addWidget(resultSummaryLabel_);
    resultLayout->addWidget(resultFailuresLabel_);
    resultLayout->addWidget(resultOutputLabel_);
    resultLayout->addLayout(resultButtonsLayout);
    resultLayout->addLayout(previewLayout, 1);
    resultWidget_->setVisible(false);

    environmentLabel_->setWordWrap(true);
    gpuDiagnosticLabel_->setWordWrap(true);
    validationLabel_->setWordWrap(true);
    validationLabel_->setStyleSheet(QStringLiteral("color: #b42318;"));

    formLayout->addLayout(presetLayout);
    formLayout->addWidget(environmentLabel_);
    formLayout->addWidget(diagnosticGroup);
    formLayout->addWidget(validationLabel_);
    formLayout->addWidget(inputModeGroup);
    formLayout->addWidget(resourceGroup);
    formLayout->addWidget(parameterGroup);
    formLayout->addWidget(advancedToggleButton_);
    formLayout->addWidget(advancedWidget_);

    mainLayout->addWidget(formSectionWidget_);
    mainLayout->addLayout(actionLayout);
    mainLayout->addWidget(progressGroup);
    mainLayout->addWidget(logOutput_);
    mainLayout->addWidget(technicalLogToggleButton_);
    mainLayout->addWidget(technicalLogWidget_);
    mainLayout->addWidget(resultWidget_);

    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(central);

    setCentralWidget(scrollArea);
    setWindowTitle(QStringLiteral("DisassembleImage"));
    setMinimumSize(720, 520);

    QSize initialSize(1180, 840);
    if (const auto *screen = QGuiApplication::primaryScreen()) {
        const QSize availableSize = screen->availableGeometry().size() - QSize(48, 48);
        if (availableSize.width() > 0 && availableSize.height() > 0) {
            initialSize = initialSize.boundedTo(availableSize);
        }
    }
    resize(initialSize);

    connect(singleBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseInputImage);
    connect(directoryBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseInputDirectory);
    connect(outputBrowseButton, &QPushButton::clicked, this, &MainWindow::chooseOutputDirectory);
    connect(inputObjBrowseButton_, &QPushButton::clicked, this, &MainWindow::chooseInputObjPath);
    connect(outputObjBrowseButton_, &QPushButton::clicked, this, &MainWindow::chooseOutputObjPath);
    connect(savePresetButton, &QPushButton::clicked, this, &MainWindow::savePreset);
    connect(loadPresetButton, &QPushButton::clicked, this, &MainWindow::loadPreset);
    connect(refreshButton, &QPushButton::clicked, this, &MainWindow::refreshEnvironmentStatus);
    connect(refreshDiagnosticButton_, &QPushButton::clicked, this, &MainWindow::refreshEnvironmentStatus);
    connect(runButton_, &QPushButton::clicked, this, &MainWindow::runConfiguredTask);
    connect(cancelButton_, &QPushButton::clicked, this, &MainWindow::requestCancel);
    connect(advancedToggleButton_, &QToolButton::clicked, this, &MainWindow::toggleAdvancedSection);
    connect(technicalLogToggleButton_, &QToolButton::clicked, this, &MainWindow::toggleTechnicalLog);
    connect(openOutputButton_, &QPushButton::clicked, this, &MainWindow::openResultDirectory);
    connect(exportSummaryButton_, &QPushButton::clicked, this, &MainWindow::exportSummaryLog);
    connect(exportZipButton_, &QPushButton::clicked, this, &MainWindow::exportResultBundle);
    connect(previewGalleryWidget_, &PreviewGalleryWidget::currentItemChanged, this, &MainWindow::handlePreviewSelectionChanged);
    connect(load3dPreviewButton_, &QPushButton::clicked, this, &MainWindow::loadSelectedPreview3d);
    connect(reset3dViewButton_, &QPushButton::clicked, preview3dWidget_, &Preview3DWidget::resetCamera);
    connect(singleModeButton_, &QRadioButton::toggled, this, &MainWindow::handleFormEdited);
    connect(directoryModeButton_, &QRadioButton::toggled, this, &MainWindow::handleFormEdited);
    connect(autoDetectModelsCheck_, &QCheckBox::toggled, this, &MainWindow::handleFormEdited);
    connect(parallelCheck_, &QCheckBox::toggled, this, &MainWindow::handleFormEdited);
    connect(workerSpin_, qOverload<int>(&QSpinBox::valueChanged), this, &MainWindow::handleFormEdited);
    connect(directionCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFormEdited);
    connect(outputPolicyCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFormEdited);
    connect(backendCombo_, qOverload<int>(&QComboBox::currentIndexChanged), this, &MainWindow::handleFormEdited);
    connect(inputImageEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(inputDirectoryEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(outputDirectoryEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(inputObjEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(outputObjEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(sizeEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(prefixEdit_, &QLineEdit::editingFinished, this, &MainWindow::handleFormEdited);
    connect(runSession_.get(), &TaskRunSession::progressChanged, this, [this](const disassemble::core::RunProgress &progress) {
        handleRunProgress(progress);
    });
    connect(runSession_.get(), &TaskRunSession::finished, this, [this](const disassemble::core::RunResult &result) {
        handleRunFinished(result);
    });
    connect(runSession_.get(), &TaskRunSession::failed, this, [this](const QString &message) {
        handleRunFailed(message);
    });

    applyRunState(false);
    resetRunFeedback();
    if (const auto restored = presetStore_.loadLastSession()) {
        formState_ = *restored;
        appendSummaryLog(QStringLiteral("已恢复上次配置。"));
    }
    applyFormToUi();
    refreshEnvironmentStatus();
}

void MainWindow::refreshEnvironmentStatus()
{
    captureFormFromUi();
    environmentStatus_ = EnvironmentCheck::inspect(applicationDir(), formState_.outputRoot);
    backendInfo_ = RunController::probeGpuBackend();

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
    updateGpuDiagnostic();

    const auto validation = TaskFormValidator::validate(formState_, environmentStatus_);
    validationLabel_->setText(utf8Text(validation.summaryText()));
    validationLabel_->setStyleSheet(validation.ok
        ? QStringLiteral("color: #067647;")
        : QStringLiteral("color: #b42318;"));
    runButton_->setEnabled(validation.ok && !taskRunning_);

    try {
        presetStore_.saveLastSession(formState_);
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("保存上次配置失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::updateGpuDiagnostic()
{
    QStringList lines;
    lines << QStringLiteral("加载器状态: %1").arg(displayText(backendInfo_.loaderStatus));
    lines << QStringLiteral("OpenCL 支持: %1").arg(backendInfo_.openClSupported ? QStringLiteral("是") : QStringLiteral("否"));
    lines << QStringLiteral("平台枚举: %1").arg(backendInfo_.platformEnumerationSucceeded ? QStringLiteral("成功") : QStringLiteral("失败"));
    lines << QStringLiteral("平台数量: %1").arg(backendInfo_.platformCount);
    lines << QStringLiteral("可用 GPU 设备: %1").arg(backendInfo_.gpuDeviceAvailable ? QStringLiteral("是") : QStringLiteral("否"));
    lines << QStringLiteral("GPU 设备数量: %1").arg(backendInfo_.gpuDeviceCount);
    lines << QStringLiteral("当前请求后端: %1").arg(backendText(formState_.processingBackend));
    lines << QStringLiteral("上次实际后端: %1").arg(lastRunResult_
            ? backendText(lastRunResult_->activeBackend)
            : QStringLiteral("尚未运行"));
    lines << QStringLiteral("失败阶段: %1").arg(displayText(backendInfo_.failureStage, QStringLiteral("未诊断")));
    lines << QStringLiteral("平台摘要:\n%1").arg(displayText(backendInfo_.platformSummary));
    lines << QStringLiteral("设备枚举:\n%1").arg(displayText(backendInfo_.deviceEnumerationStatus));
    lines << QStringLiteral("上下文状态: %1").arg(displayText(backendInfo_.contextStatus));
    lines << QStringLiteral("选中平台: %1").arg(displayText(backendInfo_.selectedPlatformName));
    lines << QStringLiteral("设备名称: %1").arg(displayText(backendInfo_.deviceName));
    lines << QStringLiteral("设备厂商: %1").arg(displayText(backendInfo_.vendorName));
    lines << QStringLiteral("驱动版本: %1").arg(displayText(backendInfo_.driverVersion));
    lines << QStringLiteral("OpenCL C 版本: %1").arg(displayText(backendInfo_.openClVersion));

    if (backendInfo_.canUseGpu()) {
        lines << QStringLiteral("诊断结论: 当前环境可启用 GPU。");
    } else {
        lines << QStringLiteral("诊断结论: %1").arg(displayText(backendInfo_.unavailableReason, QStringLiteral("当前环境无法启用 GPU。")));
    }

    if (lastRunResult_ && !lastRunResult_->fallbackReason.empty()) {
        lines << QStringLiteral("上次回退原因: %1").arg(utf8Text(lastRunResult_->fallbackReason));
    }
    lines << QStringLiteral("建议动作: %1").arg(displayText(backendInfo_.recommendation));

    gpuDiagnosticLabel_->setText(lines.join(QStringLiteral("\n")));
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
    formState_.processingBackend = backendFromIndex(backendCombo_->currentIndex());
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
    QSignalBlocker backendBlocker(backendCombo_);
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
    backendCombo_->setCurrentIndex(backendIndex(formState_.processingBackend));
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
    appendSummaryLog(QStringLiteral("已选择输入图片: %1").arg(selected));
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
    appendSummaryLog(QStringLiteral("已选择输入目录: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseOutputDirectory()
{
    const auto selected = QFileDialog::getExistingDirectory(this, QStringLiteral("选择输出目录"));
    if (selected.isEmpty()) {
        return;
    }

    outputDirectoryEdit_->setText(selected);
    appendSummaryLog(QStringLiteral("已选择输出目录: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseInputObjPath()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择 input.obj"), QString(), QStringLiteral("OBJ (*.obj)"));
    if (selected.isEmpty()) {
        return;
    }

    inputObjEdit_->setText(selected);
    appendSummaryLog(QStringLiteral("已设置 input.obj: %1").arg(selected));
    refreshEnvironmentStatus();
}

void MainWindow::chooseOutputObjPath()
{
    const auto selected = QFileDialog::getOpenFileName(this, QStringLiteral("选择 output.obj"), QString(), QStringLiteral("OBJ (*.obj)"));
    if (selected.isEmpty()) {
        return;
    }

    outputObjEdit_->setText(selected);
    appendSummaryLog(QStringLiteral("已设置 output.obj: %1").arg(selected));
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
        appendSummaryLog(QStringLiteral("已保存预设: %1").arg(name));
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("保存预设失败: %1").arg(utf8Text(error.what())));
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
            appendSummaryLog(QStringLiteral("未找到预设: %1").arg(selected));
            return;
        }

        formState_ = *loaded;
        applyFormToUi();
        appendSummaryLog(QStringLiteral("已加载预设: %1").arg(selected));
        refreshEnvironmentStatus();
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("加载预设失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::toggleAdvancedSection()
{
    advancedWidget_->setVisible(advancedToggleButton_->isChecked());
}

void MainWindow::handleFormEdited()
{
    if (taskRunning_) {
        return;
    }
    refreshEnvironmentStatus();
}

void MainWindow::runConfiguredTask()
{
    captureFormFromUi();
    refreshEnvironmentStatus();
    const auto validation = TaskFormValidator::validate(formState_, environmentStatus_);
    if (!validation.ok) {
        appendSummaryLog(QStringLiteral("当前配置未通过校验，已阻止执行。"));
        return;
    }

    try {
        const auto task = RunController::buildTask(formState_, environmentStatus_);
        lastRunTask_ = task;
        resetRunFeedback();
        applyRunState(true);
        appendSummaryLog(formState_.usesSingleImageInput()
            ? QStringLiteral("开始处理图片: %1").arg(utf8Text(formState_.inputImagePath.string()))
            : QStringLiteral("开始处理目录: %1").arg(utf8Text(formState_.inputDirectory.string())));
        runSession_->start(task);
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("处理失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::requestCancel()
{
    if (!taskRunning_ || !runSession_) {
        return;
    }
    runSession_->requestCancel();
    cancelButton_->setEnabled(false);
    cancelButton_->setText(QStringLiteral("正在取消..."));
    appendSummaryLog(QStringLiteral("已请求安全取消，程序会在当前已开始的文件处理完成后停止。"));
}

void MainWindow::toggleTechnicalLog()
{
    technicalLogWidget_->setVisible(technicalLogToggleButton_->isChecked());
}

void MainWindow::openResultDirectory()
{
    if (!lastRunResult_ || lastRunResult_->outputRoot.empty()) {
        appendSummaryLog(QStringLiteral("当前没有可打开的结果目录。"));
        return;
    }

    QDesktopServices::openUrl(QUrl::fromLocalFile(QString::fromStdString(lastRunResult_->outputRoot)));
}

void MainWindow::exportSummaryLog()
{
    if (!lastRunResult_) {
        appendSummaryLog(QStringLiteral("当前没有可导出的结果。"));
        return;
    }

    try {
        const auto summaryPath = RunController::exportLogSummary(*lastRunResult_, summaryLogLines_);
        appendSummaryLog(QStringLiteral("已导出日志摘要: %1").arg(QString::fromStdString(summaryPath.string())));
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("导出日志摘要失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::exportResultBundle()
{
    if (!lastRunResult_) {
        appendSummaryLog(QStringLiteral("当前没有可导出的结果。"));
        return;
    }

    try {
        const auto bundlePath = RunController::exportResultBundle(*lastRunResult_, summaryLogLines_);
        appendSummaryLog(QStringLiteral("已导出 zip: %1").arg(QString::fromStdString(bundlePath.string())));
    } catch (const std::exception &error) {
        appendSummaryLog(QStringLiteral("导出 zip 失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::handleRunProgress(const disassemble::core::RunProgress &progress)
{
    progressStageLabel_->setText(QStringLiteral("阶段: %1").arg(stageText(progress.stage)));
    progressFileLabel_->setText(fileNameText(progress.currentInputPath));
    progressStatsLabel_->setText(QStringLiteral("成功 %1，失败 %2，后端 %3，耗时 %4 ms")
                                     .arg(progress.successCount)
                                     .arg(progress.failedCount)
                                     .arg(backendText(progress.activeBackend))
                                     .arg(progress.elapsedMs, 0, 'f', 0));
    const int progressMaximum = progress.totalInputs > 0 ? progress.totalInputs : 1;
    const int progressValue = progress.completedInputs < progress.totalInputs
        ? progress.completedInputs
        : progress.totalInputs;
    progressBar_->setMaximum(progressMaximum);
    progressBar_->setValue(progressValue);

    if (progress.stage != lastLoggedStage_) {
        appendSummaryLog(QStringLiteral("状态更新: %1").arg(stageText(progress.stage)));
        lastLoggedStage_ = progress.stage;
    }
    if (!progress.fallbackReason.empty()) {
        const auto fallbackText = utf8Text(progress.fallbackReason);
        if (lastFallbackReason_ != fallbackText) {
            appendSummaryLog(fallbackText);
            lastFallbackReason_ = fallbackText;
        }
    }
}

void MainWindow::handleRunFinished(const disassemble::core::RunResult &result)
{
    applyRunState(false);
    lastRunResult_ = result;
    updateGpuDiagnostic();
    for (const auto &message : result.logs) {
        appendTechnicalLog(utf8Text(message));
    }

    if (result.cancelled) {
        appendSummaryLog(QStringLiteral("任务已安全取消，已生成的结果会继续保留。"));
    } else {
        appendSummaryLog(QStringLiteral("任务完成，成功 %1 个，失败 %2 个。").arg(result.successCount).arg(result.failedCount));
    }
    appendSummaryLog(QStringLiteral("本次实际后端: %1").arg(backendText(result.activeBackend)));
    if (!result.fallbackReason.empty()) {
        const auto fallbackText = utf8Text(result.fallbackReason);
        if (lastFallbackReason_ != fallbackText) {
            appendSummaryLog(fallbackText);
            lastFallbackReason_ = fallbackText;
        }
    }

    renderResult(result);
}

void MainWindow::handleRunFailed(const QString &message)
{
    applyRunState(false);
    updateGpuDiagnostic();
    appendSummaryLog(QStringLiteral("处理失败: %1").arg(message));
}

void MainWindow::applyRunState(bool running)
{
    taskRunning_ = running;
    formSectionWidget_->setEnabled(!running);
    if (running) {
        runButton_->setEnabled(false);
    } else {
        const auto validation = TaskFormValidator::validate(formState_, environmentStatus_);
        runButton_->setEnabled(validation.ok);
    }
    cancelButton_->setEnabled(running);
    cancelButton_->setText(QStringLiteral("安全取消"));
}

void MainWindow::resetRunFeedback()
{
    lastRunResult_.reset();
    previewItems_.clear();
    summaryLogLines_.clear();
    lastLoggedStage_ = disassemble::core::RunStage::Validating;
    lastFallbackReason_.clear();
    logOutput_->clear();
    technicalLogOutput_->clear();
    progressStageLabel_->setText(QStringLiteral("阶段: 待开始"));
    progressFileLabel_->setText(QStringLiteral("当前文件: 暂无"));
    progressStatsLabel_->setText(QStringLiteral("成功 0，失败 0"));
    progressBar_->setMaximum(1);
    progressBar_->setValue(0);
    previewGalleryWidget_->setItems({});
    sourcePreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的原图。"));
    outputPreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的切片。"));
    preview3dWidget_->setMesh({});
    load3dPreviewButton_->setEnabled(false);
    reset3dViewButton_->setEnabled(false);
    resultWidget_->setVisible(false);
    updateGpuDiagnostic();
}

void MainWindow::renderResult(const disassemble::core::RunResult &result)
{
    previewItems_ = disassemble::core::buildPreviewGalleryItems(result);
    for (auto &item : previewItems_) {
        item.selected = false;
    }

    QStringList summaryLines;
    summaryLines << (result.cancelled
            ? QStringLiteral("本次任务已安全取消。成功 %1 个，失败 %2 个。").arg(result.successCount).arg(result.failedCount)
            : QStringLiteral("本次任务已完成。成功 %1 个，失败 %2 个。").arg(result.successCount).arg(result.failedCount));
    summaryLines << QStringLiteral("请求后端: %1").arg(backendText(result.requestedBackend));
    summaryLines << QStringLiteral("实际后端: %1").arg(backendText(result.activeBackend));
    summaryLines << QStringLiteral("总耗时: %1 ms").arg(result.totalProcessingMs, 0, 'f', 0);
    summaryLines << QStringLiteral("GPU 热点耗时: %1 ms").arg(result.gpuHotPathMs, 0, 'f', 0);
    summaryLines << QStringLiteral("一致性摘要: %1").arg(utf8Text(result.consistencySummary));
    if (!result.acceleratorName.empty()) {
        summaryLines << QStringLiteral("GPU 设备: %1").arg(utf8Text(result.acceleratorName));
    }
    if (!result.fallbackReason.empty()) {
        summaryLines << QStringLiteral("回退说明: %1").arg(utf8Text(result.fallbackReason));
    }
    summaryLines << QStringLiteral("预览方式: 点击缩略图后再加载图片，3D 预览需手动点击");
    if (result.outputFiles.size() > previewItems_.size()) {
        summaryLines << QStringLiteral("缩略图样本: %1 / %2")
                            .arg(static_cast<qulonglong>(previewItems_.size()))
                            .arg(static_cast<qulonglong>(result.outputFiles.size()));
    }
    resultSummaryLabel_->setText(summaryLines.join(QStringLiteral("\n")));

    if (result.failures.empty()) {
        resultFailuresLabel_->setText(QStringLiteral("失败摘要: 无"));
    } else {
        QStringList failureLines;
        for (const auto &failure : result.failures) {
            failureLines << QStringLiteral("%1: %2")
                .arg(QString::fromStdString(std::filesystem::path(failure.inputPath).filename().string()))
                .arg(utf8Text(failure.reason));
        }
        resultFailuresLabel_->setText(QStringLiteral("失败摘要:\n%1").arg(failureLines.join(QStringLiteral("\n"))));
    }

    resultOutputLabel_->setText(QStringLiteral("结果目录: %1").arg(QString::fromStdString(result.outputRoot)));
    previewGalleryWidget_->setItems(previewItems_);
    if (previewItems_.empty()) {
        sourcePreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的原图。"));
        outputPreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的切片。"));
        preview3dWidget_->setMesh({});
        load3dPreviewButton_->setEnabled(false);
        reset3dViewButton_->setEnabled(false);
    } else {
        sourcePreviewPane_->clearWithMessage(QStringLiteral("点击缩略图后加载原图。"));
        outputPreviewPane_->clearWithMessage(QStringLiteral("点击缩略图后加载切片。"));
        preview3dWidget_->setMesh({});
        load3dPreviewButton_->setEnabled(false);
        reset3dViewButton_->setEnabled(false);
        if (result.outputFiles.size() > previewItems_.size()) {
            appendSummaryLog(QStringLiteral("结果较多，为避免卡顿，仅保留 %1 项缩略图样本；图片和 3D 预览改为按需加载。")
                                 .arg(static_cast<qulonglong>(previewItems_.size())));
        } else {
            appendSummaryLog(QStringLiteral("结果已生成；点击缩略图后加载图片，3D 预览需手动点击。"));
        }
    }
    resultWidget_->setVisible(true);
}

void MainWindow::handlePreviewSelectionChanged()
{
    const auto currentItem = previewGalleryWidget_->currentItem();
    if (!currentItem || !currentItem->isValid()) {
        sourcePreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的原图。"));
        outputPreviewPane_->clearWithMessage(QStringLiteral("当前没有可预览的切片。"));
        preview3dWidget_->setMesh({});
        load3dPreviewButton_->setEnabled(false);
        reset3dViewButton_->setEnabled(false);
        return;
    }

    sourcePreviewPane_->setImagePath(currentItem->inputImagePath);
    outputPreviewPane_->setImagePath(currentItem->outputImagePath);
    preview3dWidget_->setMesh({});
    reset3dViewButton_->setEnabled(false);

    if (!lastRunTask_) {
        load3dPreviewButton_->setEnabled(false);
        preview3dWidget_->setMesh({});
        return;
    }

    load3dPreviewButton_->setEnabled(true);
}

void MainWindow::loadSelectedPreview3d()
{
    const auto currentItem = previewGalleryWidget_->currentItem();
    if (!currentItem || !currentItem->isValid() || !lastRunTask_) {
        load3dPreviewButton_->setEnabled(false);
        reset3dViewButton_->setEnabled(false);
        preview3dWidget_->setMesh({});
        return;
    }

    try {
        auto itemsForInput = disassemble::core::filterPreviewGalleryItemsByInput(previewGalleryWidget_->items(),
                                                                                 currentItem->inputImagePath);
        if (!lastRunTask_->usesGroupedOutput() && itemsForInput.size() > kMaxPreview3dItems) {
            itemsForInput.resize(kMaxPreview3dItems);
            appendSummaryLog(QStringLiteral("当前输入对应切片较多，3D 预览仅加载前 %1 张，避免界面卡顿。")
                                 .arg(static_cast<int>(kMaxPreview3dItems)));
        }

        const auto mesh = disassemble::core::buildPreviewMesh(*lastRunTask_, itemsForInput);
        preview3dWidget_->setMesh(mesh);
        reset3dViewButton_->setEnabled(!mesh.empty());
    } catch (const std::exception &error) {
        preview3dWidget_->setMesh({});
        reset3dViewButton_->setEnabled(false);
        appendTechnicalLog(QStringLiteral("3D 预览构建失败: %1").arg(utf8Text(error.what())));
    }
}

void MainWindow::appendSummaryLog(const QString &message)
{
    logOutput_->appendPlainText(message);
    summaryLogLines_.push_back(message.toUtf8().toStdString());
}

void MainWindow::appendTechnicalLog(const QString &message)
{
    technicalLogOutput_->appendPlainText(message);
}

std::filesystem::path MainWindow::applicationDir() const
{
    return std::filesystem::path(QCoreApplication::applicationDirPath().toStdString());
}

} // namespace disassemble::desktop
