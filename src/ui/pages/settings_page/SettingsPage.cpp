#include "SettingsPage.h"
#include "ui/ETS2_KeyBinder/Ets2KeyBinderWizard.h"
#include "qboxlayout.h"
#include "qcoreapplication.h"
#include "qlineedit.h"
#include "qmutex.h"
#include "qpushbutton.h"
#include "qscrollarea.h"
#include "qwidget.h"
#include "common/Global.h"
#include "common/StringConstants.h"
#include "models/UserConfig.h"
#include "services/ConfigService.h"
#include "ui/widgets/CollapsibleGroupBox.h"
#include "ui/widgets/WinUISwitch.h"
#include "workers/AssistFuncWorker.h"

#include <common/Theme.h>

#include <QFileDialog>
#include <QGroupBox>
#include <QThread>
#include <QTimer>
#include <qlabel.h>
#include <winsock.h>

#include <services/LogService.h>

SettingsPage::SettingsPage(QWidget *parent)
    : QWidget{parent}
{
    init();
}

void SettingsPage::init()
{
    // 当前页面的根容器布局
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(Theme::contentMargin,Theme::contentMargin,Theme::contentMargin,Theme::contentMargin);
    rootLayout->setSpacing(Theme::contentMargin);

    // 滚动区域
    QScrollArea* scrollArea = new QScrollArea(this);
    Theme::setScrollBarStyleSheet(scrollArea);
    scrollArea->setWidgetResizable(true);

    // 当前页面的内容容器
    QWidget* contentContainer = new QWidget(this);

    // 设置当前页面的内容容器为实际滚动内容的容器
    scrollArea->setWidget(contentContainer);

    // 当前页面的内容容器布局
    QVBoxLayout* contentLayout = new QVBoxLayout(contentContainer);
    contentLayout->setSpacing(16);
    contentLayout->setContentsMargins(0,0,0,0);

    // ================================
    // 页面标题
    // ================================
    QLabel* title = new QLabel(StringConstants::settings, this);
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


    // =====================================================================================
    // 软件基础设置
    // =====================================================================================
    baseSettingsGroup = new CollapsibleGroupBox(StringConstants::baseSettings, this);
    QVBoxLayout* baseSettingsGroupLayout = new QVBoxLayout;
    baseSettingsGroup->setContentLayout(baseSettingsGroupLayout);
    // 控件
    startOnStartupSwitch = new WinUISwitch(baseSettingsGroup);
    hideWindowOnCloseSwitch = new WinUISwitch(baseSettingsGroup);
    // 布局内容
    baseSettingsGroupLayout->addWidget(Global::createSettingsItem(baseSettingsGroup, StringConstants::startOnStartup, startOnStartupSwitch));
    baseSettingsGroupLayout->addWidget(Global::createSettingsItem(baseSettingsGroup, StringConstants::hideWindowOnClose, hideWindowOnCloseSwitch));


    // =====================================================================================
    // 按键映射相关设置
    // =====================================================================================
    keyMappingSettingsGroup = new CollapsibleGroupBox(StringConstants::keyMappingSettings, this);
    QVBoxLayout* keyMappingSettingsGroupLayout = new QVBoxLayout;
    keyMappingSettingsGroup->setContentLayout(keyMappingSettingsGroupLayout);
    // 控件
    autoMappingSwicth = new WinUISwitch(keyMappingSettingsGroup);
    deviceNameUniqueModeSwicth = new WinUISwitch(keyMappingSettingsGroup);
    multiKeyFirstModeSwitch = new WinUISwitch(keyMappingSettingsGroup);
    addMappingOnlyKeyChangeSwitch = new WinUISwitch(keyMappingSettingsGroup);
    // 布局内容
    keyMappingSettingsGroupLayout->addWidget(Global::createSettingsItem(keyMappingSettingsGroup, StringConstants::autoMapping, autoMappingSwicth, new QLabel(StringConstants::autoMappingDesc)));
    keyMappingSettingsGroupLayout->addWidget(Global::createSettingsItem(keyMappingSettingsGroup, StringConstants::deviceNameUniqueMode, deviceNameUniqueModeSwicth, new QLabel(StringConstants::deviceNameUniqueModeDesc)));
    keyMappingSettingsGroupLayout->addWidget(Global::createSettingsItem(keyMappingSettingsGroup, StringConstants::multiKeyFirstMode, multiKeyFirstModeSwitch, new QLabel(StringConstants::multiKeyFirstModeDesc)));
    keyMappingSettingsGroupLayout->addWidget(Global::createSettingsItem(keyMappingSettingsGroup, StringConstants::addMappingOnlyKeyChange, addMappingOnlyKeyChangeSwitch, new QLabel(StringConstants::addMappingOnlyKeyChangeDesc)));


    // =====================================================================================
    // 死区设置
    // =====================================================================================
    CollapsibleGroupBox* deadZoneSettingsGroup = new CollapsibleGroupBox(StringConstants::deadZoneSettings, this);
    QVBoxLayout* deadZoneSettingsGroupLayout = new QVBoxLayout;
    deadZoneSettingsGroup->setContentLayout(deadZoneSettingsGroupLayout);
    // 控件
    xboxJoyStickDeadZoneLineEdit = new QLineEdit(deadZoneSettingsGroup);
    xboxTriggerDeadZoneLineEdit = new QLineEdit(deadZoneSettingsGroup);
    wheelAxisDeadZoneLineEdit = new QLineEdit(deadZoneSettingsGroup);
    pedalAxisDeadZoneLineEdit = new QLineEdit(deadZoneSettingsGroup);
    // 设置样式
    Theme::setLineEditStyleSheet(xboxJoyStickDeadZoneLineEdit);
    Theme::setLineEditStyleSheet(xboxTriggerDeadZoneLineEdit);
    Theme::setLineEditStyleSheet(wheelAxisDeadZoneLineEdit);
    Theme::setLineEditStyleSheet(pedalAxisDeadZoneLineEdit);
    // 固定输入框的宽度
    int fixedWidthLineEdit = 150;
    xboxJoyStickDeadZoneLineEdit->setFixedWidth(fixedWidthLineEdit);
    xboxTriggerDeadZoneLineEdit->setFixedWidth(fixedWidthLineEdit);
    wheelAxisDeadZoneLineEdit->setFixedWidth(fixedWidthLineEdit);
    pedalAxisDeadZoneLineEdit->setFixedWidth(fixedWidthLineEdit);
    // 布局内容
    deadZoneSettingsGroupLayout->addWidget(Global::createSettingsItem(deadZoneSettingsGroup, StringConstants::xboxJoyStickDeadZone, xboxJoyStickDeadZoneLineEdit, new QLabel(StringConstants::xboxJoyStickDeadZoneDesc)));
    deadZoneSettingsGroupLayout->addWidget(Global::createSettingsItem(deadZoneSettingsGroup, StringConstants::xboxTriggerDeadZone, xboxTriggerDeadZoneLineEdit, new QLabel(StringConstants::xboxTriggerDeadZoneDesc)));
    deadZoneSettingsGroupLayout->addWidget(Global::createSettingsItem(deadZoneSettingsGroup, StringConstants::wheelAxisDeadZone, wheelAxisDeadZoneLineEdit, new QLabel(StringConstants::wheelAxisDeadZoneDesc)));
    deadZoneSettingsGroupLayout->addWidget(Global::createSettingsItem(deadZoneSettingsGroup, StringConstants::pedalAxisDeadZone, pedalAxisDeadZoneLineEdit, new QLabel(StringConstants::pedalAxisDeadZoneDesc)));

    // =====================================================================================
    // 欧卡2相关设置
    // =====================================================================================
    CollapsibleGroupBox* ets2SettingsGroup = new CollapsibleGroupBox(StringConstants::ETS2_settings, this);
    QVBoxLayout* ets2SettingsGroupLayout = new QVBoxLayout;
    ets2SettingsGroup->setContentLayout(ets2SettingsGroupLayout);
    // 控件
    ets2PathSelectBtn = new QPushButton(StringConstants::selectDir, ets2SettingsGroup);
    ets2PathLabel = new QLabel("", ets2SettingsGroup);
    autoReleaseHandBrake = new WinUISwitch(ets2SettingsGroup);
    ets2KeyBinderBtn = new QPushButton(StringConstants::bindingGuide, ets2SettingsGroup);
    // 设置样式
    Theme::setButtonStyleSheet(ets2PathSelectBtn, ButtonLevel::normal);
    Theme::setButtonStyleSheet(ets2KeyBinderBtn, ButtonLevel::normal);
    // 布局内容
    ets2SettingsGroupLayout->addWidget(Global::createSettingsItem(ets2SettingsGroup, StringConstants::ETS2_installDir, ets2PathSelectBtn, ets2PathLabel));
    ets2SettingsGroupLayout->addWidget(Global::createSettingsItem(ets2SettingsGroup, StringConstants::autoReleaseHandBrake, autoReleaseHandBrake, new QLabel(StringConstants::autoReleaseHandBrakeDesc)));
    ets2SettingsGroupLayout->addWidget(Global::createSettingsItem(ets2SettingsGroup, StringConstants::ETS2_keyBinder, ets2KeyBinderBtn, new QLabel(StringConstants::ETS2_keyBinderDesc)));


    // =====================================================================================
    // 其它设置
    // =====================================================================================
    CollapsibleGroupBox* otherSettingsGroup = new CollapsibleGroupBox(StringConstants::otherSettings, this);
    QVBoxLayout* otherSettingsGroupLayout = new QVBoxLayout;
    otherSettingsGroup->setContentLayout(otherSettingsGroupLayout);
    // 控件
    mouseSpeedLineEdit = new QLineEdit(otherSettingsGroup);
    // 设置样式
    Theme::setLineEditStyleSheet(mouseSpeedLineEdit);
    // 固定控件宽度
    mouseSpeedLineEdit->setFixedWidth(fixedWidthLineEdit);
    // 布局内容
    otherSettingsGroupLayout->addWidget(Global::createSettingsItem(otherSettingsGroup, StringConstants::mouseSpeed, mouseSpeedLineEdit, new QLabel(StringConstants::mouseSpeedDesc)));


    // 添加各组设置到布局
    contentLayout->addWidget(overrideGlobalSettingsWidget);
    contentLayout->addWidget(baseSettingsGroup);
    contentLayout->addWidget(keyMappingSettingsGroup);
    contentLayout->addWidget(deadZoneSettingsGroup);
    contentLayout->addWidget(ets2SettingsGroup);
    contentLayout->addWidget(otherSettingsGroup);
    contentLayout->addStretch();

    // 根容器布局添加标题, 滚动区域
    rootLayout->addWidget(title);
    rootLayout->addWidget(settingsScopeWidget);
    rootLayout->addWidget(scrollArea);

    // 绑定事件
    bindingSiganlsToSlots();
    // 更新ui
    updateUI();

    // 欧卡2自动解除手刹
    if(ConfigService::get().ETS2_enableAutoCancelHandbrake){
        // 2s后启动
        QTimer::singleShot(2000, [this](){
            QCoreApplication::processEvents();
            startETS2AssitFuncWorker();
        });
    }

    // 开机自启动
    if(ConfigService::getGlobalUserConfig().SYSTEM_enableRunUponStartup){
        QTimer::singleShot(1000, [](){
            Global::setStartOnStartup(true);
        });
    }
}

