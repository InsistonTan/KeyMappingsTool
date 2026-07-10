#include<common/Global.h>
#include "Theme.h"
#include "qcolor.h"
#include "qstyle.h"
#include "qstylehints.h"
#include "common/DeviceDataTypeEnum.h"
#include "common/KeyMap.h"
#include "common/StringConstants.h"
#include "models/MappingRelation.h"
#include "services/LogService.h"
#include "utils/IconFactory.h"
#include <windows.h>
#include <mmsystem.h>

#include<QDebug>
#include<QMutexLocker>
#include<QMutex>
#include<QDateTime>
#include<QDir>
#include<QCoreApplication>
#include<QRegularExpression>
#include <QApplication>
#include <QSettings>



// 映射列表含有映射xbox的记录
bool Global::hasXboxMappingInMappingList(const QVector<MappingRelation>& mappingList){
    if(mappingList.empty()){
        return false;
    }

    for(const auto& mapping : mappingList){
        if(mapping.valid && mapping.mappingType == MappingType::Xbox){
            return true;
        }
    }

    return false;
}

// BUTTONS_VALUE_TYPE 转换为字符串
QString Global::ButtonsValueTypeToString(BUTTONS_VALUE_TYPE btnValue){
    QString btnValueStr = "";

    // "按键"
    auto btnString = StringConstants::btnString;

    for (size_t i = 0; i < DINPUT_MAX_BUTTONS; i++) {
        if (btnValue.getBit(i)) {
            btnValueStr += btnString + QString::number(i) + BUTTON_NAME_COMBINE_SPLIT;
        }
    }
    uint64_t povValue = (btnValue.operator>>(DINPUT_MAX_BUTTONS)).toUint64_t();
    if (povValue) {

        // "摇杆"
        auto joystick = StringConstants::joystick;
        // "角度"
        auto angleString = StringConstants::angle;

        for (int j = 0; j < 4; j++) {
            uint16_t angle = (povValue >> (j * 16)) & (uint16_t)-1;
            if (angle) {
                angle = ~angle; // 角度得反码
                btnValueStr += joystick
                               + QString::number(j + 1)
                               + ANGLE_NAME_CONNECT_STR
                               + angleString
                               + QString::number(angle)
                               + BUTTON_NAME_COMBINE_SPLIT;
            }
        }
    }
    if (btnValueStr.size() > 0) {
        btnValueStr.chop(1);  // 去掉最后的 "+"
    }
    return btnValueStr;
}

// 字符串 转换为 BUTTONS_VALUE_TYPE
BUTTONS_VALUE_TYPE Global::stringToButtonsValueType(const QString& btnValueStr) {
    BUTTONS_VALUE_TYPE btnValue;
    QString str = btnValueStr;
    QStringList parts = str.split(BUTTON_NAME_COMBINE_SPLIT, Qt::SkipEmptyParts); // 分割字符串 当没有"+"时, parts.size() == 1

    auto btnString = StringConstants::btnString;
    auto joystick = StringConstants::joystick;
    auto angle = StringConstants::angle;

    for (const QString& part : parts) {
        // part的样例: "按键10", 截取出按键索引值 10
        if (part.startsWith(btnString)) {
            bool ok;
            // 截取前缀后面的字符, 为按键索引值
            QString numberStr = part.mid(btnString.length());
            int buttonIndex = numberStr.toInt(&ok);
            if (ok && buttonIndex >= 0 && buttonIndex < DINPUT_MAX_BUTTONS) {
                btnValue.setBit(buttonIndex, true);
            }
        } else if (part.startsWith(joystick)) {
            // part的样例: "摇杆1-角度90", 通过分隔符 "-" 分隔字符串, 获得角度值
            QStringList subParts = part.split(ANGLE_NAME_CONNECT_STR);
            // 取得角度值
            if (subParts.size() == 2 && subParts[1].startsWith(angle)) {
                bool ok;
                int angle = subParts[1].mid(2).toInt(&ok);
                if (ok && angle >= 0 && angle < 360) {
                    uint16_t angleValue = ~angle; // 角度得反码
                    btnValue |= (BUTTONS_VALUE_TYPE)(angleValue << ((subParts[0].mid(2).toInt() - 1) * 16 + DINPUT_MAX_BUTTONS));
                }
            }
        }
    }

    return btnValue;
}


QString Global::removeUnnecessaryZero(QString str){
    return str.remove(QRegularExpression("\\.?0+$"));
}

// 根据按键值转换成按键名称
QString Global::getKeyNameFromKeyValue(const MappingRelation& mapping){
    QStringList nameList;

    // 根据映射类型和设备按键类型选择 按键名称:按键值 的map
    auto map = mapping.mappingType == MappingType::Keyboard
                   ? VK_MAP
                   : (mapping.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS
                          ? VK_XBOX_AXIS_MAP
                          : VK_XBOX_BTN_MAP);

    auto valueStrList = mapping.keyboard_value.split(KEYBOARD_COMBINE_KEY_SPE);
    for(const auto &valStr : valueStrList){
        bool ok;
        short val = valStr.toShort(&ok);
        if(!ok){
            continue;
        }

        // 根据value寻找key
        for(auto &item : map){
            if(item.second == val){
                nameList.append(item.first.data());
            }
        }
    }

    return nameList.size() > 0 ? nameList.join(KEYBOARD_COMBINE_KEY_SPE) : "";
}

void Global::setSystemTimePeriod_1ms()
{
    QMutexLocker locker(&counterMuteX);

    // 已经被设置过
    if(++setTimeBeginPeriod5msCounter > 1)
        return;

    // 设置系统定时器精度为5ms
    timeBeginPeriod(1);
}

