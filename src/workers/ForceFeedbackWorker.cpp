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


// 创建力回馈效果
bool ForceFeedbackWorker::createDynamicEffects(QString steerWheelAxis){
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

    if(g_pSpringForce == NULL){
        // 设置弹簧效果的系数
        diSpringCondition = {0, (LONG)(DI_FFNOMINALMAX * FORCE_GAIN), (LONG)(DI_FFNOMINALMAX * FORCE_GAIN), 0, 0, 0};

        diSpring = {sizeof(DIEFFECT)};
        diSpring.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
        diSpring.dwDuration = INFINITE;
        diSpring.dwSamplePeriod = 0;
        diSpring.dwGain = DI_FFNOMINALMAX * FORCE_GAIN;
        diSpring.dwTriggerButton = DIEB_NOTRIGGER;
        diSpring.dwTriggerRepeatInterval = INFINITE;
        diSpring.cAxes = 1;

        diSpring.rgdwAxes = axes;
        diSpring.rglDirection = new LONG[1]{ 0 };
        diSpring.lpEnvelope = NULL;
        diSpring.cbTypeSpecificParams = sizeof(DICONDITION);
        diSpring.lpvTypeSpecificParams = &diSpringCondition;
        diSpring.dwStartDelay = 0;

        // 创建效果
        HRESULT hr1 = pSteeringWheelAxisDeviceInstance->CreateEffect(GUID_Spring, &diSpring, &g_pSpringForce, NULL);

        res1 = SUCCEEDED(hr1);
    }else{
        res1 = true;
    }

    if(g_pDamper == NULL){
        // Condition
        diDamperCondition = {0, (LONG)(DI_FFNOMINALMAX * FORCE_GAIN), (LONG)(DI_FFNOMINALMAX * FORCE_GAIN), 0, 0, 0};

        // 阻尼（Damper）
        diDamper = {sizeof(DIEFFECT)};
        diDamper.dwFlags = DIEFF_CARTESIAN | DIEFF_OBJECTOFFSETS;
        diDamper.dwDuration = INFINITE;
        diDamper.dwSamplePeriod = 0;
        diDamper.dwGain = DI_FFNOMINALMAX  * FORCE_GAIN;
        diDamper.dwTriggerButton = DIEB_NOTRIGGER;
        diDamper.dwTriggerRepeatInterval = INFINITE;
        diDamper.cAxes = 1;

        diDamper.rgdwAxes = axes;
        diDamper.rglDirection = new LONG[1]{ 0 };
        diDamper.lpEnvelope = NULL;
        diDamper.cbTypeSpecificParams = sizeof(DICONDITION);
        diDamper.lpvTypeSpecificParams = &diDamperCondition;
        diDamper.dwStartDelay = 0;

        // 创建效果
        HRESULT hr2 = pSteeringWheelAxisDeviceInstance->CreateEffect(GUID_Damper, &diDamper, &g_pDamper, NULL);

        res2 = SUCCEEDED(hr2);
    }else{
        res2 = true;
    }

    return res1 && res2;
}

