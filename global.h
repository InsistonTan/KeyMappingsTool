#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <QList>
#include <dinput.h>
#include <mapping_relation.h>
#include <map>
#include<QQueue>
#include<QWidget>

#define WHEEL_BUTTON "wheel_button"
#define WHEEL_AXIS "wheel_axis"

#define DEFAULT_INNER_DEADAREA_VALUE 0.03

#define DINPUT_MAX_BUTTONS 128 // DirectInput 最大按键数

#define POV_ONLY_FACTOR ((BUTTONS_VALUE_TYPE)((uint64_t)-1) << DINPUT_MAX_BUTTONS)
#define KEY_ONLY_FACTOR ((((BUTTONS_VALUE_TYPE)((uint64_t)-1)) << 64) | ((BUTTONS_VALUE_TYPE)((uint64_t)-1)))

extern QWidget* g_mainWindow;

// 全局映射是否开启
extern bool isRuning;
extern std::map<QString, BUTTONS_VALUE_TYPE> g_btnBitValueMap; // 全局按键值

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

extern double xboxJoystickInnerDeadAreaValue;
extern double xboxTriggerInnerDeadAreaValue;
double getXboxJoystickInnerDeadAreaValue();
void setXboxJoystickInnerDeadAreaValue(double value);
double getXboxTriggerInnerDeadAreaValue();
void setXboxTriggerInnerDeadAreaValue(double value);

QString getAppDataDirStr();

// 映射列表含有映射xbox的记录
bool hasXboxMappingInMappingList(std::vector<MappingRelation*> mappingList);

// 是否开启设备名称强唯一模式, 开启该模式, 设备名称将附带设备路径信息
extern bool enableStrongUniqueDeviceNameMode;

// BUTTONS_VALUE_TYPE 转换为字符串
std::string ButtonsValueTypeToString(BUTTONS_VALUE_TYPE btnValue);

// string 转换为 BUTTONS_VALUE_TYPE
BUTTONS_VALUE_TYPE stringToButtonsValueType(const std::string& btnValueStr);

#endif // GLOBAL_H
