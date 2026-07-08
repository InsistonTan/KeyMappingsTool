#include "LogService.h"

#include <QDateTime>

LogService::LogService() {}

void LogService::pushToLogQueue(QString data){
    QMutexLocker locker(&mutex_logQueue);
    // 获取当前日期和时间
    QDateTime currentDateTime = QDateTime::currentDateTime();
    // 格式化输出当前日期和时间
    QString logText = "<p>[" + currentDateTime.toString("yyyy-MM-dd HH:mm:ss") + "] " + data + "</p>";

    logQueue.enqueue(logText);
}
QString LogService::popLogQueue(){
    QMutexLocker locker(&mutex_logQueue);
    if(logQueue.size() > 0){
        return logQueue.dequeue();
    }
    return "";
}
int LogService::getLogQueueSize(){
    QMutexLocker locker(&mutex_logQueue);
    return logQueue.size();
}
void LogService::parseSuccessLog(QString srcText){
     pushToLogQueue("<span style='color:rgb(0, 151, 144);'>" + srcText + "</span>");
}
void LogService::parseWarningLog(QString srcText){
    pushToLogQueue("<span style='color:rgb(206, 122, 58);'>" + srcText + "</span>");
}
void LogService::parseErrorLog(QString srcText){
    pushToLogQueue("<span style='color:red;'>" + srcText + "</span>");
}

