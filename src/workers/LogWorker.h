#pragma once

#include "qcoreapplication.h"
#include "services/LogService.h"
#include <QThread>

// 日志获取线程
class LogWorker: public QThread {
    Q_OBJECT
signals:
    void updateLogSignal(QString log);
public:
    // 线程执行体
    void run() override{
        while(true){
            auto size = LogService::getLogQueueSize();
            for(int i=0; i < size; i++){
                auto log = LogService::popLogQueue();
                if(!log.isEmpty()){
                    emit updateLogSignal(log);
                }
            }

            QThread::msleep(150);

            // 响应 QT事件
            QCoreApplication::processEvents();
        }
    }
};
