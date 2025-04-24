#include<global.h>
#include<QDebug>
#include<QMessageBox>
#include<QMutexLocker>
#include<QMutex>
#include<QDateTime>
#include "AssistFuncWindow.h"
#include "mainwindow.h"
#include<QDir>

QWidget* g_mainWindow = nullptr;

bool isRuning = false;
bool isXboxMode = false;
LPDIRECTINPUT8 g_pDirectInput = nullptr;
LPDIRECTINPUTDEVICE8 g_pDevice = nullptr;
QList<DiDeviceInfo> diDeviceList;
std::map<std::string, DIPROPRANGE> axisValueRangeMap;
BUTTONS_VALUE_TYPE g_btnBitValue;

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
double innerDeadAreaPanti = 0.03;
// 踏板轴死区
double innerDeadAreaTaban = 0.03;
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

void setIsXboxMode(bool val){
    isXboxMode = val;
}

bool getIsXboxMode(){
    return isXboxMode;
}

// DirectInput 回调函数，用于列出所有连接的游戏控制器设备
BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
    DiDeviceInfo deviceInfo;
    deviceInfo.name = QString::fromWCharArray(pdidInstance->tszProductName).toStdString();
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
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 初始化失败！");
        return false;
    }

    pushToQueue(parseSuccessLog("DirectInput 初始化成功！"));

    // 列出所有连接的游戏控制器设备
    diDeviceList.clear();
    if (FAILED(g_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 扫描设备列表失败！");
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
        QMessageBox::critical(nullptr, "错误", "扫描设备列表失败！");
        return;
    }
}

int lastDeviceIndex = -1;

void getDipropRange(long axisCode, std::string axisName){
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
        qDebug("获取方向盘%s的值范围失败!!", axisName.data());
        pushToQueue(parseWarningLog("获取" + QString(axisName.data()) + "的数值范围失败, 如果是非方向盘设备请忽略此警告"));
    }
}