template<typename Func>
void SettingsPage::modifySetting(Func modifyFunc)
{
    // 是否需要保存到文件
    bool needSaveToFile = false;
    // 是否需要发送设置变更的信号
    bool needSendSignal = false;
    // 当前是否覆盖全局设置(使用当前映射配置的设置)
    bool isOverride = false;
    {
        // 加锁, 顺序一定不能变, 否则可能死锁
        QMutexLocker lockerCurr(&ConfigService::mutex_currentMappingConfig);
        QMutexLocker lockerGlob(&ConfigService::mutex_globalUserConfig);

        // 根据是否覆盖全局设置, 获取不同的对象引用
        isOverride = !showGlobalSettings;
        UserConfig& cfg = isOverride ? ConfigService::currentMappingConfig.relatedUserConfig : ConfigService::globalUserConfig;

        // 根据是否覆盖全局设置, 主动释放用不到的锁
        isOverride ? lockerGlob.unlock() : lockerCurr.unlock();

        // 具体的修改逻辑 由调用方使用lamda表达式对cfg引用对象进行修改
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

void SettingsPage::bindingSiganlsToSlots()
{
    // 生效范围
    connect(settingScopegroup, QOverload<int>::of(&QButtonGroup::idClicked),this, [this](int id){
        bool needSaveToFile = false;

        // id==0 说明选择了0全局0设置
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
            if(checked != ConfigService::currentMappingConfig.overrideGlobalUserConfig){
                ConfigService::currentMappingConfig.overrideGlobalUserConfig = checked;
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

    // 开机自启动
    connect(startOnStartupSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变
            if(cfg.SYSTEM_enableRunUponStartup != checked){
                cfg.SYSTEM_enableRunUponStartup = checked;
                needSaveToFile = true;
            }
        });

        // 设置开机自启动
        Global::setStartOnStartup(checked);
    });
    // 关闭主窗口时隐藏到系统托盘
    connect(hideWindowOnCloseSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变
            if(cfg.SYSTEM_enableHideWindowOnClose != checked){
                cfg.SYSTEM_enableHideWindowOnClose = checked;
                needSaveToFile = true;
            }
        });
    });

    // 自动开启映射
    connect(autoMappingSwicth, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变
            if(cfg.SYSTEM_enableMappingAfterOpening != checked){
                cfg.SYSTEM_enableMappingAfterOpening = checked;
                needSaveToFile = true;
            }
        });
    });
    // 设备名称强唯一模式
    connect(deviceNameUniqueModeSwicth, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变
            if(cfg.SYSTEM_enableStrongUniqueDeviceNameMode != checked){
                cfg.SYSTEM_enableStrongUniqueDeviceNameMode = checked;
                needSaveToFile = true;
            }
        });
    });
    // 最长组合键优先模式
    connect(multiKeyFirstModeSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            //值发生改变
            if(cfg.SYSTEM_enableOnlyLongestMapping != checked){
                cfg.SYSTEM_enableOnlyLongestMapping = checked;
                needSaveToFile = true;
                needSendSignal = true;
            }
        });
    });
    // 新增映射只识别变化按键
    connect(addMappingOnlyKeyChangeSwitch, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            // 值发生改变
            if(cfg.SYSTEM_enableOnlyChangeKeyWhenNew != checked){
                cfg.SYSTEM_enableOnlyChangeKeyWhenNew = checked;
                needSaveToFile = true;
            }
        });
    });

    // 虚拟xbox手柄-摇杆内部死区
    connect(xboxJoyStickDeadZoneLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 获取输入框文本并转化成double
        bool ok;
        double value = xboxJoyStickDeadZoneLineEdit->text().toDouble(&ok);

        // 内容校验
        if (!ok || value < -1 || value > 1) {
            xboxJoyStickDeadZoneLineEdit->setText("");
            Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            return;
        }

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.xboxJoystickInnerDeadAreaValue = value;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });
    // 虚拟xbox手柄-扳机内部死区
    connect(xboxTriggerDeadZoneLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 获取输入框文本并转化成double
        bool ok;
        double value = xboxTriggerDeadZoneLineEdit->text().toDouble(&ok);

        // 内容校验
        if (!ok || value < -1 || value > 1) {
            xboxTriggerDeadZoneLineEdit->setText("");
            Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            return;
        }

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.xboxTriggerInnerDeadAreaValue = value;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });
    // 轴映射键盘按键-转向轴内部死区
    connect(wheelAxisDeadZoneLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 获取输入框文本并转化成double
        bool ok;
        double value = wheelAxisDeadZoneLineEdit->text().toDouble(&ok);

        // 内容校验
        if (!ok || value < 0 || value > 1) {
            wheelAxisDeadZoneLineEdit->setText("");
            Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            return;
        }

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.steeringAxisInnerDeadZone = value;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });
    // 轴映射键盘按键-踏板轴内部死区
    connect(pedalAxisDeadZoneLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 获取输入框文本并转化成double
        bool ok;
        double value = pedalAxisDeadZoneLineEdit->text().toDouble(&ok);

        // 内容校验
        if (!ok || value < 0 || value > 1) {
            pedalAxisDeadZoneLineEdit->setText("");
            Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            return;
        }

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.pedalAxisInnerDeadZone = value;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });

    // 选择欧卡2路径
    connect(ets2PathSelectBtn, &QPushButton::clicked, this, [this](){
        // 打开文件夹选择对话框
        QString folderPath = QFileDialog::getExistingDirectory(
            this,                       // 父窗口
            tr(StringConstants::selectETS2FolderTitle.toStdString().c_str()), // 对话框标题
            QDir::homePath(),            // 默认打开的目录（用户主目录）
            QFileDialog::ShowDirsOnly    // 只显示目录
                | QFileDialog::DontResolveSymlinks  // 不解析符号链接
            );

        // 检查用户是否选择了文件夹（没有点击取消）
        if (!folderPath.isEmpty()) {
            // 修改
            modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
                cfg.ETS2InstallPath = folderPath;
                needSaveToFile = true;
            });
        }
    });
    // 欧卡2自动解除手刹
    connect(autoReleaseHandBrake, &WinUISwitch::toggled, this, [this](bool checked){
        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.ETS2_enableAutoCancelHandbrake = checked;
            needSaveToFile = true;
        });

        updateUI();
    });
    // 欧卡原生按键绑定
    connect(ets2KeyBinderBtn, &QPushButton::clicked, this, [this](){
        ETS2KeyBinderWizard *ets2KeyBinderWizard = new ETS2KeyBinderWizard(this);
        ets2KeyBinderWizard->setAttribute(Qt::WA_DeleteOnClose, true); // 设置关闭时删除对象
        ets2KeyBinderWizard->setWindowModality(Qt::ApplicationModal);  // 设置模态
        ets2KeyBinderWizard->show();
    });

    // 模拟鼠标移动-速率
    connect(mouseSpeedLineEdit, &QLineEdit::editingFinished, this, [this](){
        // 获取输入框文本并转化成double
        bool ok;
        double value = mouseSpeedLineEdit->text().toDouble(&ok);

        // 内容校验
        if (!ok || value <= 0) {
            mouseSpeedLineEdit->setText("");
            Global::showErrorMsgBoxAndPushToLog(StringConstants::invalidValue);
            return;
        }

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.mouseMoveSpeedTimes = value;
            needSaveToFile = true;
            needSendSignal = true;
        });
    });
}

