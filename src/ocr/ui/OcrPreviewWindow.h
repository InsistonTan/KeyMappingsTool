#pragma once

#include <QWidget>

class OcrPreviewWindow : public QWidget
{
    Q_OBJECT
public:
    explicit OcrPreviewWindow(QWidget *parent = nullptr);
    // 更新ocr结果
    void updateOcrPreview(QString val);

protected:
    void paintEvent(QPaintEvent *event) override;
    // 无边框悬浮窗的移动
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private:
    // ocr预览信息
    QString previewText{};

signals:
};
