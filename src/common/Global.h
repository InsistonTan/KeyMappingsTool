#pragma once

#include <dinput.h>
#include "qcoreapplication.h"
#include "qmutex.h"
#include "qpushbutton.h"
#include "models/MappingRelation.h"
#include <services/MessageBoxService.h>

#include <QDir>
#include <QVector>
#include <QQueue>
#include <QWidget>
#include <QMutex>


// 映射配置文件后缀
#define MAPPING_FILE_SUFFIX ".di_mappings_config"

// DirectInput 最大按键数
#define DINPUT_MAX_BUTTONS 128

#define POV_ONLY_FACTOR ( ((BUTTONS_VALUE_TYPE)((uint64_t)-1)).operator<<(DINPUT_MAX_BUTTONS) )
#define KEY_ONLY_FACTOR ( (((BUTTONS_VALUE_TYPE)((uint64_t)-1)).operator<<(64)) | ((BUTTONS_VALUE_TYPE)((uint64_t)-1)) )

// 键盘按键组合键的分隔符
#define KEYBOARD_COMBINE_KEY_SPE ", "

// 设备组合按键名称的分隔符, 例如 "按键0+按键1"
#define BUTTON_NAME_COMBINE_SPLIT "+"

// 设备十字键的角度连接符, 例如 "摇杆1-角度90"
#define ANGLE_NAME_CONNECT_STR "-"

///
/// \brief The Global class
/// 全局变量, 函数
///
class Global{
private:
    inline static int setTimeBeginPeriod5msCounter = 0;
    inline static QMutex counterMuteX;
public:
    // 软件名称
    inline const static QString APP_NAME = "KeyMappingsTool";
    // 旧版本的软件名称
    inline const static QString OLD_APP_NAME = "KeyMapping";
    // 当前软件版本号
    inline const static QString CURRENT_VERSION = "1.3.0";

    // 日志api
    inline const static QString LOGGER_API = "https://keymappingstoollogger.176334479.xyz";
    // 检查更新 api
    inline const static QString CHECK_UPDATE_API = "https://keymappingstoolupdate.176334479.xyz";

    //inline static MainWindow* g_mainWindow = nullptr;

    //识别轴需要变化的最小值
    inline const static int AXIS_CHANGE_VALUE = 2000;


    // 开启按键日志
    inline static bool enableBtnLog= false;
    // 开启轴日志
    inline static bool enableAxisLog= false;
    // 摇杆日志(十字键)
    inline static bool enablePovLog= false;



    // 设置系统定时器精度为 1ms
    static void setSystemTimePeriod_1ms();
    // 恢复系统计时器精度
    static void restoreSystemTimePeriod(bool forceRestore = false);

    // 显示错误弹窗并推送日志
    static void showErrorMsgBoxAndPushToLog(QString msg);

    // 获取按键/轴的完整名称(带设备名称的)
    static QString getBtnOrAxisFullName(QString deviceName, QString btnName);
    // 获取按键/轴的完整名称(带设备名称的)
    static std::string getBtnOrAxisFullName(std::string deviceName, std::string btnName);

    // 获取软件数据文件存储的路径
    static QString getAppDataDirStr();

    static QString getCrashLogPath();

    // 获取用户保存的映射配置文件目录
    static QString getUserMappingsFileDir();
    // 根据无后缀的映射配置文件名, 生成绝对路径
    static QString getUserMappingFilePath(QString fileBaseName);

    // 映射列表是否含有映射xbox的记录
    static bool hasXboxMappingInMappingList(const QVector<MappingRelation>& mappingList);

    // BUTTONS_VALUE_TYPE 转换为字符串
    static QString ButtonsValueTypeToString(BUTTONS_VALUE_TYPE btnValue);

    // string 转换为 BUTTONS_VALUE_TYPE
    static BUTTONS_VALUE_TYPE stringToButtonsValueType(const QString& btnValueStr);

    // 将浮点型的后面多余的0去掉, 例如 "1.500000" -> "1.5"
    static QString removeUnnecessaryZero(QString str);

    // 根据按键值转换成按键名称
    static QString getKeyNameFromKeyValue(const MappingRelation& mapping);

    static void setEnableBtnLog(bool val){
        enableBtnLog = val;
    }
    static bool getEnableBtnLog(){
        return enableBtnLog;
    }
    static void setEnableAxisLog(bool val){
        enableAxisLog = val;
    }
    static bool getEnableAxisLog(){
        return enableAxisLog;
    }
    static void setEnablePovLog(bool val){
        enablePovLog = val;
    }
    static bool getEnablePovLog(){
        return enablePovLog;
    }

    // 当前是否是深色主题
    static bool isDarkTheme();

    // 切换映射模式图标
    static void switchMappingTypeIcon(QPushButton *mappingTypeBtn, MappingRelation mapping);

    // 设置开机自启动
    static void setStartOnStartup(bool enable);

    //
    // 虚拟的隐藏窗口, 用于directInput初始化
    // 因为directInput初始化设备需要窗口的hwnd
    //
    class HiddenHostWindow : public QWidget
    {
    public:
        HiddenHostWindow()
        {
            setAttribute(Qt::WA_NativeWindow, true);
            setAttribute(Qt::WA_DontShowOnScreen, true);
            create();
        }

        HWND hwnd() const {
            return (HWND)winId();
        }
    };

    inline static HiddenHostWindow* hideWindow = nullptr;

    // 获取隐藏窗口的句柄
    static HWND getHideWindowHWnd();

    // 生成 设置项容器
    static QWidget* createSettingsItem(QWidget* parent,
                                       QString settingsName,
                                       QWidget* customWidget,
                                       QLabel* settingsDescLabel = nullptr,
                                       int fixedHeight = 0);
};

///
/// \brief The RunningStatus class
/// 全局映射的运行状态
///
class RunningStatus{
private:
    // 全局映射是否开启
    inline static bool isRunning = false;
    inline static QMutex mutex_isRunning;
    // 是否暂停全局映射
    inline static bool isPause = false;
    inline static QMutex mutex_isPause;

public:
    static void setIsRuning(bool val){
        QMutexLocker locker(&mutex_isRunning);
        isRunning = val;
    }
    static bool getIsRunning(){
        QMutexLocker locker(&mutex_isRunning);
        return isRunning;
    }
    // 点击了暂停按键
    static void clickPauseBtn(){
        QMutexLocker locker(&mutex_isPause);
        isPause = !isPause;
    }
    static bool getIsPause(){
        QMutexLocker locker(&mutex_isPause);
        return isPause;
    }
};



