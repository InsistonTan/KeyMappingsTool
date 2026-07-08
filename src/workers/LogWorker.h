#ifndef LOGWORKER_H
#define LOGWORKER_H

#include "global.h"
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
            auto size = getQueueSize();
            for(int i=0; i < size; i++){
                auto log = popQueue();
                if(!log.isEmpty()){
                    emit updateLogSignal(log);
                }
            }

            QThread::msleep(150);
        }
    }
};
#endif // LOGWORKER_H
