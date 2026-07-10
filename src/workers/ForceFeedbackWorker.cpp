#include "ForceFeedbackWorker.h"
#include "common/Global.h"
#include "common/StringConstants.h"
#include "models/MappingRelation.h"
#include "services/ConfigService.h"
#include "services/DirectInputService.h"
#include "services/LogService.h"
#include "ui/widgets/CurveEditor.h"

#include <QThread>
#include <QCoreApplication>
#include <qcontainerfwd.h>
#include <qwindowdefs_win.h>
#include <thread>

#define FORCE_GAIN 1 // 力反馈的整体强度系数(0-1), 影响整体的力反馈强度
#define MIN_FORCE_POWER 300 // 最低力反馈强度的值, 影响弹簧效果(回正力)和转向阻尼的最低强度
#define MAX_SPEED 350; // 最高车速km/h

#define RHO 1.225  // 空气密度 kg/m³
#define CAR_Cd 0.3 // 汽车风阻系数
#define CAR_A 2.2  // 汽车正投影面积 m²
#define CAR_m 1500 // 汽车质量 kg

#define STATIC_DAMPER_PER 1 // 车辆静止状态下, 方向盘的阻尼系数(0-1)
#define STATIC_SPRING_PER 0.05 // 车辆静止状态下, 方向盘的弹簧(回正力)系数(0-1)
#define LOW_SPEED_SPRING_PER 0.1 // 车辆低速状态下, 方向盘的弹簧(回正力)系数(0-1)

ForceFeedbackWorker::ForceFeedbackWorker()
{
    init();
}

void ForceFeedbackWorker::init(){
    // 获取配置
    auto userConfig = ConfigService::get(ConfigService::GetSource::FFBSim);

    throttleAxis = userConfig.SYSTEM_forceFeedbackSettings_throttleAxis;
    throttleAxisDeviceName = userConfig.SYSTEM_forceFeedbackSettings_throttleAxisDeviceName;

    brakeAxis = userConfig.SYSTEM_forceFeedbackSettings_brakeAxis;
    brakeAxisDeviceName = userConfig.SYSTEM_forceFeedbackSettings_brakeAxisDeviceName;

    steeringWheelAxis = userConfig.SYSTEM_forceFeedbackSettings_steeringWheelAxis;
    steeringWheelAxisDeviceName = userConfig.SYSTEM_forceFeedbackSettings_steeringWheelAxisDeviceName;

    isBrakeReverse = userConfig.SYSTEM_forceFeedbackSettings_isBrakeReverse;
    isThrottleReverse = userConfig.SYSTEM_forceFeedbackSettings_isThrottleReverse;

    acceleration_100km_time_s = userConfig.SYSTEM_forceFeedbackSettings_acceleration_100km_time_s;
    stop_100km_dis_m = userConfig.SYSTEM_forceFeedbackSettings_stop_100km_dis_m;
    maxSpeed_m_s = getMaxSpeed_m_s(userConfig.SYSTEM_forceFeedbackSettings_maxSpeed_km_h);

    maxSpringGain = userConfig.SYSTEM_forceFeedbackSettings_maxSpringGain;
    maxDamperGain = userConfig.SYSTEM_forceFeedbackSettings_maxDamperGain;


    // maxForceFeedbackGain = userConfig.SYSTEM_forceFeedbackSettings_maxForceFeedbackGain;
    // isConstantForceMode = userConfig.SYSTEM_forceFeedbackSettings_isConstantForceMode;
    // constantCorrectiveForceGain = userConfig.SYSTEM_forceFeedbackSettings_constantCorrectiveForceGain;
    // constantDampingGain = userConfig.SYSTEM_forceFeedbackSettings_constantDampingGain;

    // 生成 回正力强度曲线的查找表
    // 该表总共1001个值(0-1000)
    springGainLUT.clear();
    for(int x = 0; x <= 1000; x++){
        // 根据x轴值(由于x轴值是车速百分比*100, 而当前x是百分比*1000, 所以需要 x/10.0), 获取y轴值
        double y = CurveEditor::getYAxisLogicalValue(userConfig.SYSTEM_forceFeedbackSettings_springCurve, x/10.0);
        // y轴值是强度百分比*100, 所以需要 y/100 才是百分比
        springGainLUT.insert(x, y/100.0);
    }
    // 生成 转向阻尼强度曲线的查找表
    // 该表总共1001个值(0-1000)
    damperGainLUT.clear();
    for(int x = 0; x <= 1000; x++){
        // 根据x轴值, 获取y轴值
        double y = CurveEditor::getYAxisLogicalValue(userConfig.SYSTEM_forceFeedbackSettings_dampingCurve, x/10.0);
        // y轴值是强度百分比*100, 所以需要 y/100 才是百分比
        damperGainLUT.insert(x, y/100.0);
    }

    // 最大油门加速度
    maxThrottleAxisA = getMaxThrottleAxisA(acceleration_100km_time_s);
    // 最大刹车加速度
    maxBrakeA = getmaxBrakeA(stop_100km_dis_m);


    if(isWorkerRunning){
        // 释放旧的设备
        for(auto& d : initedDeviceList){
            d->Unacquire();
            d->Release();
        }
        // 检查设备
        if(checkDevicesConnected() == false){
            isWorkerRunning = false;
        }

        // 如果还在运行, 根据新的配置信息重新创建力反馈效果
        if(isWorkerRunning){
            if(createDynamicEffects(this->steeringWheelAxis) == false){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_createEffectsErrorMsg);
                isWorkerRunning = false;
            }else if(playDynamicEffects() == false){
                isWorkerRunning = false;
            }
        }
    }
}

