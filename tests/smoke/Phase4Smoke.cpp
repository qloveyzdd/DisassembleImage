#include <filesystem>
#include <type_traits>

#include "../../app/windows/Preview3DWidget.h"
#include "../../app/windows/RunController.h"
#include "../../core/model/PreviewMesh.h"

namespace fs = std::filesystem;

int main()
{
    using SetMeshMethod = void (disassemble::desktop::Preview3DWidget::*)(const disassemble::core::PreviewMesh &);
    using ResetCameraMethod = void (disassemble::desktop::Preview3DWidget::*)();

    static_assert(std::is_same_v<SetMeshMethod, decltype(&disassemble::desktop::Preview3DWidget::setMesh)>);
    static_assert(std::is_same_v<ResetCameraMethod, decltype(&disassemble::desktop::Preview3DWidget::resetCamera)>);

    const fs::path sourceRoot(DISASSEMBLEIMAGE_SOURCE_ROOT);
    const fs::path tempRoot = sourceRoot / "build" / "phase4-smoke-output";
    std::error_code ec;
    fs::remove_all(tempRoot, ec);
    fs::create_directories(tempRoot, ec);

    const fs::path sourceImage = sourceRoot / "HD" / "a0000.jpg";
    auto task = disassemble::desktop::RunController::buildSmokeTask(sourceImage, tempRoot / "grouped-run", sourceRoot);
    task.outputSizes = {{640, 480}};

    const auto result = disassemble::desktop::RunController::runTask(task);
    if (!result.ok() || result.outputFiles.empty()) {
        return 1;
    }

    const auto previewItems = disassemble::core::buildPreviewGalleryItems(result);
    if (previewItems.empty()) {
        return 2;
    }

    const auto &currentItem = previewItems.front();
    if (!currentItem.isValid()) {
        return 3;
    }

    if (!fs::exists(currentItem.inputImagePath) || !fs::exists(currentItem.outputImagePath)) {
        return 4;
    }

    const auto itemsForInput = disassemble::core::filterPreviewGalleryItemsByInput(previewItems,
                                                                                    currentItem.inputImagePath);
    const auto mesh = disassemble::core::buildPreviewMesh(task, itemsForInput);
    if (mesh.empty() || mesh.faceTextureAssignments.empty()) {
        return 5;
    }

    return 0;
}
