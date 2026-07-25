#include "ForceFeedbackSimulatePage.h"
#include "qboxlayout.h"
#include "qlabel.h"
#include "qwidget.h"
#include "common/StringConstants.h"
#include "services/ConfigService.h"
#include "ui/widgets/CurveEditor.h"
#include "ui/widgets/WinUISwitch.h"
#include "workers/ForceFeedbackWorker.h"

#include <QButtonGroup>
#include <QRadioButton>
#include <QScrollArea>
#include <QVBoxLayout>

#include <common/Global.h>
#include <common/Theme.h>

#include <qcheckbox.h>
#include <qlineedit.h>
#include <qmutex.h>
#include <qnamespace.h>
#include <qpushbutton.h>
#include <qthread.h>
#include <qtimer.h>
#include <services/DirectInputService.h>
#include <services/LogService.h>
#include <ui/widgets/CollapsibleGroupBox.h>

ForceFeedbackSimulatePage::ForceFeedbackSimulatePage(QWidget *parent)
    : QWidget{parent}
{
    init();
}

void ForceFeedbackSimulatePage::init()
{
    // 当前页面的根容器布局
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(Theme::contentMargin,Theme::contentMargin,Theme::contentMargin,Theme::contentMargin);
    rootLayout->setSpacing(Theme::contentMargin);

    // ================================
    // 页面标题
    // ================================
    QLabel* title = new QLabel(StringConstants::ffbSimulate, this);
    title->setStyleSheet(R"(
        QLabel{
            font-size: 28px;
            background-color: transparent;
            color: #c0c7d4;
        }
    )");

    // ================================
    // 配置生效范围-单选按钮
    // ================================
    QWidget* settingsScopeWidget = new QWidget(this);
    settingsScopeWidget->setStyleSheet(QString("QWidget{background-color: transparent; color: %1; border-radius: 10px;}").arg(Theme::textColor()));
    QHBoxLayout* settingsScopeWidgetLayout = new QHBoxLayout(settingsScopeWidget);
    settingsScopeWidgetLayout->setContentsMargins(12, 10, 12, 10);
    settingsScopeWidgetLayout->setSpacing(16);
    // 控件
    globalSettingsRadioButton = new QRadioButton(StringConstants::globalSettings);
    currentMappingRadioButton = new QRadioButton(StringConstants::currentMappingFileSettings);
    Theme::setRadioButtonStyleSheet(globalSettingsRadioButton);
    Theme::setRadioButtonStyleSheet(currentMappingRadioButton);
    settingScopegroup = new QButtonGroup(this);
    settingScopegroup->addButton(globalSettingsRadioButton, 0);
    settingScopegroup->addButton(currentMappingRadioButton, 1);
    //默认选中
    globalSettingsRadioButton->setChecked(true);
    // 添加控件到布局
    settingsScopeWidgetLayout->addWidget(globalSettingsRadioButton);
    settingsScopeWidgetLayout->addWidget(currentMappingRadioButton);
    settingsScopeWidgetLayout->addStretch();

    // ================================
    // 是否覆盖全局设置(该组件只应该在选择了"当前映射配置专属设置"才能显示)
    // ================================
    overrideGlobalSettingsSwitch = new WinUISwitch(this);
    overrideGlobalSettingsWidget = Global::createSettingsItem(this,
                                                              StringConstants::overrideGlobalSettings,
                                                              overrideGlobalSettingsSwitch,
                                                              new QLabel(StringConstants::overrideGlobalSettingsDesc, this));


    // ================================
    // 力反馈模拟-开关
    // ================================
    QWidget* ffbSimWidget = new QWidget(this);
    Theme::setQWidgetStyleSheet(ffbSimWidget);
    QHBoxLayout* ffbSimWidgetLayout = new QHBoxLayout(ffbSimWidget);
    ffbSimWidgetLayout->setContentsMargins(16, 12, 16, 12);
    ffbSimWidgetLayout->setSpacing(16);
    // 控件
    QLabel* ffbSimText = new QLabel(StringConstants::ffbSimSwitchText, this);
    ffbSimText->setStyleSheet(QString("font: 14px;color:%1;").arg(Theme::textColor()));
    ffbSimSwitch = new WinUISwitch(this);
    ffbSimWidgetLayout->addWidget(ffbSimText);
    ffbSimWidgetLayout->addStretch();
    ffbSimWidgetLayout->addWidget(ffbSimSwitch);


    // ================================
    // 滚动区域
    // ================================
    QScrollArea* scrollArea = new QScrollArea(this);
    Theme::setScrollBarStyleSheet(scrollArea);
    scrollArea->setWidgetResizable(true);

    // 当前页面的内容容器
    QWidget* contentContainer = new QWidget(this);

    // 设置当前页面的内容容器为实际滚动内容的容器
    scrollArea->setWidget(contentContainer);

    // 当前页面的内容容器布局
    QVBoxLayout* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(12);
    contentLayout->setContentsMargins(0,0,0,0);

    // ================================
    // 设置转向, 油门, 刹车
    // ================================
    //CollapsibleGroupBox* axisSettingsGroup = new CollapsibleGroupBox(StringConstants::axisSettings, this);
    //QVBoxLayout* axisSettingsContentLayout = new QVBoxLayout;
    //axisSettingsGroup->setContentLayout(axisSettingsContentLayout);
    QFrame* axisSettingsGroup = new QFrame(this);
    Theme::setQFrameStyleSheet(axisSettingsGroup);
    QVBoxLayout* axisSettingsContentLayout = new QVBoxLayout(axisSettingsGroup);
    axisSettingsContentLayout->setContentsMargins(16, 12, 0, 12);
    // 控件
    // 标题
    QLabel* axisSettingsGroupTitle = new QLabel(StringConstants::axisSettings, axisSettingsGroup);
    axisSettingsGroupTitle->setStyleSheet(QString("font: 14px;color:%1;").arg(Theme::textColor()));
    // 转向轴设置
    steeringAxisNameLabel = new QLabel(StringConstants::notSet, axisSettingsGroup);
    setSteeringAxisBtn = new QPushButton(StringConstants::setSteringAxisBtn, axisSettingsGroup);
    // 油门轴设置
    throttleAxisNameLabel = new QLabel(StringConstants::notSet, axisSettingsGroup);
    setThrottleAxisBtn = new QPushButton(StringConstants::setThrottleAxisBtn, axisSettingsGroup);
    isReverseThrottleCheckBox = new QCheckBox(StringConstants::rotateAxis, axisSettingsGroup);
    isReverseThrottleCheckBox->setToolTip(StringConstants::reverseAxisTooltip);
    QWidget* throttleActionWidget = new QWidget(axisSettingsGroup);
    QHBoxLayout* throttleActionWidgetLayout = new QHBoxLayout(throttleActionWidget);
    throttleActionWidgetLayout->setContentsMargins(0,0,0,0);
    throttleActionWidgetLayout->setSpacing(8);
    throttleActionWidgetLayout->addWidget(isReverseThrottleCheckBox);
    throttleActionWidgetLayout->addWidget(setThrottleAxisBtn);
    // 刹车轴设置
    brakeAxisNameLabel = new QLabel(StringConstants::notSet, axisSettingsGroup);
    setBrakeAxisBtn = new QPushButton(StringConstants::setBrakeAxisBtn, axisSettingsGroup);
    isReverseBrakeCheckBox = new QCheckBox(StringConstants::rotateAxis, axisSettingsGroup);
    isReverseBrakeCheckBox->setToolTip(StringConstants::reverseAxisTooltip);
    QWidget* brakeActionWidget = new QWidget(axisSettingsGroup);
    QHBoxLayout* brakeActionWidgetLayout = new QHBoxLayout(brakeActionWidget);
    brakeActionWidgetLayout->setContentsMargins(0,0,0,0);
    brakeActionWidgetLayout->setSpacing(8);
    brakeActionWidgetLayout->addWidget(isReverseBrakeCheckBox);
    brakeActionWidgetLayout->addWidget(setBrakeAxisBtn);
    // 样式
    Theme::setButtonStyleSheet(setSteeringAxisBtn, ButtonLevel::normal, "width: 100px;height:32px;");
    Theme::setButtonStyleSheet(setThrottleAxisBtn, ButtonLevel::normal, "width: 100px;height:32px;");
    Theme::setCheckBoxStyleSheet(isReverseThrottleCheckBox);
    Theme::setButtonStyleSheet(setBrakeAxisBtn, ButtonLevel::normal, "width: 100px;height:32px;");
    Theme::setCheckBoxStyleSheet(isReverseBrakeCheckBox);
    // 添加控件
    axisSettingsContentLayout->addWidget(axisSettingsGroupTitle);
    axisSettingsContentLayout->addWidget(Global::createSettingsItem(axisSettingsGroup, StringConstants::steeringAxis, setSteeringAxisBtn, steeringAxisNameLabel));
    axisSettingsContentLayout->addWidget(Global::createSettingsItem(axisSettingsGroup, StringConstants::throttleAxis, throttleActionWidget, throttleAxisNameLabel));
    axisSettingsContentLayout->addWidget(Global::createSettingsItem(axisSettingsGroup, StringConstants::brakeAxis, brakeActionWidget, brakeAxisNameLabel));

    // ================================
    // 设置车辆参数
    // ================================
    // CollapsibleGroupBox* carSettingsGroup = new CollapsibleGroupBox(StringConstants::carParamsSettings, this);
    // QVBoxLayout* carSettingsGroupLayout = new QVBoxLayout;
    // carSettingsGroup->setContentLayout(carSettingsGroupLayout);
    QFrame* carSettingsGroup = new QFrame(this);
    Theme::setQFrameStyleSheet(carSettingsGroup);
    QVBoxLayout* carSettingsGroupLayout = new QVBoxLayout(carSettingsGroup);
    carSettingsGroupLayout->setContentsMargins(16, 12, 0, 12);
    // 控件
    QLabel* carSettingsGroupTitle = new QLabel(StringConstants::carParamsSettings, carSettingsGroup);
    speedUpLineEdit = new QLineEdit(carSettingsGroup);
    speedDownLineEdit = new QLineEdit(carSettingsGroup);
    maxSpeedLineEdit = new QLineEdit(carSettingsGroup);
    // 车速处理类型的单选, 模拟车速/ocr识别车速
    QWidget* carSpeedHandleTypeWidget = new QWidget(this);
    Theme::setQWidgetStyleSheet(carSpeedHandleTypeWidget);
    QHBoxLayout* carSpeedHandleTypeLayout = new QHBoxLayout(carSpeedHandleTypeWidget);
    carSpeedHandleTypeLayout->setSpacing(16);
    simCarSpeedRadioButton = new QRadioButton(StringConstants::simCarSpeed);
    simCarSpeedRadioButton->setChecked(true);
    ocrCarSpeedRadioButton = new QRadioButton(StringConstants::ocrCarSpeed);
    carSpeedHandleTypeSelectgroup = new QButtonGroup(this);
    carSpeedHandleTypeSelectgroup->addButton(simCarSpeedRadioButton, 0);
    carSpeedHandleTypeSelectgroup->addButton(ocrCarSpeedRadioButton, 1);
    carSpeedHandleTypeLayout->addWidget(simCarSpeedRadioButton);
    carSpeedHandleTypeLayout->addWidget(ocrCarSpeedRadioButton);
    carSpeedHandleTypeLayout->addStretch();
    // 设置ocr区域
    setOcrRegionBtn = new QPushButton(StringConstants::settings, this);
    ocrRegionLabel = new QLabel(StringConstants::notSet, this);
    // ocr车速悬浮窗开关
    ocrPreviewSwitch = new WinUISwitch(this);
    // 设置样式
    carSettingsGroupTitle->setStyleSheet(QString("font: 14px;color:%1;").arg(Theme::textColor()));
    Theme::setButtonStyleSheet(setOcrRegionBtn, ButtonLevel::normal, "width: 100px;height:32px;");
    Theme::setLineEditStyleSheet(speedUpLineEdit);
    Theme::setLineEditStyleSheet(speedDownLineEdit);
    Theme::setLineEditStyleSheet(maxSpeedLineEdit);
    Theme::setRadioButtonStyleSheet(simCarSpeedRadioButton);
    Theme::setRadioButtonStyleSheet(ocrCarSpeedRadioButton);
    // 固定输入框的宽度
    int fixedWidth = 130;
    speedUpLineEdit->setFixedWidth(fixedWidth);
    speedDownLineEdit->setFixedWidth(fixedWidth);
    maxSpeedLineEdit->setFixedWidth(fixedWidth);
    setOcrRegionBtn->setFixedWidth(fixedWidth);
    // 布局内容
    carSettingsGroupLayout->addWidget(carSettingsGroupTitle);
    carSettingsGroupLayout->addWidget(carSpeedHandleTypeWidget);

    setOcrRegionWidget = Global::createSettingsItem(carSettingsGroup, StringConstants::setOcrRegion, setOcrRegionBtn, ocrRegionLabel, 20);
    carSettingsGroupLayout->addWidget(setOcrRegionWidget);

    speedUpWidget = Global::createSettingsItem(carSettingsGroup, StringConstants::speedUp100km_h, speedUpLineEdit, new QLabel(StringConstants::speedUpDesc), 20);
    carSettingsGroupLayout->addWidget(speedUpWidget);

    speedDownWidget = Global::createSettingsItem(carSettingsGroup, StringConstants::speedDown100km_h, speedDownLineEdit, new QLabel(StringConstants::speedDownDesc), 20);
    carSettingsGroupLayout->addWidget(speedDownWidget);

    carSettingsGroupLayout->addWidget(Global::createSettingsItem(carSettingsGroup, StringConstants::maxSpeed, maxSpeedLineEdit, new QLabel(StringConstants::maxSpeedDesc), 20));

    ocrPreviewWidget = Global::createSettingsItem(carSettingsGroup, StringConstants::ocrPreviewSwitch, ocrPreviewSwitch, new QLabel(StringConstants::ocrPreviewSwitchDesc), 20);
    carSettingsGroupLayout->addWidget(ocrPreviewWidget);


    // ================================
    // 设置力反馈最大强度
    // ================================
    QFrame* gainSettingsGroup = new QFrame(this);
    Theme::setQFrameStyleSheet(gainSettingsGroup);
    QVBoxLayout* gainSettingsGroupLayout = new QVBoxLayout(gainSettingsGroup);
    gainSettingsGroupLayout->setContentsMargins(16, 12, 0, 12);
    // 控件
    // 标题
    QLabel* gainSettingsGrouppTitle = new QLabel(StringConstants::gainSettings, gainSettingsGroup);
    springGainLineEdit = new QLineEdit(gainSettingsGroup);
    damperGainLineEdit = new QLineEdit(gainSettingsGroup);
    // 设置样式
    gainSettingsGrouppTitle->setStyleSheet(QString("font: 14px;color:%1;").arg(Theme::textColor()));
    Theme::setLineEditStyleSheet(springGainLineEdit);
    Theme::setLineEditStyleSheet(damperGainLineEdit);
    springGainLineEdit->setFixedWidth(fixedWidth);
    damperGainLineEdit->setFixedWidth(fixedWidth);
    // 布局内容
    gainSettingsGroupLayout->addWidget(gainSettingsGrouppTitle);
    gainSettingsGroupLayout->addWidget(Global::createSettingsItem(gainSettingsGroup,
                                                                  StringConstants::springMaxGain,
                                                                  springGainLineEdit,
                                                                  new QLabel(StringConstants::springMaxGainDesc, gainSettingsGroup)));
    gainSettingsGroupLayout->addWidget(Global::createSettingsItem(gainSettingsGroup,
                                                                  StringConstants::damperMaxGain,
                                                                  damperGainLineEdit,
                                                                  new QLabel(StringConstants::sdamperMaxGainDesc, gainSettingsGroup)));


    // ================================
    // 自定义力反馈曲线区域
    // ================================
    QFrame* ffbCurveWidget = new QFrame(this);
    Theme::setQFrameStyleSheet(ffbCurveWidget);
    QVBoxLayout* ffbCurveWidgetLayout = new QVBoxLayout(ffbCurveWidget);
    ffbCurveWidgetLayout->setContentsMargins(16, 10, 12, 10);
    ffbCurveWidgetLayout->setSpacing(12);
    // 控件
    QLabel* ffbCurveWidgetTitle = new QLabel(StringConstants::ffbCurveSettings, ffbCurveWidget);
    ffbCurveWidgetTitle->setStyleSheet(QString("font: 14px;color:%1;").arg(Theme::textColor()));
    QLabel* ffbCurveActionDesc = new QLabel(StringConstants::ffbCurveActionDesc, ffbCurveWidget);
    // 曲线编辑器
    springCurve = new CurveEditor(StringConstants::titleSpeed, StringConstants::titleSpringGain, this);
    dampingCurve = new CurveEditor(StringConstants::titleSpeed, StringConstants::titleDampingGain, this);
    //添加控件
    ffbCurveWidgetLayout->addWidget(ffbCurveWidgetTitle);
    ffbCurveWidgetLayout->addWidget(ffbCurveActionDesc);
    ffbCurveWidgetLayout->addWidget(springCurve);
    ffbCurveWidgetLayout->addWidget(dampingCurve);


    // 力反馈强度曲线区域添加到滚动区域布局
    contentLayout->addWidget(overrideGlobalSettingsWidget);
    contentLayout->addWidget(ffbSimWidget);
    contentLayout->addWidget(axisSettingsGroup);
    contentLayout->addWidget(carSettingsGroup);
    contentLayout->addWidget(gainSettingsGroup);
    contentLayout->addWidget(ffbCurveWidget);


    // 根容器布局添加标题, 生效范围, 滚动区域
    rootLayout->addWidget(title);
    rootLayout->addWidget(settingsScopeWidget);
    rootLayout->addWidget(scrollArea);

    // 绑定事件
    bindingEvents();

    // 延迟更新UI数据
    QTimer::singleShot(1000, this, [this](){
        if(ConfigService::getCurrentMappingConfig().overrideGlobalFFBSettings){
            showGlobalSettings = false;
            currentMappingRadioButton->setChecked(true);
        }

        // 更新ui
        updateUI();
    });
}

