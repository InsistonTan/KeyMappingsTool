#include "UserConfig.h"
#include "qjsonarray.h"
#include "qjsonobject.h"
#include "common/UserConfigKey.h"
#include "ui/widgets/CurveEditor.h"

#include <QJsonArray>


UserConfig::UserConfig() {}

QJsonObject UserConfig::toJson()
{
    // 结果
    QJsonObject jsonObj;


    ///////////////////////////////////////
    /// 原死区设置
    jsonObj[UserConfigKey::keyboardWheelAxisInnerDeadAreaValue] = steeringAxisInnerDeadZone;

    jsonObj[UserConfigKey::keyboardPedalAxisInnerDeadAreaValue] = pedalAxisInnerDeadZone;

    jsonObj[UserConfigKey::xboxJoystickInnerDeadAreaValue] = xboxJoystickInnerDeadAreaValue;

    jsonObj[UserConfigKey::xboxTriggerInnerDeadAreaValue] = xboxTriggerInnerDeadAreaValue;

    jsonObj[UserConfigKey::mouseMoveSpeedTimes] = mouseMoveSpeedTimes;


    //////////////////////////////////////////
    /// 软件系统设置 (原辅助功能设置)
    jsonObj[UserConfigKey::ETS2InstallPath] = ETS2InstallPath;

    jsonObj[UserConfigKey::ETS2_enableAutoCancelHandbrake] = ETS2_enableAutoCancelHandbrake;

    jsonObj[UserConfigKey::SYSTEM_enableMappingAfterOpening] = SYSTEM_enableMappingAfterOpening;

    jsonObj[UserConfigKey::SYSTEM_enableOnlyLongestMapping] = SYSTEM_enableOnlyLongestMapping;

    jsonObj[UserConfigKey::SYSTEM_enableOnlyChangeKeyWhenNew] = SYSTEM_enableOnlyChangeKeyWhenNew;

    jsonObj[UserConfigKey::SYSTEM_enableForceFeedback] = SYSTEM_enableForceFeedback;

    jsonObj[UserConfigKey::SYSTEM_enableRunUponStartup] = SYSTEM_enableRunUponStartup;

    jsonObj[UserConfigKey::SYSTEM_enableHideWindowOnClose] = SYSTEM_enableHideWindowOnClose;

    jsonObj[UserConfigKey::SYSTEM_enableStrongUniqueDeviceNameMode] = SYSTEM_enableStrongUniqueDeviceNameMode;

    QJsonArray lastUsedDeviceNameListJsonArray;
    for(auto& item : SYSTEM_lastUsedDeviceNameList){
        lastUsedDeviceNameListJsonArray.append(item);
    }
    jsonObj[UserConfigKey::SYSTEM_lastUsedDeviceNameList] = lastUsedDeviceNameListJsonArray;

    jsonObj[UserConfigKey::SYSTEM_lastUsedMappingConfigPath] = SYSTEM_lastUsedMappingConfigPath;


    /////////////////////////////////////////////
    /// 力反馈设置
    QJsonObject ffbSettingsJsonObj;

    ffbSettingsJsonObj[UserConfigKey::throttleAxis]
        = SYSTEM_forceFeedbackSettings_throttleAxis;
    ffbSettingsJsonObj[UserConfigKey::throttleAxisDeviceName]
        = SYSTEM_forceFeedbackSettings_throttleAxisDeviceName;

    ffbSettingsJsonObj[UserConfigKey::brakeAxis]
        = SYSTEM_forceFeedbackSettings_brakeAxis;
    ffbSettingsJsonObj[UserConfigKey::brakeAxisDeviceName]
        = SYSTEM_forceFeedbackSettings_brakeAxisDeviceName;

    ffbSettingsJsonObj[UserConfigKey::steeringWheelAxis]
        = SYSTEM_forceFeedbackSettings_steeringWheelAxis;
    ffbSettingsJsonObj[UserConfigKey::steeringWheelAxisDeviceName]
        = SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName;

    ffbSettingsJsonObj[UserConfigKey::isThrottleReverse]
        = SYSTEM_forceFeedbackSettings_isThrottleReverse;
    ffbSettingsJsonObj[UserConfigKey::isBrakeReverse]
        = SYSTEM_forceFeedbackSettings_isBrakeReverse;

    ffbSettingsJsonObj[UserConfigKey::acceleration_100km_time_s]
        = SYSTEM_forceFeedbackSettings_acceleration_100km_time_s;
    ffbSettingsJsonObj[UserConfigKey::stop_100km_dis_m]
        = SYSTEM_forceFeedbackSettings_stop_100km_dis_m;
    ffbSettingsJsonObj[UserConfigKey::maxSpeed_km_h]
        = SYSTEM_forceFeedbackSettings_maxSpeed_km_h;
    // ffbSettingsJsonObj[UserConfigKey::maxForceFeedbackGain]
    //     = SYSTEM_forceFeedbackSettings_maxForceFeedbackGain;
    // ffbSettingsJsonObj[UserConfigKey::isConstantForceMode]
    //     = SYSTEM_forceFeedbackSettings_isConstantForceMode;
    // ffbSettingsJsonObj[UserConfigKey::constantCorrectiveForceGain]
    //     = SYSTEM_forceFeedbackSettings_constantCorrectiveForceGain;
    // ffbSettingsJsonObj[UserConfigKey::constantDampingGain]
    //     = SYSTEM_forceFeedbackSettings_constantDampingGain;

    QJsonArray springCurveJsonArray;
    for(auto& p : SYSTEM_forceFeedbackSettings_springCurve){
        springCurveJsonArray.append(p.toJson());
    }
    QJsonArray dampingCurveJsonArray;
    for(auto& p : SYSTEM_forceFeedbackSettings_dampingCurve){
        dampingCurveJsonArray.append(p.toJson());
    }
    ffbSettingsJsonObj[UserConfigKey::springCurve] = springCurveJsonArray;
    ffbSettingsJsonObj[UserConfigKey::dampingCurve] = dampingCurveJsonArray;

    ffbSettingsJsonObj[UserConfigKey::SYSTEM_forceFeedbackSettings_maxSpringGain] = SYSTEM_forceFeedbackSettings_maxSpringGain;
    ffbSettingsJsonObj[UserConfigKey::SYSTEM_forceFeedbackSettings_maxDamperGain] = SYSTEM_forceFeedbackSettings_maxDamperGain;


    // 力反馈模拟设置
    jsonObj[UserConfigKey::SYSTEM_forceFeedbackSettings] = ffbSettingsJsonObj;

    // 上一次使用的映射列表缓存
    jsonObj[UserConfigKey::SYSTEM_lastUsedMappingCache] = MappingRelation::listToJsonArray(SYSTEM_lastUsedMappingCache);


    return jsonObj;
}

