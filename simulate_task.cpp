#include <simulate_task.h>
#include "AssistFuncWindow.h"
#include <global.h>
#include <QDebug>
#include<QTimer>
#include<QCoreApplication>

std::vector<MappingRelation> SimulateTask::handleMultiBtnVector = {};// 当前在使用的 设备组合键映射列表
std::vector<MappingRelation> SimulateTask::handleMultiBtnVectorUnsort = {};// 未排序的 设备组合键映射列表
std::vector<MappingRelation> SimulateTask::handleMultiBtnVectorSorted = {};// 已排序的 设备组合键映射列表(根据组合键的子键数量倒序)

void SimulateTask::addMappingToHandleMap(MappingRelation* mapping){
    // 记录按键触发模式
    if(mapping != nullptr){
        std::string btnStr = mapping->deviceName.toStdString() + "-" + mapping->dev_btn_name;

        // 记录按键触发模式
        keyTriggerTypeMap.insert_or_assign(btnStr, mapping->btnTriggerType);

        // 记录按键映射类型
        keyMappingTypeMap.insert_or_assign(btnStr, mapping->mappingType);

        // 记录需要反转的轴
        if(mapping->rotateAxis == 1){
            rotateAxisList.push_back(btnStr);
        }

        // handleMap还没有该按键, 直接添加
        if(handleMap.find(btnStr) == handleMap.end()){
            handleMap.insert_or_assign(btnStr, mapping->keyboard_value);
        }
    }
}

bool SimulateTask::isAxisRotate(std::string btnName){
    for(auto item : rotateAxisList){
        if(item == btnName){
            return true;
        }
    }

    return false;
}

SimulateTask::SimulateTask(std::vector<MappingRelation*> mappingList){
    this->mappingList = mappingList;

    // 如果映射列表里有映射xbox的记录, 才需要初始化虚拟xbox手柄
    this->needStartVirtualXbox = hasXboxMappingInMappingList(mappingList);
    handleMultiBtnVector.clear();

    for(MappingRelation* mapping : mappingList){
        if(isMappingValid(mapping)){
            addMappingToHandleMap(mapping);

            // 将映射添加进多按键映射列表
            MappingRelation newMapping = *mapping;
            handleMultiBtnVector.push_back(newMapping);
        }
    }

    // 未排序的组合键映射列表
    handleMultiBtnVectorUnsort = handleMultiBtnVector;

    // 对组合键映射列表排序, 按子键的数量倒序
    std::sort(handleMultiBtnVector.begin(), handleMultiBtnVector.end(), [](MappingRelation a, MappingRelation b) {
        // 比较加号的数量 
        int aPlusCount = std::count(a.dev_btn_name.begin(), a.dev_btn_name.end(), '+');
        int bPlusCount = std::count(b.dev_btn_name.begin(), b.dev_btn_name.end(), '+');
        if (aPlusCount != bPlusCount) {
            return aPlusCount > bPlusCount;  // 按加号数量降序排列
        }
        // 如果加号数量相同, 则没有角度的靠前
        if (a.dev_btn_name.find("角度") != std::string::npos && b.dev_btn_name.find("角度") == std::string::npos) {
            return false;  // a有角度, b没有角度, a靠后
        } else if (a.dev_btn_name.find("角度") == std::string::npos && b.dev_btn_name.find("角度") != std::string::npos) {
            return true;  // a没有角度, b有角度, a靠前
        }
        // 字符串大小比较
        return a.dev_btn_name > b.dev_btn_name;
    });

    // 已排序的组合键映射列表
    handleMultiBtnVectorSorted = handleMultiBtnVector;

    // 如果不启用最长组合键优先模式, 就使用 未排序的
    if (!AssistFuncWindow::getEnableOnlyLongestMapping()) {
        handleMultiBtnVector = handleMultiBtnVectorUnsort;
    }
}

void SimulateTask::closeDevice(){
    // Close the device
    //hid_close(handle);

    // 关闭DirectInput

}

