#include <QString>

#pragma once


class UserConfigKey{
public:
    inline const static QString keyboardWheelAxisInnerDeadAreaValue
        = QStringLiteral("keyboardWheelAxisInnerDeadAreaValue");

    inline const static QString keyboardPedalAxisInnerDeadAreaValue
        = QStringLiteral("keyboardPedalAxisInnerDeadAreaValue");

    inline const static QString xboxJoystickInnerDeadAreaValue
        = QStringLiteral("xboxJoystickInnerDeadAreaValue");

    inline const static QString xboxTriggerInnerDeadAreaValue
        = QStringLiteral("xboxTriggerInnerDeadAreaValue");

    inline const static QString mouseMoveSpeedTimes
        = QStringLiteral("mouseMoveSpeedTimes");

    ////////////////////////////////////////////////
    /// 软件系统设置 的 key
    inline const static QString ETS2InstallPath
        = QStringLiteral("ETS2InstallPath");

    inline const static QString ETS2_enableAutoCancelHandbrake
        = QStringLiteral("ETS2_enableAutoCancelHandbrake");

    inline const static QString SYSTEM_enableMappingAfterOpening
        = QStringLiteral("SYSTEM_enableMappingAfterOpening");

    inline const static QString SYSTEM_enableOnlyLongestMapping
        = QStringLiteral("SYSTEM_enableOnlyLongestMapping");

    inline const static QString SYSTEM_enableOnlyChangeKeyWhenNew
        = QStringLiteral("SYSTEM_enableOnlyChangeKeyWhenNew");

    inline const static QString SYSTEM_enableForceFeedback
        = QStringLiteral("SYSTEM_enableForceFeedback");

    inline const static QString SYSTEM_enableRunUponStartup
        = QStringLiteral("SYSTEM_enableRunUponStartup");

    inline const static QString SYSTEM_enableHideWindowOnClose
        = QStringLiteral("SYSTEM_enableHideWindowOnClose");

    inline const static QString SYSTEM_enableStrongUniqueDeviceNameMode
        = QStringLiteral("SYSTEM_enableStrongUniqueDeviceNameMode");

    inline const static QString SYSTEM_lastUsedDeviceNameList
        = QStringLiteral("SYSTEM_lastUsedDeviceNameList");

    inline const static QString SYSTEM_lastUsedMappingConfigPath
        = QStringLiteral("SYSTEM_lastUsedMappingConfigPath");

    inline const static QString SYSTEM_lastUsedMappingCache
        = QStringLiteral("SYSTEM_lastUsedMappingCache");

    ////////////////////////////////////////////////////
    /// 力反馈设置的 key
    inline const static QString SYSTEM_forceFeedbackSettings
        = QStringLiteral("SYSTEM_forceFeedbackSettings");

    inline const static QString throttleAxis
        = QStringLiteral("throttleAxis");

    inline const static QString throttleAxisDeviceName
        = QStringLiteral("throttleAxisDeviceName");

    inline const static QString brakeAxis
        = QStringLiteral("brakeAxis");

    inline const static QString brakeAxisDeviceName
        = QStringLiteral("brakeAxisDeviceName");

    inline const static QString steeringWheelAxis
        = QStringLiteral("steeringWheelAxis");

    inline const static QString steeringWheelAxisDeviceName
        = QStringLiteral("steeringWheelAxisDeviceName");

    inline const static QString isThrottleReverse
        = QStringLiteral("isThrottleReverse");

    inline const static QString isBrakeReverse
        = QStringLiteral("isBrakeReverse");

    inline const static QString acceleration_100km_time_s
        = QStringLiteral("acceleration_100km_time_s");

    inline const static QString stop_100km_dis_m
        = QStringLiteral("stop_100km_dis_m");

    inline const static QString maxSpeed_km_h
        = QStringLiteral("maxSpeed_km_h");

    inline const static QString maxForceFeedbackGain
        = QStringLiteral("maxForceFeedbackGain");

    inline const static QString isConstantForceMode
        = QStringLiteral("isConstantForceMode");

    inline const static QString constantCorrectiveForceGain
        = QStringLiteral("constantCorrectiveForceGain");

    inline const static QString constantDampingGain
        = QStringLiteral("constantDampingGain");

    inline const static QString springCurve
        = QStringLiteral("springCurve");
    inline const static QString dampingCurve
        = QStringLiteral("dampingCurve");

    inline const static QString SYSTEM_forceFeedbackSettings_maxSpringGain
        = QStringLiteral("maxSpringGain");
    inline const static QString SYSTEM_forceFeedbackSettings_maxDamperGain
        = QStringLiteral("maxDamperGain");
};
