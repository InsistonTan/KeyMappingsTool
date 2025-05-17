#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <windows.h>
#include <cstring>
#include <QVBoxLayout>
#include <QComboBox>
#include <device_info.h>
#include <mapping_relation.h>
#include "logwindow.h"
#include "AssistFuncWindow.h"
#include "DeadAreaSettings.h"

#define CURRENT_VERSION "1.2.3" // 当前版本号

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
#define AXIS_CHANGE_VALUE 2000 //识别轴需要变化的最小值

#define DEFAULT_API_HOST "https://xh36dstw.lc-cn-n1-shared.com" //默认的api域名
#define X_LC_Id "xH36dsTwk1T5XoXmO4EHA1qg-gzGzoHsz"
#define X_LC_Key "z5mrlT4AtWBnPLRFysmGKyNZ"

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
    // 获取当前选择的设备的下标
    static QList<QString> getCurrentSelectedDeviceList();

private:
    Ui::MainWindow *ui;

    LogWindow *logWindow;// 日志窗口
    AssistFuncWindow *assistWindow;// 辅助功能窗口
    DeadAreaSettings *deadareaSettings;// 死区设置窗口

    std::vector<MappingRelation*> mappingList;// 已配置的按键映射列表

    std::string deviceDesc; // 设备信息
    std::string deviceName; // 设备名称
    std::string currentMappingFileName;// 当前配置文件的文件名

    QString appDataDirPath; // 软件本地数据存放的路径

    static QList<QString> currentSelectedDeviceList; // 当前选择的设备列表

    QMap<QString, QString> mappingFileNameMap; // 用户映射配置文件map, key为无后缀文件名, value为文件绝对路径

    QLabel *label;

protected:
    // 初始化
    void init();

    MappingRelation* getDevBtnData();

    std::map<std::string, short> getConstKeyMap(std::string dev_btn_type, MappingType mappingType);

    void scanMappingFile();

    // 上一次使用的设备是否在当前设备列表
    bool hasLastDevInCurrentDeviceList(std::string lastDeviceName);

    QComboBox* createAKeyBoardComboBox(std::string dev_btn_type, MappingType mappingType);
    void updateAKeyBoardComboBox(QComboBox* comboBox, std::string dev_btn_type, MappingType mappingType);
    void updateASwitchPushButton(QPushButton* btn, MappingType mappingType);

    void showErrorMessage(std::string *text);

    bool hasAddToMappingList(MappingRelation* mapping);
    bool hasAddToMappingList(QString devBtnName, QString deviceName);

    QString saveMappingsToFile(std::string filename);
    void saveLastDeviceToFile(bool isOnlySaveLastDevice = false);

    void loadMappingsFile(std::string filename);

    void loadLastDeviceFile();
    void loadSettings();

    QString getMappingModeFromFile();

    void repaintMappings();

    void paintOneLineMapping(MappingRelation *mapping, int index);

    // 清空配置列表界面
    void clearMappingsArea();

    // 获取配置列表实际映射数量
    int getMappingListActualSize();

    // 检查驱动
    bool checkDriverInstalled();

    // 获取免费api LeanCloud 的访问域名
    void getApiHost(bool isSendUsage = true);
    // 发送软件使用统计
    void sendUsageCount(QString apiHost = "");
    // 检查软件是否有更新
    void checkUpdate(QString apiHost = "");

    // 启动全局映射后, 将部分控件设置为不可点击
    void disableUiAfterStartMapping();
    // 停止全局映射后, 将部分控件恢复正常状态
    void enableUiAfterStopMapping();

    // 更新已选择设备的label
    void updateSelectedDeviceLabel();

    // 获取省略模式的文本(文本超出组件显示范围将显示省略号...)
    QString getElidedText(QWidget* widget, QString srcText);

    bool hasSameNameMappingFile(QString newFileName);

public slots:
    // 模拟服务报错的slot
    void simulateMsgboxSlot(bool isError, QString text);
    void simulateStartedSlot();
    void pauseClickSlot();
    void saveLastDeviceToFileSlot();

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

    //void on_radioButton_clicked();

    //void on_radioButton_2_clicked();

    void on_pushButton_6_clicked();
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();
    void on_pushButton_9_clicked();
    void on_pushButton_10_clicked();
    void on_pushButton_11_clicked();
};
#endif // MAINWINDOW_H