void SettingsPage::updateUI()
{
    // 显示: 当前映射配置 "aaa" 的专属设置
    if(ConfigService::currentMappingFileName.isEmpty() == false){
        currentMappingRadioButton->setText(StringConstants::currentMappingFileSettings2.arg(ConfigService::currentMappingFileName));
    }else{
        currentMappingRadioButton->setText(StringConstants::currentMappingFileSettings);
    }


    // 获取当前映射配置
    auto mappingCfg = ConfigService::getCurrentMappingConfig();
    overrideGlobalSettingsSwitch->setChecked(mappingCfg.overrideGlobalUserConfig);

    // 获取用户配置, 根据"是否显示全局设置", 获取不同的配置对象
    auto cfg = showGlobalSettings ? ConfigService::getGlobalUserConfig() : mappingCfg.relatedUserConfig;

    startOnStartupSwitch->setChecked(cfg.SYSTEM_enableRunUponStartup);
    hideWindowOnCloseSwitch->setChecked(cfg.SYSTEM_enableHideWindowOnClose);
    autoMappingSwicth->setChecked(cfg.SYSTEM_enableMappingAfterOpening);
    deviceNameUniqueModeSwicth->setChecked(cfg.SYSTEM_enableStrongUniqueDeviceNameMode);
    multiKeyFirstModeSwitch->setChecked(cfg.SYSTEM_enableOnlyLongestMapping);
    addMappingOnlyKeyChangeSwitch->setChecked(cfg.SYSTEM_enableOnlyChangeKeyWhenNew);

    //
    xboxJoyStickDeadZoneLineEdit->setText(QString::number(cfg.xboxJoystickInnerDeadAreaValue));
    xboxTriggerDeadZoneLineEdit->setText(QString::number(cfg.xboxTriggerInnerDeadAreaValue));
    wheelAxisDeadZoneLineEdit->setText(QString::number(cfg.steeringAxisInnerDeadZone));
    pedalAxisDeadZoneLineEdit->setText(QString::number(cfg.pedalAxisInnerDeadZone));

    //
    ets2PathLabel->setText((cfg.ETS2InstallPath.isEmpty())? StringConstants::notSelectedDir : cfg.ETS2InstallPath);
    autoReleaseHandBrake->setChecked(cfg.ETS2_enableAutoCancelHandbrake);

    //
    mouseSpeedLineEdit->setText(QString::number(cfg.mouseMoveSpeedTimes));


    if(showGlobalSettings){
        baseSettingsGroup->show();
        keyMappingSettingsGroup->show();

        // 显示全局设置时隐藏部分组件
        overrideGlobalSettingsWidget->hide();
    }else{
        overrideGlobalSettingsWidget->show();

        // 显示当前映射配置专属设置时, 隐藏部分组件
        baseSettingsGroup->hide();
        keyMappingSettingsGroup->hide();
    }


    // ===========================================
    // 下面是一些后台任务的状态更新
    // ===========================================
    // 当前是否覆盖全局设置, 覆盖则使用专属设置, 否则使用全局设置
    bool overrideGlobal = mappingCfg.overrideGlobalUserConfig;
    if(overrideGlobal){
        // 欧卡自动解除手刹
        if(mappingCfg.relatedUserConfig.ETS2_enableAutoCancelHandbrake){
            startETS2AssitFuncWorker();
        }else{
            stopETS2AssitFuncWorker();
        }
    }else{
        // 全局设置
        if(ConfigService::getGlobalUserConfig().ETS2_enableAutoCancelHandbrake){
            // 开启欧卡辅助功能任务
            startETS2AssitFuncWorker();
        }else{
            // 停止欧卡辅助功能任务
            stopETS2AssitFuncWorker();
        }
    }
}