UserConfig UserConfig::fromJsonObject(QJsonObject obj)
{
    UserConfig userConfig;

    // 从 json 对象读取信息到 userConfig
    readSettingsFromJson(obj, userConfig);

    return userConfig;
}

void UserConfig::readSettingsFromJson(QJsonObject jsonObj, UserConfig& out)
{
    // 读取 原死区设置的配置
    readDeadZoneSettingsFromJson(jsonObj, out);
    // 读取 原辅助功能的配置
    readAssistFuncSettingsFromJson(jsonObj, out);

    // 读取新增的设置项
    // 上一次使用的设备列表
    if(jsonObj.contains(UserConfigKey::SYSTEM_lastUsedDeviceNameList)
        && jsonObj[UserConfigKey::SYSTEM_lastUsedDeviceNameList].isArray())
    {
        QStringList list;

        QJsonArray arr = jsonObj[UserConfigKey::SYSTEM_lastUsedDeviceNameList].toArray();
        for(const auto& deviceName : arr){
            list.append(deviceName.toString(""));
        }

        out.SYSTEM_lastUsedDeviceNameList = list;
    }

    // 上一次使用的映射配置文件路径
    if(jsonObj.contains(UserConfigKey::SYSTEM_lastUsedMappingConfigPath))
    {
        out.SYSTEM_lastUsedMappingConfigPath = jsonObj[UserConfigKey::SYSTEM_lastUsedMappingConfigPath].toString();
    }

    // 上一次使用的映射列表缓存
    if(jsonObj.contains(UserConfigKey::SYSTEM_lastUsedMappingCache)
        && jsonObj[UserConfigKey::SYSTEM_lastUsedMappingCache].isArray()){
        // json array
        auto array = jsonObj[UserConfigKey::SYSTEM_lastUsedMappingCache].toArray();
        // 转成映射列表
        out.SYSTEM_lastUsedMappingCache = MappingRelation::listFromJsonArray(array);
    }

    // 关闭主窗口时隐藏到系统托盘
    if(jsonObj.contains(UserConfigKey::SYSTEM_enableHideWindowOnClose)){
        out.SYSTEM_enableHideWindowOnClose = jsonObj[UserConfigKey::SYSTEM_enableHideWindowOnClose].toBool();
    }

}

