#include "OcrPreviewWindow.h"
#include "common/StringConstants.h"
#include <Windows.h>
#include <QPainter>
#include<QDateTime>

OcrPreviewWindow::OcrPreviewWindow(QWidget *parent)
    : QWidget{parent}
{
    // 设置窗口属性为: 工具窗口, 无边框, 置于顶层
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    // 透明背景
    setAttribute(Qt::WA_TranslucentBackground);

    resize(100, 30);

    // 左上角显示
    move(0, 0);
}

void OcrPreviewWindow::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    // 背景
    painter.fillRect(rect(), QColor(0,0,0,255));

    // 设置字体
    QFont font;
    //font.setPointSize(12);

    painter.setFont(font);

    // 设置文字颜色
    painter.setPen(Qt::white);

    // 绘制文字
    painter.drawText(rect(), Qt::AlignCenter, previewText.isEmpty() ? StringConstants::noOcrResult : previewText);
}

bool OcrPreviewWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);

    if (msg->message != WM_NCHITTEST)
        return QWidget::nativeEvent({}, message, result);

    // 中间区域, 移动窗口
    *result = HTCAPTION;

    return true;
}

void OcrPreviewWindow::updateOcrPreview(QString val)
{
    this->previewText = "ocr车速: " + val;
    update();
}
