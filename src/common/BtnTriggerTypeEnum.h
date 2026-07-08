#pragma once

#include<string>
#include<map>

// 按键触发模式枚举
enum TriggerTypeEnum {
    Normal = 0,// 正常同步模式
    Delay1s = 1,// 延迟1秒触发
    Delay3s = 2,// 延迟3秒触发
    Delay5s = 3,// 延迟5秒触发

    Release = 4,// 按键松开时触发
    PressAndRelease = 5,// 按下触发并且松开时再次触发

    Delay8s = 6,// 延迟8秒触发
    Delay10s = 7,// 延迟10秒触发
    Delay15s = 8,// 延迟15秒触发

    KeepPress = 9,// 保持按住, 再次按下才释放

    End //结束标识, 用于遍历枚举
};

// 枚举对应信息map
inline std::map<TriggerTypeEnum, std::string> TRIGGER_TYPE_ENUM_MAP = {
    {Normal, "(默认)同步模式"},
    {Delay1s, "延迟1秒触发"},
    {Delay3s, "延迟3秒触发"},
    {Delay5s, "延迟5秒触发"},
    {Delay8s, "延迟8秒触发"},
    {Delay10s, "延迟10秒触发"},
    {Delay15s, "延迟15秒触发"},
    {Release, "按键松开时触发"},
    {PressAndRelease, "按下触发且松开也触发"},
    {KeepPress, "保持按住(再次按下松开)"},
};
