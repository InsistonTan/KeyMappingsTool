#ifndef GLOBAL_H
#define GLOBAL_H

#include <QList>
#include <dinput.h>
#include <mapping_relation.h>
#include <map>
#include<QQueue>

#define WHEEL_BUTTON "wheel_button"
#define WHEEL_AXIS "wheel_axis"

// 全局映射是否开启
extern bool isRuning;

// 当前运行模式, true为模拟xbox手柄, false为模拟键盘
extern bool isXboxMode;

extern LPDIRECTINPUT8 g_pDirectInput;
extern LPDIRECTINPUTDEVICE8 g_pDevice;
struct DiDeviceInfo {
    std::string name;
    GUID guidInstance;
};
// 设备列表
extern QList<DiDeviceInfo> diDeviceList;

// 轴名称对应数值范围map
extern std::map<std::string, DIPROPRANGE> axisValueRangeMap;

// 全局变量, 日志队列
extern QQueue<QString> logQueue;
void pushToQueue(QString data);
QString popQueue();
int getQueueSize();

// 开启按键日志
extern bool enableBtnLog;
// 开启轴日志
extern bool enableAxisLog;
// 摇杆日志(十字键)
extern bool enablePovLog;
void setEnableBtnLog(bool val);
bool getEnableBtnLog();
void setEnableAxisLog(bool val);
bool getEnableAxisLog();
void setEnablePovLog(bool val);
bool getEnablePovLog();

// 内部死区比例
// 盘面轴死区
extern double innerDeadAreaPanti;
// 踏板轴死区
extern double innerDeadAreaTaban;
void setInnerDeadAreaPanti(double val);
void setInnerDeadAreaTaban(double val);
double getInnerDeadAreaPanti();
double getInnerDeadAreaTaban();


BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
bool initDirectInput();
bool openDiDevice(int deviceIndex);
QList<MappingRelation*> getInputState(bool enableLog);
void cleanupDirectInput();
void getDipropRange(long axisCode, std::string axisName);
// 扫描设备
void scanDevice();

void setIsRuning(bool val);

bool getIsRunning();

void setIsXboxMode(bool val);

bool getIsXboxMode();

#endif // GLOBAL_H
