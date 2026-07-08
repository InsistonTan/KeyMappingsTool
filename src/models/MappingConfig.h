#pragma once

#include "qjsonobject.h"
#include "models/MappingRelation.h"
#include "models/UserConfig.h"

#include <QVector>
#include <QString>
#include <QJsonObject>
#include <QJsonValueRef>
#include <QJsonDocument>


///
/// 用户创建的映射配置
///
class MappingConfig{  
public:
    // 覆盖全局用户配置
    bool overrideGlobalUserConfig = false;
    bool overrideGlobalFFBSettings = false;

    // 映射列表
    QVector<MappingRelation> mappingList;

    // 当前映射配置 关联的软件设置
    // 关联的软件设置 要比 软件全局设置 优先级更高
    // 如果relatedUserConfig存在, 此配置会临时覆盖软件全局设置
    UserConfig relatedUserConfig;

    //后续增加: 关联进程, 自动检测前台进程, 自动切换映射配置文件


    // 重置当前对象
    void clear();

    // 当前对象转json字符串
    QJsonObject toJson();

    // 从json字符串 转成 当前类的对象
    static MappingConfig fromJson(QJsonDocument doc);
};
