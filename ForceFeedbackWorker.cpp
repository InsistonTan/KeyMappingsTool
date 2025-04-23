#include "ForceFeedbackWorker.h"
#include "global.h"
#include "mainwindow.h"
#include<QThread>
#include<QCoreApplication>
#include<QMessageBox>

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

// 初始化 DirectInput
bool ForceFeedbackWorker::initDirectInput2() {
    if(g_pDirectInput2 != nullptr){
        return true;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDirectInput2, NULL))) {
        qDebug() << "DirectInput 初始化失败！";
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 初始化失败！");
        return false;
    }

    pushToQueue(parseSuccessLog("<b>力反馈模拟线程</b> - DirectInput 初始化成功！"));

    return true;
}

// 打开选择的设备
bool ForceFeedbackWorker::openDiDevice2(int deviceIndex, HWND hWnd) {
    if (deviceIndex < 0 || deviceIndex >= diDeviceList.size()) {
        return false;
    }

    // 相同设备, 无需重复打开
    if (g_pDevice2 && lastDeviceIndex2 == deviceIndex) {
        return true;
    }

    // 新设备
    lastDeviceIndex2 = deviceIndex;
    if(g_pDevice2){
        g_pDevice2->Unacquire();
        g_pDevice2->Release();
        g_pDevice2 = nullptr;
    }

    // 创建设备实例
    if (FAILED(g_pDirectInput2->CreateDevice(diDeviceList[deviceIndex].guidInstance, &g_pDevice2, NULL))) {
        qDebug() << "设备创建失败！";
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 初始化设备: 设备创建失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设备创建失败！");
        return false;
    }

    if (FAILED(g_pDevice2->SetDataFormat(&c_dfDIJoystick2))) {
        qDebug() << "设置数据格式失败！";
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 初始化设备: 设置数据格式失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置数据格式失败！");
        return false;
    }

    if (FAILED(g_pDevice2->SetCooperativeLevel(hWnd, DISCL_EXCLUSIVE | DISCL_BACKGROUND ))) {
        qDebug() << "设置独占模式失败！";
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 初始化设备: 设置独占模式失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置独占模式失败！");
        return false;
    }

    // 获取控制器能力
    DIDEVCAPS capabilities;
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(g_pDevice2->GetCapabilities(&capabilities))) {
        qDebug() << "获取设备能力失败！";
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 初始化设备: 获取设备能力失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 获取设备能力失败！");
        return false;
    }

    pushToQueue(parseSuccessLog("<b>力反馈模拟线程</b> - 连接设备成功！"));

    return true;
}

// 创建力回馈效果
bool ForceFeedbackWorker::createDynamicEffects(QString steerWheelAxis){
    bool res1 = false, res2 = false;

    DWORD axes[1];
    if(steerWheelAxis == "X轴"){
        axes[0] = DIJOFS_X;
    }else if(steerWheelAxis == "Y轴"){
        axes[0] = {DIJOFS_Y};
    }else if(steerWheelAxis == "Z轴"){
        axes[0] = {DIJOFS_Z};
    }else if(steerWheelAxis == "X轴旋转"){
        axes[0] = {DIJOFS_RX};
    }else if(steerWheelAxis == "Y轴旋转"){
        axes[0] = {DIJOFS_RY};
    }else if(steerWheelAxis == "Z轴旋转"){
        axes[0] = {DIJOFS_RZ};
    }else if(steerWheelAxis == "滑动轴1"){
        axes[0] = {DIJOFS_SLIDER(0)};
    }else if(steerWheelAxis == "滑动轴2"){
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
        HRESULT hr1 = g_pDevice2->CreateEffect(GUID_Spring, &diSpring, &g_pSpringForce, NULL);

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
        HRESULT hr2 = g_pDevice2->CreateEffect(GUID_Damper, &diDamper, &g_pDamper, NULL);

        res2 = SUCCEEDED(hr2);
    }else{
        res2 = true;
    }

    return res1 && res2;
}