void SettingsPage::startETS2AssitFuncWorker()
{
    // 检查欧卡2的遥测数据共享内存dll是否存在
    if(!checkETS2Plugin()){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::startETS2AutoHandBrakeError);
        // 启动失败, 重置 "自动解除手刹" 的开关状态为 false
        autoReleaseHandBrake->setChecked(false);

        // 修改
        modifySetting([=](UserConfig& cfg, bool& needSaveToFile, bool& needSendSignal){
            cfg.ETS2_enableAutoCancelHandbrake = false;
            needSaveToFile = true;
        });
        return;
    }

    // 加锁
    QMutexLocker locker(&mutex_ETS2AssitFuncWorker);

    // 请求欧卡的辅助功能任务的计数器 加一
    ETS2AssitFuncWorkerCounter++;

    // 欧卡的辅助功能任务已经被创建，不能重复创建
    if(isETS2AssitFuncWorkerRunning){
        return;
    }

    // 创建并开启 欧卡的辅助功能任务
    AssistFuncWorker *worker = new AssistFuncWorker();
    QThread *thread = new QThread;
    worker->moveToThread(thread);

    // 连接信号槽
    connect(thread, &QThread::started, worker, &AssistFuncWorker::doWork);
    // 停止任务信号
    connect(this, &SettingsPage::stopETS2AssitFuncWorkerSignal, worker, &AssistFuncWorker::cancelWorkSlot);
    // 任务结束信号
    connect(worker, &AssistFuncWorker::workFinished, thread, &QThread::quit);
    connect(worker, &AssistFuncWorker::workFinished, worker, &AssistFuncWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();

    // 正在运行
    isETS2AssitFuncWorkerRunning = true;
}

