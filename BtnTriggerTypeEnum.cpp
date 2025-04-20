#include "BtnTriggerTypeEnum.h"

std::map<TriggerTypeEnum, std::string> TRIGGER_TYPE_ENUM_MAP = {
    {Normal, "(默认)同步模式"},
    {Delay1s, "延迟1秒触发"},
    {Delay3s, "延迟3秒触发"},
    {Delay5s, "延迟5秒触发"},
    {Release, "按键松开时触发"},
    {PressAndRelease, "按下触发且松开也触发"},
    {ETS2_SyncBlinkerLeft, "ETS2左转向灯同步"},
    {ETS2_SyncBlinkerRight, "ETS2右转向灯同步"},
    {ETS2_SyncLightsParking, "ETS2示廓灯同步"},
    {ETS2_SyncLightsBeamLow, "ETS2近光灯同步"},
    {ETS2_SyncLightsBeamHigh, "ETS2远光灯同步"},
    // {ETS2_SyncWiper, "ETS2雨刷器同步"},
};
