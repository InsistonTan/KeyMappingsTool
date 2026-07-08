#pragma once

#include "qmutex.h"
#include "qwidget.h"
#include "ui/widgets/CollapsibleGroupBox.h"
#include "ui/widgets/WinUISwitch.h"
#include <QButtonGroup>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QWidget>

class SettingsPage : public QWidget
{
    Q_OBJECT
public:
    explicit SettingsPage(QWidget *parent = nullptr);

private:
    // ==============================================
    // 记录ui控件指针
    // 生效范围的单选
    QButtonGroup* settingScopegroup;
    QRadioButton* globalSettingsRadioButton;
    QRadioButton* currentMappingRadioButton;
    // 覆盖全局设置-开关
    QWidget* overrideGlobalSettingsWidget;
    WinUISwitch* overrideGlobalSettingsSwitch;
    //软件基础设置
    CollapsibleGroupBox* baseSettingsGroup;
    WinUISwitch* startOnStartupSwitch;
    WinUISwitch* hideWindowOnCloseSwitch;
    //按键映射相关设置
    CollapsibleGroupBox* keyMappingSettingsGroup;
    WinUISwitch* autoMappingSwicth;
    WinUISwitch* deviceNameUniqueModeSwicth;
    WinUISwitch* multiKeyFirstModeSwitch;
    WinUISwitch* addMappingOnlyKeyChangeSwitch;
    //死区设置
    QLineEdit* xboxJoyStickDeadZoneLineEdit;
    QLineEdit* xboxTriggerDeadZoneLineEdit;
    QLineEdit* wheelAxisDeadZoneLineEdit;
    QLineEdit* pedalAxisDeadZoneLineEdit;
    //欧卡2相关设置
    QPushButton* ets2PathSelectBtn;
    QLabel* ets2PathLabel;
    WinUISwitch* autoReleaseHandBrake;
    QPushButton* ets2KeyBinderBtn;
    //其它设置
    QLineEdit* mouseSpeedLineEdit;
    // ==============================================


    // ==============================================
    // 欧卡2的辅助功能任务是否正在运行
    bool isETS2AssitFuncWorkerRunning = false;
    // 欧卡2的辅助功能任务被请求的计数;
    // 如果不为0，说明还有地方需要这个欧卡2的辅助功能任务，不能停止
    int ETS2AssitFuncWorkerCounter = 0;
    // 操作 欧卡2的辅助功能任务 的锁
    QMutex mutex_ETS2AssitFuncWorker;
    // 是否显示全局设置
    bool showGlobalSettings = true;
    // ==============================================


    // 初始化
    void init();
    // 统一绑定控件的事件
    void bindingSiganlsToSlots();
    // 更新ui
    void updateUI();

    // 开启自动解除手刹等 欧卡2的辅助功能任务
    void startETS2AssitFuncWorker();
    // 关闭欧卡2的辅助功能任务
    void stopETS2AssitFuncWorker();

    // 检测欧卡2的共享内存dll插件
    bool checkETS2Plugin();

    // 复制欧卡的插件dll
    bool copyETS2PluginDll(QString ets2Path, QString pluginDllFilePath);

    // 修改设置;
    // 使用template<typename Func>模板, 将lamda表达式作为参数传进来;
    // lamda表达式是具体的修改逻辑
    template<typename Func>
    void modifySetting(Func modifyFunc);

signals:
    // 停止欧卡2的辅助功能任务
    void stopETS2AssitFuncWorkerSignal();

    // 设置改动信号
    void settingsChanged();

public slots:
    // 当前选择的映射配置发生改变
    void currentSelectedMappingFileChangedSlot(){
        updateUI();
    }
};