void UserConfig::readDeadZoneSettingsFromJson(QJsonObject jsonObj, UserConfig &out)
{

    if(jsonObj.contains(UserConfigKey::xboxJoystickInnerDeadAreaValue)){
        double value = jsonObj[UserConfigKey::xboxJoystickInnerDeadAreaValue]
                           .toDouble(0.0);
        // 输入超出范围 -1, 1
        if(value < -1 || value > 1){
            value = 0;
        }

        out.xboxJoystickInnerDeadAreaValue = value;
    }

    if(jsonObj.contains(UserConfigKey::xboxTriggerInnerDeadAreaValue)){
        double value = jsonObj[UserConfigKey::xboxTriggerInnerDeadAreaValue]
                           .toDouble(0.0);
        // 输入超出范围 -1, 1
        if(value < -1 || value > 1){
            value = 0;
        }

        out.xboxTriggerInnerDeadAreaValue = value;
    }

    if(jsonObj.contains(UserConfigKey::keyboardWheelAxisInnerDeadAreaValue)){
        double value = jsonObj[UserConfigKey::keyboardWheelAxisInnerDeadAreaValue]
                           .toDouble(DEFAULT_INNER_DEADAREA_VALUE);
        // 输入超出范围 -1, 1
        if(value < 0 || value > 1){
            value = DEFAULT_INNER_DEADAREA_VALUE;
        }

        out.steeringAxisInnerDeadZone = value;
    }

    if(jsonObj.contains(UserConfigKey::keyboardPedalAxisInnerDeadAreaValue)){
        double value = jsonObj[UserConfigKey::keyboardPedalAxisInnerDeadAreaValue]
                           .toDouble(DEFAULT_INNER_DEADAREA_VALUE);
        // 输入超出范围 -1, 1
        if(value < 0 || value > 1){
            value = DEFAULT_INNER_DEADAREA_VALUE;
        }

        out.pedalAxisInnerDeadZone = value;
    }

    // mouseMoveSpeedTimes
    if(jsonObj.contains(UserConfigKey::mouseMoveSpeedTimes)){
        double value = jsonObj[UserConfigKey::mouseMoveSpeedTimes]
                           .toDouble(DEFAULT_MOUSE_MOVE_SPEED_TIMES_VALUE);
        // 输入超出范围
        if(value <= 0){
            value = DEFAULT_MOUSE_MOVE_SPEED_TIMES_VALUE;
        }

        out.mouseMoveSpeedTimes = value;
    }

}

