#ifndef MAPPING_RELATION_H
#define MAPPING_RELATION_H
#include "BtnTriggerTypeEnum.h"
#include<QString>
#include<string>
#include "BigKey.hpp"
//using namespace std;
#define BUTTONS_VALUE_TYPE BigKey

enum class MappingType{
    Keyboard,
    Xbox,
};

class MappingRelation{
public:
    int dev_btn_pos; // 设备按键位置
    std::string dev_btn_name;// 设备按键名称
    std::string dev_btn_type;// 设备按键类型(用于区分方向盘的轴和按键)
    BUTTONS_VALUE_TYPE dev_btn_bit_value; // 设备按键值
    int dev_btn_value; // 设备按键值(方向盘的轴和十字键等)
    QString keyboard_value = "";// 键盘按键值(多个值用 ", " 隔开)
    std::string keyboard_name = "";// 键盘按键名称
    std::string remark; // 备注
    int rotateAxis = 0; // 是否反转轴, 0不反转, 1反转
    TriggerTypeEnum btnTriggerType = TriggerTypeEnum::Normal; // 按键触发类型, 默认同步模式
    QString deviceName = "";// 当前映射所属设备名称
    MappingType mappingType = MappingType::Keyboard; // 映射类型, 默认键盘映射

    MappingRelation(){}
    MappingRelation(int dev_btn_pos, int dev_btn_value, QString keyboard_value, std::string keyboard_name){
        this->dev_btn_pos = dev_btn_pos;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, int dev_btn_value, QString keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type, int dev_btn_value, QString keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type, int dev_btn_value, QString keyboard_value, std::string keyboard_name, TriggerTypeEnum btnTriggerType){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
        this->btnTriggerType = btnTriggerType;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type, int dev_btn_value, QString keyboard_value, std::string keyboard_name, TriggerTypeEnum btnTriggerType, QString deviceName){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
        this->btnTriggerType = btnTriggerType;
        this->deviceName = deviceName;
    }

    void setBtnBitValue(BUTTONS_VALUE_TYPE dev_btn_bit_value){
        this->dev_btn_bit_value = dev_btn_bit_value;
    }

    void setDeviceName(QString deviceName){
        this->deviceName = deviceName;
    }

    void setMappingType(MappingType mappingType){
        this->mappingType = mappingType;
    }
};

#endif // MAPPING_RELATION_H
