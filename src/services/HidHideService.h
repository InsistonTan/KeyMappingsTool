#pragma once

#include <QList>
#include <QString>
#include <Windows.h>
#include <winioctl.h>

///
/// \brief The HidHideService class
///
class HidHideService
{

public:
    struct DevInfo{
        QString name; // 设备名称
        QString deviceInstancePath;// 设备实例路径
    };

    HidHideService();

    // 检查hidhide驱动是否已安装
    static bool checkHidHideDriver();

    // 隐藏全局映射的物理设备, 同时把当前exe添加到白名单
    // deviceInstancePaths 为 设备实例路径 列表
    static void hideMappedDevices(const QVector<DevInfo> &devs);

    // 停止
    static void stop();

    // 根据设备接口路径 获取 设备实例ID
    static QString getDevInstancePath(const QString& devInterfacePath);

private:
    // hidhide安装信息的注册表路径
    const static inline QString HIDHIDE_REG_PATH = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Nefarius Software Solutions e.U.\\HidHide";

    // 当前是否正在隐藏设备
    static inline bool isHiding = false;

    // 记录原本的状态, 用于恢复原状, false代表关闭隐藏, true代表开启隐藏
    static inline bool hidHideStateCache = false;
    // 记录新添加的黑名单设备的实例路径, 用于恢复原状
    static inline QVector<DevInfo> hidHideBlacklistCache = {};

    // 获取HidHideCLI程序路径
    static QString getHidHideCLIPath();
    // 运行 HidHideCLI 命令
    static bool runHidHideCLI(const QStringList& arguments, QString* output = nullptr);


};






