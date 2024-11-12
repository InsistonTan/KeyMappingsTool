#ifndef MAPPING_RELATION_H
#define MAPPING_RELATION_H
#include<string>

//using namespace std;

class MappingRelation{
public:
    int dev_btn_pos; // 设备按键位置
    std::string dev_btn_name;// 设备按键名称
    std::string dev_btn_type;// 设备按键类型(用于区分方向盘的轴和按键)
    int dev_btn_value; // 设备按键值
    short keyboard_value;// 键盘按键值
    std::string keyboard_name;// 键盘按键名称
    std::string remark; // 备注

    MappingRelation(){}
    MappingRelation(int dev_btn_pos, int dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_pos = dev_btn_pos;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, int dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(std::string dev_btn_name, std::string dev_btn_type,int dev_btn_value, short keyboard_value, std::string keyboard_name){
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }
};

#endif // MAPPING_RELATION_H