template<typename Func>
void ForceFeedbackSimulatePage::modifySetting(Func modifyFunc)
{
    // 是否需要保存到文件
    bool needSaveToFile = false;
    // 是否需要发送设置变更的信号
    bool needSendSignal = false;
    // 当前是否覆盖全局力反馈模拟设置(使用当前映射配置的设置)
    bool isOverride = false;
    {
        // 加锁, 顺序一定不能变, 否则可能死锁
        QMutexLocker lockerCurr(&ConfigService::mutex_currentMappingConfig);
        QMutexLocker lockerGlob(&ConfigService::mutex_globalUserConfig);
        // 根据是否覆盖全局力反馈模拟设置, 获取不同的对象引用
        isOverride = !showGlobalSettings;
        UserConfig& cfg = isOverride ? ConfigService::currentMappingConfig.relatedUserConfig : ConfigService::globalUserConfig;
        // 根据是否覆盖全局设置, 主动释放用不到的锁
        isOverride ? lockerGlob.unlock() : lockerCurr.unlock();

        // 具体的修改逻辑, 由调用方在lamda表达式中对 cfg 进行修改
        modifyFunc(cfg, needSaveToFile, needSendSignal);
    }

    if(needSaveToFile){
        // 保存
        ConfigService::saveToFile(isOverride);
    }
    if(needSendSignal){
        // 发送信号
        emit settingsChanged();
    }
}