// 检查当前按键是否一直在按住
bool SimulateTask::isCurrentKeyHolding(std::string keyStr){
    return keyHoldingMap.find(keyStr) != keyHoldingMap.end();
}

bool isCurrentBtnInList(QList<MappingRelation*> pressBtnList, std::string currentBtn){
    if(pressBtnList.empty()){
        return false;
    }

    for(auto item : pressBtnList){
        if(item->deviceName.toStdString() + "-" + item->dev_btn_name == currentBtn){
            return true;
        }
    }

    return false;
}

void SimulateTask::releaseAllKey(QList<MappingRelation*> pressBtnList){
    // 使用迭代器遍历并删除符合条件的键
    for (auto item = keyHoldingMap.begin(); item != keyHoldingMap.end(); ) {

        std::string btnStr = item->first;

        // // 找到 '#' 字符的位置
        // size_t pos = btnStr.find('#');
        // // 如果找到了 '#', 截取子串
        // if (pos != std::string::npos) {
        //     btnStr = btnStr.substr(0, pos);
        // }

        // 本次按下的按键列表为空, 或者当前按下的按键列表中不包含当前按键, 则松开当前按键
        if (pressBtnList.empty() || !isCurrentBtnInList(pressBtnList, btnStr)) {
            //qDebug("按键释放");
            // 映射键盘
            if(keyMappingTypeMap[btnStr] == MappingType::Keyboard){
                // 根据触发模式, 进行对应处理
                switch (keyTriggerTypeMap[btnStr]) {
                case TriggerTypeEnum::Release:
                case TriggerTypeEnum::PressAndRelease:
                    // 松开按键触发
                    // 模拟按下
                    simulateKeyPressMs(item->second, RELEASE_DELAY_MS);
                    break;
                case TriggerTypeEnum::Normal:
                    // 释放该位置的按键
                    simulateKeyPress(item->second, true);
                }

            }else{
                // 映射xbox
                // 根据触发模式, 进行对应处理
                switch (keyTriggerTypeMap[btnStr]) {
                case TriggerTypeEnum::Release:
                case TriggerTypeEnum::PressAndRelease:
                    // 松开按键触发
                    // 模拟按下
                    simulateXboxKeyPressMs(NormalButton, item->second, 0, RELEASE_DELAY_MS);
                    break;
                case TriggerTypeEnum::Normal:
                    // 释放该位置的按键
                    simulateXboxKeyPress(NormalButton, item->second, 0, true);
                }
            }

            item = keyHoldingMap.erase(item); // 删除并更新迭代器
        } else {
            ++item; // 继续下一个
        }
    }

    //keyPosMap.erase(keyPos);
}

bool SimulateTask::initXboxController(){
    // 初始化client
    vigemClient = vigem_alloc();
    if (vigemClient == nullptr) {
        qDebug("ViGEmClient allocation failed!");
        emit msgboxSignal(true, "虚拟手柄驱动初始化失败!\n如果重试后仍然失败, 请卸载驱动程序:\n设置-应用-安装的应用 找到 vigem bus driver 卸载\n卸载后再点击'开启全局映射'重装驱动");
        return false;
    }

    auto connectRes = vigem_connect(vigemClient);
    if (!VIGEM_SUCCESS(connectRes)) {
        qDebug("ViGEmClient connection failed!");
        emit msgboxSignal(true, "连接虚拟手柄驱动失败!\n如果重试后仍然失败, 请卸载驱动程序:\n设置-应用-安装的应用 找到 vigem bus driver 卸载\n卸载后再点击'开启全局映射'重装驱动");
        return false;
    }

    // 虚拟出一个xbox手柄
    vigemTarget = vigem_target_x360_alloc();
    auto addRes = vigem_target_add(vigemClient, vigemTarget);
    if (!VIGEM_SUCCESS(addRes)) {
        qDebug("Adding virtual Xbox 360 controller failed!");
        emit msgboxSignal(true, "添加虚拟手柄失败!\n如果重试后仍然失败, 请卸载驱动程序:\n设置-应用-安装的应用 找到 vigem bus driver 卸载\n卸载后再点击'开启全局映射'重装驱动");
        return false;
    }

    // 初始化模拟报告
    ZeroMemory(&report, sizeof(XUSB_REPORT));

    qDebug("初始化虚拟手柄控制器成功!");

    return true;
}

