#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <windows.h>
#include <cstring>
#include <QVBoxLayout>
#include <QComboBox>
#include <device_info.h>
#include <mapping_relation.h>
#include "logwindow.h"
#include "deadareasettings.h"
#include "AssistFuncWindow.h"


#define MAX_STR 255
#define MAX_BUF 2048
#define SPE "#$#"
#define MAPPINGS_FILENAME "di_mappings_cache"
#define LAST_DEVICE_FILENAME "di_last_device"
#define USER_MAPPINGS_DIR "userMappings/"
#define MAPPING_FILE_SUFFIX ".di_mappings_config"
#define MAPPING_FILE_SUFFIX_XBOX ".di_xbox_mappings_config"
#define KEYBOARD "keyboard"
#define XBOX "xbox"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:

    MainWindow(QMainWindow *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    LogWindow *logWindow;// 日志窗口
    DeadAreaSettings *settings;// 死区设置窗口
    AssistFuncWindow *assistWindow;// 辅助功能窗口

    std::vector<MappingRelation*> mappingList;// 已配置的按键映射列表
    //std::vector<DeviceInfo*> deviceList;// 设备列表
    //short vid=0,pid=0;// 当前设备的vid,pid
    //hid_device *handle;// 当前设备的连接句柄
    //bool isDeviceOpen = false;// 当前设备是否打开
    std::string deviceDesc; // 设备信息
    std::string deviceName; // 设备名称
    std::string currentMappingFileName;// 当前配置文件的文件名

    QString appDataDirPath; // 软件本地数据存放的路径

    //unsigned char buf[MAX_BUF];
    //wchar_t wstr[MAX_STR];
    //int res;


    QLabel *label;

protected:
    MappingRelation* getDevBtnData();

    //int openDevice(short vid, short pid);

    //int closeDevice();

    //int listAllDevice();

    std::map<std::string, short> getConstKeyMap(std::string dev_btn_type);

    void scanMappingFile();

    // 上一次使用的设备是否在当前设备列表
    bool hasLastDevInCurrentDeviceList(std::string lastDeviceName);

    QComboBox* createAKeyBoardComboBox(std::string dev_btn_type);

    void showErrorMessage(std::string *text);

    bool hasAddToMappingList(std::string btn_name);

    void saveMappingsToFile(std::string filename);
    void saveLastDeviceToFile();

    void loadMappingsFile(std::string filename);

    void loadLastDeviceFile();
    void loadSettings();

    void repaintMappings();

    void paintOneLineMapping(MappingRelation *mapping, int index);

    // 清空配置列表界面
    void clearMappingsArea();

    // 获取配置列表实际映射数量
    int getMappingListActualSize();

    // 检查驱动
    bool checkDriverInstalled();

public slots:
    // 模拟服务报错的slot
    void simulateMsgboxSlot(bool isError, QString text);
    void simulateStartedSlot();
    void pauseClickSlot();

private slots:
    // 按键触发模式选择下拉框 槽函数
    void onTriggerTypeComboBoxActivated(int index);

    // 开启全局映射 按钮的槽函数
    void on_pushButton_2_clicked();

    // 点击 新增映射 按钮的槽函数
    void on_pushButton_clicked();

    // 键盘按键选择下拉框 槽函数
    void onKeyBoardComboBoxActivated(int index);

    // 设备选择下拉框, '选择后'信号对应的槽函数
    void on_comboBox_activated(int index);

    // 备注输入框内容变化 槽函数
    void onLineEditTextChanged(const QString &text);

    // 删除按钮点击的槽函数
    void onDeleteBtnClicked();

    // 反转轴的勾选框槽函数
    void onCheckBoxToggle(bool checked);

    // 保存配置按钮槽函数
    void on_pushButton_4_clicked();

    void on_comboBox_2_activated(int index);

    void on_radioButton_clicked();

    void on_radioButton_2_clicked();

    void on_pushButton_5_clicked();
    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_10_clicked();
};
#endif // MAINWINDOW_H