// 根据车速更新力回馈
void ForceFeedbackWorker::updateForceFeedback(double speed_m_s, double totalA){
    //double speedKmh = speed_m_s * 3.6; // 车速, 单位 km/h

    // 当前车辆处于非静止状态
    // if(speedKmh > 3){
    //     // 对弹簧效果根据车速进行分段式处理
    //     // 回正力缓慢增加
    //     if(speedKmh < 30){
    //         springEffectValue += (LONG)(totalA * DI_FFNOMINALMAX * 0.0001);
    //     }else{
    //         // 回正力较快速增加
    //         springEffectValue += (LONG)(totalA * DI_FFNOMINALMAX * 0.0003);
    //     }

    //     // 阻尼效果
    //     damperEffectValue -= (LONG)(totalA * DI_FFNOMINALMAX * 0.001);
    // }else{
    //     springEffectValue = (LONG)(DI_FFNOMINALMAX * STATIC_SPRING_PER);
    //     damperEffectValue = (LONG)(DI_FFNOMINALMAX * STATIC_DAMPER_PER);
    // }

     double speedPer = speed_m_s / maxSpeed_m_s;// 当前车速百分比

    // 根据查找表, 找到强度系数百分比, 再乘以最大值, 得到实际强度系数
    springEffectValue = (int)(springGainLUT.value(((int)(speedPer*1000))) * DI_FFNOMINALMAX);
    damperEffectValue = (int)(dampingGainLUT.value(((int)(speedPer*1000))) * DI_FFNOMINALMAX);

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


    // 弹簧效果强度系数  // 与静止状态的强度系数相比, 取较大值; 再与最大强度系数比, 取较小值
    // diSpringCondition.lPositiveCoefficient =
    //     std::min((LONG)(DI_FFNOMINALMAX),
    //              std::max((LONG)(springEffectValue * userConfig.SYSTEM_forceFeedbackSettings_maxForceFeedbackGain),
    //                       (LONG)(DI_FFNOMINALMAX * STATIC_SPRING_PER)));;
    // diSpringCondition.lNegativeCoefficient = diSpringCondition.lPositiveCoefficient;
    // // 阻尼效果强度系数
    // diDamperCondition.lPositiveCoefficient =
    //     std::min((LONG)(DI_FFNOMINALMAX), std::max(damperEffectValue, (LONG)0));
    // diDamperCondition.lNegativeCoefficient = diDamperCondition.lPositiveCoefficient;

    // 弹簧效果强度系数
    // 方向盘向右转动的回正力系数
    diSpringCondition.lPositiveCoefficient = springEffectValue;
    // 方向盘向左转动的回正力系数
    diSpringCondition.lNegativeCoefficient = diSpringCondition.lPositiveCoefficient;
    // 阻尼效果强度系数
    diDamperCondition.lPositiveCoefficient = damperEffectValue;
    diDamperCondition.lNegativeCoefficient = diDamperCondition.lPositiveCoefficient;

    // 更新回正力
    if (g_pSpringForce) {
        g_pSpringForce->SetParameters(&diSpring, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }

    // 更新阻尼
    if (g_pDamper) {
        g_pDamper->SetParameters(&diDamper, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }
}

// 关闭资源
void ForceFeedbackWorker::cleanup() {
    if (g_pSpringForce) g_pSpringForce->Release();
    if (g_pDamper) g_pDamper->Release();
}

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
    dampingGainLUT.clear();
    for(int x = 0; x <= 1000; x++){
        // 根据x轴值, 获取y轴值
        double y = CurveEditor::getYAxisLogicalValue(userConfig.SYSTEM_forceFeedbackSettings_dampingCurve, x/10.0);
        // y轴值是强度百分比*100, 所以需要 y/100 才是百分比
        dampingGainLUT.insert(x, y/100.0);
    }

    // 最大油门加速度
    maxThrottleAxisA = getMaxThrottleAxisA(acceleration_100km_time_s);
    // 最大刹车加速度
    maxBrakeA = getmaxBrakeA(stop_100km_dis_m);

    // 各个轴的数值范围
    auto axisValueRangeMap = DirectInputService::getAxisValueRangeMap();
    // 油门踏板的数值范围
    this->throttleValueRange = axisValueRangeMap.value(Global::getBtnOrAxisFullName(throttleAxisDeviceName, throttleAxis));
    // 刹车踏板的数值范围
    this->brakeValueRange = axisValueRangeMap.value(Global::getBtnOrAxisFullName(brakeAxisDeviceName, brakeAxis));

    if(isWorkerRunning){
        // 检查设备
        if(!checkDevicesConnected()){
            isWorkerRunning = false;
        }

        // 如果还在运行, 根据新的配置信息重新创建力反馈效果
        if(isWorkerRunning){
            if(!createDynamicEffects(this->steeringWheelAxis)){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_createEffectsErrorMsg);
                isWorkerRunning = false;
            }else{
                // 开启力反馈效果
                g_pSpringForce->Start(1, 0);
                g_pDamper->Start(1, 0);
            }
        }
    }
}

