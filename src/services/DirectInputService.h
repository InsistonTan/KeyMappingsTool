#pragma once

#include "qmutex.h"
#include <QVector>
#include <dinput.h>
#include <QMap>
#include <string>

#include <models/MappingRelation.h>
#include <core/DirectInputCore.h>


///
/// \brief The DirectInputService class
/// DirectInput服务类
///
/// 使用步骤:
/// 1.initDirectInput();初始化DirectInput实例
/// 2.scanDevice();扫描设备, 设备信息存储在QVector<DiDeviceInfo> g_diDeviceList;(设备信息列表)
/// 3.openDiDevice(QVector<QString> deviceNameList);根据设备名称打开设备(初始化设备)
/// 4.QVector<MappingRelation*> getInputState( //获取设备的状态数据
///     QVector<LPDIRECTINPUTDEVICE8> initedDeviceList, // 需要读取数据的设备列表(已经初始化过的设备), 每个设备读取一次数据, 整合在返回值QList<MappingRelation*>列表中
///     bool readBtnData = true, // 是否读取按键数据
///     bool readAxisData = true,// 是否读取轴数据
///     bool enableLog = false, // 参数非必需, 是否开启日志
///     std::vector<MappingRelation> multiBtnVector = {} // 参数非必需, 组合按键映射的列表, SimulateTask任务需要的特殊处理
///   );
///
class DirectInputService
{

public:
    // 构造函数
    DirectInputService();

    // 初始化DirectInput
    static bool initDirectInput();

    // 扫描设备
    static bool scanDevice();

    // 根据设备名称 打开设备(初始化设备);
    // interruptWhenError = true, 代表当deviceNameList中的某个设备初始化失败时, 函数直接return, 不再初始化剩下的设备;
    // cooperativeLevel 默认使用: 后台+非独占模式;
    static bool openDiDevice(QVector<QString> deviceNameList, bool interruptWhenError = true, DWORD cooperativeLevel = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    static LPDIRECTINPUTDEVICE8 openDiDevice(HWND hWnd, QString deviceName, DWORD cooperativeLevel = DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);

    // 获取设备状态数据;
    // QVector<MappingRelation>& out : 外部传入的用来存储结果的对象的引用;
    // QVector<LPDIRECTINPUTDEVICE8> initedDeviceList : 需要读取数据的设备(已初始化的设备);
    // bool readBtnData : 是否读取设备按键数据;
    // bool readAxisData : 是否读取设备轴数据;
    // bool enableLog : 是否输出日志到日志窗口;
    // std::vector<MappingRelation> multiBtnVector : 组合键列表, 用于匹配组合键;
    // bool enableOnlyLongestMapping : 是否优先匹配最长的组合键
    static void getInputState(
        QVector<MappingRelation>& out,
        const QVector<LPDIRECTINPUTDEVICE8>& initedDeviceList,
        bool readBtnData = true,
        bool readAxisData = true,
        bool enableLog = false,
        std::vector<MappingRelation> multiBtnVector = {},
        bool enableOnlyLongestMapping = false);

    // 获取轴的数值范围
    static void getDipropRange(LPDIRECTINPUTDEVICE8 g_pDevice, long axisCode, QString axisName);

    // 清空已初始化的设备列表
    static void clearInitedDeviceList();

    // 清理DirectInput
    static void cleanupDirectInput();

    // 检测是否支持力反馈
    static bool checkIsSupportForceFeedback(LPDIRECTINPUTDEVICE8 device);

    // 设备是否已经初始化
    static bool isDeviceInited(QString deviceName);

    // 获取设备名称
    static QString getDeviceName(LPDIRECTINPUTDEVICE8 pDevice);

    // 根据设备产品名称, guid, 设备接口路径, 生成设备名称
    static std::string generateDeviceName(DiDeviceInfo deviceInfo);

    // 根据设备名称 获取 已初始化的设备实例
    static LPDIRECTINPUTDEVICE8 getInitedDevice(QString deviceName);

    // 检查设备是否存活
    static bool isDeviceAlive(LPDIRECTINPUTDEVICE8 device);

    // 获取设备信息列表的快照
    static QVector<DiDeviceInfo> getDeviceInfoListSnapshot(){
        QMutexLocker locker(&g_diDeviceListMutex);
        return g_diDeviceList;
    }

    // 获取已初始化设备列表的快照
    static QVector<LPDIRECTINPUTDEVICE8> getInitedDeviceListSnapshot(){
        QMutexLocker locker(&g_initedDeviceListMutex);
        return g_initedDeviceList;
    }

    // 获取所有设备轴的数值范围;
    // key为"设备名称-轴名称", 值为数值范围;
    static QMap<QString, DIPROPRANGE> getAxisValueRangeMap(){
        return g_axisValueRangeMap;
    }

    // 从 g_diDeviceList 中获取设备名称列表
    static QVector<QString> getDeviceNameListFromDeviceInfoList(){
        auto devList = getDeviceInfoListSnapshot();
        QVector<QString> result;
        for(auto const &dev : devList){
            result.append(dev.name.data());
        }

        return result;
    }

    // 获取接下来 5s 内用户操作的设备按键或者轴
    static MappingRelation getNextActionBtnOrAxis(
        QVector<LPDIRECTINPUTDEVICE8> initedDeviceList,
        bool onlyGetChangedKey,
        bool enableLogs,
        bool readBtnData = true,
        bool readAxisData = true);

private:
    // directInput设备
    inline static LPDIRECTINPUT8 g_pDirectInput= nullptr;
    // 已初始化的设备列表
    inline static QVector<LPDIRECTINPUTDEVICE8> g_initedDeviceList;
    // g_initedDeviceList的锁
    inline static QMutex g_initedDeviceListMutex;
    // 设备信息列表
    inline static QVector<DiDeviceInfo> g_diDeviceList;
    // g_diDeviceList的锁
    inline static QMutex g_diDeviceListMutex;
    // 设备的轴名称对应数值范围map
    inline static QMap<QString, DIPROPRANGE> g_axisValueRangeMap;

    // 显示初始化设备失败的信息
    static void showInitDeviceFailedMsg(DiActionResultEnum actionResult, std::string deviceName, bool showMessageBox = true);

};
