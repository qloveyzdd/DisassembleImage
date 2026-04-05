#pragma once

#include <QPixmap>
#include <QString>
#include <QWidget>

class QLabel;
class QResizeEvent;
class QScrollArea;

namespace disassemble::desktop {

class PreviewImagePane : public QWidget
{
    Q_OBJECT

public:
    explicit PreviewImagePane(const QString &title, QWidget *parent = nullptr);

    void setImagePath(const std::string &imagePath);
    void clearWithMessage(const QString &message);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void refreshPixmap();

    QLabel *titleLabel_;
    QLabel *imageLabel_;
    QScrollArea *scrollArea_;
    QString currentImagePath_;
    QPixmap currentPixmap_;
};

} // namespace disassemble::desktop
