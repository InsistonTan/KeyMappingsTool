#include "ConfigService.h"
#include "DirectInputService.h"
#include "LogService.h"
#include "qlist.h"
#include "qmutex.h"
#include "qthreadpool.h"
#include "common/Global.h"
#include "common/StringConstants.h"
#include "core/DirectInputCore.h"
#include "models/MappingRelation.h"
#include "services/MessageBoxService.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDateTime>
#include <QRegularExpression>

#include <winerror.h>


DirectInputService::DirectInputService() {}

void DirectInputService::clearInitedDeviceList()
{
    // 加锁
    QMutexLocker locker(&g_initedDeviceListMutex);

    for(auto device : g_initedDeviceList){
        // 释放资源
        if (device) {
            device->Unacquire();
            device->Release();
            device = nullptr;
        }
    }

    g_initedDeviceList.clear();
}

bool DirectInputService::initDirectInput()
{
    if(g_pDirectInput != nullptr){
        return true;
    }

    // 初始化 DirectInput 全局实例
    if (DirectInputCore::initDirectInputInstance(&g_pDirectInput) != DiActionResultEnum::Success) {
        qDebug() << "DirectInput 初始化失败！";

        MessageBoxService::showError(StringConstants::initDirectInputErrorMsg);
        return false;
    }

    LogService::parseSuccessLog(StringConstants::initDirectInputSuccessMsg);
    return true;
}

void DirectInputService::cleanupDirectInput()
{
    if (g_pDirectInput) {
        g_pDirectInput->Release();
        g_pDirectInput = nullptr;
    }
}

bool DirectInputService::isDeviceAlive(LPDIRECTINPUTDEVICE8 device){
    if(device == nullptr)
        return false;

    // 失败计数
    int failedCounter = 0;
    HRESULT hr;

    // poll连续 10 次失败, 该设备已经断开连接
    while(failedCounter < 10){
        hr = device->Poll();
        if(FAILED(hr)){
            failedCounter++;
            device->Acquire();
        }else{
            return true;
        }


        Sleep(100);
        //QCoreApplication::processEvents();
    }

    return false;
}

