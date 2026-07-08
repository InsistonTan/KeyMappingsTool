#include "WinUISwitch.h"

#include <QPainter>
#include <QMouseEvent>
#include <QEasingCurve>

WinUISwitch::WinUISwitch(QWidget *parent)
    : QWidget(parent)
{
    setFixedSize(44, 24);
    setCursor(Qt::PointingHandCursor);
    setAttribute(Qt::WA_Hover, true);
}

void WinUISwitch::setChecked(bool checked)
{
    if (m_checked == checked)
        return;

    m_checked = checked;
    startAnimation(checked);
}

void WinUISwitch::startAnimation(bool checked)
{
    QPropertyAnimation *anim = new QPropertyAnimation(this, "offset");
    anim->setDuration(180);
    anim->setStartValue(m_offset);
    anim->setEndValue(checked ? 1.0 : 0.0);
    anim->setEasingCurve(QEasingCurve::OutCubic);
    anim->start(QAbstractAnimation::DeleteWhenStopped);
}

void WinUISwitch::setOffset(qreal v)
{
    m_offset = v;
    update();
}

void WinUISwitch::mousePressEvent(QMouseEvent *)
{
    // 当前最新的checked状态
    bool ischecked = !m_checked;

    // 设置状态
    setChecked(ischecked);

    // 提交toggled事件信号
    emit toggled(ischecked);
}

void WinUISwitch::enterEvent(QEnterEvent *)
{
    m_hover = true;
    update();
}

void WinUISwitch::leaveEvent(QEvent *)
{
    m_hover = false;
    update();
}

void WinUISwitch::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    QRectF r = rect();

    // ===== 颜色（WinUI 风格）=====
    QColor trackOff("#A0A0A0");
    QColor trackOn("#4F6BED");
    QColor thumbColor("#FFFFFF");

    QColor track = m_checked ? trackOn : trackOff;

    if (m_hover) {
        track = track.lighter(110);
    }

    // ===== 轨道 =====
    p.setPen(Qt::NoPen);
    p.setBrush(track);

    qreal radius = r.height() / 2.0;
    p.drawRoundedRect(r.adjusted(1, 1, -1, -1), radius, radius);

    // ===== 滑块 =====
    qreal margin = 2;
    qreal diameter = r.height() - margin * 2;

    qreal x = margin + m_offset * (r.width() - diameter - margin * 2);
    qreal y = margin;

    // hover轻微缩放效果
    qreal scale = m_hover ? 0.95 : 1.0;
    qreal offset = (1.0 - scale) * diameter / 2.0;

    QRectF thumbRect(x + offset, y + offset,
                     diameter * scale, diameter * scale);

    p.setBrush(thumbColor);
    p.setPen(Qt::NoPen);
    p.drawEllipse(thumbRect);
}
