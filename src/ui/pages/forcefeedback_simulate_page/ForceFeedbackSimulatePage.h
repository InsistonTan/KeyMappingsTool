#pragma once

#include "models/MappingRelation.h"
#include "ocr/ui/CaptureRegionWindow.h"
#include "ocr/ui/OcrPreviewWindow.h"
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
    // 最大力反馈强度
    QLineEdit* springGainLineEdit;
    QLineEdit* damperGainLineEdit;
    // 模拟车速/ocr识别游戏车速 的单选
    QButtonGroup* carSpeedHandleTypeSelectgroup;
    QRadioButton* simCarSpeedRadioButton;
    QRadioButton* ocrCarSpeedRadioButton;
    // 设置ocr区域
    QPushButton* setOcrRegionBtn;
    QLabel* ocrRegionLabel;
    QWidget* setOcrRegionWidget;
    // ocr车速悬浮窗开关
    WinUISwitch* ocrPreviewSwitch;
    QWidget* ocrPreviewWidget;
    // 车辆参数
    QLineEdit* speedUpLineEdit;
    QWidget* speedUpWidget;
    QLineEdit* speedDownLineEdit;
    QWidget* speedDownWidget;
    QLineEdit* maxSpeedLineEdit;
    // 回正力-车速曲线
    CurveEditor* springCurve;
    // 转向阻尼-车速曲线
    CurveEditor* dampingCurve;

    // 是否显示全局设置
    bool showGlobalSettings = true;

    // 力反馈模拟当前是否正在运行
    bool isFFBSimRunning = false;

    // 设置ocr识别区域的窗口
    CaptureRegionWindow ocrRegionWindow;
    // ocr实时预览悬浮窗
    OcrPreviewWindow ocrPreviewWindow;

private:

    // 绑定控件的事件
    void bindingEvents();

    // 识别被踩下的轴
    MappingRelation getDevInputAxis();

    // 校验力反馈参数
    //bool validateForceFeedbackParams(const UserConfig& userConfig);

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
    void currentSelectedMappingFileChangedSlot();

    // 开启力反馈模拟是否成功的slot
    void startFFBSimResultSlot(bool result, QString msg);

    // 显示/隐藏 ocr预览窗口
    void enableOcrPreviewWindow(bool enable){
        if(enable){
            ocrPreviewWindow.show();
        }else{
            ocrPreviewWindow.hide();
        }
    }

    // 更新ocr结果到预览窗口
    void updateOcrResultToPreviewWindow(QString val){
        ocrPreviewWindow.updateOcrPreview(val);
    }
};