MappingRelation DirectInputService::getNextActionBtnOrAxis(
    QVector<LPDIRECTINPUTDEVICE8> initedDeviceList,
    bool onlyGetChangedKey,
    bool enableLogs,
    bool readBtnData,
    bool readAxisData)
{
    // 获取第一次数据
    bool isFirstData = true;
    //qint64 timeTickFirst = QDateTime::currentMSecsSinceEpoch();
    QMap<QString, BUTTONS_VALUE_TYPE> firstKeyStateMap;

    // 临时记录, 用来检测轴的数值变化, key为轴名称. value为轴的值
    QMap<QString, int> tempRecord;

    // 设置一个3s定时器，时间到后停止监听
    qint64 listeningTime = QDateTime::currentMSecsSinceEpoch() + 3000; // 3秒后停止监听
    qint64 timeTickRun = QDateTime::currentMSecsSinceEpoch();          // 记录当前时间戳

    // 设置系统定时器精度为1ms
    Global::setSystemTimePeriod_1ms();

    // 设备数据结果
    QVector<MappingRelation> res;

    // 监听设备按键状态
    while(QDateTime::currentMSecsSinceEpoch() < listeningTime){
        // 每50ms获取一次数据
        if (QDateTime::currentMSecsSinceEpoch() - timeTickRun < 50) {
            // 休息一下
            Sleep(10);
            // 处理事件队列，防止界面卡死
            QCoreApplication::processEvents();
            continue;
        }
        timeTickRun = QDateTime::currentMSecsSinceEpoch();

        // 获取设备状态数据
        // getInputState()会自动重置res, 然后添加新数据到res
        DirectInputService::getInputState(res, initedDeviceList, readBtnData, readAxisData, enableLogs);

        // 获取初始按键状态
        if (isFirstData && onlyGetChangedKey) {
            isFirstData = false;
            DirectInputService::getInputState(res, initedDeviceList, true, true, enableLogs);
            for (const auto& item : res) {
                if (item.dev_btn_type == DeviceDataTypeEnum::WHEEL_BUTTON) {
                    QString deviceName = item.deviceName;
                    if (!firstKeyStateMap.contains(deviceName)) {
                        // 记录第一次数据
                        firstKeyStateMap[deviceName] = item.dev_btn_bit_value;
                    } else {
                        // 不是第一次读到该按键的值, 叠加
                        firstKeyStateMap[deviceName] |= item.dev_btn_bit_value;
                    }
                }
            }
        }

        if(res.size() > 0){
            for(auto item : res){
                // 补全设备名称之后的按键/轴名称
                auto btnOrAxisStr = Global::getBtnOrAxisFullName(item.deviceName, item.dev_btn_name);

                // 方向盘的轴
                if(item.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS){
                    // 记录第一次数据
                    if(!tempRecord.contains(btnOrAxisStr)){
                        tempRecord[btnOrAxisStr] = item.dev_btn_value;
                    }else{
                        auto lastAxisValue = tempRecord[btnOrAxisStr];
                        // 不是第一次读到该轴的值, 与第一次的值比较, 大于一定量才能确定是该轴要新建映射
                        if(std::abs(item.dev_btn_value - lastAxisValue) > Global::AXIS_CHANGE_VALUE){
                            // 值的变化量
                            item.axisValueChange = item.dev_btn_value - lastAxisValue;
                            return item;
                        }
                    }
                }else{
                    // 方向盘按键
                    if(firstKeyStateMap.size() > 0 && onlyGetChangedKey){
                        QString deviceName = item.deviceName;
                        BUTTONS_VALUE_TYPE nowValue = item.dev_btn_bit_value;

                        for (auto it = firstKeyStateMap.begin(); it != firstKeyStateMap.end(); ++it) {
                            //for (auto &firstKey : firstKeyStateMap) {
                            auto key = it.key();
                            auto value = it.value();
                            if (key == deviceName && (value != nowValue)) {
                                // 找出不同的按键
                                BUTTONS_VALUE_TYPE btnValue = nowValue ^ value;
                                btnValue &= KEY_ONLY_FACTOR; // 去掉POV按键的值
                                if (value.operator>>(DINPUT_MAX_BUTTONS) != nowValue.operator>>(DINPUT_MAX_BUTTONS)) {
                                    btnValue |= (nowValue & POV_ONLY_FACTOR); // 添加变化的POV按键的值
                                }
                                item.dev_btn_name = Global::ButtonsValueTypeToString(btnValue);
                                item.dev_btn_bit_value = btnValue;
                                return item;
                            }
                        }
                    } else {
                        return item;
                    }

                }
            }
        }
    }

    // 重置系统定时器精度
    Global::restoreSystemTimePeriod();

    // 返回无效的对象(valid = false)
    return MappingRelation(false);
}

