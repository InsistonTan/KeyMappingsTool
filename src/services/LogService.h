#include <QMutex>
#include <QQueue>
#include <QString>

#pragma once

class LogService
{
private:
    // 全局变量, 日志队列
    inline static QQueue<QString> logQueue;
    inline static QMutex mutex_logQueue;
public:
    LogService();

    // 推送日志到队列
    static void pushToLogQueue(QString data);
    // 从日志队列取出日志
    static QString popLogQueue();
    // 获取日志队列大小
    static int getLogQueueSize();

    // 输出成功日志
    static void parseSuccessLog(QString srcText);
    // 输出警告日志
    static void parseWarningLog(QString srcText);
    // 输出错误日志
    static void parseErrorLog(QString srcText);
};
