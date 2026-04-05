#pragma once

#include <map>

#include <QImage>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <QPolygonF>
#include <QPoint>

#include "../../core/model/PreviewMesh.h"

class QMouseEvent;
class QWheelEvent;

namespace disassemble::desktop {

class Preview3DWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    explicit Preview3DWidget(QWidget *parent = nullptr);

    void setMesh(const disassemble::core::PreviewMesh &mesh);
    void resetCamera();

protected:
    void initializeGL() override;
    void paintGL() override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;

private:
    struct ProjectedFace {
        QPolygonF targetPolygon;
        QPolygonF sourcePolygon;
        float depth = 0.0F;
        int textureAssignmentIndex = -1;
    };

    std::vector<ProjectedFace> buildProjectedFaces() const;
    QImage textureFor(const std::string &outputImagePath) const;

    disassemble::core::PreviewMesh mesh_;
    mutable std::map<std::string, QImage> textureCache_;
    QPoint lastMousePosition_;
    float rotationX_ = -18.0F;
    float rotationY_ = 26.0F;
    float zoom_ = 1.0F;
};

} // namespace disassemble::desktop
