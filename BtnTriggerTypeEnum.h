#ifndef BTNTRIGGERTYPEENUM_H
#define BTNTRIGGERTYPEENUM_H

#include <map>
#include <string>

// 按键触发模式枚举
enum TriggerTypeEnum {
    Normal = 0,          // 正常同步模式
    Delay1s = 1,         // 延迟1秒触发
    Delay3s = 2,         // 延迟3秒触发
    Delay5s = 3,         // 延迟5秒触发
    Release = 4,         // 按键松开时触发
    PressAndRelease = 5, // 按下触发并且松开时再次触发

    ETS2_SyncBlinkerLeft,    // ETS2左转向灯同步
    ETS2_SyncBlinkerRight,   // ETS2右转向灯同步
    ETS2_SyncLightsParking,  // ETS2示廓灯同步
    ETS2_SyncLightsBeamLow,  // ETS2近光灯同步
    ETS2_SyncLightsBeamHigh, // ETS2远光灯同步
    // ETS2_SyncWiper,          // ETS2雨刷器同步

    End //结束标识, 用于遍历枚举
};

#define TRIGGER_TYPE_ENUM_ETS2_SYNC_START TriggerTypeEnum::ETS2_SyncBlinkerLeft    // ETS2同步开始标识
#define TRIGGER_TYPE_ENUM_ETS2_SYNC_END   TriggerTypeEnum::ETS2_SyncLightsBeamHigh // ETS2同步结束标识

// 枚举对应信息map
extern std::map<TriggerTypeEnum, std::string> TRIGGER_TYPE_ENUM_MAP;

#endif // BTNTRIGGERTYPEENUM_H
