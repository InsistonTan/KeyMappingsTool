#include "ConfigService.h"
#include <common/Global.h>
#include "LogService.h"

#include "common/StringConstants.h"
#include "qjsondocument.h"
#include "qmutex.h"
#include "models/UserConfig.h"
#include "ui/widgets/CardMessageDialog.h"

#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QThreadPool>
#include <QStringList>
#include <QJsonArray>
#include <qdialog.h>

ConfigService::ConfigService() {}

void ConfigService::init()
{
    // 加载配置文件
    loadGlobalConfigFromFile();
}

UserConfig ConfigService::get(GetSource src)
{
    // 加锁的顺序不能变, 否则可能会导致死锁
    // 例如: 这里先 mutex_currentMappingConfig 再 mutex_globalUserConfig
    //      如果别的地方 先 mutex_globalUserConfig 再 mutex_currentMappingConfig
    //      就可能双方都只能拿到第一个锁, 然后死锁
    QMutexLocker locker2(&mutex_currentMappingConfig);
    QMutexLocker locker(&mutex_globalUserConfig);

    // 如果是力反馈模拟请求获取, 需要单独判断, 当前映射配置是否开启了覆盖全局的力反馈模拟设置
    if(src == GetSource::FFBSim){
        return currentMappingConfig.overrideGlobalFFBSettings ? currentMappingConfig.relatedUserConfig : globalUserConfig;
    }

    // 如果当前的映射配置 开启了覆盖全局用户配置, 返回当前的映射配置的用户配置
    // 否则 返回全局用户配置
    return currentMappingConfig.overrideGlobalUserConfig ? currentMappingConfig.relatedUserConfig : globalUserConfig;
}

UserConfig ConfigService::getGlobalUserConfig()
{
    QMutexLocker locker(&mutex_globalUserConfig);
    return globalUserConfig;
}

void ConfigService::loadGlobalConfigFromFile()
{
    UserConfig newConfig;

    // 配置文件
    QFile file(Global::getAppDataDirStr().append(configFileName));

    // 新版配置文件不存在, 尝试读取旧版的配置文件
    if(!file.exists()){
        newConfig = loadFromOldVersionConfigFile();
    }else{
        // 打开文件
        if (!file.open(QIODevice::ReadOnly)) {
            LogService::parseWarningLog(StringConstants::openSettingsFileFailed);
            return;
        }

        // 读取文件内容并解析为 JSON 文档
        QByteArray jsonData = file.readAll();
        QJsonDocument doc = QJsonDocument::fromJson(jsonData);
        // json解析失败, 或者文本不是json对象格式
        if (doc.isNull() || !doc.isObject()) {
            LogService::parseWarningLog(StringConstants::parseJsonFailed);
            return;
        }

        newConfig = UserConfig::fromJsonObject(doc.object());
    }


    QMutexLocker locker(&mutex_globalUserConfig);

    // 从json对象读取设置, 生成 UserConfig 对象
    globalUserConfig = newConfig;
}

UserConfig ConfigService::loadFromOldVersionConfigFile()
{
    UserConfig cfg;

    // 读取旧版死区设置文件
    loadOldVersionDeadZoneSettingsFile(cfg);
    // 读取旧版辅助功能设置文件
    loadOldVersionAssistFuncSettingsFile(cfg);

    return cfg;
}

