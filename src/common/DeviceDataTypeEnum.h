#pragma once

#include<QString>

#define WHEEL_BUTTON_STR "wheel_button"
#define WHEEL_AXIS_STR "wheel_axis"

enum class DeviceDataTypeEnum{
    // 空值占位
    NONE = 0,

    // 方向盘的按键数据
    WHEEL_BUTTON = 1,

    // 方向盘的轴数据
    WHEEL_AXIS = 2,

    // 枚举的个数
    COUNT,
};

///
/// \brief 根据字符串值 获取 对应的枚举 (为了适配旧版本的配置文件)
/// \param strVal
/// \return 设备数据类型枚举
///
inline static DeviceDataTypeEnum getDeviceDataTypeEnum(QString strVal){
    if(strVal.isEmpty())
        return DeviceDataTypeEnum::NONE;

    if(strVal == WHEEL_AXIS_STR)
        return DeviceDataTypeEnum::WHEEL_AXIS;

    if(strVal == WHEEL_BUTTON_STR)
        return DeviceDataTypeEnum::WHEEL_BUTTON;

    return DeviceDataTypeEnum::NONE;
}
