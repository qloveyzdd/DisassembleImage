#pragma once

#include <QMainWindow>
#include <QString>

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "EnvironmentCheck.h"
#include "TaskFormState.h"
#include "TaskPresetStore.h"
#include "TaskRunSession.h"
#include "../../core/model/RunProgress.h"
#include "../../core/model/RunResult.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
class QProgressBar;
class QPushButton;
class QRadioButton;
class QSpinBox;
class QToolButton;
class QWidget;

namespace disassemble::desktop {

class MainWindow : public QMainWindow
{
public:
    MainWindow();

private:
    void refreshEnvironmentStatus();
    void captureFormFromUi();
    void applyFormToUi();
    void chooseInputImage();
    void chooseInputDirectory();
    void chooseOutputDirectory();
    void chooseInputObjPath();
    void chooseOutputObjPath();
    void savePreset();
    void loadPreset();
    void toggleAdvancedSection();
    void handleFormEdited();
    void runConfiguredTask();
    void requestCancel();
    void toggleTechnicalLog();
    void openResultDirectory();
    void exportSummaryLog();
    void exportResultBundle();
    void handleRunProgress(const disassemble::core::RunProgress &progress);
    void handleRunFinished(const disassemble::core::RunResult &result);
    void handleRunFailed(const QString &message);
    void applyRunState(bool running);
    void resetRunFeedback();
    void renderResult(const disassemble::core::RunResult &result);
    void appendSummaryLog(const QString &message);
    void appendTechnicalLog(const QString &message);
    std::filesystem::path applicationDir() const;

    TaskFormState formState_;
    TaskPresetStore presetStore_;
    EnvironmentStatus environmentStatus_;
    std::unique_ptr<TaskRunSession> runSession_;
    std::optional<disassemble::core::RunResult> lastRunResult_;
    std::vector<std::string> summaryLogLines_;
    disassemble::core::RunStage lastLoggedStage_ = disassemble::core::RunStage::Validating;
    bool taskRunning_ = false;

    QWidget *formSectionWidget_;
    QLabel *environmentLabel_;
    QLabel *validationLabel_;
    QRadioButton *singleModeButton_;
    QRadioButton *directoryModeButton_;
    QWidget *singleInputWidget_;
    QWidget *directoryInputWidget_;
    QLineEdit *inputImageEdit_;
    QLineEdit *inputDirectoryEdit_;
    QLineEdit *outputDirectoryEdit_;
    QCheckBox *autoDetectModelsCheck_;
    QLineEdit *inputObjEdit_;
    QLineEdit *outputObjEdit_;
    QPushButton *inputObjBrowseButton_;
    QPushButton *outputObjBrowseButton_;
    QLineEdit *sizeEdit_;
    QLineEdit *prefixEdit_;
    QComboBox *directionCombo_;
    QComboBox *outputPolicyCombo_;
    QToolButton *advancedToggleButton_;
    QWidget *advancedWidget_;
    QCheckBox *parallelCheck_;
    QSpinBox *workerSpin_;
    QPushButton *runButton_;
    QPushButton *cancelButton_;
    QLabel *progressStageLabel_;
    QLabel *progressFileLabel_;
    QLabel *progressStatsLabel_;
    QProgressBar *progressBar_;
    QPlainTextEdit *logOutput_;
    QToolButton *technicalLogToggleButton_;
    QPlainTextEdit *technicalLogOutput_;
    QWidget *technicalLogWidget_;
    QWidget *resultWidget_;
    QLabel *resultSummaryLabel_;
    QLabel *resultFailuresLabel_;
    QLabel *resultOutputLabel_;
    QPushButton *openOutputButton_;
    QPushButton *exportSummaryButton_;
    QPushButton *exportZipButton_;
};

} // namespace disassemble::desktop
