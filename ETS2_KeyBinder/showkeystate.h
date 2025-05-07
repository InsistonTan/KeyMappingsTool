#ifndef SHOWKEYSTATE_H
#define SHOWKEYSTATE_H

#include "BigKey.hpp"
#include <QWidget>


namespace Ui {
class ShowKeyState;
}

class ShowKeyState : public QWidget {
    Q_OBJECT

public:
    explicit ShowKeyState(QWidget* parent = nullptr);
    ~ShowKeyState();
    void initUi();
    void setKeyCount(size_t count); // 设置按键数量

public slots:
    void setKeyState(BigKey& keyState); // 设置按键状态

private:
    Ui::ShowKeyState* ui;
    size_t keyCount = 128; // 按键数量
};

#endif // SHOWKEYSTATE_H
