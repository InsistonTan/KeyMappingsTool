#include<global.h>
#include<QDebug>
#include<QMessageBox>
#include<QMutexLocker>
#include<QMutex>
#include<QDateTime>
#include "AssistFuncWindow.h"
#include<QDir>
#include<QCoreApplication>

QWidget* g_mainWindow = nullptr;

bool isRuning = false;
LPDIRECTINPUT8 g_pDirectInput = nullptr;
QList<LPDIRECTINPUTDEVICE8> initedDeviceList; // 已初始化的设备列表
QList<DiDeviceInfo> diDeviceList;
std::map<std::string, DIPROPRANGE> axisValueRangeMap;

// 清空已选择的设备列表
void clearInitedDeviceList(){
    for(auto device : initedDeviceList){
        // 释放资源
        if (device) {
            device->Unacquire();
            device->Release();
            device = nullptr;
        }
    }

    initedDeviceList.clear();
}

// 是否暂停全局映射
bool isPause = false;
// 点击了暂停按键
void clickPauseBtn(){
    isPause ? isPause = false : isPause = true;
}
bool getIsPause(){
    return isPause;
}

// 内部死区比例
// 盘面轴死区
double innerDeadAreaPanti = DEFAULT_INNER_DEADAREA_VALUE;
// 踏板轴死区
double innerDeadAreaTaban = DEFAULT_INNER_DEADAREA_VALUE;
void setInnerDeadAreaPanti(double val){
    innerDeadAreaPanti = val;
}
void setInnerDeadAreaTaban(double val){
    innerDeadAreaTaban = val;
}
double getInnerDeadAreaPanti(){
    return innerDeadAreaPanti;
}
double getInnerDeadAreaTaban(){
    return innerDeadAreaTaban;
}

// 全局变量, log队列
QQueue<QString> logQueue;
QMutex queueMutex;
void pushToQueue(QString data){
    QMutexLocker locker(&queueMutex);
    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 格式化输出当前日期和时间
    QString logText = "<p>[" + currentDateTime.toString("yyyy-MM-dd HH:mm:ss") + "] " + data + "</p>";

    logQueue.enqueue(logText);
}
QString popQueue(){
    QMutexLocker locker(&queueMutex);
    if(logQueue.size() > 0){
        return logQueue.dequeue();
    }
    return "";
}
int getQueueSize(){
    QMutexLocker locker(&queueMutex);
    return logQueue.size();
}
QString parseSuccessLog(QString srcText){
    return "<span style='color:rgb(0, 151, 144);'>" + srcText + "</span>";
}
QString parseWarningLog(QString srcText){
    return "<span style='color:rgb(206, 122, 58);'>" + srcText + "</span>";
}
QString parseErrorLog(QString srcText){
    return "<span style='color:red;'>" + srcText + "</span>";
}

// 开启按键日志
bool enableBtnLog = false;
// 开启轴日志
bool enableAxisLog = false;
// 开启摇杆日志
bool enablePovLog = false;
void setEnableBtnLog(bool val){
    enableBtnLog = val;
}
bool getEnableBtnLog(){
    return enableBtnLog;
}
void setEnableAxisLog(bool val){
    enableAxisLog = val;
}
bool getEnableAxisLog(){
    return enableAxisLog;
}
void setEnablePovLog(bool val){
    enablePovLog = val;
}
bool getEnablePovLog(){
    return enablePovLog;
}

void setIsRuning(bool val){
    isRuning = val;
}

bool getIsRunning(){
    return isRuning;
}

std::string guidToString(const GUID& guid)
{
    char buffer[39]; // GUID字符串固定为38个字符 + 终止符

    // snprintf(buffer, sizeof(buffer),
    //          "{%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX}",
    //          guid.Data1, guid.Data2, guid.Data3,
    //          guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
    //          guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    snprintf(buffer, sizeof(buffer), "(%08lX)", guid.Data1);

    return std::string(buffer);
}

