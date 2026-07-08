#include "MappingRelation.h"

#include <common/Global.h>
#include <common/MappingConfigKey.h>

#include <utils/MyUtils.h>


QJsonObject MappingRelation::toJsonObject(){
    QJsonObject jsonObj;
    jsonObj[MappingConfigKey::dev_btn_name] = dev_btn_name;
    jsonObj[MappingConfigKey::dev_btn_type] = static_cast<int>(dev_btn_type);
    jsonObj[MappingConfigKey::keyboard_name] = keyboard_name;
    jsonObj[MappingConfigKey::keyboard_value] = keyboard_value;
    jsonObj[MappingConfigKey::remark] = remark;
    jsonObj[MappingConfigKey::rotateAxis] = rotateAxis;
    jsonObj[MappingConfigKey::btnTriggerType] = btnTriggerType;
    jsonObj[MappingConfigKey::mappingType] = static_cast<int>(mappingType);
    jsonObj[MappingConfigKey::deviceName] = deviceName;

    return jsonObj;
}

MappingRelation MappingRelation::fromJsonObject(QJsonObject jsonObj){
    // 从json对象读取信息
    MappingRelation mapping(true);

    mapping.dev_btn_name = jsonObj.contains(MappingConfigKey::dev_btn_name)
                               ? jsonObj[MappingConfigKey::dev_btn_name].toString()
                               : "";

    // 映射的数据类型(按键/轴)
    if(jsonObj.contains(MappingConfigKey::dev_btn_type)){
        // 新版, 枚举
        auto intVal = jsonObj[MappingConfigKey::dev_btn_type].toInt(0);
        // 值有效, 转换成枚举
        if(intVal > (int)DeviceDataTypeEnum::NONE && intVal < (int)DeviceDataTypeEnum::COUNT){
            mapping.dev_btn_type = (DeviceDataTypeEnum)intVal;
        }else{
            // 为了适配旧版本, 尝试用字符串值获取枚举
            mapping.dev_btn_type = getDeviceDataTypeEnum(
                jsonObj[MappingConfigKey::dev_btn_type].toString()
                );
        }
    }

    mapping.keyboard_name = jsonObj.contains(MappingConfigKey::keyboard_name)
                                ? jsonObj[MappingConfigKey::keyboard_name].toString()
                                : "";

    mapping.keyboard_value = jsonObj.contains(MappingConfigKey::keyboard_value)
                                 ? jsonObj[MappingConfigKey::keyboard_value].toString()
                                 : "";

    // 如果前面toString()没有获取到值, 用 toInt()获取
    if(mapping.keyboard_value.isEmpty()){
        mapping.keyboard_value = jsonObj.contains(MappingConfigKey::keyboard_value)
        ? QString::number(jsonObj[MappingConfigKey::keyboard_value].toInt())
        : "";
    }

    mapping.remark = jsonObj.contains(MappingConfigKey::remark)
                         ? jsonObj[MappingConfigKey::remark].toString()
                         : "";

    mapping.rotateAxis = (jsonObj.contains(MappingConfigKey::rotateAxis)
                          && jsonObj[MappingConfigKey::rotateAxis].toInt() == 1)
                             ? 1
                             : 0;

    mapping.btnTriggerType =
        (jsonObj.contains(MappingConfigKey::btnTriggerType)
         && jsonObj[MappingConfigKey::btnTriggerType].toInt() > 0
         && jsonObj[MappingConfigKey::btnTriggerType].toInt() < TriggerTypeEnum::End)
            ? static_cast<TriggerTypeEnum>(jsonObj[MappingConfigKey::btnTriggerType].toInt())
            : TriggerTypeEnum::Normal;

    if (jsonObj.contains(MappingConfigKey::mappingType)) {
        // 读取映射类型
        if ((MappingType)jsonObj[MappingConfigKey::mappingType].toInt() == MappingType::Xbox) {
            mapping.mappingType = MappingType::Xbox;
        } else if ((MappingType)jsonObj[MappingConfigKey::mappingType].toInt() == MappingType::Keyboard) {
            mapping.mappingType = MappingType::Keyboard;
        }
    }

    mapping.deviceName = (jsonObj.contains(MappingConfigKey::deviceName)
                              ? jsonObj[MappingConfigKey::deviceName].toString()
                              : "");

    // 根据按键名称设置 dev_btn_bit_value，不从文件中获取
    mapping.dev_btn_bit_value = Global::stringToButtonsValueType(mapping.dev_btn_name);

    return mapping;
}

QJsonArray MappingRelation::listToJsonArray(QVector<MappingRelation> list){
    QJsonArray jsonArray;

    if(list.size() <= 0){
        return jsonArray;
    }

    for(auto& mapping : list ){
        jsonArray.append(mapping.toJsonObject());
    }

    return jsonArray;
}

QVector<MappingRelation> MappingRelation::listFromJsonArray(QJsonArray array){
    // 存储映射列表
    QVector<MappingRelation> mappingList;

    // 遍历json数组, 生成 MappingRelation 并添加到 mappingList
    for (const QJsonValue &value : array) {
        if(value.isObject()){
            // 映射关系的json对象
            QJsonObject jsonObj = value.toObject();

            // jsonObj 转 对象
            auto mapping = MappingRelation::fromJsonObject(jsonObj);

            // 设备按键名称不为空, 添加到列表
            if(!mapping.dev_btn_name.isEmpty()){
                mappingList.append(mapping);
            }
        }
    }

    return mappingList;
}
