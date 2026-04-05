#include "TaskFormValidator.h"

#include <filesystem>
#include <sstream>

#include "../../core/io/ImageCatalog.h"

namespace fs = std::filesystem;

namespace {

std::vector<fs::path> previewOutputs(const disassemble::desktop::TaskFormState &state)
{
    disassemble::core::ProcessingTask inputOnlyTask;
    if (state.usesSingleImageInput()) {
        inputOnlyTask.inputImagePath = state.inputImagePath.string();
    } else {
        inputOnlyTask.inputDirectory = state.inputDirectory.string();
    }

    const auto inputs = disassemble::core::ImageCatalog::collect(inputOnlyTask);
    const auto prefixes = disassemble::desktop::TaskFormState::parsePrefixes(state.prefixesText);

    std::vector<fs::path> outputs;
    for (const auto &input : inputs) {
        if (state.usesGroupedOutput()) {
            outputs.push_back(state.outputRoot / prefixes.front() / (prefixes.front() + input.filename().string()));
            continue;
        }

        for (const auto &prefix : prefixes) {
            outputs.push_back(state.outputRoot / prefix / (prefix + input.filename().string()));
        }
    }
    return outputs;
}

} // namespace

namespace disassemble::desktop {

std::string ValidationResult::summaryText() const
{
    std::ostringstream stream;
    if (ok) {
        stream << "当前配置可运行。";
    }

    for (size_t index = 0; index < errors.size(); ++index) {
        if (stream.tellp() > 0) {
            stream << "\n";
        }
        stream << "错误: " << errors[index];
    }
    for (size_t index = 0; index < warnings.size(); ++index) {
        if (stream.tellp() > 0) {
            stream << "\n";
        }
        stream << "提示: " << warnings[index];
    }
    return stream.str();
}

ValidationResult TaskFormValidator::validate(const TaskFormState &state, const EnvironmentStatus &environment)
{
    ValidationResult result;

    if (state.usesSingleImageInput()) {
        if (state.inputImagePath.empty()) {
            result.errors.push_back("请选择输入图片。");
        } else if (!fs::exists(state.inputImagePath)) {
            result.errors.push_back("输入图片不存在。");
        }
    } else {
        if (state.inputDirectory.empty()) {
            result.errors.push_back("请选择输入目录。");
        } else if (!fs::exists(state.inputDirectory) || !fs::is_directory(state.inputDirectory)) {
            result.errors.push_back("输入目录不存在。");
        }
    }

    if (state.outputRoot.empty()) {
        result.errors.push_back("请选择输出目录。");
    } else if (!environment.outputDirectoryWritable) {
        result.errors.push_back("输出目录不可写。");
    }

    const auto inputObjPath = state.resolvedInputObjPath(environment.detectedInputObjPath);
    if (inputObjPath.empty() || !fs::exists(inputObjPath)) {
        result.errors.push_back("input.obj 不存在，请检查模型资源。");
    }

    if (state.usesGroupedOutput()) {
        const auto outputObjPath = state.resolvedOutputObjPath(environment.detectedOutputObjPath);
        if (outputObjPath.empty() || !fs::exists(outputObjPath)) {
            result.errors.push_back("当前拼接模式需要 output.obj。");
        }
    }

    std::vector<disassemble::core::ImageSize> sizes;
    std::vector<std::string> prefixes;

    try {
        sizes = TaskFormState::parseOutputSizes(state.outputSizesText);
        if (sizes.empty()) {
            result.errors.push_back("请至少填写一个输出尺寸。");
        }
    } catch (const std::exception &error) {
        result.errors.push_back(error.what());
    }

    try {
        prefixes = TaskFormState::parsePrefixes(state.prefixesText);
        if (prefixes.empty()) {
            result.errors.push_back("请至少填写一个输出前缀。");
        }
    } catch (const std::exception &error) {
        result.errors.push_back(error.what());
    }

    if (!sizes.empty() && !prefixes.empty()) {
        if (state.usesGroupedOutput()) {
            if (sizes.size() != 1) {
                result.errors.push_back("横向或纵向拼接时只支持一个输出尺寸。");
            }
            if (prefixes.size() != 1) {
                result.errors.push_back("横向或纵向拼接时只支持一个输出前缀。");
            }
        } else if (sizes.size() != prefixes.size()) {
            result.errors.push_back("逐面输出时，输出尺寸数量必须和前缀数量一致。");
        }
    }

    if (state.enableParallel && state.maxWorkers == 0) {
        result.errors.push_back("并行线程数必须至少为 1。");
    }

    if (result.errors.empty() && state.outputConflictPolicy == disassemble::core::OutputConflictPolicy::ForbidOverwrite) {
        try {
            for (const auto &path : previewOutputs(state)) {
                if (fs::exists(path)) {
                    result.errors.push_back("发现已有输出文件，当前策略禁止覆盖: " + path.string());
                    break;
                }
            }
        } catch (const std::exception &error) {
            result.errors.push_back(error.what());
        }
    }

    result.ok = result.errors.empty();
    if (result.ok && state.outputConflictPolicy == disassemble::core::OutputConflictPolicy::AutoRename) {
        result.warnings.push_back("当前策略会在重名时自动改名保存输出文件。");
    }
    return result;
}

} // namespace disassemble::desktop