void SimulateTask::simulateXboxKeyPress(XboxInputType inputType, int inputValue1, int inputValue2, bool isRelease){
    // LeftJoystick,// 左摇杆
    // RightJoystick,// 右摇杆
    // LeftTrigger,// 左扳机
    // RightTrigger,// 右扳机
    // NormalButton,// 普通按键
    if(inputType == NormalButton){
        // 示例：模拟按下A键
        // report.wButtons |= XUSB_GAMEPAD_A;
        if(inputValue1 > 0 || inputValue1 < -100){
            if(!isRelease){
                // 按下按键
                report.wButtons |= inputValue1;
            }else{
                // 松开按键
                report.wButtons &= ~inputValue1; // 将按键对应位清零
            }
        }else{
            // {"手柄左摇杆-左移", -1},
            // {"手柄左摇杆-右移", -2},
            // {"手柄左摇杆-上移", -3},
            // {"手柄左摇杆-下移", -4},
            // {"手柄右摇杆-左移", -5},
            // {"手柄右摇杆-右移", -6},
            // {"手柄右摇杆-上移", -7},
            // {"手柄右摇杆-下移", -8},
            switch (inputValue1){
                case -1:
                    if(!isRelease){
                        report.sThumbLX -= XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbLX += XBOX_AXIS_SPEED;
                    }
                    break;
                case -2:
                    if(!isRelease){
                        report.sThumbLX += XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbLX -= XBOX_AXIS_SPEED;
                    }
                    break;
                case -3:
                    if(!isRelease){
                        report.sThumbLY += XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbLY -= XBOX_AXIS_SPEED;
                    }
                    break;
                case -4:
                    if(!isRelease){
                        report.sThumbLY -= XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbLY += XBOX_AXIS_SPEED;
                    }
                    break;
                case -5:
                    if(!isRelease){
                        report.sThumbRX -= XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbRX += XBOX_AXIS_SPEED;
                    }
                    break;
                case -6:
                    if(!isRelease){
                        report.sThumbRX += XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbRX -= XBOX_AXIS_SPEED;
                    }
                    break;
                case -7:
                    if(!isRelease){
                        report.sThumbRY += XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbRY -= XBOX_AXIS_SPEED;
                    }
                    break;
                case -8:
                    if(!isRelease){
                        report.sThumbRY -= XBOX_AXIS_SPEED;
                    }else{
                        // 松开按键, 摇杆归位
                        report.sThumbRY += XBOX_AXIS_SPEED;
                    }
                    break;
                case -9:
                    if(!isRelease){
                        qDebug("按下左扳机");
                        report.bLeftTrigger = XBOX_TRIGGER_SPEED;
                    }else{
                        qDebug("松开左扳机");
                        // 松开按键, 扳机归位
                        report.bLeftTrigger = 0;
                    }
                    break;
                case -10:
                    if(!isRelease){
                        report.bRightTrigger = XBOX_TRIGGER_SPEED;
                    }else{
                        // 松开按键, 扳机归位
                        report.bRightTrigger = 0;
                    }
                    break;
                default:
                    break;
            }
        }
    }else if(inputType == LeftJoystick){
        // 设置左摇杆的X、Y值 (-32768 到 32767)
        report.sThumbLX = inputValue1;
        //report.sThumbLY = inputValue2;

    }else if(inputType == RightJoystick){
        // 设置右摇杆的X、Y值 (-32768 到 32767)
        report.sThumbRX = inputValue1;
        //report.sThumbRY = inputValue2;

    }else if(inputType == LeftTrigger){
        // 设置左扳机的值 (0 到 255)
        report.bLeftTrigger = inputValue1;

    }else if(inputType == RightTrigger){
        // 设置右扳机的值 (0 到 255)
        report.bRightTrigger = inputValue1;
    }else{
        return;
    }

    // 发送输入状态到虚拟控制器
    vigem_target_x360_update(vigemClient, vigemTarget, report);
}

