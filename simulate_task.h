#ifndef SIMULATE_TASK_H
#define SIMULATE_TASK_H

#include<mapping_relation.h>
#include<QThread>
#include <hidapi.h>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include<windows.h>

using namespace std;

#define MAX_STR 255
#define MAX_BUF 2048
#define MOUSE_X_SPEED 6
#define MOUSE_Y_SPEED 6

class SimulateTask : public QObject {
    Q_OBJECT
private:
    hid_device *handle;// 当前设备的连接句柄
    vector<MappingRelation*> *mappingList;// 已配置的按键映射列表
    map<string, short> handleMap;// 设备按键对应键盘扫描码map

    map<string, short> keyHoldingMap;// 记录按键一直按着的map
    map<int, short> keyPosMap;// 记录按键位置对应键盘扫描码map

    bool isMouseLeftHolding = false;// 鼠标左键一直按着
    bool isMouseRightHolding = false;// 鼠标右键一直按着

    unsigned char buf[MAX_BUF];
    wchar_t wstr[MAX_STR];
    int res;

public:
    SimulateTask(hid_device *handle, vector<MappingRelation*> *mappingList);

public slots:
    // 工作任务
    void doWork();
signals:
    // 任务结束信号
    void workFinished();


protected:
    bool isMappingValid(MappingRelation* mapping){
        return mapping != nullptr && mapping->dev_btn_pos > 0 && mapping->dev_btn_value > 0;
    }

    bool isCurrentKeyHolding(string ketStr);

    // 模拟按键操作
    void simulateKeyPress(short vkey, bool isKeyRelease);

    // 释放指定位置的所有按键
    void releasePosAllKey(int keyPos);

    void closeDevice();
};

#endif // SIMULATE_TASK_H
