#include "LegacyTextConfigLoader.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

std::string trim(std::string value)
{
    auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) { return !isSpace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](unsigned char ch) { return !isSpace(ch); }).base(), value.end());
    return value;
}

std::vector<std::string> splitBySpace(const std::string &input)
{
    std::vector<std::string> values;
    std::istringstream stream(input);
    std::string part;
    while (stream >> part) {
        values.push_back(part);
    }
    return values;
}

disassemble::core::ImageSize parseImageSize(const std::string &text)
{
    const auto pos = text.find('*');
    if (pos == std::string::npos) {
        throw std::runtime_error("尺寸格式不正确: " + text);
    }

    disassemble::core::ImageSize size;
    size.width = std::stoi(text.substr(0, pos));
    size.height = std::stoi(text.substr(pos + 1));
    return size;
}

disassemble::core::ProcessingDirection parseDirection(const std::string &text)
{
    if (text == "NONE") {
        return disassemble::core::ProcessingDirection::None;
    }
    if (text == "X") {
        return disassemble::core::ProcessingDirection::X;
    }
    if (text == "Y") {
        return disassemble::core::ProcessingDirection::Y;
    }
    throw std::runtime_error("不支持的拆分方向: " + text);
}

} // namespace

namespace disassemble::core {

ProcessingTask LegacyTextConfigLoader::load(const fs::path &configPath)
{
    std::ifstream input(configPath);
    if (!input.is_open()) {
        throw std::runtime_error("无法读取配置文件: " + configPath.string());
    }

    std::vector<std::string> lines;
    std::string line;
    while (std::getline(input, line)) {
        lines.push_back(trim(line));
    }

    if (lines.size() < 6) {
        throw std::runtime_error("配置文件至少需要 6 行: " + configPath.string());
    }

    ProcessingTask task;
    task.outputSizes.reserve(splitBySpace(lines[1]).size());
    for (const auto &sizeText : splitBySpace(lines[1])) {
        task.outputSizes.push_back(parseImageSize(sizeText));
    }
    task.prefixes = splitBySpace(lines[2]);
    task.inputDirectory = fs::absolute(configPath.parent_path() / lines[3]).string();
    task.outputRoot = fs::absolute(configPath.parent_path() / lines[4]).string();
    task.direction = parseDirection(lines[5]);
    task.inputObjPath = fs::absolute(configPath.parent_path() / "input.obj").string();
    task.outputObjPath = fs::absolute(configPath.parent_path() / "output.obj").string();
    task.enableParallel = false;
    task.maxWorkers = 1;
    return task;
}

} // namespace disassemble::core