void UserConfig::readAssistFuncSettingsFromJson(QJsonObject jsonObj, UserConfig &out)
{
    // 读取信息
    bool enableETS2AutoCancelHandbrake = (jsonObj.contains(UserConfigKey::ETS2_enableAutoCancelHandbrake))
                                             ? jsonObj[UserConfigKey::ETS2_enableAutoCancelHandbrake].toBool()
                                             : false;
    bool enableAutoStartMapping = (jsonObj.contains(UserConfigKey::SYSTEM_enableMappingAfterOpening))
                                      ? jsonObj[UserConfigKey::SYSTEM_enableMappingAfterOpening].toBool()
                                      : false;
    bool enableOnlyLongestMapping = (jsonObj.contains(UserConfigKey::SYSTEM_enableOnlyLongestMapping))
                                        ? jsonObj[UserConfigKey::SYSTEM_enableOnlyLongestMapping].toBool()
                                        : false;
    bool enableOnlyChangeKeyWhenNew = (jsonObj.contains(UserConfigKey::SYSTEM_enableOnlyChangeKeyWhenNew))
                                          ? jsonObj[UserConfigKey::SYSTEM_enableOnlyChangeKeyWhenNew].toBool()
                                          : false;
    bool enableRunUponStartup = (jsonObj.contains(UserConfigKey::SYSTEM_enableRunUponStartup))
                                    ? jsonObj[UserConfigKey::SYSTEM_enableRunUponStartup].toBool()
                                    : false;

    out.SYSTEM_enableRunUponStartup = enableRunUponStartup;// 开机自启动
    out.ETS2_enableAutoCancelHandbrake = enableETS2AutoCancelHandbrake;// 欧卡2自动解除手刹
    out.SYSTEM_enableMappingAfterOpening = enableAutoStartMapping;// 打开软件自动开启映射
    out.SYSTEM_enableOnlyLongestMapping = enableOnlyLongestMapping;// 最长组合键优先
    out.SYSTEM_enableOnlyChangeKeyWhenNew = enableOnlyChangeKeyWhenNew;// 仅在新按键时更改按键

    // 力反馈模拟
    out.SYSTEM_enableForceFeedback = (jsonObj.contains(UserConfigKey::SYSTEM_enableForceFeedback))
                                         ? jsonObj[UserConfigKey::SYSTEM_enableForceFeedback].toBool()
                                         : false;

    // 设备名称强唯一模式
    out.SYSTEM_enableStrongUniqueDeviceNameMode = (jsonObj.contains(UserConfigKey::SYSTEM_enableStrongUniqueDeviceNameMode))
                                                      ? jsonObj[UserConfigKey::SYSTEM_enableStrongUniqueDeviceNameMode].toBool()
                                                      : false;

    // 欧卡2安装路径
    QString ets2Path = (jsonObj.contains(UserConfigKey::ETS2InstallPath))
                           ? jsonObj[UserConfigKey::ETS2InstallPath].toString()
                           : "";
    if(!ets2Path.isEmpty()){
        out.ETS2InstallPath = ets2Path;
    }

    // 力反馈模拟设置
    if(jsonObj.contains(UserConfigKey::SYSTEM_forceFeedbackSettings)
        && jsonObj[UserConfigKey::SYSTEM_forceFeedbackSettings].isObject())
    {
        QJsonObject settingsObj = jsonObj[UserConfigKey::SYSTEM_forceFeedbackSettings].toObject();

        out.SYSTEM_forceFeedbackSettings_throttleAxis =
            (settingsObj.contains(UserConfigKey::throttleAxis))
                ? settingsObj[UserConfigKey::throttleAxis].toString()
                : "";

        out.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName =
            (settingsObj.contains(UserConfigKey::throttleAxisDeviceName))
                ? settingsObj[UserConfigKey::throttleAxisDeviceName].toString()
                : "";

        out.SYSTEM_forceFeedbackSettings_brakeAxis =
            (settingsObj.contains(UserConfigKey::brakeAxis))
                ? settingsObj[UserConfigKey::brakeAxis].toString()
                : "";
        out.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName =
            (settingsObj.contains(UserConfigKey::brakeAxisDeviceName))
                ? settingsObj[UserConfigKey::brakeAxisDeviceName].toString()
                : "";

        out.SYSTEM_forceFeedbackSettings_steeringWheelAxis =
            (settingsObj.contains(UserConfigKey::steeringWheelAxis))
                ? settingsObj[UserConfigKey::steeringWheelAxis].toString()
                : "";
        out.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName =
            (settingsObj.contains(UserConfigKey::steeringWheelAxisDeviceName))
                ? settingsObj[UserConfigKey::steeringWheelAxisDeviceName].toString()
                : "";

        out.SYSTEM_forceFeedbackSettings_isThrottleReverse =
            (settingsObj.contains(UserConfigKey::isThrottleReverse))
                ? settingsObj[UserConfigKey::isThrottleReverse].toBool()
                : false;
        out.SYSTEM_forceFeedbackSettings_isBrakeReverse =
            (settingsObj.contains(UserConfigKey::isBrakeReverse))
                ? settingsObj[UserConfigKey::isBrakeReverse].toBool()
                : false;


        if(settingsObj.contains(UserConfigKey::acceleration_100km_time_s)){
            out.SYSTEM_forceFeedbackSettings_acceleration_100km_time_s =
                settingsObj[UserConfigKey::acceleration_100km_time_s].toDouble();
        }

        if(settingsObj.contains(UserConfigKey::stop_100km_dis_m)){
            out.SYSTEM_forceFeedbackSettings_stop_100km_dis_m =
                settingsObj[UserConfigKey::stop_100km_dis_m].toInt();
        }

        if(settingsObj.contains(UserConfigKey::maxSpeed_km_h)){
            out.SYSTEM_forceFeedbackSettings_maxSpeed_km_h =
                settingsObj[UserConfigKey::maxSpeed_km_h].toInt() > 0
                    ? settingsObj[UserConfigKey::maxSpeed_km_h].toInt()
                    : default_maxSpeed_km_h;
        }

        // if(settingsObj.contains(UserConfigKey::maxForceFeedbackGain)){
        //     out.SYSTEM_forceFeedbackSettings_maxForceFeedbackGain =
        //         settingsObj[UserConfigKey::maxForceFeedbackGain].toDouble();
        // }

        // out.SYSTEM_forceFeedbackSettings_isConstantForceMode =
        //     (settingsObj.contains(UserConfigKey::isConstantForceMode))
        //         ? settingsObj[UserConfigKey::isConstantForceMode].toBool()
        //         : false;

        // if(settingsObj.contains(UserConfigKey::constantCorrectiveForceGain)){
        //     out.SYSTEM_forceFeedbackSettings_constantCorrectiveForceGain =
        //         settingsObj[UserConfigKey::constantCorrectiveForceGain].toDouble();
        // }

        // if(settingsObj.contains(UserConfigKey::constantDampingGain)){
        //     out.SYSTEM_forceFeedbackSettings_constantDampingGain =
        //         settingsObj[UserConfigKey::constantDampingGain].toDouble();
        // }

        // 回正力曲线的点集合
        if(settingsObj.contains(UserConfigKey::springCurve) && settingsObj[UserConfigKey::springCurve].isArray()){
            for(const auto& item : settingsObj[UserConfigKey::springCurve].toArray()){
                if(item.isObject()){
                    out.SYSTEM_forceFeedbackSettings_springCurve.append(
                        CurveEditor::BezierLogicalPoint::fromJson(item.toObject()));
                }
            }
        }

        // 转向阻尼曲线的点集合
        if(settingsObj.contains(UserConfigKey::dampingCurve) && settingsObj[UserConfigKey::dampingCurve].isArray()){
            for(const auto& item : settingsObj[UserConfigKey::dampingCurve].toArray()){
                if(item.isObject()){
                    out.SYSTEM_forceFeedbackSettings_dampingCurve.append(
                        CurveEditor::BezierLogicalPoint::fromJson(item.toObject()));
                }
            }
        }

        if(out.SYSTEM_forceFeedbackSettings_springCurve.isEmpty()){
            out.SYSTEM_forceFeedbackSettings_springCurve.append(defaultSpringPoints);
        }
        if(out.SYSTEM_forceFeedbackSettings_dampingCurve.isEmpty()){
            out.SYSTEM_forceFeedbackSettings_dampingCurve.append(defaultDampingPoints);
        }


        // 回正力最大强度
        if(settingsObj.contains(UserConfigKey::SYSTEM_forceFeedbackSettings_maxSpringGain)){
            out.SYSTEM_forceFeedbackSettings_maxSpringGain =
                settingsObj[UserConfigKey::SYSTEM_forceFeedbackSettings_maxSpringGain].toDouble();
        }
        // 转向阻尼最大强度
        if(settingsObj.contains(UserConfigKey::SYSTEM_forceFeedbackSettings_maxDamperGain)){
            out.SYSTEM_forceFeedbackSettings_maxDamperGain =
                settingsObj[UserConfigKey::SYSTEM_forceFeedbackSettings_maxDamperGain].toDouble();
        }



    }


}






