#ifndef MAPPING_RELATION_H
#define MAPPING_RELATION_H
#include<string>

using namespace std;

class MappingRelation{
public:
    int dev_btn_pos; // 设备按键位置
    int dev_btn_value; // 设备按键值
    short keyboard_value;// 键盘按键值
    string keyboard_name;// 键盘按键名称
    string remark; // 备注

    MappingRelation(){}
    MappingRelation(int dev_btn_pos, int dev_btn_value, short keyboard_value, string keyboard_name){
        this->dev_btn_pos = dev_btn_pos;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }
};

#endif // MAPPING_RELATION_H
