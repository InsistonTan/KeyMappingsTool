#pragma once

#include "qmutex.h"
#include "dinput.h"
#include "models/UserConfig.h"
#include <QObject>
#include <QMutex>

#define WORKER_SLEEP_TIME_MS 10 // 线程sleep的时间间隔 ms
#define GROUND_FRICTION_COEFFICIENT -0.015 // 地面摩檫力系数
#define G 9.8 // 重力加速度

class ForceFeedbackWorker : public QObject
{
    Q_OBJECT

private:
    volatile bool isWorkerRunning = false;

    // 已初始化的设备列表(转向轴设备, 油门设备, 刹车设备)
    QVector<LPDIRECTINPUTDEVICE8> initedDeviceList;
    // initedDeviceList的锁
    QMutex mutex_initedDeviceList;

    QString throttleAxis = ""; // 油门所在的轴
    QString throttleAxisDeviceName = ""; // 油门轴的设备名称

    QString brakeAxis = ""; // 刹车所在的轴
    QString brakeAxisDeviceName = ""; // 刹车轴的设备名称

    QString steeringWheelAxis = "";// 方向盘盘面所在的轴
    QString steeringWheelAxisDeviceName = "";// 方向盘盘面所在的轴的设备名称

    bool isThrottleReverse = false;// 是否反转油门
    bool isBrakeReverse = false;// 是否反转刹车
    double acceleration_100km_time_s = default_acceleration_100km_time_s;// 百公里加速所需时间(秒)
    int stop_100km_dis_m = default_stop_100km_dis_m;// 百公里刹停所需距离(米)
    double maxSpeed_m_s = default_maxSpeed_km_h * 1000.0 / 3600.0;// 车辆最高时速(m/s)

    // 最大强度
    double maxSpringGain = default_max_forcefeedback_gain;
    double maxDamperGain = default_max_forcefeedback_gain;

    // 回正力强度-车速曲线的查找表;
    // key: 车速百分比 * 1000;
    // value: 回正力强度百分比
    QHash<int, double> springGainLUT;
    // 转向阻尼强度-车速曲线的查找表;
    // key: 车速百分比 * 1000;
    // value: 转向阻尼强度百分比
    QHash<int, double> dampingGainLUT;

    // (废弃)
    //double maxForceFeedbackGain = default_max_forcefeedback_gain; // 最大力回馈强度
    //bool isConstantForceMode = false;// 是否为恒定力反馈模式
    //double constantCorrectiveForceGain = default_constant_corrective_force_gain;// 恒定回正力强度
    //double constantDampingGain = default_constant_damping_gain;// 恒定转向阻尼强度


    // 油门踏板的数值范围
    DIPROPRANGE throttleValueRange;
    // 刹车踏板的数值范围
    DIPROPRANGE brakeValueRange;

    double maxThrottleAxisA = 0;// 最大油门加速度
    double maxBrakeA = 0;// 最大刹车加速度


    // 已初始化的转向轴设备实例
    LPDIRECTINPUTDEVICE8 pSteeringWheelAxisDeviceInstance = nullptr;

    LPDIRECTINPUTEFFECT g_pConstantForce = NULL; // 恒定力效果实例, 使用恒定力代替弹簧力
    LPDIRECTINPUTEFFECT g_pSpringForce = NULL; // 弹簧效果实例
    LPDIRECTINPUTEFFECT g_pDamper = NULL;       // 阻尼效果实例
    DIEFFECT diConstantEffect = { 0 };// 恒定力效果参数 DIEFFECT
    DIEFFECT diSpringEffect = { 0 };// 弹簧效果参数 DIEFFECT
    DIEFFECT diDamperEffect = { 0 };// 阻尼效果参数 DIEFFECT
    DICONSTANTFORCE diConstantsCondition;// 恒定力系数
    DICONDITION diSpringCondition = {0};// 弹簧效果系数参数 DICONDITION
    DICONDITION diDamperCondition = {0};// 阻尼效果系数参数 DICONDITION

    LONG springEffectValue = 0;// 弹簧效果系数(0-10000)
    LONG damperEffectValue = 0;// 阻尼效果系数(0-10000)

    // 初始化
    void init();

    // 检查设备是否已连接
    bool checkDevicesConnected();

    // 创建力回馈效果
    bool createDynamicEffects(QString steerWheelAxis);
    // 播放力反馈效果
    bool playDynamicEffects();
    // 根据车速更新力回馈
    void updateForceFeedback(double speed_m_s, double totalA);
    // 关闭资源
    void cleanup();

    // 获取已初始化设备列表的快照
    QVector<LPDIRECTINPUTDEVICE8> getInitedDeviceListSnapshot(){
        QMutexLocker locker(&mutex_initedDeviceList);
        return initedDeviceList;
    }

    double getMaxSpeed_m_s(int maxSpeed_km_h){
        return maxSpeed_km_h * 1000.0 / 3600.0;
    }

    double getMaxThrottleAxisA(double acceleration_100km_time_s){
        return 100.0 * 1000 / 3600 / acceleration_100km_time_s;
    }

    double getmaxBrakeA(double stop_100km_dis_m){
        return -771.6 / (2 * stop_100km_dis_m);
    }

public:
    explicit ForceFeedbackWorker();

signals:
    // 线程结束信号
    void workFinished();
    // 力反馈模拟开启的结果信息, 成功 result = true, 否则 = false
    void startFFBSimResult(bool result, QString msg);

public slots:
    void doWork();

    // 取消运行
    void cancelWorkSlot();

    // 力反馈模拟的设置改变
    void settingsChangeSlot();
};
