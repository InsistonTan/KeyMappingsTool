#ifndef KEY_MAP_H
#define KEY_MAP_H

#include<string>
#include<map>

using namespace std;

const static map<string, short> VK_MAP = {
    // 以下四个是自定义的四个操作(不是windows标准)
    {"鼠标向左移动",-1},
    {"鼠标向上移动",-2},
    {"鼠标向右移动",-3},
    {"鼠标向下移动",-4},
    {"鼠标左键长按(再按松开)",-5},
    {"鼠标右键长按(再按松开)",-6},
    {"鼠标左键点击",-7},
    {"鼠标右键点击",-8},
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

        // {"Left mouse button",0x01},
        // {"Right mouse button",0x02},
        // {"Middle mouse button",0x04},
        // {"BACKSPACE key",0x08},
        // {"TAB key",0x09},
        // {"CLEAR key",0x0C},
        // {"ENTER key",0x0D},
        // {"SHIFT key",0x10},
        // {"CTRL key",0x11},
        // {"ALT key",0x12},
        // {"PAUSE key",0x13},
        // {"CAPS LOCK key",0x14},
        // {"ESC key",0x1B},
        // {"SPACEBAR",0x20},
        // {"PAGE UP key",0x21},
        // {"PAGE DOWN key",0x22},
        // {"END key",0x23},
        // {"HOME key",0x24},
        // {"LEFT ARROW key",0x25},
        // {"UP ARROW key",0x26},
        // {"RIGHT ARROW key",0x27},
        // {"DOWN ARROW key",0x28},
        // {"SELECT key",0x29},
        // {"PRINT key",0x2A},
        // {"PRINT SCREEN key",0x2C},
        // {"INS key",0x2D},
        // {"DEL key",0x2E},
        // {"0 key",0x30},
        // {"1 key",0x31},
        // {"2 key",0x32},
        // {"3 key",0x33},
        // {"4 key",0x34},
        // {"5 key",0x35},
        // {"6 key",0x36},
        // {"7 key",0x37},
        // {"8 key",0x38},
        // {"9 key",0x39},
        // {"A key",0x41},
        // {"B key",0x42},
        // {"C key",0x43},
        // {"D key",0x44},
        // {"E key",0x45},
        // {"F key",0x46},
        // {"G key",0x47},
        // {"H key",0x48},
        // {"I key",0x49},
        // {"J key",0x4A},
        // {"K key",0x4B},
        // {"L key",0x4C},
        // {"M key",0x4D},
        // {"N key",0x4E},
        // {"O key",0x4F},
        // {"P key",0x50},
        // {"Q key",0x51},
        // {"R key",0x52},
        // {"S key",0x53},
        // {"T key",0x54},
        // {"U key",0x55},
        // {"V key",0x56},
        // {"W key",0x57},
        // {"X key",0x58},
        // {"Y key",0x59},
        // {"Z key",0x5A},
        // {"Left Windows key",0x5B},
        // {"Right Windows key",0x5C},
        // {"Numeric keypad 0 key",0x60},
        // {"Numeric keypad 1 key",0x61},
        // {"Numeric keypad 2 key",0x62},
        // {"Numeric keypad 3 key",0x63},
        // {"Numeric keypad 4 key",0x64},
        // {"Numeric keypad 5 key",0x65},
        // {"Numeric keypad 6 key",0x66},
        // {"Numeric keypad 7 key",0x67},
        // {"Numeric keypad 8 key",0x68},
        // {"Numeric keypad 9 key",0x69},
        // {"F1 key",0x70},
        // {"F2 key",0x71},
        // {"F3 key",0x72},
        // {"F4 key",0x73},
        // {"F5 key",0x74},
        // {"F6 key",0x75},
        // {"F7 key",0x76},
        // {"F8 key",0x77},
        // {"F9 key",0x78},
        // {"F10 key",0x79},
        // {"F11 key",0x7A},
        // {"F12 key",0x7B},
        // {"F13 key",0x7C},
        // {"F14 key",0x7D},
        // {"F15 key",0x7E},
        // {"F16 key",0x7F},
        // {"F17 key",0x80},
        // {"F18 key",0x81},
        // {"F19 key",0x82},
        // {"F20 key",0x83},
        // {"F21 key",0x84},
        // {"F22 key",0x85},
        // {"F23 key",0x86},
        // {"F24 key",0x87},
        // {"NUM LOCK key",0x90},
        // {"SCROLL LOCK key",0x91},
        // {"Left SHIFT key",0xA0},
        // {"Right SHIFT key",0xA1},
        // {"Left CONTROL key",0xA2},
        // {"Right CONTROL key",0xA3},
        // {"Left ALT key",0xA4},
        // {"Right ALT key",0xA5}
    };

#endif // KEY_MAP_H
