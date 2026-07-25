#include "CaptureRegionWindow.h"
#include <Windows.h>

#include <QPainter>
#include <QScreen>

CaptureRegionWindow::CaptureRegionWindow(QWidget *parent) : QWidget{parent}{
    // 设置窗口属性为: 工具窗口, 无边框, 置于顶层
    setWindowFlags(
        Qt::Tool |
        Qt::FramelessWindowHint |
        Qt::WindowStaysOnTopHint
        );

    // 透明背景
    setAttribute(Qt::WA_TranslucentBackground);

    resize(300, 200);
}

void CaptureRegionWindow::setGeo(int x, int y, int w, int h){
    QScreen* screen = this->screen();
    // 缩放比例
    auto scale = screen->devicePixelRatio();

    // 设置窗口位置和大小
    setGeometry({(int)(x/scale), (int)(y/scale), (int)(w/scale), (int)(h/scale)});
}

void CaptureRegionWindow::paintEvent(QPaintEvent *event){
    QPainter painter(this);

    painter.setRenderHint(QPainter::Antialiasing);

    // 半透明绿色
    painter.fillRect(rect(), QColor(0,255,0,40));

    // 边框
    QPen pen(QColor(0,255,0));

    pen.setWidth(2);

    painter.setPen(pen);

    painter.setBrush(Qt::NoBrush);

    painter.drawRect(rect().adjusted(1,1,-2,-2));
}

bool CaptureRegionWindow::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
    MSG *msg = static_cast<MSG *>(message);

    if (msg->message != WM_NCHITTEST)
        return QWidget::nativeEvent({}, message, result);

    constexpr int border = 8;

    RECT rc;
    GetWindowRect(HWND(winId()), &rc);

    POINT pt;
    GetCursorPos(&pt);

    bool left   = pt.x < rc.left + border;
    bool right  = pt.x >= rc.right - border;
    bool top    = pt.y < rc.top + border;
    bool bottom = pt.y >= rc.bottom - border;

    if (top && left)
    {
        *result = HTTOPLEFT;
        return true;
    }

    if (top && right)
    {
        *result = HTTOPRIGHT;
        return true;
    }

    if (bottom && left)
    {
        *result = HTBOTTOMLEFT;
        return true;
    }

    if (bottom && right)
    {
        *result = HTBOTTOMRIGHT;
        return true;
    }

    if (left)
    {
        *result = HTLEFT;
        return true;
    }

    if (right)
    {
        *result = HTRIGHT;
        return true;
    }

    if (top)
    {
        *result = HTTOP;
        return true;
    }

    if (bottom)
    {
        *result = HTBOTTOM;
        return true;
    }

    // 中间区域, 移动窗口
    *result = HTCAPTION;

    return true;
}
