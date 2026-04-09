#include "ObjModel.h"

#include <algorithm>
#include <cctype>
#include <fstream>
#include <sstream>
#include <stdexcept>

namespace fs = std::filesystem;

namespace {

std::vector<std::string> split(const std::string &input, char delimiter)
{
    std::vector<std::string> parts;
    std::istringstream stream(input);
    std::string token;
    while (std::getline(stream, token, delimiter)) {
        if (!token.empty()) {
            parts.push_back(token);
        }
    }
    return parts;
}

std::string trimRight(std::string value)
{
    while (!value.empty() && (value.back() == '\r' || value.back() == '\n' || std::isspace(static_cast<unsigned char>(value.back())))) {
        value.pop_back();
    }
    return value;
}

} // namespace

namespace disassemble::core {

ObjModel ObjModel::load(const fs::path &path)
{
    std::ifstream input(path);
    if (!input.is_open()) {
        throw std::runtime_error("cannot read obj file: " + path.string());
    }

    ObjModel model;
    std::string line;
    while (std::getline(input, line)) {
        line = trimRight(line);
        if (line.empty() || line[0] == '#' || line[0] == '\t') {
            continue;
        }

        if (line.rfind("v ", 0) == 0) {
            const auto parts = split(line, ' ');
            if (parts.size() < 4) {
                throw std::runtime_error("invalid obj vertex line: " + path.string());
            }
            model.spatialPoints_.emplace_back(std::stof(parts[1]), std::stof(parts[2]), std::stof(parts[3]));
            continue;
        }

        if (line.rfind("vt", 0) == 0) {
            const auto parts = split(line, ' ');
            if (parts.size() < 3) {
                throw std::runtime_error("invalid obj uv line: " + path.string());
            }
            model.uvPoints_.emplace_back(std::stof(parts[1]), 1.0F - std::stof(parts[2]));
            continue;
        }

        if (line.rfind("f ", 0) == 0) {
            const auto parts = split(line, ' ');
            if (parts.size() != 5) {
                throw std::runtime_error("obj contains non-quad face: " + path.string());
            }

            ObjFaceQuad face;
            for (size_t index = 1; index < parts.size(); ++index) {
                const auto values = split(parts[index], '/');
                if (values.size() < 2) {
                    throw std::runtime_error("invalid obj face index line: " + path.string());
                }
                face.spatialIndices[index - 1] = std::stoi(values[0]) - 1;
                face.uvIndices[index - 1] = std::stoi(values[1]) - 1;
            }
            model.faces_.push_back(face);
        }
    }

    return model;
}

const std::vector<cv::Point3f> &ObjModel::spatialPoints() const
{
    return spatialPoints_;
}

const std::vector<cv::Point2f> &ObjModel::uvPoints() const
{
    return uvPoints_;
}

const std::vector<ObjFaceQuad> &ObjModel::faces() const
{
    return faces_;
}

PaddedUvModel PaddedUvModel::load(const fs::path &path)
{
    PaddedUvModel padded;
    padded.model = ObjModel::load(path);

    if (padded.model.uvPoints().empty()) {
        return padded;
    }

    float minX = padded.model.uvPoints().front().x;
    float maxX = padded.model.uvPoints().front().x;
    float minY = padded.model.uvPoints().front().y;
    float maxY = padded.model.uvPoints().front().y;

    for (const auto &uv : padded.model.uvPoints()) {
        minX = std::min(minX, uv.x);
        maxX = std::max(maxX, uv.x);
        minY = std::min(minY, uv.y);
        maxY = std::max(maxY, uv.y);
    }

    padded.left = minX < 0.0F ? -minX : 0.0F;
    padded.right = maxX > 1.0F ? maxX - 1.0F : 0.0F;
    padded.bottom = minY < 0.0F ? -minY : 0.0F;
    padded.top = maxY > 1.0F ? maxY - 1.0F : 0.0F;
    return padded;
}

float PaddedUvModel::crosswiseMul() const
{
    return left + right + 1.0F;
}

float PaddedUvModel::lengthwaysMul() const
{
    return top + bottom + 1.0F;
}

} // namespace disassemble::core