void ConfigService::loadOldVersionDeadZoneSettingsFile(UserConfig& cfg)
{
    // 打开 JSON 文件
    QFile file(Global::getAppDataDirStr().append(oldVersionDeadAreaSettingFileName));
    if (!file.open(QIODevice::ReadOnly)) {
        LogService::parseWarningLog(StringConstants::openSettingsFileFailed);
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    // json解析失败, 或者文本不是json对象格式
    if (doc.isNull() || !doc.isObject()) {
        LogService::parseWarningLog(StringConstants::parseJsonFailed);
        return;
    }

    // 读取设置
    UserConfig::readDeadZoneSettingsFromJson(doc.object(), cfg);
}

void ConfigService::loadOldVersionAssistFuncSettingsFile(UserConfig& cfg)
{
    // 打开 JSON 文件
    QFile file(Global::getAppDataDirStr() + oldVersionAssistFuncSettingFileName);
    if (!file.open(QIODevice::ReadOnly)) {
        LogService::parseWarningLog(StringConstants::openAssistFunctionConfigFailed);
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull() || !doc.isObject()) {
        LogService::parseWarningLog(StringConstants::readAssistFunctionConfigFailed);
        return;
    }

    UserConfig::readAssistFuncSettingsFromJson(doc.object(), cfg);
}

void ConfigService::saveGlobalUserConfigToFile()
{
    // 获取用户配置对象, 并转成json
    QJsonDocument doc(getGlobalUserConfig().toJson());

    // json对象转 json字符串
    QString json = doc.toJson(QJsonDocument::Indented);

    if(json.isEmpty())
        return;


    // 异步保存
    QThreadPool::globalInstance()->start([json](){
        // 加锁
        QMutexLocker locker(&ConfigService::mutex_writeGlobalUserConfig);

        // 保存到文件
        // 创建一个 QFile 对象，并打开文件进行写入
        QFile file(Global::getAppDataDirStr() + configFileName);

        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            // 创建一个文本流对象
            QTextStream out(&file);
            // 写入文本
            out << json;

            // 关闭文件
            file.close();
        } else {
            Global::showErrorMsgBoxAndPushToLog(
                StringConstants::saveSettingsFailed + ": "
                + StringConstants::openSettingsFileFailed);
        }
    });


}

MappingConfig ConfigService::getCurrentMappingConfig()
{
    QMutexLocker locker(&mutex_currentMappingConfig);
    return currentMappingConfig;
}

QVector<MappingRelation> ConfigService::getCurrentMappingList()
{
    return getCurrentMappingConfig().mappingList;
}

QString ConfigService::saveCurrentMappingConfigToFile(QString fileBaseName)
{
    if(fileBaseName.isEmpty()){
        fileBaseName = currentMappingFileName;
        if(fileBaseName.isEmpty()){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::autoSaveMappingFileFailed);
            return "";
        }
    }

    // mappingConfig转json对象再转json文档
    QJsonDocument doc(ConfigService::getCurrentMappingConfig().toJson());

    // 要保存的文本内容
    QString text = QString::fromUtf8(doc.toJson(QJsonDocument::Indented));

    // 最终文件绝对路径
    QString filePath = Global::getUserMappingsFileDir() + fileBaseName + MAPPING_FILE_SUFFIX;

    QFile file(filePath);

    // 有重名文件, 并且文件名 != 当前选择的配置文件名, 需要提醒用户是否覆盖
    // 如果是文件名 == 当前选择的配置文件名, 说明用户正在修改当前选择的配置文件, 不需要提醒覆盖
    if(file.exists() && fileBaseName != currentMappingFileName){
        CardMessageDialog dialog(CardMessageDialog::Type::Warning,
                                 StringConstants::warning,
                                 StringConstants::existsSameFileNameMappingConfig);
        // 用户没有点击确定
        if(dialog.exec() != QDialog::Accepted){
            return "";
        }
    }

    // 写入文件
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);  // 创建一个文本流对象
        out << text;             // 写入文本

        // 关闭文件
        file.close();

        // 返回文件名, 用于后续在下拉框显示新的配置文件
        return fileBaseName;
    } else {
        Global::showErrorMsgBoxAndPushToLog(StringConstants::error_writeMappingFileFailed.arg(filePath));
    }

    return "";
}

void ConfigService::saveToFile(bool isOverride)
{
    if(isOverride){
        saveCurrentMappingConfigToFile();
    }else{
        saveGlobalUserConfigToFile();
    }
}


