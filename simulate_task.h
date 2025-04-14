#ifndef SIMULATE_TASK_H
#define SIMULATE_TASK_H

#include<mapping_relation.h>
#include<QThread>
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include<windows.h>
#include <ViGEm/Client.h>
#include<key_map.h>

#define MAX_STR 255
#define MAX_BUF 2048
#define MOUSE_X_SPEED 6
#define MOUSE_Y_SPEED 6

// xbox模拟轴移动的速度
#define XBOX_AXIS_SPEED 30000
#define XBOX_TRIGGER_SPEED 250

// 方向盘轴模拟键盘按键时, 轴的内部死区
#define AXIS_VALID_PERCENT 0.03

// 模拟释放按键的延迟时间ms
#define RELEASE_DELAY_MS 100

class SimulateTask : public QObject {
    Q_OBJECT
private:
    //hid_device *handle;// 当前设备的连接句柄
    std::vector<MappingRelation*> *mappingList;// 已配置的按键映射列表
    std::map<std::string, short> handleMap;// 设备按键对应键盘扫描码map
    std::vector<MappingRelation> handleMultiBtnVector;// 设备多个按键对应键盘扫描码map

    std::map<std::string, short> keyHoldingMap;// 记录按键一直按着的map

    std::map<std::string, TriggerTypeEnum> keyTriggerTypeMap;// 记录按键触发模式的map

    //std::map<int, short> keyPosMap;// 记录按键位置对应键盘扫描码map

    std::vector<std::string> rotateAxisList;// 记录需要反转的轴

    bool isMouseLeftHolding = false;// 鼠标左键一直按着
    bool isMouseRightHolding = false;// 鼠标右键一直按着

    PVIGEM_CLIENT vigemClient = nullptr;// ViGEm客户端
    PVIGEM_TARGET vigemTarget = nullptr;// 虚拟手柄对象
    XUSB_REPORT report;// 手柄模拟报告


    //unsigned char buf[MAX_BUF];
    //wchar_t wstr[MAX_STR];
    //int res;

public:
    SimulateTask(std::vector<MappingRelation*> *mappingList);

public slots:
    // 工作任务
    void doWork();
signals:
    // 任务结束信号
    void workFinished();

    void msgboxSignal(bool isError, QString msg);
    void startedSignal();
    void pauseClickSignal();

protected:

    QList<MappingRelation*> handleResult(QList<MappingRelation*> res);

    bool isAxisRotate(std::string btnName);

    bool isMappingValid(MappingRelation* mapping){
        return mapping != nullptr && !mapping->dev_btn_name.empty() && mapping->keyboard_value != 0;
    }

    void addMappingToHandleMap(MappingRelation* mapping);

    QList<std::string> getBtnStrListFromHandleMap(std::string btnStr);

    bool isCurrentKeyHolding(std::string ketStr);

    // 模拟按键操作
    void simulateKeyPress(short vkey, bool isKeyRelease);

    // 释放指定位置的所有按键
    void releaseAllKey(QList<MappingRelation*> pressBtnList);

    // 关闭HID设备
    void closeDevice();

    // 初始化xbox控制器
    bool initXboxController();
    // 关闭xbox控制器
    void closeXboxController();
    // 模拟xbox按键操作
    void simulateXboxKeyPress(XboxInputType inputType, int inputValue1, int inputValue2, bool isRelease);
};

#endif // SIMULATE_TASK_H
