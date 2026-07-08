#include "bracketrightwidget.h"
#include <QPainter>

BracketRightWidget::BracketRightWidget(QWidget* parent) : QWidget(parent) {
    setMinimumWidth(16);
    setMinimumHeight(60);
}

void BracketRightWidget::paintEvent(QPaintEvent*) {
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPen pen(QColor(79, 79, 79), 2);
    p.setPen(pen);

    int w = width();
    int h = height();
    int margin = 10;
    int arcW = w - margin * 2;
    int arcH = h / 2 - margin;

    // 上半圆弧
    p.drawArc(margin, margin, arcW, arcH, 0 * 16, 90 * 16);
    // 下半圆弧
    p.drawArc(margin, h / 2, arcW, arcH, 0 * 16, -90 * 16);
    // 中间竖线
    // 竖线x坐标为右端点
    int x = margin + arcW + 2;
    // 竖线y坐标为两个圆弧的中点
    int y1 = margin + arcH / 2;
    int y2 = h - margin - arcH / 2;
    p.drawLine(x, y1, x, y2);
}