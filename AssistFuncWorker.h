#ifndef ASSISTFUNCWORKER_H
#define ASSISTFUNCWORKER_H

#include <QObject>
#include <windows.h>  // 包含 DWORD 的定义

class AssistFuncWorker : public QObject
{
    Q_OBJECT

private:
    volatile bool isWorkerRunning = true;

    byte* readETS2Data();

signals:
    void workFinished();

public slots:
    void doWork();

    // 取消运行
    void cancelWorkSlot();

public:
    AssistFuncWorker();
};

#endif // ASSISTFUNCWORKER_H
