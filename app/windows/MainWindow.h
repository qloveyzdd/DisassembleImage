#pragma once

#include <QMainWindow>
#include <QString>

#include <filesystem>

#include "EnvironmentCheck.h"
#include "TaskFormState.h"
#include "TaskPresetStore.h"

class QCheckBox;
class QComboBox;
class QLabel;
class QLineEdit;
class QPlainTextEdit;
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
    void appendLog(const QString &message);
    std::filesystem::path applicationDir() const;

    TaskFormState formState_;
    TaskPresetStore presetStore_;
    EnvironmentStatus environmentStatus_;

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
    QPlainTextEdit *logOutput_;
};
} // namespace disassemble::desktop
