#pragma once

#include<string>
#include<map>

#define PAUSE_BTN_STR "暂停/恢复映射快捷键"
#define PAUSE_BTN_VAL -99
#define PAUSE_BTN_VAL_STR "-99"

// 手柄输入类型枚举
enum XboxInputType {
    LeftJoystick = 1,// 左摇杆(X轴)
    RightJoystick = 2,// 右摇杆(X轴)
    LeftTrigger = 3,// 左扳机
    RightTrigger = 4,// 右扳机
    NormalButton = 5,// 普通按键

    LeftJoystickUpperY = 6,// 左摇杆Y轴的上半轴
    LeftJoystickLowerY = 7,// 左摇杆Y轴的下半轴

    RightJoystickUpperY = 8,// 右摇杆Y轴的上半轴
    RightJoystickLowerY = 9,// 右摇杆Y轴的下半轴
};

struct VALUE_RANGE {
    int minVal;
    int maxVal;
};

//xbox 各个轴的数值范围 Map
inline const std::map<short, VALUE_RANGE> XBOX_AXIS_VALUE_RANGE_MAP = {
    {static_cast<int>(LeftJoystick), VALUE_RANGE{-32768, 32767}},
    {static_cast<int>(LeftJoystickUpperY), VALUE_RANGE{0, 32767}},
    {static_cast<int>(LeftJoystickLowerY), VALUE_RANGE{-32768, 0}},

    {static_cast<int>(RightJoystick), VALUE_RANGE{-32768, 32767}},
    {static_cast<int>(RightJoystickUpperY), VALUE_RANGE{0, 32767}},
    {static_cast<int>(RightJoystickLowerY), VALUE_RANGE{-32768, 0}},

    {static_cast<int>(LeftTrigger), VALUE_RANGE{0, 255}},
    {static_cast<int>(RightTrigger), VALUE_RANGE{0, 255}},

};

// xbox轴名称与值 的Map
inline const std::map<std::string, short> VK_XBOX_AXIS_MAP = {
    {"左摇杆-X轴", static_cast<int>(LeftJoystick)},
    {"左摇杆-Y轴上半轴", static_cast<int>(LeftJoystickUpperY)},
    {"左摇杆-Y轴下半轴", static_cast<int>(LeftJoystickLowerY)},

    {"右摇杆-X轴", static_cast<int>(RightJoystick)},
    {"右摇杆-Y轴上半轴", static_cast<int>(RightJoystickUpperY)},
    {"右摇杆-Y轴下半轴", static_cast<int>(RightJoystickLowerY)},

    {"手柄扳机-左扳机", static_cast<int>(LeftTrigger)},
    {"手柄扳机-右扳机", static_cast<int>(RightTrigger)},
};

// xbox按键名称与值 的map
inline const std::map<std::string, short> VK_XBOX_BTN_MAP = {
    // 自定义按键
    {PAUSE_BTN_STR, PAUSE_BTN_VAL},
    {"左摇杆-左移", -1},
    {"左摇杆-右移", -2},
    {"左摇杆-上移", -3},
    {"左摇杆-下移", -4},

    {"右摇杆-左移", -5},
    {"右摇杆-右移", -6},
    {"右摇杆-上移", -7},
    {"右摇杆-下移", -8},

    {"扳机-左扳机", -9},
    {"扳机-右扳机", -10},

    {"左摇杆-左上移", -11},
    {"左摇杆-左下移", -12},
    {"左摇杆-右上移", -13},
    {"左摇杆-右下移", -14},

    {"右摇杆-左上移", -15},
    {"右摇杆-左下移", -16},
    {"右摇杆-右上移", -17},
    {"右摇杆-右下移", -18},

    {"方向键-上键", 0x0001},
    {"方向键-下键", 0x0002},
    {"方向键-左键", 0x0004},
    {"方向键-右键", 0x0008},
    {"方向键-左上键", 0x0001 | 0x0004},
    {"方向键-左下键", 0x0002 | 0x0004},
    {"方向键-右上键", 0x0001 | 0x0008},
    {"方向键-右下键", 0x0002 | 0x0008},
    {"START键", 0x0010},
    {"BACK键", 0x0020},
    {"左摇杆按下", 0x0040},
    {"右摇杆按下", 0x0080},
    {"左肩键(LB)", 0x0100},
    {"右肩键(RB)", 0x0200},
    {"A键", 0x1000},
    {"B键", 0x2000},
    {"X键", 0x4000},
    {"Y键", 0x8000}
};

