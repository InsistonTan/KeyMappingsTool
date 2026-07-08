#pragma once

#include "models/MappingRelation.h"
#include "models/UserConfig.h"
#include <windows.h>
#include <ViGEm/Client.h>
#include "common/KeyMap.h"
#include "common/Global.h"

#include<QThread>

#define MOUSE_X_SPEED 6
#define MOUSE_Y_SPEED 6

// xbox模拟轴移动的速度
#define XBOX_AXIS_SPEED 30000
#define XBOX_TRIGGER_SPEED 250

// 方向盘轴模拟键盘按键时, 轴的内部死区
//#define AXIS_VALID_PERCENT 0.03

// 模拟释放按键的延迟时间ms
#define RELEASE_DELAY_MS 100

// 鼠标滚轮的步长
#define MOUSE_WHEEL_DELTA 20

class SimulateTask : public QObject {
    Q_OBJECT
private:
    UserConfig userConfig;// 用户配置
    QVector<LPDIRECTINPUTDEVICE8> readDataNeedInitedDeviceList;// 目标设备列表, 代表从这些设备读取数据
    QVector<MappingRelation> mappingList;// 已配置的按键映射列表
    QMap<QString, QString> handleMap;// 设备按键对应键盘扫描码map; key: 设备-按键名称, value: 键盘扫描码

    inline static std::vector<MappingRelation> handleMultiBtnVector = {};// 当前在使用的 设备组合键映射列表
    inline static std::vector<MappingRelation> handleMultiBtnVectorUnsort = {};// 未排序的 设备组合键映射列表
    inline static std::vector<MappingRelation> handleMultiBtnVectorSorted = {};// 已排序的 设备组合键映射列表(根据组合键的子键数量倒序)

    QMap<QString, QString> keyHoldingMap;// 记录按键一直按着的map; key: 设备-按键名称, value: 键盘扫描码

    QMap<QString, TriggerTypeEnum> keyTriggerTypeMap;// 记录按键触发模式的map; key: 设备-按键名称, value: 键盘扫描码

    QMap<QString, MappingType> keyMappingTypeMap;// 记录按键映射类型的map; key: 设备-按键名称, value: 键盘扫描码

    std::vector<QString> rotateAxisList;// 记录需要反转的轴

    bool isMouseLeftHolding = false;// 鼠标左键一直按着
    bool isMouseRightHolding = false;// 鼠标右键一直按着

    PVIGEM_CLIENT vigemClient = nullptr;// ViGEm客户端
    PVIGEM_TARGET vigemTarget = nullptr;// 虚拟手柄对象
    XUSB_REPORT report;// 手柄模拟报告

    bool needStartVirtualXbox = false;// 是否需要启动虚拟xbox手柄

public:
    // 构造函数
    SimulateTask(QVector<MappingRelation> mappingList, const QVector<LPDIRECTINPUTDEVICE8>& readDataNeedInitedDeviceList);

    // 辅助功能-最长组合键优先模式发生改动
    static void changeEnableOnlyLongestMapping();

public slots:
    // 工作任务
    void doWork();

    // 设置改动的slot
    void settingsChangedSlot();

signals:
    // 任务结束信号
    void workFinished();

    void msgboxSignal(bool isError, QString msg);
    void startedSignal();
    void pauseClickSignal();

protected:
    // 处理设备数据结果, 整理数据
    void handleResult(QVector<MappingRelation>& res);

    // 是否反转轴
    bool isAxisRotate(QString btnName);

    // 映射是否有效
    bool isMappingValid(const MappingRelation& mapping){
        // 键盘按键值是否有效
        bool isKeyboardValueValid = false;

        auto valueList = mapping.keyboard_value.split(KEYBOARD_COMBINE_KEY_SPE);
        int validCounter = 0;
        if(valueList.size() > 0){
            for(const auto& value : valueList){
                bool ok;
                int valueInt = value.toInt(&ok);
                if(ok && valueInt != 0){
                    validCounter++;
                }
            }

            if(validCounter == valueList.size()){
                isKeyboardValueValid = true;
            }
        }

        return mapping.valid && !mapping.dev_btn_name.isEmpty() && isKeyboardValueValid;
    }

    // 添加映射到 按键模拟需要用的设备按键对应键盘扫描码map
    void addMappingToHandleMap(const MappingRelation& mapping);

    // 获取按键名称列表
    QVector<QString> getBtnStrListFromHandleMap(QString btnStr);

    // 按键是否一直在按下
    bool isCurrentKeyHolding(QString ketStr);

    ///
    /// \brief simulateKeyPress 模拟按键操作
    /// \param vkey 按键的硬件扫描码
    /// \param isKeyRelease 是否是松开按键, false代表按下按键, true代表松开按键
    ///
    void simulateKeyPress(short vkey, bool isKeyRelease);
    ///
    /// \brief simulateKeyPressMs 模拟按键按下和自动松开
    /// \param vkey 按键值
    /// \param pressMs 按键按下的持续时间, 超过持续时间自动松开按键
    ///
    void simulateKeyPressMs(short vkey, size_t pressMs);
    ///
    /// \brief simulateKeyDelayPressMs 按键等待一段时间后才按下, 之后自动松开按键
    /// \param vkey 按键
    /// \param pressMs 按键按下的持续时间, 持续时间结束后自动松开按键
    /// \param delayMs 按键按下的延迟时间, 延迟时间结束才会按下按键
    ///
    void simulateKeyDelayPressMs(short vkey, size_t pressMs, size_t delayMs);

    // 释放指定位置的所有按键
    void releaseAllKey(const QVector<MappingRelation>& pressBtnList);

    // 关闭HID设备
    void closeDevice();

    // 初始化xbox控制器
    bool initXboxController();
    // 关闭xbox控制器
    void closeXboxController();

    ///
    /// \brief simulateXboxKeyPress 模拟xbox按键操作
    /// \param inputType xbox输入的类型, 按键或者轴
    /// \param inputValue1 按键值 或者 轴的值
    /// \param inputValue2 备用
    /// \param isRelease 是否是松开按键, false代表按下按键, true代表松开按键
    ///
    void simulateXboxKeyPress(XboxInputType inputType, int inputValue1, int inputValue2, bool isRelease);
    ///
    /// \brief simulateXboxKeyPressMs 模拟xbox按键操作并自动松开按键
    /// \param pressMs 按键按下的持续时间, 超过持续时间自动松开按键
    ///
    void simulateXboxKeyPressMs(XboxInputType inputType, int inputValue1, int inputValue2, size_t pressMs);
    ///
    /// \brief simulateXboxKeyDelayPressMs 延迟按下xbox按键, 且自动松开按键
    /// \param delayMs 按键按下的延迟时间, 延迟时间结束才会按下按键
    ///
    void simulateXboxKeyDelayPressMs(XboxInputType inputType, int inputValue1, int inputValue2, size_t pressMs, size_t delayMs);

    // 根据设置的内部死区, 计算出xbox轴的最终值
    int calXboxSingleAxisFinalValue(double innerDeadAreaPer, double devAxisDataPer, int xboxAxisMaxValue, int xboxAxisMinValue, bool isDecrease);
};

