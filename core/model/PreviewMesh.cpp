#include "PreviewMesh.h"

#include <algorithm>
#include <filesystem>
#include <stdexcept>

#include "ObjModel.h"
#include "RunResult.h"

namespace fs = std::filesystem;

namespace {

using disassemble::core::FaceTextureAssignment;
using disassemble::core::ObjModel;
using disassemble::core::PreviewFaceQuad;
using disassemble::core::PreviewGalleryItem;
using disassemble::core::PreviewMesh;
using disassemble::core::PreviewVertex;

PreviewVertex makeVertex(const cv::Point3f &position, const cv::Point2f &uv)
{
    PreviewVertex vertex;
    vertex.x = position.x;
    vertex.y = position.y;
    vertex.z = position.z;
    vertex.u = uv.x;
    vertex.v = uv.y;
    return vertex;
}

PreviewGalleryItem makeFallbackItem(const std::string &outputImagePath)
{
    PreviewGalleryItem item;
    item.outputImagePath = outputImagePath;
    item.displayName = fs::path(outputImagePath).stem().string();
    return item;
}

void appendFace(PreviewMesh &mesh,
                const std::array<cv::Point3f, 4> &positions,
                const std::array<cv::Point2f, 4> &uvs,
                const FaceTextureAssignment &assignment)
{
    const int assignmentIndex = static_cast<int>(mesh.faceTextureAssignments.size());
    mesh.faceTextureAssignments.push_back(assignment);

    PreviewFaceQuad face;
    for (size_t vertexIndex = 0; vertexIndex < positions.size(); ++vertexIndex) {
        face.vertexIndices[vertexIndex] = static_cast<int>(mesh.vertices.size());
        mesh.vertices.push_back(makeVertex(positions[vertexIndex], uvs[vertexIndex]));
    }
    face.textureAssignmentIndex = assignmentIndex;
    mesh.faces.push_back(face);
}

std::array<cv::Point3f, 4> collectSpatialQuad(const std::vector<cv::Point3f> &points,
                                              const std::array<int, 4> &indices)
{
    std::array<cv::Point3f, 4> quad{};
    for (size_t index = 0; index < quad.size(); ++index) {
        quad[index] = points[indices[index]];
    }
    return quad;
}

std::array<cv::Point2f, 4> collectUvQuad(const std::vector<cv::Point2f> &points,
                                         const std::array<int, 4> &indices)
{
    std::array<cv::Point2f, 4> quad{};
    for (size_t index = 0; index < quad.size(); ++index) {
        quad[index] = points[indices[index]];
    }
    return quad;
}

PreviewMesh buildGroupedPreviewMesh(const disassemble::core::ProcessingTask &task,
                                    const std::vector<PreviewGalleryItem> &itemsForInput)
{
    if (itemsForInput.empty()) {
        return {};
    }

    const auto inputObj = ObjModel::load(task.inputObjPath);
    const auto outputObj = ObjModel::load(task.outputObjPath);

    const auto &spatialPoints = inputObj.spatialPoints();
    const auto &spatialFaces = inputObj.faces();
    const auto &outputUvPoints = outputObj.uvPoints();
    const auto &outputUvFaces = outputObj.faces();

    if (spatialFaces.size() != outputUvFaces.size()) {
        throw std::runtime_error(u8"构建 grouped 预览 mesh 失败: input.obj 与 output.obj 面数不一致");
    }

    PreviewMesh mesh;
    const PreviewGalleryItem &item = itemsForInput.front();
    for (size_t faceIndex = 0; faceIndex < spatialFaces.size(); ++faceIndex) {
        FaceTextureAssignment assignment;
        assignment.inputImagePath = item.inputImagePath;
        assignment.outputImagePath = item.outputImagePath;
        assignment.displayName = item.displayName;
        appendFace(mesh,
                   collectSpatialQuad(spatialPoints, spatialFaces[faceIndex].spatialIndices),
                   collectUvQuad(outputUvPoints, outputUvFaces[faceIndex].uvIndices),
                   assignment);
    }

    return mesh;
}

PreviewMesh buildFacePreviewMesh(const disassemble::core::ProcessingTask &task,
                                 const std::vector<PreviewGalleryItem> &itemsForInput)
{
    const auto inputObj = ObjModel::load(task.inputObjPath);
    const auto &spatialPoints = inputObj.spatialPoints();
    const auto &spatialFaces = inputObj.faces();

    std::vector<PreviewGalleryItem> orderedItems = itemsForInput;
    std::sort(orderedItems.begin(), orderedItems.end(), [](const PreviewGalleryItem &left, const PreviewGalleryItem &right) {
        return left.faceIndex < right.faceIndex;
    });

    PreviewMesh mesh;
    static const std::array<cv::Point2f, 4> localUvs = {
        cv::Point2f(0.0F, 0.0F),
        cv::Point2f(1.0F, 0.0F),
        cv::Point2f(1.0F, 1.0F),
        cv::Point2f(0.0F, 1.0F)
    };

    for (const auto &item : orderedItems) {
        if (item.faceIndex < 0 || item.faceIndex >= static_cast<int>(spatialFaces.size())) {
            continue;
        }

        FaceTextureAssignment assignment;
        assignment.inputImagePath = item.inputImagePath;
        assignment.outputImagePath = item.outputImagePath;
        assignment.displayName = item.displayName;
        appendFace(mesh,
                   collectSpatialQuad(spatialPoints, spatialFaces[item.faceIndex].spatialIndices),
                   localUvs,
                   assignment);
    }

    return mesh;
}

} // namespace

namespace disassemble::core {

bool PreviewGalleryItem::isValid() const
{
    return !inputImagePath.empty() && !outputImagePath.empty();
}

bool PreviewMesh::empty() const
{
    return vertices.empty() || faces.empty() || faceTextureAssignments.empty();
}

std::vector<PreviewGalleryItem> buildPreviewGalleryItems(const RunResult &result)
{
    if (!result.previewItems.empty()) {
        return result.previewItems;
    }

    std::vector<PreviewGalleryItem> fallbackItems;
    fallbackItems.reserve(result.outputFiles.size());
    for (const auto &outputFile : result.outputFiles) {
        fallbackItems.push_back(makeFallbackItem(outputFile));
    }
    return fallbackItems;
}

std::vector<PreviewGalleryItem> filterPreviewGalleryItemsByInput(const std::vector<PreviewGalleryItem> &items,
                                                                 const std::string &inputImagePath)
{
    std::vector<PreviewGalleryItem> filtered;
    for (const auto &item : items) {
        if (item.inputImagePath == inputImagePath) {
            filtered.push_back(item);
        }
    }
    return filtered;
}

PreviewMesh buildPreviewMesh(const ProcessingTask &task,
                             const std::vector<PreviewGalleryItem> &itemsForInput)
{
    if (itemsForInput.empty()) {
        return {};
    }

    if (task.usesGroupedOutput()) {
        return buildGroupedPreviewMesh(task, itemsForInput);
    }
    return buildFacePreviewMesh(task, itemsForInput);
}

} // namespace disassemble::core

