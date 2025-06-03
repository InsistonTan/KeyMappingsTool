#ifndef ETS2KEYBINDERWIZARD_H
#define ETS2KEYBINDERWIZARD_H

// 参考开源项目：https://github.com/Sab1e-GitHub/ETS2-KeyBinder
// 参考开源项目：https://github.com/InsistonTan/KeyMappingsTool

#include "BigKey.hpp"
#include "global.h"
#include "showkeystate.h"
#include <QDir>
#include <QStandardPaths>
#include <QWizard>
#include <dinput.h>
#include <string.h>
#include <windows.h>

// 枚举类型定义
enum class BindingType
{
    lightoff,    // 关闭灯光
    lightpark,   // 示廓灯
    lighton,     // 近光灯
    lblinkerh,   // 左转向灯
    rblinkerh,   // 右转向灯
    hblight,     // 远光灯
    lighthorn,   // 灯光喇叭
    wipers0,     // 雨刷器关闭
    wipers1,     // 雨刷器1档
    wipers2,     // 雨刷器2档
    wipers3,     // 雨刷器3档
    wipers4,     // 雨刷器点动
    gearsel1off, // 档位开关1关闭
    gearsel1on,  // 档位开关1打开
    gearsel2off, // 档位开关2关闭
    gearsel2on,  // 档位开关2打开
};

typedef std::map<int, bool> ActionEffect; // 受影响的按键位置和对应的新状态

namespace Ui {
class ETS2KeyBinderWizard;
}

class ETS2KeyBinderWizard : public QWizard {
    Q_OBJECT

public:
    explicit ETS2KeyBinderWizard(QWidget* parent = nullptr);
    ~ETS2KeyBinderWizard();

    QStringList getDeviceNameGameList();

    QString convertToETS2_String(const QString& gameJoyPosStr, const ActionEffect& actionEffect, size_t maxButtonCount = DINPUT_MAX_BUTTONS);
    QString convertToUiString(const QString& ets2BtnStr);

    void oneKeyBind(BindingType bindingType, const QString& message);
    void multiKeyBind(std::map<BindingType, ActionEffect> actionEffectMap);

    std::vector<BigKey> getMultiKeyState(const QString& title, const QStringList& messages);

signals:
    void keyStateUpdate_Signal(BigKey& keyState); // 更新按键状态信号

public slots:
    void modifyControlsSii_Slot(BindingType bindingType, ActionEffect actionEffect);

private slots:

    void on_comboBox_activated(int index);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_pushButton_3_clicked();

    void on_pushButton_4_clicked();

    void on_comboBox_3_activated(int index);

    void on_comboBox_2_activated(int index);

    void on_pushButton_16_clicked();

    void on_pushButton_5_clicked();

    void on_pushButton_6_clicked();

    void on_pushButton_7_clicked();

    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void on_pushButton_10_clicked();

    void on_pushButton_11_clicked();

    void on_pushButton_12_clicked();

    void on_pushButton_13_clicked();

    void on_pushButton_14_clicked();

    void on_pushButton_15_clicked();

    void on_pushButton_17_clicked();

    void on_pushButton_18_clicked();

    void on_pushButton_19_clicked();

    void on_pushButton_20_clicked();

    void on_pushButton_21_clicked();

    void on_pushButton_22_clicked();

    void on_pushButton_23_clicked();

    void on_pushButton_24_clicked();

private:
    Ui::ETS2KeyBinderWizard* ui;

    std::string deviceName;     // 硬件设备名称
    std::string gameDeviceName; // 游戏输入设备名称

    QStringList gameJoyPosNameList = {
        "joy ", "joy2", "joy3", "joy4", "joy5",
    };
    QString steamProfiles[2] = {QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Euro Truck Simulator 2/steam_profiles",
                                QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/American Truck Simulator/steam_profiles"};
    QString profiles[2] = {QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/Euro Truck Simulator 2/profiles",
                           QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/American Truck Simulator/profiles"};
    QString selectedProfilePath; // 选择的配置文件路径

    QList<QPair<QString, QDateTime>> steamProfileFolders;
    QList<QPair<QString, QDateTime>> profileFolders; // 所有配置文件列表

    std::map<BindingType, QPushButton*> uiBtnMap;

    LPDIRECTINPUT8 pDirectInput = NULL;
    LPDIRECTINPUTDEVICE8 pDevice = NULL;
    int lastDeviceIndex = -99;
    DIDEVCAPS capabilities;
    bool isDeviceReady = false; // 设备是否准备好

    ShowKeyState* showKeyState = nullptr; // 显示按键状态窗口
    QTimer* timer = nullptr;              // 定时器

    DIJOYSTATE2 getInputState();
    BigKey getKeyState();

    bool hasLastDevInCurrentDeviceList(std::string lastDeviceName);
    bool openDiDevice(int deviceIndex, HWND hWnd);
    void scanDevice();
    bool initDirectInput();
    bool checkHardwareDeviceAndMsgBox(BindingType bindingType = BindingType::lightpark);

    void updateUserProfile();
    bool backupProfile();

    bool generateMappingFile(ActionEffect hblight, ActionEffect lighthorn);

    void readControlsSii(const QString& controlsFilePath);

    void modifyControlsSii(const QString& controlsFilePath, BindingType bindingType, const QString& ets2BtnStr);

    void showManuallyBinder(BindingType bindingType);
};

#endif // ETS2KEYBINDERWIZARD_H
