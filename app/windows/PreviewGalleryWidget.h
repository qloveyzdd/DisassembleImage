#pragma once

#include <optional>
#include <vector>

#include <QWidget>

#include "../../core/model/PreviewGalleryItem.h"

class QListWidget;

namespace disassemble::desktop {

class PreviewGalleryWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewGalleryWidget(QWidget *parent = nullptr);

    void setItems(const std::vector<disassemble::core::PreviewGalleryItem> &items);
    std::optional<disassemble::core::PreviewGalleryItem> currentItem() const;
    std::vector<disassemble::core::PreviewGalleryItem> items() const;

signals:
    void currentItemChanged();

private:
    void handleCurrentRowChanged(int row);

    QListWidget *listWidget_;
    std::vector<disassemble::core::PreviewGalleryItem> items_;
};

} // namespace disassemble::desktop