// 根据车速更新力回馈
void ForceFeedbackWorker::updateForceFeedback(double speed_m_s, double maxSpeed, double totalA){
    double speedKmh = speed_m_s * 3.6; // 车速, 单位 km/h
    //double speedPer = speed_m_s / maxSpeed;// 当前车速百分比

    // 当前车辆处于非静止状态
    if(speedKmh > 3){
        // 对弹簧效果根据车速进行分段式处理
        // 回正力缓慢增加
        if(speedKmh < 30){
            springEffectValue += (LONG)(totalA * DI_FFNOMINALMAX * 0.0001);
        }else{
            // 回正力较快速增加
            springEffectValue += (LONG)(totalA * DI_FFNOMINALMAX * 0.0003);
        }

        // 阻尼效果
        damperEffectValue -= (LONG)(totalA * DI_FFNOMINALMAX * 0.001);
    }else{
        springEffectValue = (LONG)(DI_FFNOMINALMAX * STATIC_SPRING_PER);
        damperEffectValue = (LONG)(DI_FFNOMINALMAX * STATIC_DAMPER_PER);
    }

    // 弹簧效果强度系数  // 与静止状态的强度系数相比, 取较大值; 再与最大强度系数比, 取较小值
    diSpringCondition.lPositiveCoefficient = std::min((LONG)(DI_FFNOMINALMAX), std::max((LONG)(springEffectValue * this->maxForceFeedbackGain), (LONG)(DI_FFNOMINALMAX * STATIC_SPRING_PER)));;
    diSpringCondition.lNegativeCoefficient = diSpringCondition.lPositiveCoefficient;
    // 阻尼效果强度系数
    diDamperCondition.lPositiveCoefficient = std::min((LONG)(DI_FFNOMINALMAX), std::max(damperEffectValue, (LONG)0));
    diDamperCondition.lNegativeCoefficient = diDamperCondition.lPositiveCoefficient;

    // 更新回正力
    if (g_pSpringForce) {
        //diSpring.lpvTypeSpecificParams = &diSpringCondition;
        g_pSpringForce->SetParameters(&diSpring, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }

    // 更新阻尼
    if (g_pDamper) {
        //diDamper.lpvTypeSpecificParams = &diDamperCondition;
        g_pDamper->SetParameters(&diDamper, DIEP_TYPESPECIFICPARAMS | DIEP_START);
    }
}

// 关闭资源
void ForceFeedbackWorker::cleanup() {
    // 释放资源
    if (g_pDevice2) {
        g_pDevice2->Unacquire();
        g_pDevice2->Release();
        g_pDevice2 = nullptr;
    }

    if (g_pDirectInput) {
        g_pDirectInput->Release();
        g_pDirectInput = nullptr;
    }

    if (g_pSpringForce) g_pSpringForce->Release();
    if (g_pDamper) g_pDamper->Release();
}

// 获取设备状态信息
QList<MappingRelation*> ForceFeedbackWorker::getInputState2() {
    QList<MappingRelation*> list;

    // 方向盘按键状态
    DIJOYSTATE2 js;
    g_pDevice2->Acquire();
    HRESULT hr = g_pDevice2->Poll();

    // 检查连接状态
    if (FAILED(hr)) {
        hr = g_pDevice2->Acquire();
    }

    // 检查是否成功获取
    if (FAILED(hr)) {
        qDebug() << "设备poll()失败，错误代码：" << HRESULT_CODE(hr);
        pushToQueue(parseErrorLog("设备poll()失败，错误代码：" + QString(std::to_string(HRESULT_CODE(hr)).data())));
        return list;
    }

    // 获取按键状态
    if (SUCCEEDED(g_pDevice2->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
        if(js.lX == js.lY && js.lY == js.lRx && js.lZ == js.lRx && js.lRx == js.lRy && js.lRy == js.lRz){
            return list;
        }else{
            // LONG lX;                  // X轴位置
            //if()
            //qDebug() << "lx: " << js.lX;
            list.append(new MappingRelation("X轴", WHEEL_AXIS, js.lX, 0, ""));

            // LONG lY;                  // Y轴位置
            //qDebug() << "lY: " << js.lY;
            list.append(new MappingRelation("Y轴", WHEEL_AXIS, js.lY, 0, ""));

            // LONG lZ;                  // Z轴位置
            //qDebug() << "lZ: " << js.lZ;
            list.append(new MappingRelation("Z轴", WHEEL_AXIS, js.lZ, 0, ""));

            // LONG lRx;                 // X轴旋转
            //qDebug() << "lRx: " << js.lRx;
            list.append(new MappingRelation("X轴旋转", WHEEL_AXIS, js.lRx, 0, ""));

            // LONG lRy;                 // Y轴旋转
            //qDebug() << "lRy: " << js.lRy;
            list.append(new MappingRelation("Y轴旋转", WHEEL_AXIS, js.lRy, 0, ""));

            // LONG lRz;                 // Z轴旋转
            //qDebug() << "lRz: " << js.lRz;
            list.append(new MappingRelation("Z轴旋转", WHEEL_AXIS, js.lRz, 0, ""));

            // LONG rglSlider[2];        // 滑动轴（通常是推杆或油门控制）
            //qDebug() << "rglSlider[0]: " << js.rglSlider[0];
            list.append(new MappingRelation("滑动轴1", WHEEL_AXIS, js.rglSlider[0], 0, ""));

            //qDebug() << "rglSlider[1]: " << js.rglSlider[1];
            list.append(new MappingRelation("滑动轴2", WHEEL_AXIS, js.rglSlider[1], 0, ""));
            //qDebug("");
        }
    }else{
        qDebug() << "获取设备状态信息失败!";
        qDebug() << "GetDeviceState failed with error:" << HRESULT_CODE(hr);

        pushToQueue(parseErrorLog("获取设备数据失败，错误代码：" + QString(std::to_string(HRESULT_CODE(hr)).data())));
    }

    return list;
}


ForceFeedbackWorker::ForceFeedbackWorker(ForceFeedbackSettingsWindow* forceFeedbackSettings)
{
    this->forceFeedbackSettings = forceFeedbackSettings;

    init();
}

void ForceFeedbackWorker::init(){
    this->throttleAxis = this->forceFeedbackSettings->throttleAxis;
    this->brakeAxis = this->forceFeedbackSettings->brakeAxis;
    this->steeringWheelAxis = this->forceFeedbackSettings->steeringWheelAxis;

    this->isBrakeReverse = this->forceFeedbackSettings->isBrakeReverse;
    this->isThrottleReverse = this->forceFeedbackSettings->isThrottleReverse;

    this->acceleration_100km_time_s = this->forceFeedbackSettings->acceleration_100km_time_s;
    this->stop_100km_dis_m = this->forceFeedbackSettings->stop_100km_dis_m;
    this->maxSpeed_m_s = this->forceFeedbackSettings->maxSpeed_km_h * 1000.0 / 3600.0;
    this->maxForceFeedbackGain = this->forceFeedbackSettings->maxForceFeedbackGain;

    // 油门踏板的数值范围
    this->throttleValueRange = axisValueRangeMap.find(this->throttleAxis.toStdString())->second;
    // 刹车踏板的数值范围
    this->brakeValueRange = axisValueRangeMap.find(this->brakeAxis.toStdString())->second;

    this->maxThrottleAxisA = 100.0 * 1000 / 3600 / this->acceleration_100km_time_s;// 最大油门加速度
    this->maxBrakeA = -771.6 / (2 * this->stop_100km_dis_m);// 最大刹车加速度

    if(isWorkerRunning){
        if(!initDirectInput2() || !openDiDevice2(MainWindow::getCurrentSelectedDeviceIndex(), reinterpret_cast<HWND>(g_mainWindow->winId()))){
            pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 开启力反馈模拟失败: 初始化设备/独占模式打开设备失败!"));
            isWorkerRunning = false;
        }

        if(!createDynamicEffects(this->steeringWheelAxis)){
            pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 开启力反馈模拟失败: 创建力反馈效果失败!"));
            isWorkerRunning = false;
        }else{
            // 开启力反馈效果
            g_pSpringForce->Start(1, 0);
            g_pDamper->Start(1, 0);
        }
    }

}


void ForceFeedbackWorker::doWork(){
    double currentV = 0.0;// 当前车速
    double groundA = GROUND_FRICTION_COEFFICIENT * G;// 地面滚动摩檫力加速度
    double cycleTimeOfEachRound = (WORKER_SLEEP_TIME_MS + 10.0) / 1000.0;// 每轮循环所花费的时间(单位s)

    isWorkerRunning = true;

    if(!initDirectInput2() || !openDiDevice2(MainWindow::getCurrentSelectedDeviceIndex(), reinterpret_cast<HWND>(g_mainWindow->winId()))){
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 开启力反馈模拟失败: 初始化设备/独占模式打开设备失败!"));
        isWorkerRunning = false;
    }

    if(!createDynamicEffects(this->steeringWheelAxis)){
        pushToQueue(parseErrorLog("<b>力反馈模拟线程</b> - 开启力反馈模拟失败: 创建力反馈效果失败!"));
        isWorkerRunning = false;
    }else{
        // 开启力反馈效果
        g_pSpringForce->Start(1, 0);
        g_pDamper->Start(1, 0);

        pushToQueue(parseSuccessLog("<b>力反馈模拟线程</b> - 开启力反馈模拟成功!"));
    }


    while(isWorkerRunning){
        // 轮询设备状态
        auto res = getInputState2();

        double totalA = 0.0;// 总的加速度

        for(auto devData : res){
            // 获取油门数据
            if(devData->dev_btn_name == this->throttleAxis.toStdString()){
                // 油门踩下的程度(0-1)
                double throttlePer = (!this->isThrottleReverse)
                                         ? (static_cast<double>(devData->dev_btn_value) - this->throttleValueRange.lMin)/(this->throttleValueRange.lMax - this->throttleValueRange.lMin)
                                        : (this->throttleValueRange.lMax - static_cast<double>(devData->dev_btn_value))/(this->throttleValueRange.lMax - this->throttleValueRange.lMin);

                // 油门加速度
                double throttleAxisA = throttlePer * this->maxThrottleAxisA;

                // 添加到总的加速度
                totalA += throttleAxisA;
            }
            // 获取刹车数据
            if(devData->dev_btn_name == this->brakeAxis.toStdString()){
                // 刹车踩下的程度(0-1)
                double brakePer = (!this->isBrakeReverse)
                                         ? (static_cast<double>(devData->dev_btn_value) - this->brakeValueRange.lMin)/(this->brakeValueRange.lMax - this->brakeValueRange.lMin)
                                         : (this->brakeValueRange.lMax - static_cast<double>(devData->dev_btn_value))/(this->brakeValueRange.lMax - this->brakeValueRange.lMin);
                // 刹车加速度
                double brakeAxisA = brakePer * this->maxBrakeA;

                // 添加到总的加速度
                totalA += brakeAxisA;
            }
        }

        // 计算当前车速下空气阻力f
        double airF = 0.5 * RHO * CAR_Cd * CAR_A * currentV * currentV;
        // 得到空气阻力的加速度a
        double airA = - airF / CAR_m;

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

        qDebug() << "current V: " << currentV << " m/s, " << (currentV * 3600 / 1000 ) << "km/h";

        // 根据车速模拟力反馈效果
        updateForceFeedback(currentV, this->maxSpeed_m_s, totalA);


        // 释放res内存
        qDeleteAll(res);  // 删除所有指针指向的对象
        res.clear();      // 清空列表

        // sleep
        QThread::msleep(WORKER_SLEEP_TIME_MS);
        // 处理事件队列
        QCoreApplication::processEvents();
    }

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
