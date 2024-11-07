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

class SimulateTask : public QObject {
    Q_OBJECT
private:
    hid_device *handle;// 当前设备的连接句柄
    vector<MappingRelation*> *mappingList;// 已配置的按键映射列表
    map<string, short> handleMap;

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
        return mapping != nullptr && mapping->dev_btn_pos > 0 && mapping->dev_btn_value > 0 && mapping->keyboard_value > 0;
    }

    // 模拟按键操作
    void simulateKeyPress(short vkey);

    void closeDevice();
};

#endif // SIMULATE_TASK_H
