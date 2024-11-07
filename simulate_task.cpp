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

        for(int k=0; k<res; k++){
            auto item = handleMap.find(to_string(k)+"#"+to_string((int)buf[k]));

            // 该按键存在映射, 模拟映射的键盘按键操作
            if (item != handleMap.end()) {
                qDebug("按键存在映射, 正在模拟对应操作");
                simulateKeyPress(item->second);
            }
        }


        Sleep(150);
    }

    // 关闭设备
    //closeDevice();

    // 提交工作结束信号
    emit workFinished();
}

// 模拟按键操作
void SimulateTask::simulateKeyPress(short vkey) {
    // 模拟按下按键
    INPUT input[2] = {};

    // 按下按键
    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = vkey;  // Virtual key code for 'A'
    input[0].ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input[0].ki.dwFlags = 0;  // KEYEVENTF_KEYDOWN

    // 松开按键
    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = vkey;
    input[1].ki.wScan = MapVirtualKey(vkey, MAPVK_VK_TO_VSC);
    input[1].ki.dwFlags = KEYEVENTF_KEYUP;  // KEYEVENTF_KEYUP

    // 发送事件
    SendInput(2, input, sizeof(INPUT));
}
