#include <filesystem>
#include <iostream>

#include <QCoreApplication>

#include "../../app/windows/EnvironmentCheck.h"
#include "../../app/windows/RunController.h"
#include "../../app/windows/TaskFormState.h"
#include "../../app/windows/TaskFormValidator.h"
#include "../../app/windows/TaskPresetStore.h"

namespace fs = std::filesystem;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    const fs::path sourceRoot(DISASSEMBLEIMAGE_SOURCE_ROOT);
    const fs::path tempRoot = sourceRoot / "build" / "phase2-smoke-output";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);

    disassemble::desktop::TaskFormState state;
    state.inputMode = disassemble::desktop::InputMode::SingleImage;
    state.inputImagePath = sourceRoot / "HD" / "a0000.jpg";
    state.outputRoot = tempRoot;
    state.autoDetectModels = true;
    state.direction = disassemble::core::ProcessingDirection::X;
    state.outputSizesText = "6144*6720";
    state.prefixesText = "radian";
    state.outputConflictPolicy = disassemble::core::OutputConflictPolicy::AutoRename;

    const auto environment = disassemble::desktop::EnvironmentCheck::inspect(sourceRoot, state.outputRoot);
    const auto validation = disassemble::desktop::TaskFormValidator::validate(state, environment);
    if (!validation.ok) {
        std::cerr << "合法配置未通过校验" << std::endl;
        std::cerr << validation.summaryText() << std::endl;
        return 1;
    }

    disassemble::desktop::TaskPresetStore presetStore(tempRoot / "preset-store");
    presetStore.saveLastSession(state);
    const auto restored = presetStore.loadLastSession();
    if (!restored || restored->prefixesText != state.prefixesText || restored->outputSizesText != state.outputSizesText) {
        std::cerr << "上次配置恢复失败" << std::endl;
        return 2;
    }

    presetStore.saveNamedPreset("phase2-smoke", state);
    const auto preset = presetStore.loadNamedPreset("phase2-smoke");
    if (!preset || preset->inputMode != state.inputMode) {
        std::cerr << "命名预设加载失败" << std::endl;
        return 3;
    }

    disassemble::desktop::TaskFormState invalid = state;
    invalid.outputSizesText = "bad-size";
    const auto invalidValidation = disassemble::desktop::TaskFormValidator::validate(invalid, environment);
    if (invalidValidation.ok) {
        std::cerr << "非法配置没有被拦住" << std::endl;
        return 4;
    }

    const auto result = disassemble::desktop::RunController::runTask(state, environment);
    if (!result.ok() || result.outputFiles.empty()) {
        std::cerr << "Phase 2 smoke 未生成输出" << std::endl;
        return 5;
    }

    for (const auto &output : result.outputFiles) {
        if (!fs::exists(output)) {
            std::cerr << "输出文件不存在: " << output << std::endl;
            return 6;
        }
    }

    return 0;
}
