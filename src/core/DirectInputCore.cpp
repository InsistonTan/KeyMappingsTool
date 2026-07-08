#include "DirectInputCore.h"

#include <string>


DirectInputCore::DirectInputCore() {}

std::string wstringToString(const wchar_t* wstr)
{
    if (!wstr) return {};

    int size_needed = WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        nullptr,
        0,
        nullptr,
        nullptr
        );

    std::string str(size_needed - 1, 0);

    WideCharToMultiByte(
        CP_UTF8,
        0,
        wstr,
        -1,
        &str[0],
        size_needed,
        nullptr,
        nullptr
        );

    return str;
}

DiActionResultEnum DirectInputCore::initDirectInputInstance(LPDIRECTINPUT8* pDirectInput)
{
    HINSTANCE hInstance = GetModuleHandle(NULL);
    if (FAILED(DirectInput8Create(
            hInstance,
            DIRECTINPUT_VERSION,
            IID_IDirectInput8,
            (void**)pDirectInput, NULL))) {

        //qDebug() << "DirectInput 初始化失败！";
        return DiActionResultEnum::Failed;
    }

    return DiActionResultEnum::Success;
}

DiActionResultEnum DirectInputCore::enumDeviceList(
    LPDIRECTINPUT8 pDirectInput,
    LPDIENUMDEVICESCALLBACKW lpCallback)
{
    if(pDirectInput == nullptr)
        return DiActionResultEnum::Failed;

    return FAILED(pDirectInput->EnumDevices(
               DI8DEVCLASS_GAMECTRL,
               lpCallback,
               NULL,
               DIEDFL_ATTACHEDONLY))
        ? DiActionResultEnum::Failed : DiActionResultEnum::Success;
}

DiActionResultEnum DirectInputCore::getDeviceList(
    LPDIRECTINPUT8 pDirectInput,
    std::vector<DiDeviceInfo>* pDeviceInfoList)
{
    if(pDirectInput == nullptr || pDeviceInfoList == nullptr)
        return DiActionResultEnum::Failed;

    tempPDirectInput = pDirectInput;
    tempPDeviceInfoList = pDeviceInfoList;

    // 枚举设备, 并将设备信息添加进 pDeviceInfoList指针指向的列表
    return enumDeviceList(pDirectInput, EnumDevicesCallback);
}

DiActionResultEnum DirectInputCore::initDevice(
    HWND hWnd,
    LPDIRECTINPUT8 pDirectInput,
    DiDeviceInfo deviceInfo,
    LPDIRECTINPUTDEVICE8* pDevice,
    DWORD cooperativeLevel)
{
    if(pDirectInput == nullptr || deviceInfo.name.empty())
        return DiActionResultEnum::Failed;

    // 创建设备实例
    if (FAILED(pDirectInput->CreateDevice(deviceInfo.guidInstance, pDevice, NULL))) {
        return DiActionResultEnum::Failed_CreateDevice;
    }

    // 设置数据格式
    if (FAILED((*pDevice)->SetDataFormat(&c_dfDIJoystick2))) {
        return DiActionResultEnum::Failed_SetDataFormat;
    }

    // 设置协作模式
    if (FAILED((*pDevice)->SetCooperativeLevel(hWnd, cooperativeLevel))) {
        return DiActionResultEnum::Failed_SetCooperativeLevel;
    }

    // 获取控制器能力
    DIDEVCAPS capabilities;
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED((*pDevice)->GetCapabilities(&capabilities))) {
        return DiActionResultEnum::Failed_GetCapabilities;
    }

    return DiActionResultEnum::Success;
}


BOOL DirectInputCore::EnumDevicesCallback(
    const DIDEVICEINSTANCE *pdidInstance,
    [[maybe_unused]]void *pContext)
{
    // 设备信息
    DiDeviceInfo deviceInfo;

    // 获取设备信息 到 deviceInfo
    auto actionResult = getDeviceInfo(tempPDirectInput, nullptr, &deviceInfo, pdidInstance);

    // 添加设备信息到临时列表
    if(actionResult == DiActionResultEnum::Success)
        tempPDeviceInfoList->push_back(deviceInfo);

    return DIENUM_CONTINUE;
}

