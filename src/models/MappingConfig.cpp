#include "MappingConfig.h"
#include "common/StringConstants.h"
#include "models/MappingRelation.h"
#include "models/UserConfig.h"
#include <common/MappingConfigKey.h>

#include <QMap>
#include <QVariant>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>

#include <common/Global.h>


void MappingConfig::clear()
{
    overrideGlobalUserConfig = false;
    overrideGlobalFFBSettings = false;
    relatedUserConfig = UserConfig();
    mappingList = {};
}

QJsonObject MappingConfig::toJson()
{
    QJsonObject obj;

    obj[MappingConfigKey::overrideGlobalUserConfig] = overrideGlobalUserConfig;
    obj[MappingConfigKey::overrideGlobalFFBSettings] = overrideGlobalFFBSettings;
    obj[MappingConfigKey::mappingList] = MappingRelation::listToJsonArray(mappingList);
    obj[MappingConfigKey::relatedUserConfig] = relatedUserConfig.toJson();

    return obj;
}


MappingConfig MappingConfig::fromJson(QJsonDocument doc)
{
    MappingConfig mc;

    // 读取新版配置成功
    if (!doc.isNull()) {
        // 新版配置是一个对象
        if(doc.isObject()){
            // json对象
            QJsonObject jsonObj = doc.object();

            // 读取是否覆盖全局用户配置 overrideGlobalFFBSettings
            if(jsonObj.contains(MappingConfigKey::overrideGlobalUserConfig)
                && jsonObj[MappingConfigKey::overrideGlobalUserConfig].isBool()){
                mc.overrideGlobalUserConfig = jsonObj[MappingConfigKey::overrideGlobalUserConfig].toBool();
            }

            // 读取是否覆盖全局力反馈模拟配置
            if(jsonObj.contains(MappingConfigKey::overrideGlobalFFBSettings)
                && jsonObj[MappingConfigKey::overrideGlobalFFBSettings].isBool()){
                mc.overrideGlobalFFBSettings = jsonObj[MappingConfigKey::overrideGlobalFFBSettings].toBool();
            }

            // 读取映射列表
            if(jsonObj.contains(MappingConfigKey::mappingList)
                && jsonObj[MappingConfigKey::mappingList].isArray()){

                // 把映射列表设置到 MappingConfig
                mc.mappingList = MappingRelation::listFromJsonArray(jsonObj[MappingConfigKey::mappingList].toArray());
            }

            // 读取关联的软件配置
            if(jsonObj.contains(MappingConfigKey::relatedUserConfig)
                && jsonObj[MappingConfigKey::relatedUserConfig].isObject()){

                // 获取json对象
                auto obj = jsonObj[MappingConfigKey::relatedUserConfig].toObject();
                // 读取关联的软件设置
                mc.relatedUserConfig = UserConfig::fromJsonObject(obj);
            }

        }
        // 旧版配置, 只有一个映射列表
        else if(doc.isArray()){
            // 读取旧版配置
            mc.mappingList = MappingRelation::listFromJsonArray(doc.array());
        }
    }else{
        Global::showErrorMsgBoxAndPushToLog(StringConstants::error_parseMappingFileFailed);
    }

    return mc;
}
