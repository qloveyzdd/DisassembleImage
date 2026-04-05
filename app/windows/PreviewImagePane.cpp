#include "PreviewImagePane.h"

#include <QLabel>
#include <QResizeEvent>
#include <QScrollArea>
#include <QVBoxLayout>

namespace disassemble::desktop {

PreviewImagePane::PreviewImagePane(const QString &title, QWidget *parent)
    : QWidget(parent),
      titleLabel_(new QLabel(title, this)),
      imageLabel_(new QLabel(this)),
      scrollArea_(new QScrollArea(this))
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(titleLabel_);

    imageLabel_->setAlignment(Qt::AlignCenter);
    imageLabel_->setWordWrap(true);
    imageLabel_->setMinimumSize(240, 180);

    scrollArea_->setWidget(imageLabel_);
    scrollArea_->setWidgetResizable(true);
    layout->addWidget(scrollArea_);

    clearWithMessage(QStringLiteral("当前没有可预览的图片。"));
}

void PreviewImagePane::setImagePath(const std::string &imagePath)
{
    currentImagePath_ = QString::fromStdString(imagePath);
    currentPixmap_ = QPixmap(currentImagePath_);
    if (currentPixmap_.isNull()) {
        clearWithMessage(QStringLiteral("图片加载失败。"));
        return;
    }
    refreshPixmap();
}

void PreviewImagePane::clearWithMessage(const QString &message)
{
    currentImagePath_.clear();
    currentPixmap_ = QPixmap();
    imageLabel_->setPixmap(QPixmap());
    imageLabel_->setText(message);
}

void PreviewImagePane::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshPixmap();
}

void PreviewImagePane::refreshPixmap()
{
    if (currentPixmap_.isNull()) {
        return;
    }

    const int width = scrollArea_->viewport()->width() - 12;
    const int height = scrollArea_->viewport()->height() - 12;
    imageLabel_->setText(QString());
    imageLabel_->setPixmap(currentPixmap_.scaled(width > 0 ? width : currentPixmap_.width(),
                                                 height > 0 ? height : currentPixmap_.height(),
                                                 Qt::KeepAspectRatio,
                                                 Qt::SmoothTransformation));
}

} // namespace disassemble::desktop