void SettingsPage::stopETS2AssitFuncWorker()
{
    // 加锁
    QMutexLocker locker(&mutex_ETS2AssitFuncWorker);
    // 计数器减一
    ETS2AssitFuncWorkerCounter--;

    // 如果当前没有在运行，不需要停止
    // 或者计数器大于0，说明还有地方需要辅助功能的运行，还不能停止
    if(isETS2AssitFuncWorkerRunning == false || ETS2AssitFuncWorkerCounter > 0){
        return;
    }

    // 停止运行
    isETS2AssitFuncWorkerRunning = false;
    ETS2AssitFuncWorkerCounter = 0;
    // 提交停止运行信号
    emit stopETS2AssitFuncWorkerSignal();
}

bool SettingsPage::checkETS2Plugin()
{
    // 欧卡安装路径
    auto ETS2InstallPath = ConfigService::get().ETS2InstallPath;

    if(ETS2InstallPath.isEmpty()){
        LogService::parseErrorLog(StringConstants::eTS2PathIsEmptyError);
        return false;
    }

    // 欧卡安装目录
    QDir ets2Dir(ETS2InstallPath);

    if(!copyETS2PluginDll(ets2Dir.filePath("bin/win_x64/plugins/"), "plugins/Win64/scs-telemetry.dll")){
        return false;
    }
    copyETS2PluginDll(ets2Dir.filePath("bin/win_x86/plugins/"), "plugins/Win32/scs-telemetry.dll");

    return true;
}