void SimulateTask::simulateXboxKeyPressMs(XboxInputType inputType, int inputValue1, int inputValue2, size_t pressMs){
    // 按下按键
    simulateXboxKeyPress(inputType, inputValue1, inputValue2, false);
    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
        // 延时后松开按键
        QTimer::singleShot(pressMs, [=](){
            simulateXboxKeyPress(inputType, inputValue1, inputValue2, true);
        });
    }, Qt::QueuedConnection);
}

void SimulateTask::simulateXboxKeyDelayPressMs(XboxInputType inputType, int inputValue1, int inputValue2, size_t pressMs, size_t delayMs) {
    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
        // 延时后按下按键
        QTimer::singleShot(delayMs, [=](){
            simulateXboxKeyPress(inputType, inputValue1, inputValue2, false);
            // 延时后松开按键
            QTimer::singleShot(pressMs, [=](){
                simulateXboxKeyPress(inputType, inputValue1, inputValue2, true);
            });
        });
    }, Qt::QueuedConnection);
}

void SimulateTask::closeXboxController(){
    // 清理
    if (vigemTarget) {
        vigem_target_remove(vigemClient, vigemTarget);
        vigem_target_free(vigemTarget);
    }
    if (vigemClient) {
        vigem_disconnect(vigemClient);
        vigem_free(vigemClient);
    }
}

QList<std::string> SimulateTask::getBtnStrListFromHandleMap(std::string btnStr){
    QList<std::string> list;

    // 没有该按键的映射
    if(handleMap.find(btnStr) == handleMap.end()){
        return list;
    }else{
        list.append(btnStr);
    }


    for (int i=1;;i++){
        // 没有该按键, 直接返回
        if(handleMap.find(btnStr + "#" + std::to_string(i)) == handleMap.end()){
            return list;
        }else{
            list.append(btnStr + "#" + std::to_string(i));
        }
    }
}

QList<MappingRelation*> SimulateTask::handleResult(QList<MappingRelation*> res){
    if(!res.isEmpty()){
        for(auto &currentBtn : res){
            // 按键名称补上设备名称
            auto btnStr = currentBtn->deviceName.toStdString() + "-" + currentBtn->dev_btn_name;
          
            // 轴映射键盘
            if((keyMappingTypeMap[btnStr] == MappingType::Keyboard) && btnStr.find("轴") != std::string::npos){
                // 当前轴的值范围
                auto currentRange = axisValueRangeMap.find(btnStr)->second;
                int currentMin = currentRange.lMin, currentMax = currentRange.lMax;
                int mid = (currentMax + currentMin)/2;

                // 配置了盘面轴左转映射
                if(handleMap.find(btnStr + "左转") != handleMap.end()){
                    // 设置了轴反转
                    if(isAxisRotate(btnStr + "左转")){
                        if(currentBtn->dev_btn_value >= static_cast<int>(mid + ((currentMax - mid) * getInnerDeadAreaPanti()))){
                            //btnStr += "左转";
                            currentBtn->dev_btn_name += "左转";
                        }
                    }else{
                        //qDebug("当前值: %d, 设定值: %d", currentBtn->dev_btn_value, static_cast<int>(mid - ((currentMax - mid) * getInnerDeadAreaPanti())));
                        if(currentBtn->dev_btn_value <= static_cast<int>(mid - ((currentMax - mid) * getInnerDeadAreaPanti()))){
                            //btnStr += "左转";
                            currentBtn->dev_btn_name += "左转";
                        }
                    }

                }
                // 配置了盘面轴右转映射
                if(handleMap.find(btnStr + "右转") != handleMap.end()){
                    // 设置了轴反转
                    if(isAxisRotate(btnStr + "右转")){
                        if(currentBtn->dev_btn_value <= (mid - ((currentMax - mid) * getInnerDeadAreaPanti()))){
                            //btnStr += "右转";
                            currentBtn->dev_btn_name += "右转";
                        }
                    }else{
                        if(currentBtn->dev_btn_value >= (mid + ((currentMax - mid) * getInnerDeadAreaPanti()))){
                            //btnStr += "右转";
                            currentBtn->dev_btn_name += "右转";
                        }
                    }

                }
                // 配置了踏板轴映射
                if(handleMap.find(btnStr) != handleMap.end()){
                    // 设置了轴反转
                    if(isAxisRotate(btnStr)){
                        // 值小于内部死区范围不生效
                        if(currentBtn->dev_btn_value > (currentMax - ((currentMax - currentMin) * getInnerDeadAreaTaban()))){
                            //btnStr = "000000";
                            currentBtn->dev_btn_name += "000000";
                        }
                    }else{
                        // 值小于内部死区范围不生效
                        if(currentBtn->dev_btn_value < (currentMin + ((currentMax - currentMin) * getInnerDeadAreaTaban()))){
                            //btnStr = "000000";
                            currentBtn->dev_btn_name += "000000";
                        }
                    }
                }
            }
        }
    }

    return res;
}

