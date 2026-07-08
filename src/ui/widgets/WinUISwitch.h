#pragma once

#include <QWidget>
#include <QPropertyAnimation>
#include <QEnterEvent>

//
// 自定义组件, 效果类似于 WinUI 的开关
//
class WinUISwitch : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(qreal offset READ offset WRITE setOffset)

public:
    explicit WinUISwitch(QWidget *parent = nullptr);

    bool isChecked() const { return m_checked; }
    void setChecked(bool checked);

    qreal offset() const { return m_offset; }
    void setOffset(qreal v);

signals:
    void toggled(bool checked);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

    void resizeEvent(QResizeEvent *event) override
    {
        QWidget::resizeEvent(event);

        if (height() != 24)
        {
            setFixedHeight(24);
        }
    }

private:
    void startAnimation(bool checked);

private:
    bool m_checked = false;
    qreal m_offset = 0.0;     // 0 = off, 1 = on
    bool m_hover = false;
};