bool ForceFeedbackWorker::checkDevicesConnected()
{
    //auto userConfig = ConfigService::get(ConfigService::GetSource::FFBSim);

    // 转向未设置
    if(steeringWheelAxisDeviceName.isEmpty())
    {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_WheelDeviceEmptyErrorMsg);
        return false;
    }
    // 检查油门轴和刹车轴设备
    if(throttleAxisDeviceName.isEmpty() || brakeAxisDeviceName.isEmpty()){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceEmptyErrorMsg);
        return false;
    }

    // 扫描设备
    DirectInputService::scanDevice();

    // 用后台独占模式初始化转向设备, 并获取已初始化的转向轴设备实例 DISCL_EXCLUSIVE | DISCL_BACKGROUND  DISCL_FOREGROUND
    pSteeringWheelAxisDeviceInstance = DirectInputService::openDiDevice(Global::getHideWindowHWnd(),
                                                                        steeringWheelAxisDeviceName,
                                                                        DISCL_EXCLUSIVE | DISCL_BACKGROUND);

    if(pSteeringWheelAxisDeviceInstance == nullptr){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_WheelDeviceOpenErrorMsg);
        return false;
    }

    // 检查设备是否支持力反馈
    if(DirectInputService::checkIsSupportForceFeedback(pSteeringWheelAxisDeviceInstance) == false)
    {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_steeringWheelDeviceErrorMsg.arg(steeringWheelAxisDeviceName));
        return false;
    }


    // 加锁
    QMutexLocker locker(&mutex_initedDeviceList);
    // 重置
    initedDeviceList.clear();

    // 添加设备到列表
    initedDeviceList.append(pSteeringWheelAxisDeviceInstance);

    // 防止相同设备重复初始化
    if(throttleAxisDeviceName != steeringWheelAxisDeviceName){
        QVector<QString> deviceNameList = {throttleAxisDeviceName};
        if(brakeAxisDeviceName != throttleAxisDeviceName){
            deviceNameList.append(brakeAxisDeviceName);
        }

        // 油门轴和刹车轴设备连接失败
        if(DirectInputService::openDiDevice(deviceNameList) == false){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceOpenErrorMsg);
            return false;
        }else{
            for(auto& deviceName : deviceNameList){
                auto initedDevice = DirectInputService::getInitedDevice(deviceName);
                if(initedDevice == nullptr){
                    Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceGetInstanceErrorMsg);
                    return false;
                }else{
                    // 添加到已初始化设备列表
                    initedDeviceList.append(initedDevice);
                }
            }
        }
    }


    // 获取设备轴的数值范围map
    auto axisValueRangeMap = DirectInputService::getAxisValueRangeMap();

    // 获取不到转向轴的数值范围
    auto steeringAxisKey = Global::getBtnOrAxisFullName(steeringWheelAxisDeviceName, steeringWheelAxis);
    if(axisValueRangeMap.contains(steeringAxisKey) == false)
    {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_steeringAxisRangeErrorMsg);
        return false;
    }

    // 获取不到油门踏板的数值范围
    auto throttleAxisKey = Global::getBtnOrAxisFullName(throttleAxisDeviceName, throttleAxis);
    if(axisValueRangeMap.contains(throttleAxisKey) == false)
    {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleAxisRangeErrorMsg);
        return false;
    }

    // 获取不到刹车踏板的数值范围
    auto brakeAxisKey = Global::getBtnOrAxisFullName(brakeAxisDeviceName, brakeAxis);
    if(axisValueRangeMap.contains(brakeAxisKey) == false)
    {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_brakeAxisRangeErrorMsg);
        return false;
    }

    // 油门踏板的数值范围
    this->throttleValueRange = axisValueRangeMap.value(Global::getBtnOrAxisFullName(throttleAxisDeviceName, throttleAxis));
    // 刹车踏板的数值范围
    this->brakeValueRange = axisValueRangeMap.value(Global::getBtnOrAxisFullName(brakeAxisDeviceName, brakeAxis));

    return true;
}