void SimulateTask::doWork(){
    qDebug("全局映射启动!");

    // 需要启动虚拟手柄
    if(this->needStartVirtualXbox){
        // 初始化失败
        if(!initXboxController()){
            // 关闭虚拟xbox设备
            closeXboxController();

            // 提交工作结束信号
            emit workFinished();
            return;
        }
    }

    setIsRuning(true);
    emit startedSignal();
    emit msgboxSignal(false, "启动全局映射成功!\n如果游戏里不生效, 请使用管理员身份重新运行本程序 ");

    pushToQueue(parseSuccessLog("启动全局映射成功!"));

    while(getIsRunning()){
        // 轮询设备状态
        auto res = getInputState(false, handleMultiBtnVector);

        // 对res进行处理
        res = handleResult(res);

        // 根据本次设备状态, 松开本次没有被按下的按键
        releaseAllKey(res);

        // 对当前按下的按键列表循环操作
        for(int i=0; !res.empty() && i < res.size(); i++){
            // 当前设备按钮的字符串
            //auto btnStrList = getBtnStrListFromHandleMap(res[i]->dev_btn_name);

            // 当前方向盘按键
            auto currentBtn = res[i];

            // 按键名称补上设备名称
            std::string btnStr = currentBtn->deviceName.toStdString() + "-" + currentBtn->dev_btn_name;

            // 先检查当前按钮是否在持续按下
            if(isCurrentKeyHolding(btnStr)){
                // 持续按下
                continue;
            }

            // 查找设备按键映射键盘扫描码map
            auto item = handleMap.find(btnStr);

            // 该按键存在映射, 模拟映射的键盘按键操作
            if (item != handleMap.end()) {
                //qDebug() << "按键[" << btnStr.data() << "]存在映射, 正在模拟对应操作";

                // 按下了配置的暂停按键
                if(item->second == PAUSE_BTN_VAL){
                    clickPauseBtn();
                    releaseAllKey({});

                    // 提交暂停按键被按下的信号
                    emit pauseClickSignal();

                    Sleep(500);
                }

                // qDebug() << "当前isPause值: " << (isPause ? "true" : "false");

                // 如果当前是暂停状态, 跳过后续的映射操作
                if(getIsPause()){
                    continue;
                }

                // 映射键盘
                if(keyMappingTypeMap[btnStr] == MappingType::Keyboard){
                    // 对映射鼠标左键(-7), 鼠标右键(-8), 鼠标中键(-11), 以及其它键盘按键进行按下记录
                    if(item->second == -7 || item->second == -8 || item->second == -11 ||  item->second > 0){
                        // 记录按键按下
                        keyHoldingMap.insert_or_assign(btnStr, item->second);
                    }

                    // 根据触发模式, 进行对应处理
                    switch (keyTriggerTypeMap[btnStr]) {
                    case TriggerTypeEnum::Delay1s:
                        //qDebug() << "延迟1s触发";
                        // 延迟1s触发
                        simulateKeyDelayPressMs(item->second, RELEASE_DELAY_MS, 1000);
                        break;
                    case TriggerTypeEnum::Delay3s:
                        //qDebug() << "延迟3s触发";
                        // 延迟3s触发
                        simulateKeyDelayPressMs(item->second, RELEASE_DELAY_MS, 3000);
                        break;
                    case TriggerTypeEnum::Delay5s:
                        //qDebug() << "延迟5s触发";
                        // 延迟5s触发
                        simulateKeyDelayPressMs(item->second, RELEASE_DELAY_MS, 5000);
                        break;
                    case TriggerTypeEnum::Release:
                        // 松开按键才触发
                        break;
                    case TriggerTypeEnum::PressAndRelease:
                        // 按下按键触发一次 按下松开 且松开按键再次触发一次 按下松开
                        simulateKeyPressMs(item->second, RELEASE_DELAY_MS);
                        break;
                    case TriggerTypeEnum::Normal:
                        //qDebug() << "默认的同步模式";
                        // 默认的同步模式
                        simulateKeyPress(item->second, false);
                        break;
                    }

                    //qDebug("映射键盘模式-按键按下");

                }else{
                    // 映射xbox
                    //auto currentBtn = res[i];

                    // 映射普通xbox按键
                    if(currentBtn->dev_btn_type == (std::string)WHEEL_BUTTON){
                        // qDebug("映射Xbox模式-按键按下:%s", btnStr.data());

                        // 记录按键按下
                        keyHoldingMap.insert_or_assign(btnStr, item->second);

                        // 根据触发模式, 进行对应处理
                        switch (keyTriggerTypeMap[btnStr]) {
                        case TriggerTypeEnum::Delay1s:
                            //qDebug() << "延迟1s触发";
                            // 延迟1s触发
                            simulateXboxKeyDelayPressMs(NormalButton, item->second, 0, RELEASE_DELAY_MS, 1000);
                            break;
                        case TriggerTypeEnum::Delay3s:
                            //qDebug() << "延迟3s触发";
                            // 延迟3s触发
                            simulateXboxKeyDelayPressMs(NormalButton, item->second, 0, RELEASE_DELAY_MS, 3000);
                            break;
                        case TriggerTypeEnum::Delay5s:
                            //qDebug() << "延迟5s触发";
                            // 延迟5s触发
                            simulateXboxKeyDelayPressMs(NormalButton, item->second, 0, RELEASE_DELAY_MS, 5000);
                            break;
                        case TriggerTypeEnum::Release:
                            // 松开按键才触发
                            break;
                        case TriggerTypeEnum::PressAndRelease:
                            // 按下按键触发一次 按下松开 且松开按键再次触发一次 按下松开
                            simulateXboxKeyPressMs(NormalButton, item->second, 0, RELEASE_DELAY_MS);
                            break;
                        case TriggerTypeEnum::Normal:
                            //qDebug() << "默认的同步模式";
                            // 默认的同步模式
                            // 按下xbox对应按键
                            simulateXboxKeyPress(NormalButton, item->second, 0, false);
                            break;
                        }

                    }else{

                        // 映射xbox轴
                        // 计算映射到手柄轴的实际值
                        auto xboxRange = XBOX_AXIS_VALUE_RANGE_MAP.find(item->second)->second;
                        int xboxMin = xboxRange.minVal, xboxMax = xboxRange.maxVal;

                        auto currentRange = axisValueRangeMap.find(btnStr)->second;
                        int currentMin = currentRange.lMin, currentMax = currentRange.lMax;

                        // 按键类型
                        XboxInputType inputType = static_cast<XboxInputType>(item->second);

                        double devAxisDataPer = 0.0;// 设备的值占设备值的范围的百分比
                        if(!isAxisRotate(btnStr)){
                            devAxisDataPer = (static_cast<double>(currentBtn->dev_btn_value) - currentMin) / (currentMax - currentMin);
                        }else{
                            devAxisDataPer = (currentMax - static_cast<double>(currentBtn->dev_btn_value)) /(currentMax - currentMin);
                        }

                        int finalValue = 0;// 最终映射成手柄的值

                        // 设置的摇杆内部死区值
                        if(inputType == XboxInputType::LeftJoystick || inputType == XboxInputType::RightJoystick){
                            // 摇杆内部死区值
                            int innerDeadAreaValue = (xboxMax - xboxMin) / 2 * getXboxJoystickInnerDeadAreaValue();

                            int leftMin = xboxMin, leftMax = innerDeadAreaValue; // 手柄左半区的最小值最大值
                            int rightMin = -innerDeadAreaValue, rightMax = xboxMax;// 手柄右半区的最小值最大值

                            // 对应手柄的左半区
                            if(devAxisDataPer <= 0.5){
                                // 计算出对应手柄的值, 并设置有效值范围为 <= 0
                                finalValue = std::min((int)(leftMin + (leftMax - leftMin) * devAxisDataPer * 2), 0);
                            }else{
                                // 手柄右半区, 计算出对应手柄的值, 并设置有效值范围为 >= 0
                                finalValue = std::max((int)(rightMin + (rightMax - rightMin) * (devAxisDataPer - 0.5) * 2), 0);
                            }
                        }

                        // 设置的扳机内部死区值
                        if(inputType == XboxInputType::LeftTrigger || inputType == XboxInputType::RightTrigger){
                            // 扳机内部死区值
                            int innerDeadAreaValue = (xboxMax - xboxMin) * getXboxTriggerInnerDeadAreaValue();

                            // 施加死区影响之后的值的区间范围
                            int tempMin = xboxMin - innerDeadAreaValue;
                            int tempMax = xboxMax - innerDeadAreaValue;

                            if(innerDeadAreaValue >= 0){
                                // 当前值处于区间的内的值
                                int tempValue = (int)(tempMin + (tempMax - tempMin) * devAxisDataPer);

                                tempMin = xboxMin;
                                tempMax = xboxMax - innerDeadAreaValue;

                                // 新区间百分比
                                double newAreaPer = (double)tempValue / (tempMax - tempMin);
                                newAreaPer = std::max(newAreaPer, 0.0);

                                // 根据新区间百分比 计算出 原xbox区间的值
                                finalValue = std::max(xboxMin, std::min((int)(newAreaPer * (xboxMax - xboxMin)), xboxMax));
                            }else{

                                tempMin = xboxMin - innerDeadAreaValue;
                                tempMax = xboxMax;

                                // 根据新区间百分比 计算出 原xbox区间的值
                                finalValue = std::max(xboxMin, std::min((int)(tempMin + (tempMax - tempMin) * devAxisDataPer), xboxMax));
                            }

                        }

                        // 模拟xbox轴
                        simulateXboxKeyPress(inputType, finalValue, 0, false);
                    }
                }

            }
        }

        // 释放res内存
        qDeleteAll(res);  // 删除所有指针指向的对象
        res.clear();      // 清空列表

        Sleep(5);
    }

    // 关闭虚拟xbox设备
    closeXboxController();

    // 提交工作结束信号
    emit workFinished();

    emit msgboxSignal(false, "全局映射已停止!");

    pushToQueue("全局映射已停止!");
}

