#pragma once


#include <dinput.h>
#include <string>
#include <vector>


///
/// \brief The DiDeviceInfo class
/// DirectInput设备信息结构体
///
struct DiDeviceInfo {
    std::string name; // 名称, 默认为空, 需要根据情况设置
    std::string productName;// 设备产品名称, 如: "MOZA R3 Wheel xxx"
    std::string guidString; // // GUID字符串, 固定为38个字符 + 终止符, 样例: "(0B13045E-0000-0000-0000-504944564944)"
    std::string devInterfacePath;// 设备接口路径, 样例: "\\?\hid#{00001812-0000-1000-8000-00805f9b34fb}&dev&vid_045e&pid_0b13&rev_0509&0c3526e7bcae&ig_00#c&725d0d6&0&0000#{4d1e55b2-f16f-11cf-88cb-001111000030}"
    GUID guidInstance; // 设备guid实例
};

///
/// \brief The DiActionResultEnum enum
/// 操作结果枚举
///
enum class DiActionResultEnum{
    Success,
    Failed,
    Failed_CreateDevice,
    Failed_SetDataFormat,
    Failed_SetCooperativeLevel,
    Failed_GetCapabilities,
};


///
/// \brief The DirectInputCore class
///  DirectInput核心类
/// 主要负责与DirectInput交互
///
class DirectInputCore
{

public:
    DirectInputCore();

    // 初始化DireactInput实例
    static DiActionResultEnum initDirectInputInstance(LPDIRECTINPUT8* pDirectInput);

    // 枚举设备列表
    // pDirectInput为DirectInput实例
    // lpCallback为回调函数
    static DiActionResultEnum enumDeviceList(
        LPDIRECTINPUT8 pDirectInput,
        LPDIENUMDEVICESCALLBACKW lpCallback);

    // 获取设备信息列表
    static DiActionResultEnum getDeviceList(
        LPDIRECTINPUT8 pDirectInput,
        std::vector<DiDeviceInfo>* pDeviceInfoList);

    // 初始化设备
    // HWND hWnd 绑定的窗口句柄
    // cooperativeLevel默认为非独占模式
    static DiActionResultEnum initDevice(
        HWND hWnd,
        LPDIRECTINPUT8 pDirectInput,
        DiDeviceInfo deviceInfo,
        LPDIRECTINPUTDEVICE8* pDevice,
        DWORD cooperativeLevel = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    // 枚举设备的回调函数
    static BOOL EnumDevicesCallback(const DIDEVICEINSTANCE *pdidInstance, void *pContext);

    // 获取设备接口路径
    static std::string getDevInterfacePath(
        LPDIRECTINPUT8 pDirectInput,
        const DIDEVICEINSTANCE* pdidInstance);

    // guid实例获取guid字符串
    static std::string guidToString(const GUID &guid);

    // 获取设备状态数据
    static DiActionResultEnum getDeviceStateData(LPDIRECTINPUTDEVICE8 pDevice,  DIJOYSTATE2* pJoyStateResult);

    // 获取设备信息
    static DiActionResultEnum getDeviceInfo(LPDIRECTINPUT8 pDirectInput, LPDIRECTINPUTDEVICE8 pDevice, DiDeviceInfo* deviceInfoResult,const DIDEVICEINSTANCE* pdidInstance);

private:
    // 临时的directinput实例, 用于枚举设备
    inline static LPDIRECTINPUT8 tempPDirectInput;
    // 临时的设备列表指针, 用于保存设备
    inline static std::vector<DiDeviceInfo>* tempPDeviceInfoList;

};
