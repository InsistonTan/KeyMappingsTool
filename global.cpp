#include<global.h>
#include<QDebug>
#include<QMessageBox>

bool isRuning = false;
bool isXboxMode = false;
LPDIRECTINPUT8 g_pDirectInput = nullptr;
LPDIRECTINPUTDEVICE8 g_pDevice = nullptr;
QList<DiDeviceInfo> diDeviceList;
std::map<std::string, DIPROPRANGE> axisValueRangeMap;

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
    qDebug() << "DirectInput 初始化成功！";

    // 列出所有连接的游戏控制器设备
    diDeviceList.clear();
    if (FAILED(g_pDirectInput->EnumDevices(DI8DEVCLASS_GAMECTRL, EnumDevicesCallback, NULL, DIEDFL_ATTACHEDONLY))) {
        QMessageBox::critical(nullptr, "错误", "初始化DirectInput: 扫描设备列表失败！");
        return false;
    }
    qDebug() << "扫描设备成功！";

    return true;
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
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设备创建失败！");
        return false;
    }

    if (FAILED(g_pDevice->SetDataFormat(&c_dfDIJoystick2))) {
        qDebug() << "设置数据格式失败！";
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置数据格式失败！");
        return false;
    }

    if (FAILED(g_pDevice->SetCooperativeLevel(GetForegroundWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) {
        qDebug() << "设置协作模式失败！";
        QMessageBox::critical(nullptr, "错误", "初始化设备: 设置协作模式失败！");
        return false;
    }

    // 获取控制器能力
    DIDEVCAPS capabilities;
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(g_pDevice->GetCapabilities(&capabilities))) {
        qDebug() << "获取设备能力失败！";
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

    if(axisValueRangeMap.size() < 8){
        QMessageBox::critical(nullptr, "错误", "初始化设备: 获取方向盘各个轴的数值范围失败!");
        return false;
    }

    return true;
}
// 获取设备状态信息
QList<MappingRelation*> getInputState() {
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
        qDebug() << "设备获取失败，错误代码：" << HRESULT_CODE(hr);
        return list;
    }

    // 获取按键状态
    if (SUCCEEDED(g_pDevice->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
        // 遍历按键，查看按键是否按下
        for (int i = 0; i < 128; i++) {
            if (js.rgbButtons[i] & 0x80) {
                //qDebug() << "按键" << i << "被按下";
                std::string btnStr = "按键" + std::to_string(i);

                list.append(new MappingRelation(btnStr, WHEEL_BUTTON, i, 0, ""));
            }
        }

        // 映射xbox
        if(getIsXboxMode()){
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

    }else{
        qDebug() << "获取设备状态信息失败!";
        qDebug() << "GetDeviceState failed with error:" << HRESULT_CODE(hr);
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


