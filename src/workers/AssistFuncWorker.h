#pragma once

#include <QObject>
#include <windows.h>  // 包含 DWORD 的定义
#include "SCS_Telemetry/scs-telemetry-common.hpp"

class AssistFuncWorker : public QObject
{
    Q_OBJECT

private:
    volatile bool isWorkerRunning = true;

signals:
    void workFinished();
 
public slots:
    void doWork();

    // 取消运行
    void cancelWorkSlot();

public:
    AssistFuncWorker();

    scsTelemetryMap_t* readETS2Data();
};
