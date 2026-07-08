#pragma once

#include <QWidget>
#include <QToolButton>
#include <QVBoxLayout>
#include <QPropertyAnimation>

//
// 可折叠的成组容器
//
class CollapsibleGroupBox : public QWidget
{
    Q_OBJECT
public:
    explicit CollapsibleGroupBox(const QString& title, QWidget *parent = nullptr, QLayout *contentLayout = nullptr);

    // 设置可折叠的成组容器的需要显示的内容layout
    void setContentLayout(QLayout *layout);

    // 设置折叠区域是否可见
    void setContentVisible(bool visible);

private:
    QToolButton* headerButton;
    QWidget* contentWidget;
    QVBoxLayout* mainLayout;
    QIcon arrowRight;
    QIcon arrowDown;
};