void ForceFeedbackSimulatePage::bindingEvents()
{
    // 生效范围
    connect(settingScopegroup, QOverload<int>::of(&QButtonGroup::idClicked),this, [this](int id){
        // id==0 说明选择了全局设置
        bool selectedGlobal = id == 0;

        if(selectedGlobal == false && ConfigService::currentMappingFileName.isEmpty()){
            // 检查当前选择的映射配置是否保存过在本地
            // 如果没有保存过, 提醒用户先去保存
            Global::showErrorMsgBoxAndPushToLog(StringConstants::needSaveMappingFileTip);
            globalSettingsRadioButton->setChecked(true);
            return;
        }

        // 修改
        showGlobalSettings = selectedGlobal;
        // 更新ui
        updateUI();
    });

    // 覆盖全局设置
    connect(overrideGlobalSettingsSwitch, &WinUISwitch::toggled, this, [=](bool checked){
        bool needSaveToFile = false;
        {
            // 加锁 修改
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            if(checked != ConfigService::currentMappingConfig.overrideGlobalFFBSettings){
                ConfigService::currentMappingConfig.overrideGlobalFFBSettings = checked;
                needSaveToFile = true;
            }
        }
        if(needSaveToFile){
            ConfigService::saveCurrentMappingConfigToFile();
            // 发送信号
            emit settingsChanged();
            updateUI();
        }
    });

    // 力反馈模拟服务的开关
    connect(ffbSimSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变, 更新
            if(cfg.SYSTEM_enableForceFeedback != checked){
                cfg.SYSTEM_enableForceFeedback = checked;
                needSaveToFile = true;
            }
        });

        updateUI();
    });

    // 转向
    connect(setSteeringAxisBtn, &QPushButton::clicked, this, [this](){
        // 获取接下来用户操作的设备的轴
        setSteeringAxisBtn->setEnabled(false);
        setSteeringAxisBtn->setText(StringConstants::ffbsettingWindow_BtnText_steeringWheel_waitingForRotate);
        this->repaint();
        // 用户操作的轴
        auto mapping = getDevInputAxis();
        setSteeringAxisBtn->setText(StringConstants::setSteringAxisBtn);
        setSteeringAxisBtn->setEnabled(true);

        // 是否需要更新ui
        bool needUpdateUI = false;
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 用户操作有效
            if(mapping.valid){
                if(!mapping.dev_btn_name.isEmpty()){
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_brakeAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::brake));
                        return;
                    }
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_throttleAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::accelerator));
                        return;
                    }

                    cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis = mapping.dev_btn_name;
                    cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName = mapping.deviceName;
                    needSaveToFile = true;
                    needSendSignal = true;
                    needUpdateUI = true;
                }
            }else{
                Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbsettingWindow_Error_steeringWheelNotConnect);
                cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis = "";
                cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName = "";
                needSaveToFile = true;
                needSendSignal = true;
                needUpdateUI = true;
            }
        });

        if(needUpdateUI){
            updateUI();
        }
    });

    // 油门
    connect(setThrottleAxisBtn, &QPushButton::clicked, this, [this](){
        // 获取接下来用户操作的设备的轴
        setThrottleAxisBtn->setEnabled(false);
        setThrottleAxisBtn->setText(StringConstants::ffbsettingWindow_BtnText_thottle_waitingForRotate);
        this->repaint();
        // 用户操作的轴
        auto mapping = getDevInputAxis();
        setThrottleAxisBtn->setText(StringConstants::setThrottleAxisBtn);
        setThrottleAxisBtn->setEnabled(true);

        // 是否需要更新ui
        bool needUpdateUI = false;
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 用户操作有效
            if(mapping.valid){
                if(!mapping.dev_btn_name.isEmpty()){
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_brakeAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::brake));
                        return;
                    }
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::steeringWheelAxis));
                        return;
                    }

                    cfg.SYSTEM_forceFeedbackSettings_throttleAxis = mapping.dev_btn_name;
                    cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName = mapping.deviceName;
                    // 设置是否反转该轴
                    cfg.SYSTEM_forceFeedbackSettings_isThrottleReverse = (mapping.rotateAxis == 1);
                    needSaveToFile = true;
                    needSendSignal = true;
                    needUpdateUI = true;
                }
            }else{
                Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbsettingWindow_Error_thottleNotConnect);
                cfg.SYSTEM_forceFeedbackSettings_throttleAxis = "";
                cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName = "";
                cfg.SYSTEM_forceFeedbackSettings_isThrottleReverse = false;
                needSaveToFile = true;
                needSendSignal = true;
                needUpdateUI = true;
            }
        });

        if(needUpdateUI){
            updateUI();
        }
    });
    connect(isReverseThrottleCheckBox, &QCheckBox::clicked, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            if(cfg.SYSTEM_forceFeedbackSettings_isThrottleReverse != checked){
                cfg.SYSTEM_forceFeedbackSettings_isThrottleReverse = checked;
                needSaveToFile = true;
                needSendSignal = true;
            }
        });
    });

    // 刹车
    connect(setBrakeAxisBtn, &QPushButton::clicked, this, [this](){
        // 获取接下来用户操作的设备的轴
        setBrakeAxisBtn->setEnabled(false);
        setBrakeAxisBtn->setText(StringConstants::ffbsettingWindow_BtnText_brake_waitingForRotate);
        this->repaint();
        auto mapping = getDevInputAxis();
        setBrakeAxisBtn->setText(StringConstants::setBrakeAxisBtn);
        setBrakeAxisBtn->setEnabled(true);

        // 是否需要更新ui
        bool needUpdateUI = false;
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 用户操作有效
            if(mapping.valid){
                if(!mapping.dev_btn_name.isEmpty()){
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_throttleAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::accelerator));
                        return;
                    }
                    if(mapping.deviceName == cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName
                        && mapping.dev_btn_name == cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis){
                        MessageBoxService::showError(StringConstants::error_thisAxisAlreadySet.arg(StringConstants::steeringWheelAxis));
                        return;
                    }
                    cfg.SYSTEM_forceFeedbackSettings_brakeAxis = mapping.dev_btn_name;
                    cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName = mapping.deviceName;
                    cfg.SYSTEM_forceFeedbackSettings_isBrakeReverse = mapping.rotateAxis == 1;

                    needSaveToFile = true;
                    needSendSignal = true;
                    needUpdateUI = true;
                }
            }else{
                Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbsettingWindow_Error_brakeNotConnect);
                cfg.SYSTEM_forceFeedbackSettings_brakeAxis = "";
                cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName = "";
                cfg.SYSTEM_forceFeedbackSettings_isBrakeReverse = false;

                needSaveToFile = true;
                needSendSignal = true;
                needUpdateUI = true;
            }
        });

        if(needUpdateUI){
            updateUI();
        }
    });
    connect(isReverseBrakeCheckBox, &QCheckBox::clicked, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            if(cfg.SYSTEM_forceFeedbackSettings_isBrakeReverse != checked){
                cfg.SYSTEM_forceFeedbackSettings_isBrakeReverse = checked;
                needSaveToFile = true;
                needSendSignal = true;
            }
        });
    });

    // 百公里加速
    connect(speedUpLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 检验输入的值
            bool ok;
            double value = speedUpLineEdit->text().toDouble(&ok);
            if(ok && value > 0){
                cfg.SYSTEM_forceFeedbackSettings_acceleration_100km_time_s = value;
                needSaveToFile = true;
                needSendSignal = true;
            }else{
                speedUpLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_acceleration_100km_time_s));
                Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            }
        });
    });

    // 百公里刹停
    connect(speedDownLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 检验输入的值
            bool ok;
            double value = speedDownLineEdit->text().toDouble(&ok);
            if(ok && value > 0){
                cfg.SYSTEM_forceFeedbackSettings_stop_100km_dis_m = value;
                needSaveToFile = true;
                needSendSignal = true;
            }else{
                speedDownLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_stop_100km_dis_m));
                Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            }
        });
    });

    // 极速
    connect(maxSpeedLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 检验输入的值
            bool ok;
            double value = maxSpeedLineEdit->text().toDouble(&ok);
            if(ok && value > 0){
                cfg.SYSTEM_forceFeedbackSettings_maxSpeed_km_h = value;
                needSaveToFile = true;
                needSendSignal = true;
            }else{
                maxSpeedLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxSpeed_km_h));
                Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            }
        });
    });


    // 回正力最大强度
    connect(springGainLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 检验输入的值
            bool ok;
            double value = springGainLineEdit->text().toDouble(&ok);
            if(ok && value >= 0.0 && value <= 1.0){
                cfg.SYSTEM_forceFeedbackSettings_maxSpringGain = value;
                needSaveToFile = true;
                needSendSignal = true;
            }else{
                springGainLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxSpringGain));
                Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            }
        });
    });

    // 回正力最大强度
    connect(damperGainLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 检验输入的值
            bool ok;
            double value = damperGainLineEdit->text().toDouble(&ok);
            if(ok && value >= 0.0 && value <= 1.0){
                cfg.SYSTEM_forceFeedbackSettings_maxDamperGain = value;
                needSaveToFile = true;
                needSendSignal = true;
            }else{
                damperGainLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxDamperGain));
                Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            }
        });
    });


    // 回正力强度曲线
    connect(springCurve, &CurveEditor::pointChanged, this, [this](){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 获取点的列表
            auto points = springCurve->getPoints();
            // 保存
            cfg.SYSTEM_forceFeedbackSettings_springCurve = points;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });

    // 转向阻尼强度曲线
    connect(dampingCurve, &CurveEditor::pointChanged, this, [this](){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 获取点的列表
            auto points = dampingCurve->getPoints();
            // 保存
            cfg.SYSTEM_forceFeedbackSettings_dampingCurve = points;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });

    // 车速处理类型选择, 模拟车速/ocr识别车速
    connect(carSpeedHandleTypeSelectgroup, QOverload<int>::of(&QButtonGroup::idClicked),this, [this](int id){
        bool enableOcr = (id == 1);
        bool needUpdateUi = false;

        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            if(enableOcr != cfg.SYSTEM_forceFeedbackSettings_enableOcr){
                cfg.SYSTEM_forceFeedbackSettings_enableOcr = enableOcr;
                needSaveToFile = true;
                needSendSignal = true;
                needUpdateUi = true;
            }
        });

        if(needUpdateUi){
            // 更新ui
            updateUI();
        }
    });

    // 设置ocr识别区域的按钮
    connect(setOcrRegionBtn, &QPushButton::clicked, this, [this](){
        // 显示ocr识别区域窗口
        if(ocrRegionWindow.isHidden()){
            // ocr识别区域窗口的位置和大小
            auto cfg = ConfigService::get();
            if(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_width != 0
                    && cfg.SYSTEM_forceFeedbackSettings_ocrRegion_height != 0){
                ocrRegionWindow.setGeo(
                    cfg.SYSTEM_forceFeedbackSettings_ocrRegion_x,
                    cfg.SYSTEM_forceFeedbackSettings_ocrRegion_y,
                    cfg.SYSTEM_forceFeedbackSettings_ocrRegion_width,
                    cfg.SYSTEM_forceFeedbackSettings_ocrRegion_height
                );
            }

            ocrRegionWindow.show();
            setOcrRegionBtn->setText(StringConstants::setOcrRegionBtnConfirmText);
            return;
        }

        // 获取窗口的位置大小
        QScreen* screen = ocrRegionWindow.screen();
        // 缩放比例
        auto scale = screen->devicePixelRatio();

        // 窗口所在屏幕名称
        //QString screenName = screen->name();

        // 窗口位置和大小
        int x = int(ocrRegionWindow.x() * scale);
        int y = int(ocrRegionWindow.y() * scale);
        int w = int(ocrRegionWindow.width() * scale);
        int h = int(ocrRegionWindow.height() * scale);

        // 区域的宽度和高度不能为0
        if(w == 0 || h == 0){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ocrRegionTooSmall);
            return;
        }

        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            //cfg.SYSTEM_forceFeedbackSettings_ocrScreenName = screenName;
            cfg.SYSTEM_forceFeedbackSettings_ocrRegion_x = x;
            cfg.SYSTEM_forceFeedbackSettings_ocrRegion_y = y;
            cfg.SYSTEM_forceFeedbackSettings_ocrRegion_width = w;
            cfg.SYSTEM_forceFeedbackSettings_ocrRegion_height = h;

            needSaveToFile = true;
            needSendSignal = true;
        });

        // 隐藏窗口
        ocrRegionWindow.hide();
        setOcrRegionBtn->setText(StringConstants::settings);

        // 更新UI
        updateUI();
    });

    // ocr车速悬浮窗开关
    connect(ocrPreviewSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变, 更新
            if(cfg.SYSTEM_forceFeedbackSettings_enableOcrPreview != checked){
                cfg.SYSTEM_forceFeedbackSettings_enableOcrPreview = checked;
                needSaveToFile = true;
                needSendSignal = true;
            }
        });
    });
}