// DirectInput 回调函数，用于列出所有连接的游戏控制器设备
BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
    DiDeviceInfo deviceInfo;
    deviceInfo.name = QString::fromWCharArray(pdidInstance->tszProductName).toStdString();
    deviceInfo.name += guidToString(pdidInstance->guidProduct);
    deviceInfo.guidInstance = pdidInstance->guidInstance;

    diDeviceList.append(deviceInfo);

    return DIENUM_CONTINUE;
}

// 初始化 DirectInput 并列出所有设备
bool initDirectInput() {
    if(g_pDirectInput != nullptr){
        return true;
    }

    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (FAILED(DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&g_pDirectInput, NULL))) {
        qDebug() << "DirectInput 初始化失败！";

        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 初始化失败！");
        }, Qt::QueuedConnection);

        return false;
    }

    pushToQueue(parseSuccessLog("DirectInput 初始化成功！"));

    // 列出所有连接的游戏控制器设备
    diDeviceList.clear();
    if (FAILED(g_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {

        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 扫描设备列表失败！");
        }, Qt::QueuedConnection);

        return false;
    }
    pushToQueue(parseSuccessLog("扫描设备成功！"));

    return true;
}

void scanDevice(){
    if(g_pDirectInput == nullptr){
        return;
    }

    if (FAILED(g_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {
        pushToQueue(parseErrorLog("扫描设备列表失败！"));

        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            QMessageBox::critical(nullptr, "错误", "扫描设备列表失败！");
        }, Qt::QueuedConnection);

        return;
    }
}

int lastDeviceIndex = -1;

void getDipropRange(LPDIRECTINPUTDEVICE8 g_pDevice, long axisCode, std::string axisName){
    // 获取方向盘的各个轴的数值范围
    DIPROPRANGE dipr1;
    dipr1.diph.dwSize = sizeof(DIPROPRANGE);
    dipr1.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipr1.diph.dwObj = axisCode;       // X 轴
    dipr1.diph.dwHow = DIPH_BYOFFSET;
    HRESULT hr1 = g_pDevice->GetProperty(DIPROP_RANGE, &dipr1.diph);
    if (SUCCEEDED(hr1)) {
        axisValueRangeMap.insert_or_assign(axisName, dipr1);
    }else{
        qDebug("获取设备[%s]的值范围失败!!", axisName.data());
        pushToQueue(parseWarningLog("获取[" + QString(axisName.data()) + "]的数值范围失败, 如果是非方向盘设备请忽略此警告"));
    }
}

QString getDeviceName(LPDIRECTINPUTDEVICE8 pDevice){
    // 获取设备名称
    QString deviceName = "";
    DIDEVICEINSTANCE diInstance;
    diInstance.dwSize = sizeof(DIDEVICEINSTANCE);
    HRESULT hr2 = pDevice->GetDeviceInfo(&diInstance);
    if (SUCCEEDED(hr2)){
        deviceName = QString::fromWCharArray(diInstance.tszProductName) + guidToString(diInstance.guidProduct).data();
    }

    return deviceName;
}

bool isDeviceInited(QString deviceName){
    for(auto initedDevice : initedDeviceList){
        if(deviceName == getDeviceName(initedDevice)){
            return true;
        }
    }

    return false;
}

