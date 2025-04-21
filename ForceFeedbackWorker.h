#ifndef FORCEFEEDBACKWORKER_H
#define FORCEFEEDBACKWORKER_H
#include "ForceFeedbackSettingsWindow.h"
#include "dinput.h"
#include <QObject>

#define WORKER_SLEEP_TIME_MS 10 // 线程sleep的时间间隔 ms
#define GROUND_FRICTION_COEFFICIENT -0.015 // 地面摩檫力系数
#define G 9.8 // 重力加速度

class ForceFeedbackWorker : public QObject
{
    Q_OBJECT

private:
    volatile bool isWorkerRunning = false;
    ForceFeedbackSettingsWindow* forceFeedbackSettings;

    QString throttleAxis = ""; // 油门所在的轴
    QString brakeAxis = ""; // 刹车所在的轴
    QString steeringWheelAxis = "";// 方向盘盘面所在的轴
    bool isThrottleReverse = false;// 是否反转油门
    bool isBrakeReverse = false;// 是否反转刹车
    double acceleration_100km_time_s = default_acceleration_100km_time_s;// 百公里加速所需时间(秒)
    int stop_100km_dis_m = default_stop_100km_dis_m;// 百公里刹停所需距离(米)
    double maxSpeed_m_s = default_maxSpeed_km_h * 1000.0 / 3600.0;// 车辆最高时速(m/s)
    double maxForceFeedbackGain = default_max_forcefeedback_gain; // 最大力回馈强度

    // 油门踏板的数值范围
    DIPROPRANGE throttleValueRange;
    // 刹车踏板的数值范围
    DIPROPRANGE brakeValueRange;

    double maxThrottleAxisA = 0;// 最大油门加速度
    double maxBrakeA = 0;// 最大刹车加速度

    LPDIRECTINPUT8 g_pDirectInput2 = NULL;
    LPDIRECTINPUTDEVICE8 g_pDevice2 = NULL;// 输入设备
    int lastDeviceIndex2 = -99;

    LPDIRECTINPUTEFFECT g_pSpringForce = NULL; // 弹簧效果
    LPDIRECTINPUTEFFECT g_pDamper = NULL;       // 阻尼效果
    DIEFFECT diSpring = { 0 };// 弹簧效果参数 DIEFFECT
    DIEFFECT diDamper = { 0 };// 阻尼效果参数 DIEFFECT
    DICONDITION diSpringCondition = {0};// 弹簧效果参数 DICONDITION
    DICONDITION diDamperCondition = {0};// 阻尼效果参数 DICONDITION

    void init();

    // 初始化 DirectInput
    bool initDirectInput2();
    // 打开选择的设备
    bool openDiDevice2(int deviceIndex, HWND hWnd);
    // 创建力回馈效果
    bool createDynamicEffects(QString steerWheelAxis);
    // 根据车速更新力回馈
    void updateForceFeedback(double speed_m_s, double maxSpeed);
    // 关闭资源
    void cleanup();
    // 获取设备状态信息
    QList<MappingRelation*> getInputState2();

public:
    explicit ForceFeedbackWorker(ForceFeedbackSettingsWindow* forceFeedbackSettings);

signals:
    void workFinished();

public slots:
    void doWork();

    // 取消运行
    void cancelWorkSlot();

    // 力反馈模拟的设置改变
    void settingsChangeSlot();
};

#endif // FORCEFEEDBACKWORKER_H