// 模拟按键操作
void SimulateTask::simulateKeyPress(short scanCode, bool isKeyRelease) {
    // 模拟键盘操作
    if(scanCode > 0){
        INPUT input = {0};

        short tmpDwFlags;// 设置为使用硬件扫描码, 并为某些功能按键添加扩展码
        if(scanCode >= 0xC5 && scanCode <= 0xDF ){
            tmpDwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_EXTENDEDKEY;
        }else{
            tmpDwFlags = KEYEVENTF_SCANCODE;
        }

        // 模拟按下键
        input.type = INPUT_KEYBOARD;
        input.ki.dwFlags = tmpDwFlags;
        input.ki.wScan = scanCode;              // 设置扫描码

        //SendInput(1, &input, sizeof(INPUT));

        //Sleep(50);

        // 模拟释放键
        if(isKeyRelease){
            input.ki.dwFlags = tmpDwFlags | KEYEVENTF_KEYUP;
        }

        SendInput(1, &input, sizeof(INPUT));
    }else{
        // 模拟鼠标移动操作
        // 构造鼠标事件
        INPUT input = {0};
        input.type = INPUT_MOUSE;
        input.mi.dwFlags = MOUSEEVENTF_MOVE;  // 相对移动
        // 鼠标左移
        if(scanCode == -1){
            input.mi.dx = -MOUSE_X_SPEED;
            input.mi.dy = 0;
        }
        // 鼠标上移
        else if(scanCode == -2){
            input.mi.dx = 0;
            input.mi.dy = -MOUSE_Y_SPEED;
        }
        // 鼠标右移
        else if(scanCode == -3){
            input.mi.dx = MOUSE_X_SPEED;
            input.mi.dy = 0;
        }
        // 鼠标下移
        else if(scanCode == -4){
            input.mi.dx = 0;
            input.mi.dy = MOUSE_Y_SPEED;
        }

        // 鼠标左键点击
        else if(scanCode == -7){
            input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        }
        // 鼠标右键点击
        else if(scanCode == -8){
            input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        }
        // 鼠标滚轮上滚
        else if(scanCode == -9){
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = MOUSE_WHEEL_DELTA;  // 正数向上滚动，负数向下滚动
            input.mi.time = 0;
            input.mi.dwExtraInfo = 0;
        }
        // 鼠标滚轮下滚
        else if(scanCode == -10){
            input.mi.dwFlags = MOUSEEVENTF_WHEEL;
            input.mi.mouseData = -MOUSE_WHEEL_DELTA;  // 正数向上滚动，负数向下滚动
            input.mi.time = 0;
            input.mi.dwExtraInfo = 0;
        }
        // 鼠标中键
        else if(scanCode == -11){
            input.type = INPUT_MOUSE;
            input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_MIDDLEDOWN : MOUSEEVENTF_MIDDLEUP;
        }
        else{
            // 其它无效操作不模拟
            return;
        }


        // 发送鼠标事件
        SendInput(1, &input, sizeof(INPUT));
    }

}

void SimulateTask::simulateKeyPressMs(short vkey, size_t pressMs) {
    simulateKeyPress(vkey, false);
    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
        //释放按键
        QTimer::singleShot(pressMs, [=](){
            simulateKeyPress(vkey, true);
        });
    }, Qt::QueuedConnection);
}

void SimulateTask::simulateKeyDelayPressMs(short vkey, size_t pressMs, size_t delayMs) {
    QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
        // 延时后按下按键
        QTimer::singleShot(delayMs, [=](){
            simulateKeyPress(vkey, false);
            //释放按键
            QTimer::singleShot(pressMs, [=](){
                simulateKeyPress(vkey, true);
            });
        });
    }, Qt::QueuedConnection);
}

void SimulateTask::changeEnableOnlyLongestMapping(){
    // 如果启用最长组合键优先模式, 就使用 已排序的, 否则使用未排序的
    AssistFuncWindow::getEnableOnlyLongestMapping()
        ? handleMultiBtnVector = handleMultiBtnVectorSorted
        : handleMultiBtnVector = handleMultiBtnVectorUnsort;

}
