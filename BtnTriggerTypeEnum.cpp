#include "BtnTriggerTypeEnum.h"

std::map<TriggerTypeEnum, std::string> TRIGGER_TYPE_ENUM_MAP = {
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
