#pragma once

#include "common/BigKey.hpp"
#include "common/DeviceDataTypeEnum.h"
#include "common/BtnTriggerTypeEnum.h"

#include <QJsonArray>
#include <QJsonObject>
#include <QVector>
#include <QString>



#define BUTTONS_VALUE_TYPE BigKey

// 映射模式;
// 映射键盘鼠标, 映射xbox手柄
enum class MappingType{
    Keyboard,
    Xbox,
};

///
/// \brief The MappingRelation class;
/// 映射关系类
///
class MappingRelation{
public:
    ////////////////////////////
    /// 不需要持久化的属性
    ////////////////////////////
    // 当前对象是否有效(用于判空);
    bool valid = true;

    // 设备按键位置
    int dev_btn_pos = -1;

    // 设备按键位值
    BUTTONS_VALUE_TYPE dev_btn_bit_value = 0;

    // 存储设备按键值(方向盘的轴和十字键等)
    int dev_btn_value = -1;

    // 在新增映射时, 轴的值的变化量, 用于判断方向盘是左转还是右转;
    // 值为正数, 说明右转, 值为负数, 说明左转;
    int axisValueChange = 0;


    ////////////////////////////
    /// 以下是需要持久化的属性
    ////////////////////////////
    // 设备按键名称
    QString dev_btn_name = "";
    // 设备数据类型(轴数据, 按键数据)
    DeviceDataTypeEnum dev_btn_type = DeviceDataTypeEnum::NONE;
    // 映射的键盘按键/xbox按键/xbox轴的名称
    QString keyboard_name = "";
    // 键盘/xbox按键值(多个值用 ", " 隔开)
    QString keyboard_value = "";
    // 备注
    QString remark = "";
    // 是否反转轴, 0不反转, 1反转
    int rotateAxis = 0;
    // 按键触发类型, 默认同步模式
    TriggerTypeEnum btnTriggerType = TriggerTypeEnum::Normal;
    // 当前映射所属设备名称
    QString deviceName = "";
    // 映射类型(映射键盘/映射xbox手柄), 默认键盘映射
    MappingType mappingType = MappingType::Keyboard;


    // 当前对象转json对象(只转需要持久化的属性)
    QJsonObject toJsonObject();

    // QJsonObject 转 当前对象
    static MappingRelation fromJsonObject(QJsonObject jsonObj);

    // MappingRelation列表 转 json列表
    static QJsonArray listToJsonArray(QVector<MappingRelation> list);

    // 从 json列表 转 MappingRelation列表
    static QVector<MappingRelation> listFromJsonArray(QJsonArray array);


    MappingRelation(){}

    MappingRelation(bool valid){
        this->valid = valid;
    }

    MappingRelation(int dev_btn_pos, int dev_btn_value, QString keyboard_value, QString keyboard_name){
        this->valid = true;
        this->dev_btn_pos = dev_btn_pos;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(QString dev_btn_name, int dev_btn_value, QString keyboard_value, QString keyboard_name){
        this->valid = true;
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(QString dev_btn_name,
                    DeviceDataTypeEnum dev_btn_type,
                    int dev_btn_value,
                    QString keyboard_value,
                    QString keyboard_name){
        this->valid = true;
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
    }

    MappingRelation(QString dev_btn_name,
                    DeviceDataTypeEnum dev_btn_type,
                    int dev_btn_value,
                    QString keyboard_value,
                    QString keyboard_name,
                    TriggerTypeEnum btnTriggerType){
        this->valid = true;
        this->dev_btn_name = dev_btn_name;
        this->dev_btn_type = dev_btn_type;
        this->dev_btn_value = dev_btn_value;
        this->keyboard_value = keyboard_value;
        this->keyboard_name = keyboard_name;
        this->btnTriggerType = btnTriggerType;
    }

    MappingRelation(QString dev_btn_name,
                    DeviceDataTypeEnum dev_btn_type,
                    int dev_btn_value,
                    QString keyboard_value,
                    QString keyboard_name,
                    TriggerTypeEnum btnTriggerType,
                    QString deviceName){
        this->valid = true;
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
