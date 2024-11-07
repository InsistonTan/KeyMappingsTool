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
void SimulateTask::simulateKeyPress(short scanCode) {
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
}
