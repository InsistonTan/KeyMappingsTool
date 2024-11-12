#ifndef GLOBAL_H
#define GLOBAL_H

#include <QList>
#include <dinput.h>
#include <mapping_relation.h>
#include <map>

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


BOOL CALLBACK EnumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext);
bool initDirectInput();
bool openDiDevice(int deviceIndex);
QList<MappingRelation*> getInputState();
void cleanupDirectInput();
void getDipropRange(long axisCode, std::string axisName);

void setIsRuning(bool val);

bool getIsRunning();

void setIsXboxMode(bool val);

bool getIsXboxMode();

#endif // GLOBAL_H
