#include <simulate_task.h>
#include <global.h>
#include <hidapi.h>

SimulateTask::SimulateTask(hid_device *handle, vector<MappingRelation*> *mappingList){
    this->handle = handle;
    this->mappingList = mappingList;

    for(MappingRelation* mapping : *mappingList){
        if(isMappingValid(mapping)){
            handleMap.insert_or_assign(to_string(mapping->dev_btn_pos) + "#" + to_string(mapping->dev_btn_value)
                                       , mapping->keyboard_value);
        }
    }
}

void SimulateTask::closeDevice(){
    // Close the device
    hid_close(handle);
}

void SimulateTask::doWork(){
    qDebug("任务启动!");

    //qDebug("isRunning值:%d", getIsRunning());
    while(getIsRunning()){
        //qDebug("循环继续");

        // 读一次输入报告
        res = hid_read(handle, buf, MAX_BUF);

        QString *str = new QString("report: ");

        for(int k=0; k<res; k++){
            //
            str->append(to_string((int)buf[k]) + " ");

            auto item = handleMap.find(to_string(k)+"#"+to_string((int)buf[k]));

            // 该按键存在映射, 模拟映射的键盘按键操作
            if (item != handleMap.end()) {
                //qDebug("按键存在映射, 正在模拟对应操作");

                // 长按鼠标左键, 再按一次才松开
                if(item->second == -5){
                    isMouseLeftHolding = !isMouseLeftHolding;
                }
                // 长按鼠标右键, 再按一次才松开
                else if(item->second == -6){
                    isMouseRightHolding = !isMouseRightHolding;
                }

                simulateKeyPress(item->second);
            }
        }

        qDebug(str->toStdString().data());

        Sleep(150);
    }

    // 关闭设备
    //closeDevice();

    // 提交工作结束信号
    emit workFinished();
}

// 模拟按键操作
void SimulateTask::simulateKeyPress(short scanCode) {
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
        SendInput(1, &input, sizeof(INPUT));

        Sleep(50);

        // 模拟释放键
        input.ki.dwFlags = tmpDwFlags | KEYEVENTF_KEYUP;
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
        else if(scanCode == -5){
            input.mi.dwFlags = isMouseLeftHolding ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
        }
        // 鼠标右键长按
        else if(scanCode == -6){
            input.mi.dwFlags = isMouseRightHolding ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
        }
        // 鼠标左键点击
        else if(scanCode == -7){
            input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN | MOUSEEVENTF_LEFTUP;
        }
        // 鼠标右键点击
        else if(scanCode == -8){
            input.mi.dwFlags = MOUSEEVENTF_RIGHTDOWN | MOUSEEVENTF_RIGHTUP;
        }else{
            // 其它无效操作不模拟
            return;
        }


        // 发送鼠标事件
        SendInput(1, &input, sizeof(INPUT));
    }

}