void Global::restoreSystemTimePeriod(bool forceRestore)
{
    QMutexLocker locker(&counterMuteX);

    // 只有计数器 == 0时才恢复, 大于0说明还有地方需要这个精度, 小于0说明已经被重置过了
    // 如果开启了 forceRestore, 将强制恢复
    if(--setTimeBeginPeriod5msCounter == 0 || forceRestore){
        // 恢复系统定时器精度
        timeEndPeriod(1);
    }
}

// 显示错误弹窗并推送日志
void Global::showErrorMsgBoxAndPushToLog(QString msg){
    MessageBoxService::showError(msg);
    LogService::parseErrorLog(msg);
}

// 获取隐藏窗口的句柄
HWND Global::getHideWindowHWnd(){
    return hideWindow->hwnd();
}

// 获取按键/轴的完整名称(带设备名称的)
QString Global::getBtnOrAxisFullName(QString deviceName, QString btnName){
    return deviceName + "-" + btnName;
}
// 获取按键/轴的完整名称(带设备名称的)
std::string Global::getBtnOrAxisFullName(std::string deviceName, std::string btnName){
    return deviceName + "-" + btnName;
}

// 获取软件数据文件存储的路径
QString Global::getAppDataDirStr(){
    QString path = QDir::homePath() + "/AppData/Local/KeyMappingToolData/";

    // 如果目录不存在, 就创建目录
    QDir().mkpath(path);

    return path;
}

QString Global::getCrashLogPath()
{
    return getAppDataDirStr() + "crash_log.txt";
}

QString Global::getUserMappingsFileDir()
 {
     QString path = getAppDataDirStr() + "userMappings/";

     // 如果目录不存在, 就创建目录
     QDir().mkpath(path);

     return path;
 }

 QString Global::getUserMappingFilePath(QString fileBaseName)
 {
     return Global::getUserMappingsFileDir() + fileBaseName + MAPPING_FILE_SUFFIX;
 }

bool Global::isDarkTheme()
{
    return static_cast<QApplication *>(QCoreApplication::instance())->styleHints()->colorScheme() == Qt::ColorScheme::Dark;
}

void Global::switchMappingTypeIcon(QPushButton *mappingTypeBtn, MappingRelation mapping)
{
    // 切换图标
    if (mapping.mappingType == MappingType::Keyboard) {
        mappingTypeBtn->setToolTip(StringConstants::mappingKeyBoard);
        mappingTypeBtn->setIcon(IconFactory::icon(IconFactory::IconEnum::keyboard, 28));
    } else {
        mappingTypeBtn->setToolTip(StringConstants::mappingXbox);
        mappingTypeBtn->setIcon(IconFactory::icon(IconFactory::IconEnum::gamepad, 28));
    }
    mappingTypeBtn->style()->unpolish(mappingTypeBtn);
    mappingTypeBtn->style()->polish(mappingTypeBtn);
}

void Global::setStartOnStartup(bool enable)
{
    // 获取应用程序的路径
    QString appPath = QFileInfo(QCoreApplication::applicationFilePath()).absoluteFilePath();

    // 创建 QSettings 对象，操作 Windows 注册表
    QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run", QSettings::NativeFormat);

    // 删除旧版本的注册表(如果存在)
    settings.remove(Global::OLD_APP_NAME);

    if(enable){
        // 在注册表中添加应用程序的启动项, 并且添加 --hide运行参数, 在main.cpp中检查运行参数, 带有--hide的隐藏主窗口运行
        settings.setValue(Global::APP_NAME, "\"" + appPath.replace("/", "\\") + "\" --hide");

        LogService::parseSuccessLog(StringConstants::enableStartOnStartup);
    }else{
        // 取消开机自启动
        // 从注册表中移除应用程序的启动项
        settings.remove(Global::APP_NAME);

        LogService::parseSuccessLog(StringConstants::disableStartOnStartup);
    }

}

QWidget *Global::createSettingsItem(QWidget *parent, QString settingsName, QWidget *customWidget, QLabel *settingsDescLabel, int fixedHeight)
{
    // 整个的容器和布局
    QWidget* widget = new QWidget(parent);
    Theme::setQWidgetStyleSheet(widget);

    QHBoxLayout* layout = new QHBoxLayout(widget);
    layout->setContentsMargins(16,8,16,8);
    layout->setSpacing(0);

    // 设置项的名称和描述 的容器
    QWidget* labelWidget = new QWidget(parent);
    QVBoxLayout* labelWidgetLayout = new QVBoxLayout(labelWidget);

    // 设置项的名称
    QLabel* settingsNameLabel = new QLabel(settingsName, parent);
    settingsNameLabel->setStyleSheet(QString("background-color:transparent; color:%1; font-size:13px;").arg(Theme::textColor())); //#c0c7d4
    labelWidgetLayout->addWidget(settingsNameLabel);
    labelWidgetLayout->setSpacing(0);
    labelWidgetLayout->setContentsMargins(0,0,0,0);

    //设置项的描述label
    if(settingsDescLabel != nullptr){
        settingsDescLabel->setStyleSheet(QString("background-color:transparent; color:%2; font-size:12px;").arg(Theme::secondTextColor())); //#8f98a3
        labelWidgetLayout->addWidget(settingsDescLabel);
        labelWidgetLayout->setSpacing(4);
    }

    // 添加到布局
    layout->addWidget(labelWidget);
    layout->addStretch();
    layout->addWidget(customWidget);


    // 固定的高度
    if(fixedHeight <= 36){
        fixedHeight = 36;
    }

    int contentMarginHeight = 8 * 2;

    // 容器设置固定高度
    widget->setFixedHeight(fixedHeight + contentMarginHeight);
    labelWidget->setFixedHeight(fixedHeight);
    customWidget->setFixedHeight(fixedHeight);

    return widget;
}