std::string DirectInputCore::getDevInterfacePath(LPDIRECTINPUT8 pDirectInput, const DIDEVICEINSTANCE *pdidInstance)
{
    std::string result = "";
    LPDIRECTINPUTDEVICE8 pDevice = nullptr;
    // 创建设备实例
    if (FAILED(pDirectInput->CreateDevice(pdidInstance->guidInstance, &pDevice, NULL))) {
        //qDebug() << "获取设备接口路径失败: 设备创建失败！";
        return result;
    }

    // 设备属性结构体
    DIPROPGUIDANDPATH diprop;
    memset(&diprop, 0, sizeof(diprop));  // 清零结构体
    // 设置 DIPROPHEADER
    diprop.diph.dwSize = sizeof(DIPROPGUIDANDPATH);
    diprop.diph.dwHeaderSize = sizeof(DIPROPHEADER);
    diprop.diph.dwObj = 0;               // 0 表示设备本身，而不是特定对象
    diprop.diph.dwHow = DIPH_DEVICE;     // 表示获取设备属性

    if (SUCCEEDED(pDevice->GetProperty(DIPROP_GUIDANDPATH, &diprop.diph))) {
        // diprop.wszPath 包含设备路径（如 \\?\HID#VID_1234&PID_5678...）
        result = wstringToString(diprop.wszPath);

        // 不进行正则匹配, 保留完整的设备接口路径
        // static const QRegularExpression reg1("(^.+(?=vid))|(#{.*$)");
        // wszPath.replace(reg1, "");
    }

    if(pDevice){
        pDevice->Release();
        pDevice = nullptr;
    }

    return result;
}

std::string DirectInputCore::guidToString(const GUID &guid)
{
    char buffer[39];

    // GUID字符串固定为38个字符 + 终止符
    snprintf(buffer, sizeof(buffer),
             "(%08lX-%04hX-%04hX-%02hhX%02hhX-%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX)",
             guid.Data1, guid.Data2, guid.Data3,
             guid.Data4[0], guid.Data4[1], guid.Data4[2], guid.Data4[3],
             guid.Data4[4], guid.Data4[5], guid.Data4[6], guid.Data4[7]);

    //snprintf(buffer, sizeof(buffer), "(%08lX)", guid.Data1);

    return std::string(buffer);
}

DiActionResultEnum DirectInputCore::getDeviceStateData(LPDIRECTINPUTDEVICE8 pDevice, DIJOYSTATE2 *pJoyStateResult)
{
    // 刷新一次设备数据
    auto hr = pDevice->Poll();
    if(FAILED(hr)){
        // 激活设备
        pDevice->Acquire();
        // 重新刷新一次数据
        hr = pDevice->Poll();
    }
    // 两次poll都失败
    if(FAILED(hr)){
        return DiActionResultEnum::Failed;
    }


    // 获取设备状态数据成功
    if (SUCCEEDED(pDevice->GetDeviceState(sizeof(DIJOYSTATE2), pJoyStateResult))){
        return DiActionResultEnum::Success;
    }

    return DiActionResultEnum::Failed;
}

DiActionResultEnum DirectInputCore::getDeviceInfo(LPDIRECTINPUT8 pDirectInput, LPDIRECTINPUTDEVICE8 pDevice, DiDeviceInfo *deviceInfoResult, const DIDEVICEINSTANCE* pdidInstance)
{
    // di设备信息实例
    DIDEVICEINSTANCE didInstance;

    // 如果 pdidInstance 指针不为空, 使用该指针指向的 di设备实例
    if(pdidInstance != nullptr){
        didInstance = *pdidInstance;
    }else if(pDevice != nullptr){
        // 获取设备信息
        didInstance.dwSize = sizeof(DIDEVICEINSTANCE);
        HRESULT hr2 = pDevice->GetDeviceInfo(&didInstance);
        // 获取设备信息 失败
        if(FAILED(hr2)){
            return DiActionResultEnum::Failed;
        }
    }

    // 设备的产品名称
    deviceInfoResult->productName = wstringToString(didInstance.tszProductName);
    // 设备接口路径
    deviceInfoResult->devInterfacePath = DirectInputCore::getDevInterfacePath(pDirectInput, &didInstance);
    // 设备guid字符串
    deviceInfoResult->guidString = DirectInputCore::guidToString(didInstance.guidProduct);
    // guid实例
    deviceInfoResult->guidInstance = didInstance.guidInstance;

    return DiActionResultEnum::Success;
}


