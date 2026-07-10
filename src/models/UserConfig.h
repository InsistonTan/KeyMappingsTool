#pragma once

#include "qcontainerfwd.h"
#include "models/MappingRelation.h"
#include "ui/widgets/CurveEditor.h"
#include <common/UserConfigKey.h>

#include <QString>
#include <QMap>
#include <QMetaType>
#include <QVariant>
#include <QJsonObject>


// 轴映射按键时内部死区的默认值
#define DEFAULT_INNER_DEADAREA_VALUE 0.03
// 模拟鼠标移动时, 鼠标移动速度的倍率默认值
#define DEFAULT_MOUSE_MOVE_SPEED_TIMES_VALUE 1.0

#define default_acceleration_100km_time_s 8 //默认百公里加速时间单位s
#define default_stop_100km_dis_m 30 // 默认百公里刹停距离单位m
#define default_maxSpeed_km_h 200 // 默认最高时速单位km/h
#define default_max_forcefeedback_gain 0.5 // 默认最大力回馈强度
#define default_constant_corrective_force_gain 0.5 // 默认恒定回正力强度
#define default_constant_damping_gain 0.5 // 默认恒定转向阻尼强度

///
/// \brief The UserConfig class
/// 用户配置信息类型
///
class UserConfig
{


public:
    // 轴映射按键时,转向轴内部死区
    double steeringAxisInnerDeadZone = DEFAULT_INNER_DEADAREA_VALUE;
    // 轴映射按键时,踏板轴内部死区
    double pedalAxisInnerDeadZone = DEFAULT_INNER_DEADAREA_VALUE;
    // 手柄摇杆内部死区值
    double xboxJoystickInnerDeadAreaValue = 0.0;
    // 手柄扳机内部死区值
    double xboxTriggerInnerDeadAreaValue = 0.0;
    //模拟鼠标移动速度的倍率
    double mouseMoveSpeedTimes = DEFAULT_MOUSE_MOVE_SPEED_TIMES_VALUE;


    // 欧卡2安装目录
    QString ETS2InstallPath = "";
    // 欧卡2辅助功能_开启自动解除手刹
    bool ETS2_enableAutoCancelHandbrake = false;
    // 映射软件的系统功能_开启软件后立即开启映射
    bool SYSTEM_enableMappingAfterOpening = false;
    // 组合键按下时只执行组合键映射，不执行对应子键映射
    bool SYSTEM_enableOnlyLongestMapping = false;
    // 新增映射时，只返回变化的按键
    bool SYSTEM_enableOnlyChangeKeyWhenNew = false;
    // 开启力反馈模拟
    bool SYSTEM_enableForceFeedback = false;
    // 开启开机自启动
    bool SYSTEM_enableRunUponStartup = false;
    // 关闭主窗口隐藏到系统托盘
    bool SYSTEM_enableHideWindowOnClose = true;
    // 是否开启设备名称强唯一模式, 开启该模式, 设备名称将附带设备接口路径信息
    bool SYSTEM_enableStrongUniqueDeviceNameMode = false;
    // 上一次使用的设备名称列表
    QVector<QString> SYSTEM_lastUsedDeviceNameList = {};
    // 上一次使用的映射配置文件路径
    QString SYSTEM_lastUsedMappingConfigPath = "";
    // 上一次使用的映射列表缓存;
    // 正常情况下是根据"SYSTEM_lastUsedMappingConfigPath"上一次使用的映射配置文件路径 进行加载;
    // 但如果用户没有保存映射配置到文件, 然后启动了全局映射, 就需要这个cache来记录用户使用的映射列表;
    // 保证下次启动能恢复上一次使用的映射;
    QVector<MappingRelation> SYSTEM_lastUsedMappingCache = {};


    QString SYSTEM_forceFeedbackSettings_throttleAxis = ""; // 油门所在的轴
    QString SYSTEM_forceFeedbackSettings_throttleAxisDeviceName = ""; // 油门轴的设备名称

    QString SYSTEM_forceFeedbackSettings_brakeAxis = ""; // 刹车所在的轴
    QString SYSTEM_forceFeedbackSettings_brakeAxisDeviceName = ""; // 刹车轴的设备名称

    QString SYSTEM_forceFeedbackSettings_steeringWheelAxis = "";// 方向盘盘面所在的轴
    QString SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName = "";// 方向盘盘面所在的轴的设备名称

    bool SYSTEM_forceFeedbackSettings_isThrottleReverse = false;// 是否反转油门
    bool SYSTEM_forceFeedbackSettings_isBrakeReverse = false;// 是否反转刹车

    double SYSTEM_forceFeedbackSettings_acceleration_100km_time_s = default_acceleration_100km_time_s;// 百公里加速所需时间(秒)
    int SYSTEM_forceFeedbackSettings_stop_100km_dis_m = default_stop_100km_dis_m;// 百公里刹停所需距离(米)
    int SYSTEM_forceFeedbackSettings_maxSpeed_km_h = default_maxSpeed_km_h;// 车辆最高时速(km/h)
    double SYSTEM_forceFeedbackSettings_maxSpringGain = default_max_forcefeedback_gain;// 最大回正力强度
    double SYSTEM_forceFeedbackSettings_maxDamperGain = default_max_forcefeedback_gain;// 最大阻尼强度
    //double SYSTEM_forceFeedbackSettings_maxForceFeedbackGain = default_max_forcefeedback_gain; // 最大力回馈强度
    //bool SYSTEM_forceFeedbackSettings_isConstantForceMode = false;// 是否为恒定力反馈模式
    //double SYSTEM_forceFeedbackSettings_constantCorrectiveForceGain = default_constant_corrective_force_gain;// 恒定回正力强度
    //double SYSTEM_forceFeedbackSettings_constantDampingGain = default_constant_damping_gain;// 恒定转向阻尼强度
    // 力反馈模拟-回正力曲线的点集合
    QVector<CurveEditor::BezierLogicalPoint> SYSTEM_forceFeedbackSettings_springCurve = {};
    // 力反馈模拟-转向阻尼曲线的点集合
    QVector<CurveEditor::BezierLogicalPoint> SYSTEM_forceFeedbackSettings_dampingCurve = {};

    UserConfig();

    // 将当前用户配置对象转 json对象
    QJsonObject toJson();


    // 将json对象转当前对象
    static UserConfig fromJsonObject(QJsonObject obj);

    // 从json对象中读取配置
    static void readSettingsFromJson(QJsonObject jsonObj, UserConfig& out);
    // 从json对象中读取旧版死区设置
    static void readDeadZoneSettingsFromJson(QJsonObject jsonObj, UserConfig& out);
    // 从json对象中读取旧版辅助功能设置
    static void readAssistFuncSettingsFromJson(QJsonObject jsonObj, UserConfig& out);

private:
    // 默认的回正力-车速曲线
    inline static const QVector<CurveEditor::BezierLogicalPoint> defaultSpringPoints = {
        {
            {0, 0}, // 主点
            {0, 0}, // in
            {25.4, 53}, // out
        },
        {
            {100, 55}, // 主点
            {78.8, 41}, // in
            {100, 55}, // out
        },
        };
    // 默认的转向阻尼-车速曲线
    inline static const QVector<CurveEditor::BezierLogicalPoint> defaultDampingPoints = {
        {
            {0, 50}, // 主点
            {0, 50}, // in
            {3.4, 31.333333}, // out
        },
        {
            {19.2, 19}, // 主点
            {14.2, 19}, // in
            {26.4, 13.333333}, // out
        },
        {
            {100,80}, // 主点
            {69.6, 46}, // in
            {100, 79.666666}, // out
        },
        };
};
