#pragma once

#include "qstringliteral.h"
#include "models/MappingConfig.h"
#include "models/UserConfig.h"

#include <QMutex>
#include <QString>
#include <QStringLiteral>



///
/// \brief The ConfigService class
/// 用户配置服务类, 维护一个单例静态的 UserConfig对象, 该对象记录了整个软件的用户配置;
/// 同时管理用户当前正在使用的映射配置, 根据用户的设置, 如果前正在使用的映射配置开启了覆盖全局设置, 将使用映射配置中的UserConfig设置, 而不是globalUserConfig
///
class ConfigService
{
private:
    // 新版配置文件名, 整合了所有用户配置
    inline const static QString configFileName = QStringLiteral("user_config.json");

    // 旧版本的死区设置文件
    inline const static QString oldVersionDeadAreaSettingFileName
        = QStringLiteral("deadarea_settings.json");

    // 旧版本的辅助功能设置文件
    inline const static QString oldVersionAssistFuncSettingFileName
        = QStringLiteral("assist_func_settings.json");

    // 加载旧版的配置文件(为了适配旧版本的配置文件)
    static UserConfig loadFromOldVersionConfigFile();
    // 加载旧版本的死区设置文件
    static void loadOldVersionDeadZoneSettingsFile(UserConfig& cfg);
    // 加载旧版本的辅助功能设置文件
    static void loadOldVersionAssistFuncSettingsFile(UserConfig& cfg);

public:
    // 全局用户配置对象
    // 读: 使用 get()
    // 写: 加锁后修改
    inline static UserConfig globalUserConfig;
    // 操作用户配置对象的锁, 用处: 获取用户配置对象快照, 修改用户配置对象
    inline static QMutex mutex_globalUserConfig;
    // 把配置保存到本地文件的锁, 用处: 异步保存配置到本地
    inline static QMutex mutex_writeGlobalUserConfig;

    // 用户当前正在使用的映射配置
    // 读: 使用getCurrentMappingConfig()
    // 写: 加锁后修改
    inline static MappingConfig currentMappingConfig;
    // 操作 用户当前正在使用的映射配置 的锁
    inline static QMutex mutex_currentMappingConfig;
    // 把配置保存到本地文件的锁, 用处: 异步保存配置到本地
    inline static QMutex mutex_writeCurrentMappingConfig;
    // 当前选择的映射配置文件的文件名(无后缀)
    inline static QString currentMappingFileName;
    // 用户映射配置文件map, key为无后缀文件名, value为文件绝对路径
    //inline static QMap<QString, QString> mappingFileNameMap;

    ConfigService();

    // 初始化, 需要在程序开始运行时调用一次
    static void init();


    enum class GetSource{
        Normal,
        FFBSim
    };
    // 获取用户配置的快照, 返回globalUserConfig或者currentMappingConfig.userConfig
    static UserConfig get(GetSource src = GetSource::Normal);

    // 获取用户配置的快照, 返回globalUserConfig
    static UserConfig getGlobalUserConfig();

    // 提交对用户配置的更新
    //static void commit(QMap<QString, QJsonValue> patch);

    // 加载配置文件
    static void loadGlobalConfigFromFile();

    // 保存全局用户设置到文件
    static void saveGlobalUserConfigToFile();

    // 获取用户当前正在使用的映射配置的快照
    static MappingConfig getCurrentMappingConfig();
    static QVector<MappingRelation> getCurrentMappingList();
    // 保存当前正在使用的映射配置到文件
    static QString saveCurrentMappingConfigToFile(QString fileBaseName = "");

    // 如果 isOverride==true,保存当前映射配置到本地
    // 否则 保存全局用户配置到本地
    static void saveToFile(bool isOverride);
};