bool DirectInputService::openDiDevice(QVector<QString> deviceNameList, bool interruptWhenError, DWORD cooperativeLevel)
{
    if (deviceNameList.isEmpty()) {
        return false;
    }

    if(g_pDirectInput == nullptr)
        initDirectInput();

    // 重新扫描一次设备列表
    scanDevice();

    int initedCounter = 0;

    // 遍历设备列表
    for(auto const &deviceName : deviceNameList){
        auto pInitedDevice = getInitedDevice(deviceName);

        if(pInitedDevice != nullptr){
            // 如果当前设备已经初始化, 且还存活, 则无需重复初始化
            if(isDeviceAlive(pInitedDevice)){
                initedCounter++;
                continue;
            }else{
                // 设备已无效, 释放设备
                pInitedDevice->Unacquire();
                pInitedDevice->Release();
                pInitedDevice = nullptr;
            }
        }

        // 是否在 g_diDeviceList 找到 deviceName对应的设备
        bool foundDevice = false;

        // 根据设备名称找到设备guidInstance, 并初始化设备, 添加初始化后的设备到已选择设备列表
        for(auto const &deviceInfo : getDeviceInfoListSnapshot()){
            if(deviceName.toStdString() == deviceInfo.name){
                foundDevice = true;

                LPDIRECTINPUTDEVICE8 g_pDevice = nullptr;

                // 初始化设备
                auto result = DirectInputCore::initDevice(Global::getHideWindowHWnd(), g_pDirectInput, deviceInfo, &g_pDevice, cooperativeLevel);

                // 初始化失败
                if(result != DiActionResultEnum::Success){
                    // 显示错误信息
                    showInitDeviceFailedMsg(result, deviceInfo.name, interruptWhenError);

                    // 开启了发生错误时中断后续操作
                    if(interruptWhenError){
                        return false;
                    }else{
                        // 跳过这个设备, 继续处理后面的设备
                        break;
                    }
                }

                // 获取各个轴的值范围
                getDipropRange(g_pDevice, DIJOFS_X, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisX));
                getDipropRange(g_pDevice, DIJOFS_Y, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisY));
                getDipropRange(g_pDevice, DIJOFS_Z, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisZ));
                getDipropRange(g_pDevice, DIJOFS_RX, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRX));
                getDipropRange(g_pDevice, DIJOFS_RY, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRY));
                getDipropRange(g_pDevice, DIJOFS_RZ, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRZ));
                getDipropRange(g_pDevice, DIJOFS_SLIDER(0), Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisS1));
                getDipropRange(g_pDevice, DIJOFS_SLIDER(1), Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisS1));

                //qDebug() << "连接设备成功！axisValueRangeMap.size():" << g_axisValueRangeMap.size();

                LogService::parseSuccessLog(StringConstants::connectDeviceSuccessMsg.arg(deviceInfo.name.data()));

                {
                    QMutexLocker locker(&g_initedDeviceListMutex);
                    // 添加设备到已初始化设备列表
                    g_initedDeviceList.append(g_pDevice);
                }


                // 计数+1
                initedCounter++;

                // 当前设备已处理完成, 操作下一个设备
                break;
            }
        }

        // 该设备没有在设备信息列表找到
        if(foundDevice == false){
            // 开启了发生错误时中断后续操作
            if(interruptWhenError){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::deviceNotFoundErrorMsg.arg(deviceName));
                return false;
            }else{
                LogService::parseErrorLog(StringConstants::deviceNotFoundErrorMsg.arg(deviceName));
            }
        }
    }

    // 成功初始化的计数 跟 设备列表大小一致, 返回true
    return initedCounter == deviceNameList.size();
}

LPDIRECTINPUTDEVICE8 DirectInputService::openDiDevice(HWND hWnd, QString deviceName, DWORD cooperativeLevel)
{
    if (deviceName.isEmpty()) {
        return nullptr;
    }

    if(g_pDirectInput == nullptr)
        initDirectInput();

    // 重新扫描一次设备列表
    scanDevice();

    // 是否在 g_diDeviceList 找到 deviceName对应的设备
    bool foundDevice = false;

    // 根据设备名称找到设备guidInstance, 并初始化设备, 添加初始化后的设备到已选择设备列表
    for(auto const &deviceInfo : getDeviceInfoListSnapshot()){
        if(deviceName.toStdString() == deviceInfo.name){
            foundDevice = true;

            LPDIRECTINPUTDEVICE8 g_pDevice = nullptr;

            // 初始化设备
            auto result = DirectInputCore::initDevice(hWnd, g_pDirectInput, deviceInfo, &g_pDevice, cooperativeLevel);

            // 初始化失败
            if(result != DiActionResultEnum::Success){
                // 显示错误信息
                showInitDeviceFailedMsg(result, deviceInfo.name, true);
                return nullptr;
            }

            // 获取各个轴的值范围
            getDipropRange(g_pDevice, DIJOFS_X, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisX));
            getDipropRange(g_pDevice, DIJOFS_Y, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisY));
            getDipropRange(g_pDevice, DIJOFS_Z, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisZ));
            getDipropRange(g_pDevice, DIJOFS_RX, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRX));
            getDipropRange(g_pDevice, DIJOFS_RY, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRY));
            getDipropRange(g_pDevice, DIJOFS_RZ, Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisRZ));
            getDipropRange(g_pDevice, DIJOFS_SLIDER(0), Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisS1));
            getDipropRange(g_pDevice, DIJOFS_SLIDER(1), Global::getBtnOrAxisFullName(deviceInfo.name.data(), StringConstants::axisS1));

            LogService::parseSuccessLog(StringConstants::connectDeviceSuccessMsg.arg(deviceInfo.name.data()));


            // 当前设备已处理完成, 结束
            return g_pDevice;
        }
    }

    // 该设备没有在设备信息列表找到
    if(foundDevice == false){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::deviceNotFoundErrorMsg.arg(deviceName));
    }

    return nullptr;
}

