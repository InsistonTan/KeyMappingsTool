#include <simulate_task.h>
#include <global.h>
#include <QMessageBox>

void SimulateTask::addMappingToHandleMap(MappingRelation* mapping){
    // 记录需要反转的轴
    if(mapping->rotateAxis == 1){
        rotateAxisList.push_back(mapping->dev_btn_name);
    }

    // handleMap还没有该按键, 直接添加
    if(handleMap.find(mapping->dev_btn_name) == handleMap.end()){
        handleMap.insert_or_assign(mapping->dev_btn_name, mapping->keyboard_value);
        return;
    }

    for(int i=1;;i++){
        if(handleMap.find(mapping->dev_btn_name + "#" + std::to_string(i)) == handleMap.end()){
            handleMap.insert_or_assign(mapping->dev_btn_name + "#" + std::to_string(i), mapping->keyboard_value);
            return;
        }
    }

}

bool SimulateTask::isAixsRotate(std::string btnName){
    for(auto item : rotateAxisList){
        if(item == btnName){
            return true;
        }
    }

    return false;
}

SimulateTask::SimulateTask(std::vector<MappingRelation*> *mappingList){
    this->mappingList = mappingList;

    for(MappingRelation* mapping : *mappingList){
        if(isMappingValid(mapping)){
            addMappingToHandleMap(mapping);
        }
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
        if(item->dev_btn_name == currentBtn){
            return true;
        }
    }

    return false;
}

void SimulateTask::releaseAllKey(QList<MappingRelation*> pressBtnList){
    // 使用迭代器遍历并删除符合条件的键
    for (auto it = keyHoldingMap.begin(); it != keyHoldingMap.end(); ) {

        std::string btnStr = it->first;

        // 找到 '#' 字符的位置
        size_t pos = btnStr.find('#');
        // 如果找到了 '#', 截取子串
        if (pos != std::string::npos) {
            btnStr = btnStr.substr(0, pos);
        }

        // 本次按下的按键列表为空, 或者当前按下的按键列表中不包含当前按键, 则松开当前按键
        if (pressBtnList.empty() || !isCurrentBtnInList(pressBtnList, btnStr)) {
            qDebug("按键释放");

            // 映射键盘
            if(!getIsXboxMode()){
                // 释放该位置的按键
                simulateKeyPress(it->second, true);
            }else{
                // 映射xbox
                // 释放该位置的按键
                simulateXboxKeyPress(NormalButton, it->second, 0, true);
            }


            it = keyHoldingMap.erase(it); // 删除并更新迭代器
        } else {
            ++it; // 继续下一个
        }
    }

    //keyPosMap.erase(keyPos);
}

bool SimulateTask::initXboxController(){
    // 初始化client
    vigemClient = vigem_alloc();
    if (vigemClient == nullptr) {
        qDebug("ViGEmClient allocation failed!");
        return false;
    }

    if (!VIGEM_SUCCESS(vigem_connect(vigemClient))) {
        qDebug("ViGEmClient connection failed!");
        return false;
    }

    // 虚拟出一个xbox手柄
    vigemTarget = vigem_target_x360_alloc();
    if (!VIGEM_SUCCESS(vigem_target_add(vigemClient, vigemTarget))) {
        qDebug("Adding virtual Xbox 360 controller failed!");
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

void SimulateTask::doWork(){
    qDebug("全局映射启动!");

    // 模拟xbox手柄, 需要初始化
    if(getIsXboxMode()){
        if(!initXboxController()){
            QMessageBox::critical(nullptr, "错误", "初始化虚拟xbox手柄失败, 请重试");

            // 关闭虚拟xbox设备
            closeXboxController();

            // 提交工作结束信号
            emit workFinished();
            return;
        }
    }

    while(getIsRunning()){
        // 轮询设备状态
        auto res = getInputState();

        // 根据本次设备状态, 松开本次没有被按下的按键
        releaseAllKey(res);

        // 对当前按下的按键列表循环操作
        for(int i=0; !res.empty() && i < res.size(); i++){
            // 当前设备按钮的字符串
            auto btnStrList = getBtnStrListFromHandleMap(res[i]->dev_btn_name);

            for(auto btnStr : btnStrList){
                // 先检查当前按钮是否在持续按下
                if(isCurrentKeyHolding(btnStr)){
                    // 持续按下
                    continue;
                }

                // 查找设备按键映射键盘扫描码map
                auto item = handleMap.find(btnStr);

                // 该按键存在映射, 模拟映射的键盘按键操作
                if (item != handleMap.end()) {
                    //qDebug("按键存在映射, 正在模拟对应操作");

                    // 映射键盘
                    if(!getIsXboxMode()){
                        // 对映射鼠标移动(-1到-4)之外的按键操作进行按下记录
                        if(item->second < -4 || item->second > 0){
                            // 记录按键按下
                            keyHoldingMap.insert_or_assign(btnStr, item->second);
                        }

                        qDebug("映射键盘模式-按键按下");
                        simulateKeyPress(item->second, false);
                    }else{
                        // 映射xbox
                        auto currentBtn = res[i];

                        // 映射普通xbox按键
                        if(currentBtn->dev_btn_type == (std::string)WHEEL_BUTTON){
                            qDebug("映射Xbox模式-按键按下:%s", btnStr.data());

                            // 记录按键按下
                            keyHoldingMap.insert_or_assign(btnStr, item->second);

                            // 按下xbox对应按键
                            simulateXboxKeyPress(NormalButton, item->second, 0, false);

                        }else{

                            // 映射xbox轴
                            // 计算映射到手柄轴的实际值
                            auto xboxRange = XBOX_AXIS_VALUE_RANGE_MAP.find(item->second)->second;
                            int xboxMin = xboxRange.minVal, xboxMax = xboxRange.maxVal;

                            auto currentRange = axisValueRangeMap.find(currentBtn->dev_btn_name)->second;
                            int currentMin = currentRange.lMin, currentMax = currentRange.lMax;

                            int finalValue = ((static_cast<double>(currentBtn->dev_btn_value) - currentMin)
                                                /(currentMax - currentMin) * (xboxMax - xboxMin))
                                             + xboxMin;

                            // 反转轴的值
                            if(isAixsRotate(btnStr)){
                                finalValue = ((currentMax - static_cast<double>(currentBtn->dev_btn_value))
                                                /(currentMax - currentMin) * (xboxMax - xboxMin))
                                             + xboxMin;

                                //qDebug("当前值:%d {%d, %d}, xbox范围:{%d, %d}, 转换成xbox值:%d"
                                //       , currentBtn->dev_btn_value, currentMin, currentMax, xboxMin, xboxMax, finalValue);
                            }


                            // 模拟xbox轴
                            simulateXboxKeyPress(static_cast<XboxInputType>(item->second), finalValue, 0, false);
                        }
                    }

                }
            }
        }



        //qDebug(str->toStdString().data());

        Sleep(10);
    }

    // 关闭虚拟xbox设备
    closeXboxController();

    // 提交工作结束信号
    emit workFinished();
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
        // 鼠标左键长按
        // else if(scanCode == -5){
        //     input.mi.dwFlags = isMouseLeftHolding ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        // }
        // // 鼠标右键长按
        // else if(scanCode == -6){
        //     input.mi.dwFlags = isMouseRightHolding ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        // }

        // 鼠标左键点击
        else if(scanCode == -7){
            input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        }
        // 鼠标右键点击
        else if(scanCode == -8){
            input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        }else{
            // 其它无效操作不模拟
            return;
        }


        // 发送鼠标事件
        SendInput(1, &input, sizeof(INPUT));
    }

}