// 打开选择的设备
bool openDiDevice(QList<QString> deviceList) {
    if (deviceList.isEmpty()) {
        return false;
    }

    // if(!initedDeviceList.isEmpty()){
    //     clearInitedDeviceList();
    // }

    // 遍历已选择的设备名称字符串
    for(auto deviceName : deviceList){
        // 如果当前设备已经初始化, 无需重复初始化
        if(isDeviceInited(deviceName)){
            continue;
        }

        // 根据设备名称找到设备guidInstance, 并初始化设备, 添加初始化后的设备到已选择设备列表
        for(auto deviceInfo : diDeviceList){
            if(deviceName.toStdString() == deviceInfo.name){
                LPDIRECTINPUTDEVICE8 g_pDevice = nullptr;
                // 创建设备实例
                if (FAILED(g_pDirectInput->CreateDevice(deviceInfo.guidInstance, &g_pDevice, NULL))) {
                    qDebug() << "设备创建失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设备创建失败！"));

                    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
                        QMessageBox::critical(nullptr, "错误", "初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设备创建失败！");
                    }, Qt::QueuedConnection);

                    clearInitedDeviceList();
                    return false;
                }

                if (FAILED(g_pDevice->SetDataFormat(&c_dfDIJoystick2))) {
                    qDebug() << "设置数据格式失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置数据格式失败！"));

                    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
                        QMessageBox::critical(nullptr, "错误", "初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置数据格式失败！");
                    }, Qt::QueuedConnection);

                    clearInitedDeviceList();
                    return false;
                }

                if (FAILED(g_pDevice->SetCooperativeLevel(GetForegroundWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                    qDebug() << "设置协作模式失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置协作模式失败！"));

                    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
                        QMessageBox::critical(nullptr, "错误", "初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置协作模式失败！");
                    }, Qt::QueuedConnection);

                    clearInitedDeviceList();
                    return false;
                }

                // 获取控制器能力
                DIDEVCAPS capabilities;
                capabilities.dwSize = sizeof(DIDEVCAPS);
                if (FAILED(g_pDevice->GetCapabilities(&capabilities))) {
                    qDebug() << "获取设备能力失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 获取设备能力失败！"));

                    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
                        QMessageBox::critical(nullptr, "错误", "初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 获取设备能力失败！");
                    }, Qt::QueuedConnection);

                    clearInitedDeviceList();
                    return false;
                }

                // 获取各个轴的值范围
                getDipropRange(g_pDevice, DIJOFS_X, deviceInfo.name + "-" + "X轴");
                getDipropRange(g_pDevice, DIJOFS_Y, deviceInfo.name + "-" + "Y轴");
                getDipropRange(g_pDevice, DIJOFS_Z, deviceInfo.name + "-" + "Z轴");
                getDipropRange(g_pDevice, DIJOFS_RX, deviceInfo.name + "-" + "X轴旋转");
                getDipropRange(g_pDevice, DIJOFS_RY, deviceInfo.name + "-" + "Y轴旋转");
                getDipropRange(g_pDevice, DIJOFS_RZ, deviceInfo.name + "-" + "Z轴旋转");
                getDipropRange(g_pDevice, DIJOFS_SLIDER(0), deviceInfo.name + "-" + "滑动轴1");
                getDipropRange(g_pDevice, DIJOFS_SLIDER(1), deviceInfo.name + "-" + "滑动轴2");

                qDebug() << "连接设备成功！axisValueRangeMap.size():" << axisValueRangeMap.size();

                pushToQueue(parseSuccessLog(QString("连接设备[").append(deviceInfo.name.data()).append("]成功！")));

                initedDeviceList.append(g_pDevice);
            }
        }
    }

    return true;
}

