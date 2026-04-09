#include "FacePositionPrefix.h"

#include <algorithm>
#include <cmath>
#include <iomanip>
#include <sstream>
#include <stdexcept>
#include <vector>

#include "ObjModel.h"

namespace {

using disassemble::core::PaddedUvModel;

struct FaceCenter {
    size_t originalIndex = 0;
    float x = 0.0F;
    float y = 0.0F;
    float ySpan = 0.0F;
};

struct RowGroup {
    std::vector<FaceCenter> faces;
    float averageY = 0.0F;
};

float medianYSpan(std::vector<FaceCenter> faces)
{
    if (faces.empty()) {
        return 0.0F;
    }

    std::sort(faces.begin(), faces.end(), [](const FaceCenter &left, const FaceCenter &right) {
        return left.ySpan < right.ySpan;
    });
    return faces[faces.size() / 2].ySpan;
}

std::string numberedLabel(const std::string &prefix, size_t index)
{
    std::ostringstream stream;
    stream << prefix << std::setw(2) << std::setfill('0') << (index + 1);
    return stream.str();
}

std::string rowLabel(size_t index, size_t total)
{
    if (total == 1) {
        return "center";
    }
    if (total == 2) {
        return index == 0 ? "top" : "bottom";
    }
    if (total == 3) {
        if (index == 0) {
            return "top";
        }
        if (index == 1) {
            return "middle";
        }
        return "bottom";
    }
    if (index == 0) {
        return "top";
    }
    if (index + 1 == total) {
        return "bottom";
    }
    return numberedLabel("row_", index);
}

std::string columnLabel(size_t index, size_t total)
{
    if (total == 1) {
        return "center";
    }
    if (total == 2) {
        return index == 0 ? "left" : "right";
    }
    if (total == 3) {
        if (index == 0) {
            return "left";
        }
        if (index == 1) {
            return "center";
        }
        return "right";
    }
    if (index == 0) {
        return "left";
    }
    if (index + 1 == total) {
        return "right";
    }
    return numberedLabel("col_", index);
}

std::vector<FaceCenter> loadFaceCenters(const std::string &inputObjPath)
{
    const auto inputObj = PaddedUvModel::load(inputObjPath);
    const auto &uvPoints = inputObj.model.uvPoints();
    const auto &faces = inputObj.model.faces();

    std::vector<FaceCenter> result;
    result.reserve(faces.size());
    for (size_t faceIndex = 0; faceIndex < faces.size(); ++faceIndex) {
        const auto &face = faces[faceIndex];
        float sumX = 0.0F;
        float sumY = 0.0F;
        float minY = 1.0F;
        float maxY = 0.0F;

        for (int pointIndex = 0; pointIndex < 4; ++pointIndex) {
            const auto &uv = uvPoints[face.uvIndices[pointIndex]];
            sumX += uv.x;
            sumY += uv.y;
            minY = std::min(minY, uv.y);
            maxY = std::max(maxY, uv.y);
        }

        result.push_back(FaceCenter{
            faceIndex,
            sumX / 4.0F,
            sumY / 4.0F,
            maxY - minY
        });
    }

    return result;
}

std::vector<RowGroup> clusterRows(std::vector<FaceCenter> faces)
{
    std::sort(faces.begin(), faces.end(), [](const FaceCenter &left, const FaceCenter &right) {
        if (left.y == right.y) {
            return left.x < right.x;
        }
        return left.y < right.y;
    });

    std::vector<RowGroup> rows;
    const float tolerance = std::max(0.02F, medianYSpan(faces) * 0.5F);
    for (const auto &face : faces) {
        if (rows.empty() || std::fabs(face.y - rows.back().averageY) > tolerance) {
            rows.push_back({{face}, face.y});
            continue;
        }

        auto &row = rows.back();
        row.faces.push_back(face);
        row.averageY = (row.averageY * static_cast<float>(row.faces.size() - 1) + face.y)
            / static_cast<float>(row.faces.size());
    }

    for (auto &row : rows) {
        std::sort(row.faces.begin(), row.faces.end(), [](const FaceCenter &left, const FaceCenter &right) {
            return left.x < right.x;
        });
    }

    return rows;
}

} // namespace

namespace disassemble::core {

std::vector<std::string> buildFacePositionPrefixes(const std::string &inputObjPath)
{
    if (inputObjPath.empty()) {
        throw std::runtime_error("missing input.obj for face position prefixes");
    }

    const auto rows = clusterRows(loadFaceCenters(inputObjPath));
    size_t totalFaces = 0;
    for (const auto &row : rows) {
        totalFaces += row.faces.size();
    }

    std::vector<std::string> prefixes(totalFaces);
    for (size_t rowIndex = 0; rowIndex < rows.size(); ++rowIndex) {
        const auto &row = rows[rowIndex];
        const auto currentRowLabel = rowLabel(rowIndex, rows.size());
        for (size_t columnIndex = 0; columnIndex < row.faces.size(); ++columnIndex) {
            const auto &face = row.faces[columnIndex];
            prefixes[face.originalIndex] = currentRowLabel + "_" + columnLabel(columnIndex, row.faces.size());
        }
    }

    return prefixes;
}

} // namespace disassemble::core
