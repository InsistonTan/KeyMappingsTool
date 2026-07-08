#include "AssistFuncWorker.h"
#include<QThread>
#include<QCoreApplication>
#include <tchar.h>
#include <cctype>
#include<QTimer>
#include"global.h"

#define MAPPING_FILE_NAME SCS_PLUGIN_MMF_NAME
#define SHARED_MEMORY_SIZE (3 * 1024)

AssistFuncWorker::AssistFuncWorker() {}

void AssistFuncWorker::cancelWorkSlot(){
    this->isWorkerRunning = false;
}

scsTelemetryMap_t* AssistFuncWorker::readETS2Data(){
    // 共享内存句柄
    HANDLE hMapFile = nullptr;

    bool isWarningLogShow = false;
    while(isWorkerRunning){
        // 打开共享内存
        hMapFile = OpenFileMapping(
            FILE_MAP_READ,        // 只读访问
            FALSE,                // 不继承句柄
            _T(MAPPING_FILE_NAME) // 共享内存名称
            );

        if (hMapFile == nullptr) {
            if(!isWarningLogShow){
                pushToQueue(parseWarningLog("无法打开共享内存:" + QString(MAPPING_FILE_NAME) + ", 欧卡2可能未运行, 将等待欧卡2运行..."));
                isWarningLogShow = true;
            }
            QThread::msleep(1000);

            // 处理事件队列
            QCoreApplication::processEvents();

            continue;
        }else{
            break;
        }
    }

    if(hMapFile == nullptr){
        return nullptr;
    }

    pushToQueue(parseSuccessLog("打开共享内存:" + QString(MAPPING_FILE_NAME) + "成功!"));

    // 映射到进程地址空间
    byte* bytes = (byte*)MapViewOfFile(
        hMapFile,           // 共享内存句柄
        FILE_MAP_READ,      // 只读访问
        0, 0,               // 偏移量
        SHARED_MEMORY_SIZE // 映射大小
        );


    if (bytes == nullptr) {
        pushToQueue(parseErrorLog("从共享内存获取数据失败!"));
        CloseHandle(hMapFile);
        return nullptr;
    }

    pushToQueue(parseSuccessLog("从共享内存获取数据成功!"));

    return (scsTelemetryMap_t*)bytes;
}

// 模拟按键操作
void simulateKeyPress(short scanCode, bool isKeyRelease) {
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
    }
    // else{
    //     // 模拟鼠标移动操作
    //     // 构造鼠标事件
    //     INPUT input = {0};
    //     input.type = INPUT_MOUSE;
    //     input.mi.dwFlags = MOUSEEVENTF_MOVE;  // 相对移动
    //     // 鼠标左移
    //     if(scanCode == -1){
    //         input.mi.dx = -MOUSE_X_SPEED;
    //         input.mi.dy = 0;
    //     }
    //     // 鼠标上移
    //     else if(scanCode == -2){
    //         input.mi.dx = 0;
    //         input.mi.dy = -MOUSE_Y_SPEED;
    //     }
    //     // 鼠标右移
    //     else if(scanCode == -3){
    //         input.mi.dx = MOUSE_X_SPEED;
    //         input.mi.dy = 0;
    //     }
    //     // 鼠标下移
    //     else if(scanCode == -4){
    //         input.mi.dx = 0;
    //         input.mi.dy = MOUSE_Y_SPEED;
    //     }
    //     // 鼠标左键长按
    //     // else if(scanCode == -5){
    //     //     input.mi.dwFlags = isMouseLeftHolding ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    //     // }
    //     // // 鼠标右键长按
    //     // else if(scanCode == -6){
    //     //     input.mi.dwFlags = isMouseRightHolding ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    //     // }

    //     // 鼠标左键点击
    //     else if(scanCode == -7){
    //         input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_LEFTDOWN : MOUSEEVENTF_LEFTUP;
    //     }
    //     // 鼠标右键点击
    //     else if(scanCode == -8){
    //         input.mi.dwFlags = !isKeyRelease ? MOUSEEVENTF_RIGHTDOWN : MOUSEEVENTF_RIGHTUP;
    //     }else{
    //         // 其它无效操作不模拟
    //         return;
    //     }


    //     // 发送鼠标事件
    //     SendInput(1, &input, sizeof(INPUT));
    // }

}

void AssistFuncWorker::doWork(){
    // 从共享内存读取遥测数据
    scsTelemetryMap_t* bytes = readETS2Data();

    if(bytes == nullptr){
        //pushToQueue(parseErrorLog("欧卡2辅助功能线程即将结束!"));
        isWorkerRunning = false;
    }

    bool handbrakeResult = false; // 手刹值(0-1)
    float acceleratorResult;      // 油门值(0-1)

    while(isWorkerRunning){

        handbrakeResult = bytes->truck_b.parkBrake; // 手刹值(0-1)
        acceleratorResult = bytes->truck_f.gameThrottle; // 油门值(0-1)
        // 手刹为启用状态, 并且油门踩下大于50%, 模拟键盘的空格键解除手刹
        // qDebug("手刹:%d, 油门:%.4f", handbrakeResult, acceleratorResult);
        // qDebug("lightsParking:%d, lightsBeamLow:%d, lightsBeamHigh:%d, lightsBeacon:%d, lightsBrake:%d, lightsReverse:%d, lightsHazard:%d", bytes->truck_b.lightsParking, bytes->truck_b.lightsBeamLow, bytes->truck_b.lightsBeamHigh, bytes->truck_b.lightsBeacon, bytes->truck_b.lightsBrake, bytes->truck_b.lightsReverse, bytes->truck_b.lightsHazard);
        if(handbrakeResult == 1 && acceleratorResult > 0.5f){
            pushToQueue("当前手刹为启用状态, 且油门大于50%, 正在模拟空格键解除手刹...");

            // 模拟空格键按下
            simulateKeyPress(0x39, false);
            QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
                //释放按键
                QTimer::singleShot(100, [=](){
                    simulateKeyPress(0x39, true);
                });
            }, Qt::QueuedConnection);
        }

        // sleep
        QThread::msleep(100);
        // 处理事件队列
        QCoreApplication::processEvents();
    }


    //qDebug("finished");
    //pushToQueue("欧卡2辅助功能线程结束");

    emit workFinished();
}
