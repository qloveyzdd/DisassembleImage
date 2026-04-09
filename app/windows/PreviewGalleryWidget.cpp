#include "PreviewGalleryWidget.h"

#include <algorithm>

#include <QColor>
#include <QHBoxLayout>
#include <QIcon>
#include <QImageReader>
#include <QListWidget>
#include <QPixmap>

namespace disassemble::desktop {

namespace {

QPixmap buildThumbnail(const std::string &imagePath)
{
    QImageReader reader(QString::fromStdString(imagePath));
    reader.setAutoTransform(true);
    const QSize sourceSize = reader.size();
    if (sourceSize.isValid()) {
        QSize scaledSize = sourceSize;
        scaledSize.scale(112, 112, Qt::KeepAspectRatio);
        reader.setScaledSize(scaledSize);
    }

    const QImage image = reader.read();
    if (image.isNull()) {
        QPixmap placeholder(112, 112);
        placeholder.fill(QColor(232, 234, 237));
        return placeholder;
    }
    return QPixmap::fromImage(image);
}

} // namespace

PreviewGalleryWidget::PreviewGalleryWidget(QWidget *parent)
    : QWidget(parent),
      listWidget_(new QListWidget(this))
{
    auto *layout = new QHBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);

    listWidget_->setViewMode(QListView::IconMode);
    listWidget_->setResizeMode(QListView::Adjust);
    listWidget_->setMovement(QListView::Static);
    listWidget_->setIconSize(QSize(112, 112));
    listWidget_->setSpacing(8);
    listWidget_->setMinimumHeight(180);
    listWidget_->setSelectionMode(QAbstractItemView::SingleSelection);
    layout->addWidget(listWidget_);

    connect(listWidget_, &QListWidget::currentRowChanged, this, [this](int row) {
        handleCurrentRowChanged(row);
    });
}

void PreviewGalleryWidget::setItems(const std::vector<disassemble::core::PreviewGalleryItem> &items)
{
    items_ = items;
    listWidget_->clear();

    int selectedRow = -1;
    for (int index = 0; index < static_cast<int>(items_.size()); ++index) {
        const auto &item = items_[index];
        auto *listItem = new QListWidgetItem(QIcon(buildThumbnail(item.outputImagePath)),
                                             QString::fromStdString(item.displayName));
        listItem->setTextAlignment(Qt::AlignCenter);
        listWidget_->addItem(listItem);
        if (item.selected) {
            selectedRow = index;
        }
    }

    if (selectedRow >= 0) {
        listWidget_->setCurrentRow(selectedRow);
        handleCurrentRowChanged(selectedRow);
        return;
    }

    emit currentItemChanged();
}

std::optional<disassemble::core::PreviewGalleryItem> PreviewGalleryWidget::currentItem() const
{
    const int currentRow = listWidget_->currentRow();
    if (currentRow >= 0 && currentRow < static_cast<int>(items_.size())) {
        return items_[currentRow];
    }

    const auto selected = std::find_if(items_.begin(), items_.end(), [](const auto &item) {
        return item.selected;
    });
    if (selected != items_.end()) {
        return *selected;
    }

    return std::nullopt;
}

std::vector<disassemble::core::PreviewGalleryItem> PreviewGalleryWidget::items() const
{
    return items_;
}

void PreviewGalleryWidget::handleCurrentRowChanged(int row)
{
    for (auto &item : items_) {
        item.selected = false;
    }

    if (row >= 0 && row < static_cast<int>(items_.size())) {
        items_[row].selected = true;
    }

    emit currentItemChanged();
}

} // namespace disassemble::desktop
