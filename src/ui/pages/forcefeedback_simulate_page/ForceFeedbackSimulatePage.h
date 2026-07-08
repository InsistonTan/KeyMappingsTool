#pragma once

#include "models/MappingRelation.h"
#include "models/UserConfig.h"
#include "ui/widgets/CurveEditor.h"
#include "ui/widgets/WinUISwitch.h"
#include <QWidget>

class ForceFeedbackSimulatePage : public QWidget
{
    Q_OBJECT
public:
    explicit ForceFeedbackSimulatePage(QWidget *parent = nullptr);

    void init();

private:
    // 生效范围的单选
    QButtonGroup* settingScopegroup;
    QRadioButton* globalSettingsRadioButton;
    QRadioButton* currentMappingRadioButton;
    // 覆盖全局设置-开关
    QWidget* overrideGlobalSettingsWidget;
    WinUISwitch* overrideGlobalSettingsSwitch;
    // 开启/关闭力反馈模拟
    WinUISwitch* ffbSimSwitch;
    // 转向
    QLabel* steeringAxisNameLabel;
    QPushButton* setSteeringAxisBtn;
    // 油门
    QLabel* throttleAxisNameLabel;
    QPushButton* setThrottleAxisBtn;
    QCheckBox* isReverseThrottleCheckBox;
    // 刹车
    QLabel* brakeAxisNameLabel;
    QPushButton* setBrakeAxisBtn;
    QCheckBox* isReverseBrakeCheckBox;
    // 车辆参数
    QLineEdit* speedUpLineEdit;
    QLineEdit* speedDownLineEdit;
    QLineEdit* maxSpeedLineEdit;
    // 回正力-车速曲线
    CurveEditor* springCurve;
    // 转向阻尼-车速曲线
    CurveEditor* dampingCurve;

    // 是否显示全局设置
    bool showGlobalSettings = true;

    // 默认的回正力-车速曲线
    QVector<CurveEditor::BezierLogicalPoint> defaultSpringPoints = {
        {
            {0,0}, // 主点
            {0-CurveEditor::dis, 0}, // in
            {0+CurveEditor::dis, 0}, // out
            false, // 不显示 in
            true // 显示 out
        },
        {
            {100,100}, // 主点
            {100-CurveEditor::dis, 100}, // in
            {100+CurveEditor::dis, 100}, // out
            true, // 显示 in
            false // 不显示 out
        },
    };
    // 默认的转向阻尼-车速曲线
    QVector<CurveEditor::BezierLogicalPoint> defaultDampingPoints = {
        {
            {0,0}, // 主点
            {0-CurveEditor::dis, 0}, // in
            {0+CurveEditor::dis, 0}, // out
            false, // 不显示 in
            true // 显示 out
        },
        {
            {100,100}, // 主点
            {100-CurveEditor::dis, 100}, // in
            {100+CurveEditor::dis, 100}, // out
            true, // 显示 in
            false // 不显示 out
        },
    };

    // 力反馈模拟当前是否正在运行
    bool isFFBSimRunning = false;

    // 绑定控件的事件
    void bindingEvents();

    // 识别被踩下的轴
    MappingRelation getDevInputAxis();

    // 校验力反馈参数
    bool validateForceFeedbackParams(const UserConfig& userConfig);

    // 开启力反馈模拟任务
    void startForceFeedback();

    // 更新ui
    void updateUI();

    // 修改设置;
    // 使用template<typename Func>模板, 将lamda表达式作为参数传进来;
    // lamda表达式是具体的修改逻辑
    template<typename Func>
    void modifySetting(Func modifyFunc);

signals:
    // 设置发生改动信号
    void settingsChanged();
    // 停止力反馈模拟信号
    void stopForceFeedbackSignal();

public slots:
    // 当前选择的映射配置发生改变
    void currentSelectedMappingFileChangedSlot(){
        updateUI();
    }
};
