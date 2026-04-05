#include "TaskFormState.h"

#include <algorithm>
#include <cctype>
#include <sstream>
#include <stdexcept>

namespace {

std::string trim(std::string value)
{
    auto isSpace = [](unsigned char ch) { return std::isspace(ch) != 0; };
    value.erase(value.begin(), std::find_if(value.begin(), value.end(), [&](unsigned char ch) { return !isSpace(ch); }));
    value.erase(std::find_if(value.rbegin(), value.rend(), [&](unsigned char ch) { return !isSpace(ch); }).base(), value.end());
    return value;
}

std::vector<std::string> splitTokens(const std::string &text)
{
    std::string normalized = text;
    std::replace(normalized.begin(), normalized.end(), ',', ' ');
    std::replace(normalized.begin(), normalized.end(), ';', ' ');

    std::vector<std::string> tokens;
    std::istringstream stream(normalized);
    std::string token;
    while (stream >> token) {
        tokens.push_back(trim(token));
    }
    return tokens;
}

} // namespace

namespace disassemble::desktop {

bool TaskFormState::usesSingleImageInput() const
{
    return inputMode == InputMode::SingleImage;
}

bool TaskFormState::usesDirectoryInput() const
{
    return inputMode == InputMode::Directory;
}

bool TaskFormState::usesGroupedOutput() const
{
    return direction != disassemble::core::ProcessingDirection::None;
}

std::filesystem::path TaskFormState::resolvedInputObjPath(const std::filesystem::path &detectedPath) const
{
    return autoDetectModels ? detectedPath : inputObjPath;
}

std::filesystem::path TaskFormState::resolvedOutputObjPath(const std::filesystem::path &detectedPath) const
{
    return autoDetectModels ? detectedPath : outputObjPath;
}

std::vector<disassemble::core::ImageSize> TaskFormState::parseOutputSizes(const std::string &text)
{
    std::vector<disassemble::core::ImageSize> sizes;
    for (auto token : splitTokens(text)) {
        const auto starPos = token.find('*');
        const auto xPos = token.find('x');
        const auto upperXPos = token.find('X');
        const auto separator = starPos != std::string::npos ? starPos : (xPos != std::string::npos ? xPos : upperXPos);
        if (separator == std::string::npos) {
            throw std::runtime_error("输出尺寸格式不正确，应为 宽*高");
        }

        disassemble::core::ImageSize size;
        size.width = std::stoi(token.substr(0, separator));
        size.height = std::stoi(token.substr(separator + 1));
        if (size.width <= 0 || size.height <= 0) {
            throw std::runtime_error("输出尺寸必须大于 0");
        }
        sizes.push_back(size);
    }

    return sizes;
}

std::vector<std::string> TaskFormState::parsePrefixes(const std::string &text)
{
    std::vector<std::string> prefixes;
    for (auto token : splitTokens(text)) {
        if (!token.empty()) {
            prefixes.push_back(token);
        }
    }
    return prefixes;
}

std::string TaskFormState::formatOutputSizes(const std::vector<disassemble::core::ImageSize> &sizes)
{
    std::ostringstream stream;
    for (size_t index = 0; index < sizes.size(); ++index) {
        if (index != 0) {
            stream << ", ";
        }
        stream << sizes[index].width << "*" << sizes[index].height;
    }
    return stream.str();
}

std::string TaskFormState::formatPrefixes(const std::vector<std::string> &prefixes)
{
    std::ostringstream stream;
    for (size_t index = 0; index < prefixes.size(); ++index) {
        if (index != 0) {
            stream << ", ";
        }
        stream << prefixes[index];
    }
    return stream.str();
}

} // namespace disassemble::desktop