// 枚举设备支持的力反馈效果回调
BOOL CALLBACK EnumEffectsCallback(const DIEFFECTINFO *pdei, VOID *pvRef){
    qDebug() << "Effect:" << QString::fromWCharArray(pdei->tszName);

    //qDebug() << "GUID:" << pdei->guid.guid.Data1;

    return DIENUM_CONTINUE;
}

// 创建力回馈效果
bool ForceFeedbackWorker::createDynamicEffects(QString steerWheelAxis){
    // 枚举设备支持的力反馈效果
    // pSteeringWheelAxisDeviceInstance->EnumEffects(
    //     EnumEffectsCallback,
    //     nullptr,
    //     DIEFT_ALL
    //     );

    bool res1 = false, res2 = false;

    DWORD axes[1];
    if(steerWheelAxis == StringConstants::axisX){
        axes[0] = DIJOFS_X;
    }else if(steerWheelAxis == StringConstants::axisY){
        axes[0] = {DIJOFS_Y};
    }else if(steerWheelAxis == StringConstants::axisZ){
        axes[0] = {DIJOFS_Z};
    }else if(steerWheelAxis == StringConstants::axisRX){
        axes[0] = {DIJOFS_RX};
    }else if(steerWheelAxis == StringConstants::axisRY){
        axes[0] = {DIJOFS_RY};
    }else if(steerWheelAxis == StringConstants::axisRZ){
        axes[0] = {DIJOFS_RZ};
    }else if(steerWheelAxis == StringConstants::axisS1){
        axes[0] = {DIJOFS_SLIDER(0)};
    }else if(steerWheelAxis == StringConstants::axisS2){
        axes[0] = {DIJOFS_SLIDER(1)};
    }

    if(g_pSpringForce != NULL){
        g_pSpringForce->Release();
        g_pSpringForce = NULL;
    }

    // 创建弹簧效果
    if(g_pSpringForce == NULL){
        // 结构说明
        // typedef struct DICONDITION {
        //     LONG lOffset;          // 弹簧中心位置
        //     LONG lPositiveCoefficient; // 正方向弹簧系数
        //     LONG lNegativeCoefficient; // 负方向弹簧系数
        //     LONG lPositiveSaturation;  // 正方向最大力
        //     LONG lNegativeSaturation;  // 负方向最大力
        //     LONG lDeadBand;            // 死区
        // } DICONDITION

        // 设置弹簧效果的系数
        diSpringCondition = {0, 3000, 3000, DI_FFNOMINALMAX, DI_FFNOMINALMAX, 0};

        // 结构说明
        // typedef struct DIEFFECT {
        //     DWORD dwSize;                       // 结构体大小，必须初始化为 sizeof(DIEFFECT)
        //     DWORD dwFlags;                      // 关键标志，组合使用：• 坐标系：DIEFF_CARTESIAN（笛卡尔）、DIEFF_POLAR（极坐标）、DIEFF_SPHERICAL（球坐标）三选一。• 对象标识：DIEFF_OBJECTIDS（使用对象 ID）或 DIEFF_OBJECTOFFSETS（使用数据偏移量，如 DIJOFS_X）二选一。
        //     DWORD dwDuration;                   // 效果总持续时间（微秒），INFINITE 表示无限
        //     DWORD dwSamplePeriod;               // 采样周期（微秒），0 表示使用设备默认
        //     DWORD dwGain;                       // 全局增益（0~10000），10000 为最大强度
        //     DWORD dwTriggerButton;              // 触发按钮标识，DIEB_NOTRIGGER 表示无按钮触发
        //     DWORD dwTriggerRepeatInterval;      // 触发重复间隔（微秒），INFINITE 表示不重复
        //     DWORD cAxes;                        // 效果涉及的轴数量
        //     LPDWORD rgdwAxes;                   // 指向轴标识符数组（如 { DIJOFS_X }）
        //     LPLONG rglDirection;                // 指向方向数组，坐标类型由 dwFlags 指定
        //     LPDIENVELOPE lpEnvelope;            // 包络线指针（淡入淡出），NULL 表示无包络
        //     DWORD cbTypeSpecificParams;         // 类型特定参数结构体的大小（字节）
        //     LPVOID lpvTypeSpecificParams;       // 指向类型特定参数结构体的指针
        //     DWORD dwStartDelay;                 // 启动延迟时间（微秒），0 表示无延迟
        // } DIEFFECT, *LPDIEFFECT;

        diSpringEffect = {sizeof(DIEFFECT)};
        diSpringEffect.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
        diSpringEffect.dwDuration = INFINITE;
        diSpringEffect.dwSamplePeriod = 0;
        diSpringEffect.dwGain = DI_FFNOMINALMAX;
        diSpringEffect.dwTriggerButton = DIEB_NOTRIGGER;
        diSpringEffect.dwTriggerRepeatInterval = INFINITE;
        diSpringEffect.cAxes = 1;

        diSpringEffect.rgdwAxes = axes;
        LONG dir = 0;
        diSpringEffect.rglDirection = &dir;
        diSpringEffect.lpEnvelope = NULL;
        diSpringEffect.cbTypeSpecificParams = sizeof(DICONDITION);
        diSpringEffect.lpvTypeSpecificParams = &diSpringCondition;
        diSpringEffect.dwStartDelay = 0;

        // 创建效果
        HRESULT hr1 = pSteeringWheelAxisDeviceInstance->CreateEffect(GUID_Spring, &diSpringEffect, &g_pSpringForce, NULL);

        res1 = SUCCEEDED(hr1);
    }


    if(g_pDamper != NULL){
        g_pDamper->Release();
        g_pDamper = NULL;
    }

    // 创建阻尼效果
    if(g_pDamper == NULL){
        // Condition
        diDamperCondition = {0, 3000, 3000, DI_FFNOMINALMAX, DI_FFNOMINALMAX, 0};

        // 阻尼（Damper）
        diDamperEffect = {sizeof(DIEFFECT)};
        diDamperEffect.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
        diDamperEffect.dwDuration = INFINITE;
        diDamperEffect.dwSamplePeriod = 0;
        diDamperEffect.dwGain = DI_FFNOMINALMAX;
        diDamperEffect.dwTriggerButton = DIEB_NOTRIGGER;
        diDamperEffect.dwTriggerRepeatInterval = INFINITE;
        diDamperEffect.cAxes = 1;

        diDamperEffect.rgdwAxes = axes;
        LONG dir = 0;
        diDamperEffect.rglDirection = &dir;
        diDamperEffect.lpEnvelope = NULL;
        diDamperEffect.cbTypeSpecificParams = sizeof(DICONDITION);
        diDamperEffect.lpvTypeSpecificParams = &diDamperCondition;
        diDamperEffect.dwStartDelay = 0;

        // 创建效果
        HRESULT hr2 = pSteeringWheelAxisDeviceInstance->CreateEffect(GUID_Damper, &diDamperEffect, &g_pDamper, NULL);

        res2 = SUCCEEDED(hr2);
    }

    return res1 && res2;
}