void DirectInputService::getInputState(
    QVector<MappingRelation>& out,
    const QVector<LPDIRECTINPUTDEVICE8>& initedDeviceList,
    bool readBtnData,
    bool readAxisData,
    bool enableLog,
    std::vector<MappingRelation> multiBtnVector,
    bool enableOnlyLongestMapping)
{
    // 关键参数无效
    if(initedDeviceList.empty() || (readBtnData == false && readAxisData == false)){
        return;
    }

    // 重置输出
    out.clear();

    // 方向盘按键状态
    DIJOYSTATE2 js;

    // "角度"
    auto angleString = StringConstants::angle;
    // 设备名称
    QString deviceName;

    // 开启按键日志
    bool enableBtnLog = enableLog && Global::getEnableBtnLog();
    // 开启十字键/摇杆日志
    bool enablePovLog = enableLog && Global::getEnablePovLog();
    // 开启十字键/摇杆日志
    bool enableAxisLog = enableLog && Global::getEnableAxisLog();

    // 日志文本
    QString btnLog;
    QString povLog;

    // 按键名称
    QString btnStr;

    // 每个设备读取一遍数据
    for(auto pDevice : initedDeviceList){
        // 获取设备名称,
        deviceName = getDeviceName(pDevice);

        // 获取设备状态数据失败
        if(DirectInputCore::getDeviceStateData(pDevice, &js) != DiActionResultEnum::Success){
            LogService::parseErrorLog(StringConstants::getJoyStateErrorMsg.arg(deviceName));
            continue;
        }

        // 重置日志
        btnLog.clear();
        povLog.clear();

        // 读取按键数据
        if(readBtnData){
            if(enableBtnLog){
                btnLog.append(StringConstants::btnDataLog).append(": { ");
            }

            // 重置按键名称
            btnStr.clear();

            // 遍历按键，查看按键是否按下
            BUTTONS_VALUE_TYPE btnBitValue;
            for (size_t i = 0; i < DINPUT_MAX_BUTTONS; i++) {
                if (js.rgbButtons[i] & 0x80) {
                    btnStr.append(StringConstants::btnString)
                          .append(QString::number(i))
                          .append(BUTTON_NAME_COMBINE_SPLIT);
                    btnBitValue.setBit(i, true);
                }

                // 记录日志
                if(enableBtnLog){
                    int val = static_cast<int>(js.rgbButtons[i]);
                    if(val > 0){
                        btnLog.append("<span style='color:green;'><b>")
                            .append(QString::number(val))
                            .append("</b></span>")
                            .append(", ");
                    }else{
                        btnLog.append(QString::number(val))
                            .append(", ");
                    }

                }
            }
            if(enableBtnLog){
                btnLog.append("}");
            }

            // 遍历摇杆数据
            // DWORD	rgdwPOV[4];
            if(enablePovLog){
                povLog.append(StringConstants::joystickDataLog)
                    .append(": { ");
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
                    // 这里用反码操作, 因为摇杆的值是0-360度, 而没有操作时返回的也是0, 所以需要反码操作来表示操作
                    BUTTONS_VALUE_TYPE povBitValue = (uint16_t)~formatVal;
                    btnBitValue |= povBitValue.operator<<((j * 16 + DINPUT_MAX_BUTTONS));

                    // 摇杆/十字键 按键名称样例: "摇杆1-角度90"
                    btnStr.append(StringConstants::joystick) // "摇杆"
                        .append(QString::number(j+1)) // 摇杆编号, 如: 1
                        .append(ANGLE_NAME_CONNECT_STR) // "-"
                        .append(StringConstants::angle) // "角度"
                        .append(QString::number(formatVal)) // 摇杆角度值, 如: 90
                        .append(BUTTON_NAME_COMBINE_SPLIT);// 组合按键时, 按键名称分隔符, "+"
                }

                // 记录日志
                if(enablePovLog){
                    if(val > -1){
                        povLog.append("<span style='color:green;'><b>")
                            .append(QString::number(val))
                            .append("</b></span>")
                            .append(", ");
                    }else{
                        povLog.append(QString::number(val)).append(", ");
                    }

                }
            }
            if(enablePovLog){
                povLog.append("}");
            }

            // 按键名称不为空
            if (!btnStr.isEmpty()){
                // 去掉最后的 "+"
                btnStr = btnStr.left(btnStr.length() - 1);
                //QStringList btnStrList = QString(btnStr.data()).split(BUTTON_NAME_COMBINE_SPLIT);

                // multiBtnVector不为空, 需要进行多按键映射处理
                if (multiBtnVector.size() > 0){

                    // 多按键映射，需要匹配按键，并拆分为多个 MappingRelation对象, 根据keyValue进行拆分
                    for (const auto& multiBtn : multiBtnVector){
                        // 按键位值判断组合键是否被按下
                        BUTTONS_VALUE_TYPE multiBtnBitValue = multiBtn.dev_btn_bit_value;

                        if (multiBtn.deviceName == deviceName
                            && (multiBtnBitValue)
                            && ((multiBtnBitValue & btnBitValue) == multiBtnBitValue)){

                            // 如果按键包含十字键, 则需要保证 multiBtnBitValue的第三段 和 btnBitValue的第三段值相同, 否则跳过
                            if(QString(multiBtn.dev_btn_name.data()).contains(angleString)
                                && (multiBtnBitValue.operator>>(DINPUT_MAX_BUTTONS)) != (btnBitValue.operator>>(DINPUT_MAX_BUTTONS))){
                                continue;
                            }

                            // 找到对应的按键, 进行映射
                            if (enableOnlyLongestMapping) {
                                btnBitValue &= (~multiBtnBitValue);  // 清除当前按键的值
                            }

                            // 初始化一个 MappingRelation对象
                            MappingRelation mapping(
                                multiBtn.dev_btn_name,
                                DeviceDataTypeEnum::WHEEL_BUTTON,
                                0,
                                0,
                                "",
                                TriggerTypeEnum::Normal,
                                deviceName);
                            // 设置按键位值
                            mapping.setBtnBitValue(multiBtnBitValue);
                            out.append(mapping);
                        }
                    }
                }
                else{
                    MappingRelation mapping(
                        btnStr,
                        DeviceDataTypeEnum::WHEEL_BUTTON,
                        0,
                        0,
                        "",
                        TriggerTypeEnum::Normal,
                        deviceName);
                    // 设置按键位值
                    mapping.setBtnBitValue(btnBitValue);
                    out.append(mapping);
                }
            }

        }

        // 读取轴数据
        if(readAxisData){
            // 过滤无效数据
            if(js.lX != 0 && js.lX == js.lY && js.lY == js.lRx && js.lRx == js.lRy && js.lRy == js.lRz){}
            else{
                // X轴
                //qDebug() << "lx: " << js.lX;
                out.append(MappingRelation(StringConstants::axisX, DeviceDataTypeEnum::WHEEL_AXIS, js.lX, 0, "", TriggerTypeEnum::Normal, deviceName));

                // Y轴
                //qDebug() << "lY: " << js.lY;
                out.append(MappingRelation(StringConstants::axisY, DeviceDataTypeEnum::WHEEL_AXIS, js.lY, 0, "", TriggerTypeEnum::Normal, deviceName));

                // Z轴
                //qDebug() << "lZ: " << js.lZ;
                out.append(MappingRelation(StringConstants::axisZ, DeviceDataTypeEnum::WHEEL_AXIS, js.lZ, 0, "", TriggerTypeEnum::Normal, deviceName));

                // X轴旋转
                //qDebug() << "lRx: " << js.lRx;
                out.append(MappingRelation(StringConstants::axisRX, DeviceDataTypeEnum::WHEEL_AXIS, js.lRx, 0, "", TriggerTypeEnum::Normal, deviceName));

                // Y轴旋转
                //qDebug() << "lRy: " << js.lRy;
                out.append(MappingRelation(StringConstants::axisRY, DeviceDataTypeEnum::WHEEL_AXIS, js.lRy, 0, "", TriggerTypeEnum::Normal, deviceName));

                // Z轴旋转
                //qDebug() << "lRz: " << js.lRz;
                out.append(MappingRelation(StringConstants::axisRZ, DeviceDataTypeEnum::WHEEL_AXIS, js.lRz, 0, "", TriggerTypeEnum::Normal, deviceName));

                // 滑动轴1（通常是推杆或油门控制）
                //qDebug() << "rglSlider[0]: " << js.rglSlider[0];
                out.append(MappingRelation(StringConstants::axisS1, DeviceDataTypeEnum::WHEEL_AXIS, js.rglSlider[0], 0, "", TriggerTypeEnum::Normal, deviceName));

                // 滑动轴2（通常是推杆或油门控制）
                //qDebug() << "rglSlider[1]: " << js.rglSlider[1];
                out.append(MappingRelation(StringConstants::axisS2, DeviceDataTypeEnum::WHEEL_AXIS, js.rglSlider[1], 0, "", TriggerTypeEnum::Normal, deviceName));
                //qDebug("");
            }
        }

        // 记录日志
        if(enableLog){
            // 格式化输出
            QString logText =  StringConstants::deviceStatusDataLog.arg(deviceName);

            if(enableBtnLog){
                logText.append("    ").append(btnLog).append("<br/>");
            }
            if(enablePovLog){
                logText.append("    ").append(povLog).append("<br/>");
            }
            if(enableAxisLog){
                logText.append("    ").append(StringConstants::axisDataLog).append(": { ");
                logText.append(StringConstants::axisX).append(": ").append(QString::number((js.lX))).append(", ");
                logText.append(StringConstants::axisY).append(": ").append(QString::number(js.lY)).append(", ");
                logText.append(StringConstants::axisZ).append(": ").append(QString::number(js.lZ)).append(", ");
                logText.append(StringConstants::axisRX).append(": ").append(QString::number(js.lRx)).append(", ");
                logText.append(StringConstants::axisRY).append(": ").append(QString::number(js.lRy)).append(", ");
                logText.append(StringConstants::axisRZ).append(": ").append(QString::number(js.lRz)).append(", ");
                logText.append(StringConstants::axisS1).append(": ").append(QString::number(js.rglSlider[0])).append(", ");
                logText.append(StringConstants::axisS2).append(": ").append(QString::number(js.rglSlider[1])).append(", ");
                logText.append("}");
            }

            logText.append("</p>");

            // 异步推送到日志队列, 因为pushToLogQueue()函数里面有锁, 防止当前函数被锁阻塞
            QThreadPool::globalInstance()->start([logText](){
                LogService::pushToLogQueue(logText);
            });

        }

    }
}

