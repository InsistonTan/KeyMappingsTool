#include "qabstractitemview.h"
#include <QComboBox>
#include <QWheelEvent>

#pragma once


//
// 禁用鼠标滚轮事件的下拉框
//
class NoWheelComboBox : public QComboBox
{
public:
    NoWheelComboBox(QWidget *parent = nullptr) : QComboBox(parent)
    {
        setFocusPolicy(Qt::StrongFocus);
    }

protected:
    void wheelEvent(QWheelEvent *e) override
    {
        if (!view()->isVisible()) {
            e->ignore();
            return;
        }
        QComboBox::wheelEvent(e);
    }
};
