#ifndef MANUALLYBINDER_H
#define MANUALLYBINDER_H

#include "ets2keybinderwizard.h"
#include <QDialog>

namespace Ui {
class ManuallyBinder;
}

class ManuallyBinder : public QDialog {
    Q_OBJECT

public:
    explicit ManuallyBinder(QWidget* parent = nullptr);
    ~ManuallyBinder();
    void setKeyCount(size_t count);

signals:
    void keyBound(BindingType bindingType, ActionEffect actionEffect);

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::ManuallyBinder* ui;

    size_t keyCount = 128;     // 按键数量
    ActionEffect actionEffect; // 动作效果
};

#endif // MANUALLYBINDER_H