void DirectInputService::getDipropRange(LPDIRECTINPUTDEVICE8 g_pDevice, long axisCode, QString axisName)
{
    // 获取方向盘的各个轴的数值范围
    DIPROPRANGE dipr1;
    dipr1.diph.dwSize = sizeof(DIPROPRANGE);
    dipr1.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    dipr1.diph.dwObj = axisCode;
    dipr1.diph.dwHow = DIPH_BYOFFSET;
    HRESULT hr1 = g_pDevice->GetProperty(DIPROP_RANGE, &dipr1.diph);
    if (SUCCEEDED(hr1)) {
        g_axisValueRangeMap.insert(axisName, dipr1);
    }else{
        //qDebug("获取设备[%s]的值范围失败!", axisName.data());
        LogService::parseWarningLog(StringConstants::getAxisValueRangeErrorMsg.arg(QString(axisName.data())));
    }
}

bool DirectInputService::scanDevice()
{
    if(g_pDirectInput == nullptr){
        auto res = initDirectInput();
        if(res == false){
            return false;
        }
    }

    // 加锁
    QMutexLocker locker(&g_diDeviceListMutex);

    // 重置设备列表
    g_diDeviceList.clear();


    // 临时存放的设备列表
    std::vector<DiDeviceInfo> tempDeviceInfoList;

    if (DirectInputCore::getDeviceList(g_pDirectInput, &tempDeviceInfoList) != DiActionResultEnum::Success) {
        MessageBoxService::showError(StringConstants::scanDeviceErrorMsg);
        return false;
    }

    // 对临时设备列表处理
    for(auto deviceInfo : tempDeviceInfoList){
        // 生成设备名称
        deviceInfo.name = generateDeviceName(deviceInfo);

        // 添加设备到列表
        g_diDeviceList.append(deviceInfo);
    }

    //LogService::parseSuccessLog(StringConstants::scanDeviceSuccessMsg);
    return true;
}