bool SettingsPage::copyETS2PluginDll(QString ets2PulginPath, QString pluginDllFilePath)
{
    QDir ETS2Dir(ets2PulginPath);

    // 目录不存在，创建目录
    if(!ETS2Dir.exists()){
        ETS2Dir.mkpath(ets2PulginPath);
    }

    // dll文件路径
    QString dllPluginFileAbsulutelyPath = QDir(QCoreApplication::applicationDirPath()).filePath(pluginDllFilePath);

    // dll文件
    QFile dllPluginFile(dllPluginFileAbsulutelyPath);

    // dll不存在
    if(!dllPluginFile.exists()){
        LogService::parseErrorLog(StringConstants::copyPluginFolderNotExistsError.arg(dllPluginFileAbsulutelyPath));
        return false;
    }else{
        // 目标路径
        QString targetFilePath = ETS2Dir.filePath(QFileInfo(dllPluginFileAbsulutelyPath).fileName());

        // 如果欧卡目录已经有了dll, 不需要再复制
        if(QFile(targetFilePath).exists()){
            return true;
        }

        // 复制dll文件
        if(!dllPluginFile.copy(targetFilePath)){
            LogService::parseErrorLog(StringConstants::copyPluginFailed);
            return false;
        }
    }

    LogService::parseSuccessLog(StringConstants::copyPluginSuccess);
    return true;
}


