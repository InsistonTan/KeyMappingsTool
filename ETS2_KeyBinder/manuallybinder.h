#ifndef MANUALLYBINDER_H
#define MANUALLYBINDER_H

#include "ets2keybinderwizard.h"
#include <QDialog>
#include <map>

namespace Ui {
class ManuallyBinder;
}

class ManuallyBinder : public QDialog {
    Q_OBJECT

public:
    const std::map<BindingType, QString> bindingTypeComboxString = {
        {BindingType::lightoff, "关闭灯光"},
        {BindingType::lightpark, "示廓灯"},
        {BindingType::lighton, "近光灯"},
        {BindingType::lblinkerh, "左转向灯"},
        {BindingType::rblinkerh, "右转向灯"},
        {BindingType::hblight, "远光灯"},
        {BindingType::lighthorn, "灯光喇叭"},
        {BindingType::wipers0, "雨刷器关闭"},
        {BindingType::wipers1, "雨刷器1档"},
        {BindingType::wipers2, "雨刷器2档"},
        {BindingType::wipers3, "雨刷器3档"},
        {BindingType::wipers4, "雨刷器4档（点动）"},
        {BindingType::gearsel1off, "档位开关1低档位"},
        {BindingType::gearsel1on, "档位开关1高档位"},
        {BindingType::gearsel2off, "档位开关2低档位"},
        {BindingType::gearsel2on, "档位开关2高档位"},
    };

    explicit ManuallyBinder(QWidget* parent = nullptr);
    ~ManuallyBinder();
    void setKeyCount(size_t count);
    void setBindingType(BindingType bindingType);

signals:
    void keyBound(BindingType bindingType, ActionEffect actionEffect);

private slots:
    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

private:
    Ui::ManuallyBinder* ui;

    size_t keyCount = DINPUT_MAX_BUTTONS;     // 按键数量
    ActionEffect actionEffect; // 动作效果
};

#endif // MANUALLYBINDER_H
