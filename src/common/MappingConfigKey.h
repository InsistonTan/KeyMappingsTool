#pragma once

#include "qstringliteral.h"
#include <QString>


///
/// 映射配置文件的 key
///
class MappingConfigKey{
public:
    // 覆盖全局用户配置
    inline const static QString overrideGlobalUserConfig = QStringLiteral("overrideGlobalUserConfig");
    // 覆盖全局的力反馈模拟设置
    inline const static QString overrideGlobalFFBSettings = QStringLiteral("overrideGlobalFFBSettings");
    // 映射列表
    inline const static QString mappingList = QStringLiteral("mappingList");
    // 关联的用户的软件设置
    inline const static QString relatedUserConfig = QStringLiteral("relatedUserConfig");

    // 映射关系的属性
    inline const static QString dev_btn_name = QStringLiteral("dev_btn_name");
    inline const static QString dev_btn_type = QStringLiteral("dev_btn_type");
    inline const static QString keyboard_name = QStringLiteral("keyboard_name");
    inline const static QString keyboard_value = QStringLiteral("keyboard_value");
    inline const static QString remark = QStringLiteral("remark");
    inline const static QString rotateAxis = QStringLiteral("rotateAxis");
    inline const static QString btnTriggerType = QStringLiteral("btnTriggerType");
    inline const static QString deviceName = QStringLiteral("deviceName");
    inline const static QString mappingType = QStringLiteral("mappingType");



};
