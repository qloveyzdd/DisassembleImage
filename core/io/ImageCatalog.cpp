#include "ImageCatalog.h"

#include <algorithm>
#include <cctype>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

bool isSupportedImage(const fs::path &path)
{
    if (!path.has_extension()) {
        return false;
    }

    std::string ext = path.extension().string();
    std::transform(ext.begin(), ext.end(), ext.begin(), [](unsigned char ch) {
        return static_cast<char>(std::tolower(ch));
    });
    return ext == ".jpg" || ext == ".jpeg" || ext == ".png" || ext == ".tga";
}

} // namespace

namespace disassemble::core {

std::vector<fs::path> ImageCatalog::collect(const ProcessingTask &task)
{
    std::vector<fs::path> inputs;

    if (task.hasSingleImage()) {
        fs::path singlePath(task.inputImagePath);
        if (!fs::exists(singlePath)) {
            throw std::runtime_error("输入图片不存在: " + singlePath.string());
        }
        inputs.push_back(fs::absolute(singlePath));
        return inputs;
    }

    if (!task.hasInputDirectory()) {
        throw std::runtime_error("未提供输入图片或输入目录");
    }

    fs::path inputDir(task.inputDirectory);
    if (!fs::exists(inputDir) || !fs::is_directory(inputDir)) {
        throw std::runtime_error("输入目录不存在: " + inputDir.string());
    }

    for (const auto &entry : fs::directory_iterator(inputDir)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        if (!isSupportedImage(entry.path())) {
            continue;
        }
        inputs.push_back(fs::absolute(entry.path()));
    }

    std::sort(inputs.begin(), inputs.end());
    return inputs;
}

} // namespace disassemble::core
