#ifndef GLOBAL_H
#define GLOBAL_H

#include <QList>
#include <dinput.h>
#include <mapping_relation.h>
#include <map>
#include<QQueue>
#include<QWidget>

#define WHEEL_BUTTON "wheel_button"
#define WHEEL_AXIS "wheel_axis"

extern QWidget* g_mainWindow;

// 全局映射是否开启
extern bool isRuning;

// 当前默认的映射类型
extern MappingType defaultMappingType;

// 是否暂停全局映射
extern bool isPause;
void clickPauseBtn();
bool getIsPause();

extern LPDIRECTINPUT8 g_pDirectInput;
//extern LPDIRECTINPUTDEVICE8 g_pDevice;
extern QList<LPDIRECTINPUTDEVICE8> initedDeviceList; // 已初始化的设备列表
void clearInitedDeviceList();// 清空已初始化的设备列表
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
QString parseSuccessLog(QString srcText);
QString parseWarningLog(QString srcText);
QString parseErrorLog(QString srcText);

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
// 初始化DirectInput
bool initDirectInput();
// 打开设备
bool openDiDevice(QList<QString> deviceList);
// 获取设备状态数据
QList<MappingRelation*> getInputState(bool enableLog = false, std::vector<MappingRelation> multiBtnVector = {});
void cleanupDirectInput();
// 获取轴的数值范围
void getDipropRange(LPDIRECTINPUTDEVICE8 g_pDevice, long axisCode, std::string axisName);
// 扫描设备
void scanDevice();

// 检测是否支持力反馈
bool checkIsSupportForceFeedback(QString deviceName);

void setIsRuning(bool val);

bool getIsRunning();

void setDefaultMappingType(MappingType val);

MappingType getDefaultMappingType();

extern double xboxJoystickInnerDeadAreaValue;
extern double xboxTriggerInnerDeadAreaValue;
double getXboxJoystickInnerDeadAreaValue();
void setXboxJoystickInnerDeadAreaValue(double value);
double getXboxTriggerInnerDeadAreaValue();
void setXboxTriggerInnerDeadAreaValue(double value);

QString getAppDataDirStr();

#endif // GLOBAL_H
