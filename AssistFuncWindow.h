#ifndef ASSISTFUNCWINDOW_H
#define ASSISTFUNCWINDOW_H
#include "ForceFeedbackSettingsWindow.h"

#include <QMainWindow>

#define ASSIST_FUNC_SETTINGS "assist_func_settings.json"
#define ETS2_PATH "\\steamapps\\common\\Euro Truck Simulator 2\\"
#define ETS2_PLUGINS_DIR "plugins"

namespace Ui {
class AssistFuncWindow;
}

class AssistFuncWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit AssistFuncWindow(QWidget *parent = nullptr);
    ~AssistFuncWindow();

    static bool getEnableMappingAfterOpening();
    static bool getEnableOnlyLongestMapping();
    static bool getEnableOnlyChangeKeyWhenNew();

signals:
    void stopWork();
    void changeEnableOnlyLongestMappingSignal();
    void stopForceFeedbackSignal();
    void forceFeedbackSettingsChangeSignal();
    void saveLastDeviceToFileSignal();

private slots:
    void on_checkBox_stateChanged(int arg1);

    void on_checkBox_2_stateChanged(int arg1);

    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_checkBox_3_clicked();

    void on_pushButton_3_clicked();

    void onForceFeedbackSettingsChange();

    void on_checkBox_4_stateChanged(int arg1);

    void on_checkBox_4_clicked();

    void on_checkBox_5_clicked();

    void on_pushButton_4_clicked();

    void on_checkBox_7_clicked();

    void on_checkBox_8_clicked();

private:
    Ui::AssistFuncWindow *ui;

    ForceFeedbackSettingsWindow *forceFeedbackSettingsWindow;// 力反馈模拟设置窗口

    QString appDataDirPath;// 软件数据存放路径

    QString ETS2InstallPath;// 欧卡2安装目录

    static bool ETS2_enableAutoCancelHandbrake;// 欧卡2辅助功能_开启自动解除手刹
    static bool SYSTEM_enableMappingAfterOpening;// 映射软件的系统功能_开启软件后立即开启映射
    static bool SYSTEM_enableOnlyLongestMapping;// 组合键按下时只执行组合键映射，不执行对应子键映射
    static bool SYSTEM_enableOnlyChangeKeyWhenNew;// 新增映射时，只返回变化的按键
    bool SYSTEM_enableForceFeedback = false;// 开启力反馈模拟
    bool SYSTEM_enableRunUponStartup = false;// 开启开机自启动

    void scanETS2InstallPath();// 扫描欧卡2安装路径

    bool checkETS2Plugin();// 检查欧卡2的遥测数据插件

    void startAssistFuncWork();// 开启辅助功能任务
    void startForceFeedback();// 开启力反馈模拟任务

    void saveSettings();// 保存设置到文件
    void loadSettings();// 加载设置

    void unsave();// 改动未保存
    void save();// 改动已保存

    bool validateForceFeedbackParams();

    // 设置开机自启动(isSetStartup = true), 取消开机自启动(isSetStartup = false)
    void setRunUponStartup(bool isSetStartup);
};

#endif // ASSISTFUNCWINDOW_H
