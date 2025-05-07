#include "showkeystate.h"
#include "ui_showkeystate.h"
#include <QLabel>
#include <QLayout>

ShowKeyState::ShowKeyState(QWidget* parent) : QWidget(parent), ui(new Ui::ShowKeyState) {
    ui->setupUi(this);
}

ShowKeyState::~ShowKeyState() {
    delete ui;
}

void ShowKeyState::setKeyCount(size_t count) {
    if (count < 1 || count > 128) {
        count = 128; // 最大支持128个按键
    }
    keyCount = count; // 按键数量

    initUi(); // 初始化UI
}

void ShowKeyState::initUi() {
    // 清除原先的 label
    QLayoutItem* item;
    while ((item = ui->gridLayout->takeAt(0)) != nullptr) {
        delete item->widget(); // 删除 widget
        delete item;           // 删除布局项
    }

    // 给窗口添加 keyCount 个Label
    for (size_t i = 0; i < keyCount; i++) {
        QLabel* label = new QLabel(this);
        label->setText("按键" + QString::number(i + 1));
        label->setAlignment(Qt::AlignCenter);
        // ui->gridLayout->addWidget(label);
        // 设置为2列布局
        if (i  < keyCount / 2) {
            ui->gridLayout->addWidget(label, i, 0); // 第一列
        } else {
            ui->gridLayout->addWidget(label, i - keyCount / 2, 1); // 第二列
        }
    }
}

void ShowKeyState::setKeyState(BigKey& keyState) {
    // 设置每个 label 的文本为按键状态，按下的颜色设置为红色，未按下的没有颜色
    for (size_t i = 0; i < keyCount; i++) {
        QLabel* label = qobject_cast<QLabel*>(ui->gridLayout->itemAt(i)->widget());
        if (label) {
            if (keyState.getBit(i)) {
                label->setStyleSheet("background-color: red; color: white;"); // 设置背景色为红色，文字颜色为白色
            } else {
                label->setStyleSheet(""); // 清除样式
            }
        }
    }
    
}