MappingRelation ForceFeedbackSimulatePage::getDevInputAxis()
{
    // 扫描设备
    DirectInputService::scanDevice();

    // 连接当前设备列表中的所有设备
    DirectInputService::openDiDevice(DirectInputService::getDeviceNameListFromDeviceInfoList(), false);

    std::map<std::string, int> tempRecord;

    bool enableLogs = Global::getEnablePovLog() || Global::getEnableBtnLog() || Global::getEnableAxisLog();

    // 获取设备操作的轴
    auto item = DirectInputService::getNextActionBtnOrAxis(DirectInputService::getInitedDeviceListSnapshot(), false, enableLogs, false, true);

    // 方向盘的轴
    if(item.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS){
        // 如果值由大变小, 说明这个轴是反的, 需要反转
        if(item.axisValueChange < 0){
            item.rotateAxis = 1;
        }

        return item;
    }

    // 返回无效对象 valid = false
    return MappingRelation(false);
}

void ForceFeedbackSimulatePage::startForceFeedback()
{
    // 不需要重复创建力反馈模拟线程
    if(isFFBSimRunning){
        return;
    }

    ForceFeedbackWorker *worker = new ForceFeedbackWorker();
    QThread *thread = new QThread;
    worker->moveToThread(thread);

    // 连接信号槽
    connect(thread, &QThread::started, worker, &ForceFeedbackWorker::doWork);

    // 绑定力反馈开启是否成功的信号
    connect(worker, &ForceFeedbackWorker::startFFBSimResult,
            this, &ForceFeedbackSimulatePage::startFFBSimResultSlot);

    // 绑定力反馈模拟设置更新信号 到 力反馈工作对象的 settingsChangeSlot
    connect(this, &ForceFeedbackSimulatePage::settingsChanged,
            worker, &ForceFeedbackWorker::settingsChangeSlot);

    // 停止任务信号
    connect(this, &ForceFeedbackSimulatePage::stopForceFeedbackSignal,
            worker, &ForceFeedbackWorker::cancelWorkSlot);

    // 任务结束信号
    connect(worker, &ForceFeedbackWorker::workFinished, this, [this]{
        LogService::parseWarningLog(StringConstants::ffbSimulateThread_finishedMsg);
        ffbSimSwitch->setChecked(false);
        updateUI();
    });
    // 显示/隐藏ocr预览
    connect(worker, &ForceFeedbackWorker::enableOcrPreviewWindow,
            this, &ForceFeedbackSimulatePage::enableOcrPreviewWindow);
    // 更新ocr结果
    connect(worker, &ForceFeedbackWorker::updateOcrResultToPreviewWindow,
            this, &ForceFeedbackSimulatePage::updateOcrResultToPreviewWindow);

    connect(worker, &ForceFeedbackWorker::workFinished, thread, &QThread::quit);
    connect(worker, &ForceFeedbackWorker::workFinished, worker, &ForceFeedbackWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

    isFFBSimRunning = true;
}

void ForceFeedbackSimulatePage::updateUI()
{
    // 显示: 当前映射配置 "aaa" 的专属设置
    if(ConfigService::currentMappingFileName.isEmpty() == false){
        currentMappingRadioButton->setText(StringConstants::currentMappingFileSettings2.arg(ConfigService::currentMappingFileName));
    }else{
        currentMappingRadioButton->setText(StringConstants::currentMappingFileSettings);
    }

    // 判断当前是否是映射配置专属设置(覆盖全局)
    auto mappingCfg = ConfigService::getCurrentMappingConfig();
    overrideGlobalSettingsSwitch->setChecked(mappingCfg.overrideGlobalFFBSettings);

    // 获取用户配置, 根据"是否显示全局设置", 获取不同的配置对象
    auto cfg = showGlobalSettings ? ConfigService::getGlobalUserConfig() : mappingCfg.relatedUserConfig;

    // 是否开启力反馈模拟
    ffbSimSwitch->setChecked(cfg.SYSTEM_enableForceFeedback);

    // 转向轴名称
    QString s1 = (!cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis.isEmpty()
                  && !cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName.isEmpty())
                        ? cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName + ": "
                                + cfg.SYSTEM_forceFeedbackSettings_steeringWheelAxis
                        : StringConstants::notSet;
    steeringAxisNameLabel->setText(s1);

    // 油门轴名称
    QString s2 = (!cfg.SYSTEM_forceFeedbackSettings_throttleAxis.isEmpty()
                  && !cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName.isEmpty())
                     ? cfg.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName + ": "
                           + cfg.SYSTEM_forceFeedbackSettings_throttleAxis
                     : StringConstants::notSet;
    throttleAxisNameLabel->setText(s2);
    isReverseThrottleCheckBox->setChecked(cfg.SYSTEM_forceFeedbackSettings_isThrottleReverse);

    // 刹车轴名称
    QString s3 = (!cfg.SYSTEM_forceFeedbackSettings_brakeAxis.isEmpty()
                  && !cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName.isEmpty())
                     ? cfg.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName + ": "
                           + cfg.SYSTEM_forceFeedbackSettings_brakeAxis
                     : StringConstants::notSet;
    brakeAxisNameLabel->setText(s3);
    isReverseBrakeCheckBox->setChecked(cfg.SYSTEM_forceFeedbackSettings_isBrakeReverse);

    // 车辆参数
    speedUpLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_acceleration_100km_time_s));
    speedDownLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_stop_100km_dis_m));
    maxSpeedLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxSpeed_km_h));

    // 最大强度
    springGainLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxSpringGain));
    damperGainLineEdit->setText(QString::number(cfg.SYSTEM_forceFeedbackSettings_maxDamperGain));

    // 力反馈强度曲线
    springCurve->setPoints(cfg.SYSTEM_forceFeedbackSettings_springCurve);
    dampingCurve->setPoints(cfg.SYSTEM_forceFeedbackSettings_dampingCurve);

    // 车速处理的单选, 模拟车速/ocr识别车速
    if(cfg.SYSTEM_forceFeedbackSettings_enableOcr){
        ocrCarSpeedRadioButton->setChecked(true);
    }else{
        simCarSpeedRadioButton->setChecked(true);
    }

    // ocr识别区域信息
    if(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_width == 0 || cfg.SYSTEM_forceFeedbackSettings_ocrRegion_height == 0){
        ocrRegionLabel->setText(StringConstants::notSet);
    }else{
        QString ocrRegionLabelText;
        ocrRegionLabelText.append(StringConstants::ocrRegionLabelText.arg(
            QString::number(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_x),
            QString::number(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_y),
            QString::number(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_width),
            QString::number(cfg.SYSTEM_forceFeedbackSettings_ocrRegion_height)
        ));

        ocrRegionLabel->setText(ocrRegionLabelText);
    }

    // ocr车速悬浮窗开关
    ocrPreviewSwitch->setChecked(cfg.SYSTEM_forceFeedbackSettings_enableOcrPreview);


    // ===============================================================
    // 隐藏部分组件
    // ===============================================================
    if(showGlobalSettings){
        overrideGlobalSettingsWidget->hide();
    }else{
        overrideGlobalSettingsWidget->show();
    }

    if(cfg.SYSTEM_forceFeedbackSettings_enableOcr){
        setOcrRegionWidget->show();
        ocrPreviewWidget->show();
        speedUpWidget->hide();
        speedDownWidget->hide();
    }else{
        setOcrRegionWidget->hide();
        ocrPreviewWidget->hide();
        speedUpWidget->show();
        speedDownWidget->show();
    }


    // ===============================================================
    // 下面是一些后台任务的状态更新
    // ===============================================================

    // 当前是否覆盖全局设置, 覆盖则使用专属设置, 否则使用全局设置
    auto ffbCfg = ConfigService::get(ConfigService::GetSource::FFBSim);
    // 力反馈模拟状态更新
    if(ffbCfg.SYSTEM_enableForceFeedback){
        startForceFeedback();
    }else{
        isFFBSimRunning = false;
        emit stopForceFeedbackSignal();
    }
}

void ForceFeedbackSimulatePage::currentSelectedMappingFileChangedSlot()
{
    if(ConfigService::getCurrentMappingConfig().overrideGlobalFFBSettings){
        showGlobalSettings = false;
        currentMappingRadioButton->setChecked(true);
    }else{
        showGlobalSettings = true;
        globalSettingsRadioButton->setChecked(true);
    }
    updateUI();
}

void ForceFeedbackSimulatePage::startFFBSimResultSlot(bool result, QString msg)
{
    // 开启力反馈模拟失败
    if(result == false){
        ffbSimSwitch->setChecked(false);
        modifySetting([&](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.SYSTEM_enableForceFeedback = false;
            needSaveToFile = true;
        });
        updateUI();
    }

}




