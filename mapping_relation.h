#ifndef MAPPING_RELATION_H
#define MAPPING_RELATION_H
#include "BtnTriggerTypeEnum.h"
#include<string>

//using namespace std;
#define BUTTONS_VALUE_TYPE int32_t
#define MAX_BUTTONS (sizeof(BUTTONS_VALUE_TYPE) * 8)

class MappingRelation{
public:
    int dev_btn_pos; // 设备按键位置
    std::string dev_btn_name;// 设备按键名称
    std::string dev_btn_type;// 设备按键类型(用于区分方向盘的轴和按键)
    BUTTONS_VALUE_TYPE dev_btn_value; // 设备按键值
    short keyboard_value;// 键盘按键值
    std::string keyboard_name;// 键盘按键名称
    std::string remark; // 备注
    int rotateAxis = 0; // 是否反转轴, 0不反转, 1反转
    TriggerTypeEnum btnTriggerType; // 按键触发类型

    MappingRelation(){}
    MappingRelation(int dev_btn_pos, BUTTONS_VALUE_TYPE dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_pos = dev_btn_pos;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, BUTTONS_VALUE_TYPE dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type,BUTTONS_VALUE_TYPE dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type,BUTTONS_VALUE_TYPE dev_btn_value, short keyboard_value, std::string keyboard_name, TriggerTypeEnum btnTriggerType){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
        this->btnTriggerType = btnTriggerType;
    }

    static BUTTONS_VALUE_TYPE strToButtonsValueType(const std::string& str) {
        if (str.empty()) {
            return 0;
        }
        BUTTONS_VALUE_TYPE result = 0;
        for (char c : str) {
            result = result * 10 + (c - '0');
        }
        return result;
    }

    static std::string buttonsValueTypeToStr(BUTTONS_VALUE_TYPE value) {
        // std::string result;
        // while (value > 0) {
        //     result = static_cast<char>('0' + (value % 10)) + result;
        //     value /= 10;
        // }
        // return result.empty() ? "0" : result;
        return std::to_string(value);
    }

    static std::string toBitStr(BUTTONS_VALUE_TYPE value) {
        std::string bitStr;
        for (int i = 0; i < sizeof(BUTTONS_VALUE_TYPE) * 8; ++i) {
            bitStr += (value & (1 << i)) ? '1' : '0';
        }
        return bitStr;
    }
};

#endif // MAPPING_RELATION_H