// 打开选择的设备
bool openDiDevice(int deviceIndex) {
    if (deviceIndex < 0 || deviceIndex >= diDeviceList.size()) {
        return false;
    }

    // 相同设备, 无需重复打开
    if (g_pDevice && lastDeviceIndex == deviceIndex) {
        return true;
    }

    // 新设备
    lastDeviceIndex = deviceIndex;
    if(g_pDevice){
        g_pDevice->Unacquire();
        g_pDevice->Release();
        g_pDevice = nullptr;
    }

    // 创建设备实例
    if (FAILED(g_pDirectInput->CreateDevice(diDeviceList[deviceIndex].guidInstance, &g_pDevice, NULL))) {
        qDebug() << "设备创建失败！";
        pushToQueue(parseErrorLog("初始化设备: 设备创建失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设备创建失败！");
        return false;
    }

    if (FAILED(g_pDevice->SetDataFormat(&c_dfDIJoystick2))) {
        qDebug() << "设置数据格式失败！";
        pushToQueue(parseErrorLog("初始化设备: 设置数据格式失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置数据格式失败！");
        return false;
    }

    if (FAILED(g_pDevice->SetCooperativeLevel(GetForegroundWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
        qDebug() << "设置协作模式失败！";
        pushToQueue(parseErrorLog("初始化设备: 设置协作模式失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置协作模式失败！");
        return false;
    }

    // 获取控制器能力
    DIDEVCAPS capabilities;
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(g_pDevice->GetCapabilities(&capabilities))) {
        qDebug() << "获取设备能力失败！";
        pushToQueue(parseErrorLog("初始化设备: 获取设备能力失败！"));
        QMessageBox::critical(nullptr, "错误", "初始化设备: 获取设备能力失败！");
        return false;
    }

    // 获取各个轴的值范围
    getDipropRange(DIJOFS_X, "X轴");
    getDipropRange(DIJOFS_Y, "Y轴");
    getDipropRange(DIJOFS_Z, "Z轴");
    getDipropRange(DIJOFS_RX, "X轴旋转");
    getDipropRange(DIJOFS_RY, "Y轴旋转");
    getDipropRange(DIJOFS_RZ, "Z轴旋转");
    getDipropRange(DIJOFS_SLIDER(0), "滑动轴1");
    getDipropRange(DIJOFS_SLIDER(1), "滑动轴2");

    qDebug() << "连接设备成功！axisValueRangeMap.size():" << axisValueRangeMap.size();

    pushToQueue(parseSuccessLog("连接设备成功！"));

    //if(axisValueRangeMap.size() < 8){
        //QMessageBox::warning(nullptr, "警告", "初始化设备: 获取方向盘各个轴的数值范围失败, 如果当前选择的设备不是方向盘请忽略此提醒");
        //return false;
    //}

    return true;
}
// 获取设备状态信息
QList<MappingRelation*> getInputState(bool enableLog, std::vector<MappingRelation> multiBtnVector) {
    QList<MappingRelation*> list;

    // 方向盘按键状态
    DIJOYSTATE2 js;
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
        g_btnBitValue = btnBitValue;  // 更新全局按键值

        if (!btnStr.empty()) {
            // 去掉最后的 "+"
            btnStr = btnStr.substr(0, btnStr.length() - 1);

            // multiBtnVector不为空, 需要进行多按键映射处理
            if (multiBtnVector.size() > 0) {
                // 多按键映射，需要匹配按键，并拆分为多个 MappingRelation对象, 根据keyValue进行拆分
                for (auto multiBtn : multiBtnVector) {
                    BUTTONS_VALUE_TYPE multiBtnBitValue = multiBtn.dev_btn_bit_value;
                    if ((multiBtnBitValue) && ((multiBtnBitValue & btnBitValue) == multiBtnBitValue)) {
                        // 找到对应的按键, 进行映射
                        if (AssistFuncWindow::getEnableOnlyLongestMapping()) {
                            btnBitValue &= (~multiBtnBitValue);  // 清除当前按键的值
                        }
                        MappingRelation *mapping = new MappingRelation(multiBtn.dev_btn_name, WHEEL_BUTTON, 0, 0, "");
                        mapping->setBtnBitValue(multiBtnBitValue);  // 设置按键值
                        list.append(mapping);
                    }
                }
            }else{
                list.append(new MappingRelation(btnStr, WHEEL_BUTTON, 0, 0, ""));
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
                list.append(new MappingRelation(btnStr, WHEEL_BUTTON, val, 0, ""));
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
        //if(getIsXboxMode()){
        if(true){
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

    return list;
}
// 关闭 DirectInput
void cleanupDirectInput() {
    // 释放资源
    if (g_pDevice) {
        g_pDevice->Unacquire();
        g_pDevice->Release();
        g_pDevice = nullptr;
    }

    if (g_pDirectInput) {
        g_pDirectInput->Release();
        g_pDirectInput = nullptr;
    }
}

// 检测设备是否支持力反馈
bool checkIsSupportForceFeedback(){
    qDebug() << "当前选择的设备下标: " << MainWindow::getCurrentSelectedDeviceIndex();

    if(initDirectInput() && openDiDevice(MainWindow::getCurrentSelectedDeviceIndex())){
        DIDEVCAPS diDevCaps;
        diDevCaps.dwSize = sizeof(DIDEVCAPS);
        g_pDevice->GetCapabilities(&diDevCaps);

        if (diDevCaps.dwFlags & DIDC_FORCEFEEDBACK) {
            // 设备支持力反馈
            qDebug() << "当前选择的设备支持力反馈";

            return true;
        }else{
            qDebug() << "当前设备不支持力反馈!";
            pushToQueue(parseErrorLog("当前设备不支持力反馈!"));
        }
    }else{
        qDebug() << "检测设备是否支持力反馈失败: 初始化设备失败!";
        pushToQueue(parseErrorLog("检测设备是否支持力反馈失败: 初始化设备失败!"));
    }

    return false;
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