// 键盘按键名称与值 的map
inline const std::map<std::string, short> VK_MAP = {
    // 以下十个个是自定义的操作(不是windows标准)
    {PAUSE_BTN_STR, PAUSE_BTN_VAL},
    {"鼠标向左移动",-1},
    {"鼠标向上移动",-2},
    {"鼠标向右移动",-3},
    {"鼠标向下移动",-4},
    {"鼠标左键",-7},
    {"鼠标右键",-8},
    {"鼠标滚轮上滚",-9},
    {"鼠标滚轮下滚",-10},
    {"鼠标中键",-11},

    // 下面是键盘的硬件扫描码
    {"Esc",0x01},
    {"1",0x02},
    {"2",0x03},
    {"3",0x04},
    {"4",0x05},
    {"5",0x06},
    {"6",0x07},
    {"7",0x08},
    {"8",0x09},
    {"9",0x0A},
    {"0",0x0B},
    {"-",0x0C},
    {"=",0x0D},
    {"BackSpace",0x0E},
    {"Tab",0x0F},
    {"Q",0x10},
    {"W",0x11},
    {"E",0x12},
    {"R",0x13},
    {"T",0x14},
    {"Y",0x15},
    {"U",0x16},
    {"I",0x17},
    {"O",0x18},
    {"P",0x19},
    {"[",0x1A},
    {"]",0x1B},
    {"Enter",0x1C},
    {"Ctrl(Left)",0x1D},
    {"A",0x1E},
    {"S",0x1F},
    {"D",0x20},
    {"F",0x21},
    {"G",0x22},
    {"H",0x23},
    {"J",0x24},
    {"K",0x25},
    {"L",0x26},
    {";",0x27},
    {"'",0x28},
    {"`",0x29},
    {"Shift(Left)",0x2A},
    {"\\",0x2B},
    {"Z",0x2C},
    {"X",0x2D},
    {"C",0x2E},
    {"V",0x2F},
    {"B",0x30},
    {"N",0x31},
    {"M",0x32},
    {",",0x33},
    {".",0x34},
    {"/",0x35},
    {"Shift(Right)",0x36},
    {"*(Numpad)",0x37},
    {"Alt(Left)",0x38},
    {"Space",0x39},
    {"Caps Lock",0x3A},
    {"F1",0x3B},
    {"F2",0x3C},
    {"F3",0x3D},
    {"F4",0x3E},
    {"F5",0x3F},
    {"F6",0x40},
    {"F7",0x41},
    {"F8",0x42},
    {"F9",0x43},
    {"F10",0x44},
    {"Num Lock",0x45},
    {"Scroll Lock",0x46},
    {"7(Numpad)",0x47},
    {"8(Numpad)",0x48},
    {"9(Numpad)",0x49},
    {"-(Numpad)",0x4A},
    {"4(Numpad)",0x4B},
    {"5(Numpad)",0x4C},
    {"6(Numpad)",0x4D},
    {"+(Numpad)",0x4E},
    {"1(Numpad)",0x4F},
    {"2(Numpad)",0x50},
    {"3(Numpad)",0x51},
    {"0(Numpad)",0x52},
    {".(Numpad)",0x53},
    {"F11",0x57},
    {"F12",0x58},
    {"F13",0x64},
    {"F14",0x65},
    {"F15",0x66},
    {"$",0x7D},
    {"=",0x8D},
    {"^",0x90},
    {"@",0x91},
    {":",0x92},
    {"_",0x93},
    {"Enter(Numpad)",0x9C},
    {"Ctrl(Right)",0x9D},
    {",(Numpad)",0xB3},
    {"/(Numpad)",0xB5},
    {"Sys Rq",0xB7},
    {"Alt(Right)",0xB8},
    {"Pause",0xC5},
    {"Home",0xC7},
    {"↑",0xC8},
    {"Page Up",0xC9},
    {"←",0xCB},
    {"→",0xCD},
    {"End",0xCF},
    {"↓",0xD0},
    {"Page Down",0xD1},
    {"Insert",0xD2},
    {"Delete",0xD3},
    {"Windows",0xDB},
    {"Windows",0xDC},
    {"Menu",0xDD},
    {"Power",0xDE},
    {"Windows",0xDF}
};


