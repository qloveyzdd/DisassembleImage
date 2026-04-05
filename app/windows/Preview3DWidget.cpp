#include "Preview3DWidget.h"

#include <algorithm>
#include <cmath>

#include <QMatrix4x4>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QVector3D>
#include <QVector4D>
#include <QWheelEvent>

namespace disassemble::desktop {

Preview3DWidget::Preview3DWidget(QWidget *parent)
    : QOpenGLWidget(parent)
{
    setMinimumSize(260, 220);
}

void Preview3DWidget::setMesh(const disassemble::core::PreviewMesh &mesh)
{
    mesh_ = mesh;
    textureCache_.clear();
    update();
}

void Preview3DWidget::resetCamera()
{
    rotationX_ = -18.0F;
    rotationY_ = 26.0F;
    zoom_ = 1.0F;
    update();
}

void Preview3DWidget::initializeGL()
{
    initializeOpenGLFunctions();
    glClearColor(0.96F, 0.97F, 0.98F, 1.0F);
}

void Preview3DWidget::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.fillRect(rect(), QColor(245, 247, 250));

    if (mesh_.empty()) {
        painter.setPen(QColor(102, 112, 133));
        painter.drawText(rect(), Qt::AlignCenter, QStringLiteral("当前没有可预览的 3D 结果。"));
        return;
    }

    auto faces = buildProjectedFaces();
    std::sort(faces.begin(), faces.end(), [](const ProjectedFace &left, const ProjectedFace &right) {
        return left.depth < right.depth;
    });

    for (const auto &face : faces) {
        painter.save();

        QPainterPath clipPath;
        clipPath.addPolygon(face.targetPolygon);
        painter.setClipPath(clipPath);

        if (face.textureAssignmentIndex >= 0
            && face.textureAssignmentIndex < static_cast<int>(mesh_.faceTextureAssignments.size())) {
            const auto &assignment = mesh_.faceTextureAssignments[face.textureAssignmentIndex];
            const QImage texture = textureFor(assignment.outputImagePath);
            if (!texture.isNull()) {
                QTransform transform;
                if (QTransform::quadToQuad(face.sourcePolygon, face.targetPolygon, transform)) {
                    painter.setTransform(transform, true);
                    painter.drawImage(QPointF(0.0, 0.0), texture);
                }
            } else {
                painter.fillPath(clipPath, QColor(208, 213, 221));
            }
        } else {
            painter.fillPath(clipPath, QColor(208, 213, 221));
        }

        painter.restore();
        painter.setPen(QPen(QColor(16, 24, 40, 120), 1.0));
        painter.drawPolygon(face.targetPolygon);
    }
}

void Preview3DWidget::mousePressEvent(QMouseEvent *event)
{
    lastMousePosition_ = event->position().toPoint();
    QOpenGLWidget::mousePressEvent(event);
}

void Preview3DWidget::mouseMoveEvent(QMouseEvent *event)
{
    const QPoint currentPosition = event->position().toPoint();
    const QPoint delta = currentPosition - lastMousePosition_;
    lastMousePosition_ = currentPosition;

    rotationY_ += static_cast<float>(delta.x()) * 0.55F;
    rotationX_ += static_cast<float>(delta.y()) * 0.55F;
    update();
    QOpenGLWidget::mouseMoveEvent(event);
}

void Preview3DWidget::wheelEvent(QWheelEvent *event)
{
    const QPoint angleDelta = event->angleDelta();
    if (!angleDelta.isNull()) {
        zoom_ += static_cast<float>(angleDelta.y()) / 1200.0F;
        zoom_ = std::clamp(zoom_, 0.4F, 2.6F);
        update();
    }
    QOpenGLWidget::wheelEvent(event);
}

std::vector<Preview3DWidget::ProjectedFace> Preview3DWidget::buildProjectedFaces() const
{
    std::vector<ProjectedFace> projectedFaces;
    if (mesh_.empty()) {
        return projectedFaces;
    }

    QVector3D minPoint(mesh_.vertices.front().x, mesh_.vertices.front().y, mesh_.vertices.front().z);
    QVector3D maxPoint = minPoint;
    for (const auto &vertex : mesh_.vertices) {
        minPoint.setX(std::min(minPoint.x(), vertex.x));
        minPoint.setY(std::min(minPoint.y(), vertex.y));
        minPoint.setZ(std::min(minPoint.z(), vertex.z));
        maxPoint.setX(std::max(maxPoint.x(), vertex.x));
        maxPoint.setY(std::max(maxPoint.y(), vertex.y));
        maxPoint.setZ(std::max(maxPoint.z(), vertex.z));
    }

    const QVector3D center = (minPoint + maxPoint) * 0.5F;
    const float extent = std::max({maxPoint.x() - minPoint.x(), maxPoint.y() - minPoint.y(), maxPoint.z() - minPoint.z(), 1.0F});

    QMatrix4x4 transform;
    transform.scale((zoom_ / extent) * 1.8F);
    transform.rotate(rotationX_, 1.0F, 0.0F, 0.0F);
    transform.rotate(rotationY_, 0.0F, 1.0F, 0.0F);

    const float cameraDistance = 4.0F;
    const float projectionScale = std::min(width(), height()) * 0.46F;

    projectedFaces.reserve(mesh_.faces.size());
    for (const auto &face : mesh_.faces) {
        ProjectedFace projectedFace;
        projectedFace.textureAssignmentIndex = face.textureAssignmentIndex;

        float depth = 0.0F;
        for (const int vertexIndex : face.vertexIndices) {
            if (vertexIndex < 0 || vertexIndex >= static_cast<int>(mesh_.vertices.size())) {
                continue;
            }

            const auto &vertex = mesh_.vertices[vertexIndex];
            QVector4D transformed = transform * QVector4D(vertex.x - center.x(),
                                                          vertex.y - center.y(),
                                                          vertex.z - center.z(),
                                                          1.0F);
            const float perspective = cameraDistance / (cameraDistance - transformed.z());
            const float screenX = (transformed.x() * perspective * projectionScale) + (width() * 0.5F);
            const float screenY = (-transformed.y() * perspective * projectionScale) + (height() * 0.5F);
            projectedFace.targetPolygon << QPointF(screenX, screenY);
            depth += transformed.z();

            if (face.textureAssignmentIndex >= 0
                && face.textureAssignmentIndex < static_cast<int>(mesh_.faceTextureAssignments.size())) {
                const auto &assignment = mesh_.faceTextureAssignments[face.textureAssignmentIndex];
                const QImage texture = textureFor(assignment.outputImagePath);
                const qreal textureWidth = texture.isNull() ? 1.0 : texture.width();
                const qreal textureHeight = texture.isNull() ? 1.0 : texture.height();
                projectedFace.sourcePolygon << QPointF(vertex.u * textureWidth, vertex.v * textureHeight);
            }
        }

        projectedFace.depth = depth / 4.0F;
        projectedFaces.push_back(projectedFace);
    }

    return projectedFaces;
}

QImage Preview3DWidget::textureFor(const std::string &outputImagePath) const
{
    const auto found = textureCache_.find(outputImagePath);
    if (found != textureCache_.end()) {
        return found->second;
    }

    const QImage texture(QString::fromStdString(outputImagePath));
    textureCache_[outputImagePath] = texture;
    return texture;
}

} // namespace disassemble::desktop
