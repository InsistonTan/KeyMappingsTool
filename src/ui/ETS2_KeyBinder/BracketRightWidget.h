#pragma once
#include <QWidget>

class BracketRightWidget : public QWidget
{
    Q_OBJECT
public:
    explicit BracketRightWidget(QWidget *parent = nullptr);

protected:
    void paintEvent(QPaintEvent *event) override;
};