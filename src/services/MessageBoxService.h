#pragma once

#include "qcontainerfwd.h"
#include "common/StringConstants.h"
#include <QCoreApplication>
#include <QMetaObject>
#include <ui/widgets/CardMessageDialog.h>





///
/// \brief The MessageBoxService class
/// 消息弹窗服务
///
class MessageBoxService{
public:

    inline static void showError(QString msg){
        showError(StringConstants::error, msg);
    }
    inline static void showError(QString title, QString msg){
        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            CardMessageDialog::error(nullptr, title, msg);
        }, Qt::QueuedConnection);
    }


    inline static void showInfo(QString msg){
        showInfo(StringConstants::info, msg);
    }
    inline static void showInfo(QString title, QString msg){
        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            CardMessageDialog::info(nullptr, title, msg);
        }, Qt::QueuedConnection);
    }


    inline static void showWarning(QString msg){
        showWarning(StringConstants::warning, msg);
    }
    inline static void showWarning(QString title, QString msg){
        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            CardMessageDialog::warning(nullptr, title, msg);
        }, Qt::QueuedConnection);
    }

    inline static void showSuccess(QString msg){
        showSuccess(StringConstants::success, msg);
    }
    inline static void showSuccess(QString title, QString msg){
        QMetaObject::invokeMethod(QCoreApplication::instance(), [=](){
            CardMessageDialog::success(nullptr, title, msg);
        }, Qt::QueuedConnection);
    }
};