bool ForceFeedbackWorker::playDynamicEffects()
{
    HRESULT hr = pSteeringWheelAxisDeviceInstance->Acquire();
    if(hr != DI_OK){
        pSteeringWheelAxisDeviceInstance->Unacquire();
        hr = pSteeringWheelAxisDeviceInstance->Acquire();
    }

    if(hr != DI_OK){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::acquireSteeringDeviceFailed);
        return false;
    }

    // 开启力反馈效果
    if(g_pSpringForce->Start(1, 0) != DI_OK || g_pDamper->Start(1, 0) != DI_OK){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::playEffectsFailed);
        return false;
    }

    return true;
}

// 根据车速更新力回馈
void ForceFeedbackWorker::updateForceFeedback(double speed_m_s, double totalA){
    // 当前车速百分比
    double speedPer = speed_m_s / maxSpeed_m_s;

    // 根据查找表, 找到强度系数百分比, 再乘以强度最大值, 再乘以最大强度百分比 得到实际强度系数
    springEffectValue = (LONG)(springGainLUT.value(((int)(speedPer*1000))) * DI_FFNOMINALMAX * maxSpringGain);
    damperEffectValue = (LONG)(damperGainLUT.value(((int)(speedPer*1000))) * DI_FFNOMINALMAX * maxDamperGain);

    // 值检验
    if(springEffectValue < 0){
        springEffectValue = 0;
    }
    else if (springEffectValue > DI_FFNOMINALMAX){
        springEffectValue = DI_FFNOMINALMAX;
    }

    if(damperEffectValue < 0){
        damperEffectValue = 0;
    }
    else if (damperEffectValue > DI_FFNOMINALMAX){
        damperEffectValue = DI_FFNOMINALMAX;
    }

    //qDebug() << "springEffectValue: " << springEffectValue << "speed_m_s: " << speed_m_s << ", maxSpeed_m_s: " <<  maxSpeed_m_s << ", speedPer: " << speedPer << ", springGainLUT.value:" << springGainLUT.value(((int)(speedPer*1000)));
    //qDebug() << "speedPer: " << speedPer << ", springEffectValue: " << springEffectValue << ", damperEffectValue: " << damperEffectValue;

    // 弹簧效果强度系数
    // 方向盘向右转动的回正力系数
    diSpringCondition.lPositiveCoefficient = springEffectValue;
    // 方向盘向左转动的回正力系数
    diSpringCondition.lNegativeCoefficient = diSpringCondition.lPositiveCoefficient;

    // 阻尼效果强度系数
    diDamperCondition.lPositiveCoefficient = damperEffectValue;
    diDamperCondition.lNegativeCoefficient = diDamperCondition.lPositiveCoefficient;


    // (弃用)
    // 更新恒定力强度增益
    // 恒定力(代替弹簧力)强度增益
    // diConstantEffect.dwGain = springEffectValue;
    // if(g_pConstantForce){
    //     g_pConstantForce->SetParameters(&diConstantEffect, DIEP_GAIN);
    // }

    // 更新回正力
    if (g_pSpringForce) {
        //g_pSpringForce->SetParameters(&diSpring, DIEP_GAIN);
        g_pSpringForce->SetParameters(&diSpringEffect, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }

    // 更新阻尼
    if (g_pDamper) {
        g_pDamper->SetParameters(&diDamperEffect, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }
}

// 关闭资源
void ForceFeedbackWorker::cleanup() {
    if (g_pSpringForce) g_pSpringForce->Release();
    if (g_pDamper) g_pDamper->Release();
}


void ForceFeedbackWorker::doWork(){
    double currentV = 0.0;// 当前车速
    double groundA = GROUND_FRICTION_COEFFICIENT * G;// 地面滚动摩檫力加速度
    double cycleTimeOfEachRound = (WORKER_SLEEP_TIME_MS + 10.0) / 1000.0;// 每轮循环所花费的时间(单位s)
    //double linerThrottleASpeedPer = 0.5;// 油门产生的加速度与踩下的深度呈线性增加关系时的最大车速百分比

    isWorkerRunning = true;

    // 检查方向盘设备是否连接
    if(checkDevicesConnected() == false){
        LogService::parseErrorLog(StringConstants::ffbSimulateThread_initDevicesErrorMsg);
        isWorkerRunning = false;
        emit startFFBSimResult(false, StringConstants::ffbSimulateThread_initDevicesErrorMsg);
    }

    if(isWorkerRunning){
        if(createDynamicEffects(this->steeringWheelAxis) == false || playDynamicEffects() == false){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_createEffectsErrorMsg);
            isWorkerRunning = false;
            emit startFFBSimResult(false, StringConstants::ffbSimulateThread_createEffectsErrorMsg);
        }else{
            LogService::parseSuccessLog(StringConstants::ffbSimulateThread_successMsg);
        }
    }

    // 设置系统计时器精度
    Global::setSystemTimePeriod_1ms();

    // 高精度时钟, 用于固定while循环的执行频率
    using clock = std::chrono::high_resolution_clock;
    // 本轮执行的开始时间
    auto currentExcuteTime = clock::now();

    // 执行频率 hz
    auto excuteFrequency = 200;
    // 每轮执行的最大时间 毫秒
    auto each_mstime = 1000 / excuteFrequency;

    // 存储设备数据结果
    QVector<MappingRelation> res;

    while(isWorkerRunning){
        auto now = clock::now();
        // 如果本轮执行的开始时间 已经滞后, 重置本轮执行的开始时间
        if(currentExcuteTime < now){
            currentExcuteTime = now;
        }

        // 激活设备
        pSteeringWheelAxisDeviceInstance->Acquire();
        if(FAILED(pSteeringWheelAxisDeviceInstance->Poll())){
            pSteeringWheelAxisDeviceInstance->Acquire();
        }

        // 获取设备状态数据, 只获取轴数据, 不获取按键数据
        DirectInputService::getInputState(res, initedDeviceList, false, true);

        double totalA = 0.0;// 总的加速度

        // 计算当前车速下空气阻力f
        double airF = 0.5 * RHO * CAR_Cd * CAR_A * currentV * currentV;
        // 得到空气阻力的加速度a
        double airA = - airF / CAR_m;

        for(auto devData : res){
            // 获取油门数据
            if(devData.deviceName == this->throttleAxisDeviceName && devData.dev_btn_name == this->throttleAxis){
                // 油门踩下的程度(0-1)
                double throttlePer = (!this->isThrottleReverse)
                                         ? (static_cast<double>(devData.dev_btn_value) - this->throttleValueRange.lMin)/(this->throttleValueRange.lMax - this->throttleValueRange.lMin)
                                        : (this->throttleValueRange.lMax - static_cast<double>(devData.dev_btn_value))/(this->throttleValueRange.lMax - this->throttleValueRange.lMin);

                double throttleAxisA = 0.0;// 油门加速度

                // 油门产生的加速度与踩下的深度呈线性
                //if(currentV < maxSpeed_m_s * linerThrottleASpeedPer){
                    // 油门加速度
                //    throttleAxisA = throttlePer * this->maxThrottleAxisA;
                //}else{
                    // 油门产生的加速度将缓慢下降
                    // 油门加速度
                    //throttleAxisA = throttlePer * (this->maxThrottleAxisA - (currentV / maxSpeed_m_s) * this->maxThrottleAxisA - airA - groundA);
                //}

                // 线性
                throttleAxisA = throttlePer * this->maxThrottleAxisA;

                // 添加到总的加速度
                totalA += throttleAxisA;

            }
            // 获取刹车数据
            if(devData.deviceName == this->brakeAxisDeviceName && devData.dev_btn_name == this->brakeAxis){
                // 刹车踩下的程度(0-1)
                double brakePer = (!this->isBrakeReverse)
                                         ? (static_cast<double>(devData.dev_btn_value) - this->brakeValueRange.lMin)/(this->brakeValueRange.lMax - this->brakeValueRange.lMin)
                                         : (this->brakeValueRange.lMax - static_cast<double>(devData.dev_btn_value))/(this->brakeValueRange.lMax - this->brakeValueRange.lMin);
                // 刹车加速度
                double brakeAxisA = brakePer * this->maxBrakeA;

                // 添加到总的加速度
                totalA += brakeAxisA;
            }
        }

        // 添加到总的加速度
        totalA += groundA + airA;

        // 根据总的加速度 计算出当前速度
        currentV += totalA * each_mstime/1000;

        // 车速达到上限
        if(currentV >= this->maxSpeed_m_s){
            currentV = this->maxSpeed_m_s;
        }
        // 车速最低值为0
        if(currentV <= 0){
            currentV = 0;
        }

        //qDebug() << "current V: " << currentV << " m/s, " << (currentV * 3600 / 1000 ) << "km/h";

        // 根据车速模拟力反馈效果
        updateForceFeedback(currentV, totalA);

        // 处理事件队列
        QCoreApplication::processEvents();

        // 本轮执行的结束时间 = 开始时间 + 固定的每轮执行时间
        currentExcuteTime += std::chrono::milliseconds(each_mstime);;

        // 休眠至设置的 本次执行的结束时间
        std::this_thread::sleep_until(currentExcuteTime);
    }

    // 恢复默认系统计时器精度
    Global::restoreSystemTimePeriod();

    cleanup();
    emit workFinished();
}

// 取消运行
void ForceFeedbackWorker::cancelWorkSlot(){
    this->isWorkerRunning = false;
}

// 力反馈模拟的设置改变
void ForceFeedbackWorker::settingsChangeSlot(){
    qDebug() << "ForceFeedbackWorker: 力反馈设置更新";
    init();
}