bool ForceFeedbackWorker::checkDevicesConnected()
{
    // 转向设备为空
    if(steeringWheelAxisDeviceName.isEmpty()){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_WheelDeviceEmptyErrorMsg);
        return false;
    }

    // 扫描设备
    DirectInputService::scanDevice();

    // 打开转向设备失败
    if(DirectInputService::openDiDevice({steeringWheelAxisDeviceName}) == false){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_WheelDeviceOpenErrorMsg);
        return false;
    }

    // 获取已初始化的转向轴设备实例
    pSteeringWheelAxisDeviceInstance = DirectInputService::getInitedDevice(steeringWheelAxisDeviceName);
    if(pSteeringWheelAxisDeviceInstance == nullptr){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_WheelDeviceOpenErrorMsg);
        return false;
    }

    // 加锁
    QMutexLocker locker(&mutex_initedDeviceList);
    // 重置
    initedDeviceList.clear();

    // 添加设备到列表
    initedDeviceList.append(pSteeringWheelAxisDeviceInstance);

    // 检查油门轴和刹车轴设备
    // 设备为空
    if(throttleAxisDeviceName.isEmpty() || brakeAxisDeviceName.isEmpty()){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceEmptyErrorMsg);
        return false;
    }
    // 设备连接失败
    if(DirectInputService::openDiDevice({throttleAxisDeviceName, brakeAxisDeviceName}) == false){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceOpenErrorMsg);
        return false;
    }else{
        auto pThrottleAxisDevice = DirectInputService::getInitedDevice(throttleAxisDeviceName);
        auto pBrakeAxisDevice = DirectInputService::getInitedDevice(brakeAxisDeviceName);
        if(pThrottleAxisDevice == nullptr || pBrakeAxisDevice == nullptr){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_throttleOrBrakeDeviceGetInstanceErrorMsg);
            return false;
        }else{
            // 添加到已初始化设备列表
            initedDeviceList.append(pThrottleAxisDevice);
            initedDeviceList.append(pBrakeAxisDevice);
        }
    }

    return true;
}


void ForceFeedbackWorker::doWork(){
    double currentV = 0.0;// 当前车速
    double groundA = GROUND_FRICTION_COEFFICIENT * G;// 地面滚动摩檫力加速度
    double cycleTimeOfEachRound = (WORKER_SLEEP_TIME_MS + 10.0) / 1000.0;// 每轮循环所花费的时间(单位s)
    double linerThrottleASpeedPer = 0.5;// 油门产生的加速度与踩下的深度呈线性增加关系时的最大车速百分比

    isWorkerRunning = true;

    // 检查方向盘设备是否连接
    if(checkDevicesConnected()){
        LogService::parseErrorLog(StringConstants::ffbSimulateThread_initDevicesErrorMsg);
        isWorkerRunning = false;
    }

    if(isWorkerRunning){
        if(!createDynamicEffects(this->steeringWheelAxis)){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::ffbSimulateThread_createEffectsErrorMsg);
            isWorkerRunning = false;
        }else{
            // 开启力反馈效果
            g_pSpringForce->Start(1, 0);
            g_pDamper->Start(1, 0);

            LogService::parseSuccessLog(StringConstants::ffbSimulateThread_successMsg);
        }
    }

    // 设置系统计时器精度
    Global::setSystemTimePeriod_1ms();

    // 高精度时钟, 用于固定while循环的执行频率
    using clock = std::chrono::high_resolution_clock;
    // 本轮执行的开始时间
    auto currentExcuteTime = clock::now();

    // 执行频率
    auto excuteFrequency = 100;
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

        // 恒定力回馈模式
        // if(this->isConstantForceMode){
        //     // 弹簧效果强度系数
        //     diSpringCondition.lPositiveCoefficient = (LONG)(DI_FFNOMINALMAX * this->constantCorrectiveForceGain);
        //     diSpringCondition.lNegativeCoefficient = diSpringCondition.lPositiveCoefficient;
        //     // 阻尼效果强度系数
        //     diDamperCondition.lPositiveCoefficient = (LONG)(DI_FFNOMINALMAX * this->constantDampingGain);
        //     diDamperCondition.lNegativeCoefficient = diDamperCondition.lPositiveCoefficient;

        //     // 释放锁
        //     //mutexConstantForce.unlock();

        //     // 更新回正力
        //     if (g_pSpringForce) {
        //         g_pSpringForce->SetParameters(&diSpring, DIEP_TYPESPECIFICPARAMS | DIEP_START);
        //     }

        //     // 更新阻尼
        //     if (g_pDamper) {
        //         g_pDamper->SetParameters(&diDamper, DIEP_TYPESPECIFICPARAMS | DIEP_START);
        //     }

        //     // sleep
        //     QThread::msleep(200);

        //     // 处理事件队列
        //     QCoreApplication::processEvents();

        //     continue;
        // }

        // 获取设备状态数据, 只获取轴数据, 不获取按键数据
        DirectInputService::getInputState(res, getInitedDeviceListSnapshot(), false, true);

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
                if(currentV < maxSpeed_m_s * linerThrottleASpeedPer){
                    // 油门加速度
                    throttleAxisA = throttlePer * this->maxThrottleAxisA;
                }else{
                    // 油门产生的加速度将缓慢下降
                    // 油门加速度
                    throttleAxisA = throttlePer * (this->maxThrottleAxisA - (currentV / maxSpeed_m_s) * this->maxThrottleAxisA - airA - groundA);
                }

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
        currentV += totalA * cycleTimeOfEachRound;

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