// 获取设备状态信息
QList<MappingRelation*> getInputState(bool enableLog, std::vector<MappingRelation> multiBtnVector) {
    QList<MappingRelation*> list;

    // if(!MainWindow::getCurrentSelectedDeviceList().isEmpty() && initedDeviceList.size() < MainWindow::getCurrentSelectedDeviceList().size()){
    //     openDiDevice(MainWindow::getCurrentSelectedDeviceList());
    // }

    // 方向盘按键状态
    DIJOYSTATE2 js;

    for(auto g_pDevice : initedDeviceList){
        g_pDevice->Acquire();
        HRESULT hr = g_pDevice->Poll();

        // 检查连接状态
        if (FAILED(hr)) {
            hr = g_pDevice->Acquire();
            // while ( hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
            //     hr = g_pDevice->Acquire();
            // }
        }

        // 检查是否成功获取
        if (FAILED(hr)) {
            qDebug() << "设备poll()失败，错误代码：" << HRESULT_CODE(hr);
            pushToQueue(parseErrorLog("设备poll()失败，错误代码：" + QString(std::to_string(HRESULT_CODE(hr)).data())));
            return list;
        }

        // 获取设备名称
        QString deviceName = getDeviceName(g_pDevice);


        // 获取按键状态
        if (SUCCEEDED(g_pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {

            QString btnLog = "";
            if(enableLog && getEnableBtnLog()){
                btnLog.append("按键数据: { ");
            }


            // 遍历按键，查看按键是否按下
            std::string btnStr = "";
            BUTTONS_VALUE_TYPE btnBitValue;
            for (size_t i = 0; i < MAX_BUTTONS; i++) {
                if (js.rgbButtons[i] & 0x80) {
                    btnStr += "按键" + std::to_string(i) + "+";
                    btnBitValue.setBit(i, true);
                }

                // 记录日志
                if(enableLog && getEnableBtnLog()){
                    int val = static_cast<int>(js.rgbButtons[i]);
                    if(val > 0){
                        btnLog.append("<span style='color:green;'><b>" + std::to_string(val) + "</b></span>").append(", ");
                    }else{
                        btnLog.append(std::to_string(val)).append(", ");
                    }

                }
            }

            if (!btnStr.empty()) {
                // 去掉最后的 "+"
                btnStr = btnStr.substr(0, btnStr.length() - 1);

                // multiBtnVector不为空, 需要进行多按键映射处理
                if (multiBtnVector.size() > 0) {
                    // 多按键映射，需要匹配按键，并拆分为多个 MappingRelation对象, 根据keyValue进行拆分
                    for (auto multiBtn : multiBtnVector) {
                        BUTTONS_VALUE_TYPE multiBtnBitValue = multiBtn.dev_btn_bit_value;
                        if (multiBtn.deviceName == deviceName && (multiBtnBitValue) && ((multiBtnBitValue & btnBitValue) == multiBtnBitValue)) {
                            // 找到对应的按键, 进行映射
                            if (AssistFuncWindow::getEnableOnlyLongestMapping()) {
                                btnBitValue &= (~multiBtnBitValue);  // 清除当前按键的值
                            }
                            MappingRelation *mapping = new MappingRelation(multiBtn.dev_btn_name, WHEEL_BUTTON, 0, 0, "", TriggerTypeEnum::Normal, deviceName);
                            mapping->setBtnBitValue(multiBtnBitValue);  // 设置按键值
                            list.append(mapping);
                        }
                    }
                }else{
                    list.append(new MappingRelation(btnStr, WHEEL_BUTTON, 0, 0, "", TriggerTypeEnum::Normal, deviceName));
                }
            }
            if(enableLog && getEnableBtnLog()){
                btnLog.append("}");
            }


            // 遍历摇杆数据
            // DWORD	rgdwPOV[4];
            QString povLog = "";
            if(enableLog && getEnablePovLog()){
                povLog.append("摇杆/十字键数据: { ");
            }
            for(int j=0; j<4; j++){
                auto val = static_cast<int>(js.rgdwPOV[j]);

                if(val > -1){
                    // 格式化
                    int formatVal = val;
                    while(true){
                        if(formatVal < 361){
                            break;
                        }

                        formatVal /= 10;
                    }
                    std::string btnStr = "摇杆" + std::to_string(j+1) + "-角度" + std::to_string(formatVal);
                    list.append(new MappingRelation(btnStr, WHEEL_BUTTON, val, 0, "", TriggerTypeEnum::Normal, deviceName));
                }

                // 记录日志
                if(enableLog && getEnableBtnLog()){
                    if(val > -1){
                        povLog.append("<span style='color:green;'><b>" + std::to_string(val) + "</b></span>").append(", ");
                    }else{
                        povLog.append(std::to_string(val)).append(", ");
                    }

                }
            }
            if(enableLog && getEnablePovLog()){
                povLog.append("}");
            }


            // 映射xbox
            // if(getDefaultMappingType() == MappingType::Xbox){
            if(true){
                if(js.lX == js.lY && js.lY == js.lRx && js.lZ == js.lRx && js.lRx == js.lRy && js.lRy == js.lRz){
                    return list;
                }else{
                    // LONG lX;                  // X轴位置
                    //if()
                    //qDebug() << "lx: " << js.lX;
                    list.append(new MappingRelation("X轴", WHEEL_AXIS, js.lX, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG lY;                  // Y轴位置
                    //qDebug() << "lY: " << js.lY;
                    list.append(new MappingRelation("Y轴", WHEEL_AXIS, js.lY, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG lZ;                  // Z轴位置
                    //qDebug() << "lZ: " << js.lZ;
                    list.append(new MappingRelation("Z轴", WHEEL_AXIS, js.lZ, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG lRx;                 // X轴旋转
                    //qDebug() << "lRx: " << js.lRx;
                    list.append(new MappingRelation("X轴旋转", WHEEL_AXIS, js.lRx, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG lRy;                 // Y轴旋转
                    //qDebug() << "lRy: " << js.lRy;
                    list.append(new MappingRelation("Y轴旋转", WHEEL_AXIS, js.lRy, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG lRz;                 // Z轴旋转
                    //qDebug() << "lRz: " << js.lRz;
                    list.append(new MappingRelation("Z轴旋转", WHEEL_AXIS, js.lRz, 0, "", TriggerTypeEnum::Normal, deviceName));

                    // LONG rglSlider[2];        // 滑动轴（通常是推杆或油门控制）
                    //qDebug() << "rglSlider[0]: " << js.rglSlider[0];
                    list.append(new MappingRelation("滑动轴1", WHEEL_AXIS, js.rglSlider[0], 0, "", TriggerTypeEnum::Normal, deviceName));

                    //qDebug() << "rglSlider[1]: " << js.rglSlider[1];
                    list.append(new MappingRelation("滑动轴2", WHEEL_AXIS, js.rglSlider[1], 0, "", TriggerTypeEnum::Normal, deviceName));
                    //qDebug("");
                }
            }

            // 记录日志
            if(enableLog){
                // 获取当前日期和时间
                //QDateTime currentDateTime = QDateTime::currentDateTime();

                // 格式化输出当前日期和时间
                QString logText = "设备状态数据:<br/>";

                if(getEnableBtnLog()){
                    logText.append("    ").append(btnLog).append("<br/>");
                }
                if(getEnablePovLog()){
                    logText.append("    ").append(povLog).append("<br/>");
                }

                if(getEnableAxisLog()){
                    logText.append("    ").append("轴数据: { ");
                    logText.append("X轴: " + std::to_string(js.lX)).append(", ");
                    logText.append("Y轴: " + std::to_string(js.lY)).append(", ");
                    logText.append("Z轴: " + std::to_string(js.lZ)).append(", ");
                    logText.append("X轴旋转: " + std::to_string(js.lRx)).append(", ");
                    logText.append("Y轴旋转: " + std::to_string(js.lRy)).append(", ");
                    logText.append("Z轴旋转: " + std::to_string(js.lRz)).append(", ");
                    logText.append("滑动轴1: " + std::to_string(js.rglSlider[0])).append(", ");
                    logText.append("滑动轴2: " + std::to_string(js.rglSlider[1])).append(", ");
                    logText.append("}");
                }

                logText.append("</p>");

                pushToQueue(logText);
            }
        }else{
            qDebug() << "获取设备状态信息失败!";
            qDebug() << "GetDeviceState failed with error:" << HRESULT_CODE(hr);

            pushToQueue(parseErrorLog("获取设备数据失败，错误代码：" + QString(std::to_string(HRESULT_CODE(hr)).data())));
        }
    }

    return list;
}
// 关闭 DirectInput
void cleanupDirectInput() {
    if (g_pDirectInput) {
        g_pDirectInput->Release();
        g_pDirectInput = nullptr;
    }
}

// 检测设备是否支持力反馈
bool checkIsSupportForceFeedback(QString deviceName){
    qDebug() << "当前选择的转向轴设备: " << deviceName;

    bool result = false;

    if(initDirectInput()){
        // 根据设备名称找到设备guidInstance, 并初始化设备
        for(auto deviceInfo : diDeviceList){
            if(deviceName.toStdString() == deviceInfo.name){
                LPDIRECTINPUTDEVICE8 device = nullptr;
                // 创建设备实例
                if (FAILED(g_pDirectInput->CreateDevice(deviceInfo.guidInstance, &device, NULL))) {
                    qDebug() << "设备创建失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设备创建失败！"));
                    break;
                }

                if (FAILED(device->SetDataFormat(&c_dfDIJoystick2))) {
                    qDebug() << "设置数据格式失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置数据格式失败！"));
                    break;
                }

                if (FAILED(device->SetCooperativeLevel(GetForegroundWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
                    qDebug() << "设置协作模式失败！";
                    pushToQueue(parseErrorLog("初始化" + QString("设备[").append(deviceInfo.name.data()).append("]") + ": 设置协作模式失败！"));
                    break;
                }

                DIDEVCAPS diDevCaps;
                diDevCaps.dwSize = sizeof(DIDEVCAPS);
                device->GetCapabilities(&diDevCaps);

                if (diDevCaps.dwFlags & DIDC_FORCEFEEDBACK) {
                    // 设备支持力反馈
                    qDebug() << "当前选择的设备支持力反馈";
                    result = true;
                }else{
                    qDebug() << "当前设备不支持力反馈!";
                    pushToQueue(parseErrorLog("当前设置的转向轴设备不支持力反馈!"));
                }

                if(device){
                    device->Unacquire();
                    device->Release();
                    device = nullptr;
                }
            }
        }

    }else{
        qDebug() << "检测设备是否支持力反馈失败: 初始化设备失败!";
        pushToQueue(parseErrorLog("检测设备是否支持力反馈失败: 初始化设备失败!"));
    }

    return result;
}


// 手柄摇杆内部死区值
double xboxJoystickInnerDeadAreaValue = 0.0;
// 手柄扳机内部死区值
double xboxTriggerInnerDeadAreaValue = 0.0;
// 获取手柄摇杆内部死区值
double getXboxJoystickInnerDeadAreaValue(){
    return xboxJoystickInnerDeadAreaValue;
}
// 设置手柄摇杆内部死区值
void setXboxJoystickInnerDeadAreaValue(double value){
    xboxJoystickInnerDeadAreaValue = value;
}
// 获取手柄扳机内部死区值
double getXboxTriggerInnerDeadAreaValue(){
    return xboxTriggerInnerDeadAreaValue;
}
// 设置手柄扳机内部死区值
void setXboxTriggerInnerDeadAreaValue(double value){
    xboxTriggerInnerDeadAreaValue = value;
}


// 获取软件数据文件存储的路径
QString getAppDataDirStr(){
    return QDir::homePath() + "/AppData/Local/KeyMappingToolData/";
}

// 映射列表含有映射xbox的记录
bool hasXboxMappingInMappingList(std::vector<MappingRelation*> mappingList){
    if(mappingList.empty()){
        return false;
    }

    for(auto mapping : mappingList){
        if(mapping != nullptr && mapping->mappingType == MappingType::Xbox){
            return true;
        }
    }

    return false;
}
// BUTTONS_VALUE_TYPE 转换为字符串
std::string ButtonsValueTypeToString(BUTTONS_VALUE_TYPE btnValue){
    std::string btnValueStr = "";
    for (size_t i = 0; i < MAX_BUTTONS; i++) {
        if (btnValue.getBit(i)) {
            btnValueStr += "按键" + std::to_string(i) + "+";
        }
    }
    if (btnValueStr.size() > 0) {
        btnValueStr.pop_back();  // 去掉最后的 "+"
    }
    return btnValueStr;
}
