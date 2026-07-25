#pragma once

#include <QWidget>

class CaptureRegionWindow : public QWidget
{
    Q_OBJECT
public:
    explicit CaptureRegionWindow(QWidget *parent = nullptr);

    // 设置窗口的大小和位置
    void setGeo(int x, int y, int w, int h);

protected:
    void paintEvent(QPaintEvent *event) override;

    // 无边框悬浮窗的移动和缩放
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

signals:
};