bool DirectInputService::checkIsSupportForceFeedback(LPDIRECTINPUTDEVICE8 device)
{
    if(device == nullptr){
        return false;
    }

    // 获取设备能力
    DIDEVCAPS diDevCaps;
    diDevCaps.dwSize = sizeof(DIDEVCAPS);
    device->GetCapabilities(&diDevCaps);

    // 设备支持力反馈
    if (diDevCaps.dwFlags & DIDC_FORCEFEEDBACK) {
        return true;
    }

    return false;
}

bool DirectInputService::isDeviceInited(QString deviceName)
{
    if(g_pDirectInput == nullptr)
        initDirectInput();

    for(auto initedDevice : getInitedDeviceListSnapshot()){
        if(deviceName == getDeviceName(initedDevice)){
            return true;
        }
    }

    return false;
}

QString DirectInputService::getDeviceName(LPDIRECTINPUTDEVICE8 pDevice)
{
    if(g_pDirectInput == nullptr)
        initDirectInput();

    DiDeviceInfo deviceInfo;
    auto actionResult = DirectInputCore::getDeviceInfo(g_pDirectInput, pDevice, &deviceInfo, nullptr);
    if (actionResult == DiActionResultEnum::Success){
        // 生成设备名称
        return generateDeviceName(deviceInfo).data();
    }

    return "";
}

void DirectInputService::showInitDeviceFailedMsg(DiActionResultEnum actionResult, std::string deviceName, bool showMessageBox)
{
    // 初始化失败
    if(actionResult != DiActionResultEnum::Success){
        // 错误信息
        QString msg;
        switch(actionResult){
        case DiActionResultEnum::Failed_CreateDevice:{
            //qDebug() << "设备创建失败！";
            msg = StringConstants::createDeviceErrorMsg.arg(deviceName.data());
            break;
        }
        case DiActionResultEnum::Failed_SetDataFormat:{
            //qDebug() << "设置数据格式失败！";
            msg = StringConstants::setDataFormatErrorMsg.arg(deviceName.data());
            break;
        }
        case DiActionResultEnum::Failed_SetCooperativeLevel:{
            //qDebug() << "设置协作模式失败！";
            msg = StringConstants::setCooperativeLevelErrorMsg.arg(deviceName.data());
            break;
        }
        case DiActionResultEnum::Failed_GetCapabilities:{
            //qDebug() << "获取设备能力失败！";
            msg = StringConstants::getCapabilitiesErrorMsg.arg(deviceName.data());
            break;
        }
        default:{
            msg = StringConstants::initDeviceUnknownError.arg(deviceName.data());
        }
        }

        if(showMessageBox){
            // 显示错误提示
            Global::showErrorMsgBoxAndPushToLog(msg);
        }else{
            LogService::parseErrorLog(msg);
        }
    }
}

std::string DirectInputService::generateDeviceName(DiDeviceInfo deviceInfo){
    std::string fianalName;

    // 开启设备名称强唯一模式
    if(ConfigService::get().SYSTEM_enableStrongUniqueDeviceNameMode){
        // 接口路径处理, 正则匹配, 去掉一些无效字符
        static const QRegularExpression reg1("(^.+(?=vid))|(#{.*$)");
        QString tempInterfacePath = deviceInfo.devInterfacePath.data();
        tempInterfacePath.replace(reg1, "");

        // 设备产品名称 + 设备接口路径
        fianalName = deviceInfo.productName + "(" + tempInterfacePath.toStdString() + ")";
    }else{
        // 设备产品名称 + 第一段guid值, guid样例 "(0B13045E-0000-0000-0000-504944564944)"
        fianalName = deviceInfo.productName + "("
                     + deviceInfo.guidString.substr(1, std::min<size_t>(8, deviceInfo.guidString.size()))
                     + ")";
    }

    return fianalName;
}

LPDIRECTINPUTDEVICE8 DirectInputService::getInitedDevice(QString deviceName)
{
    if(g_pDirectInput == nullptr)
        initDirectInput();

    for(auto initedDevice : getInitedDeviceListSnapshot()){
        if(deviceName == getDeviceName(initedDevice)){
            return initedDevice;
        }
    }

    return nullptr;
}




