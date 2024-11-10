#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <windows.h>
#include <hidapi.h>
#include <cstring>
#include <QVBoxLayout>
#include <QComboBox>
#include<device_info.h>
#include<mapping_relation.h>


using namespace std;

#define MAX_STR 255
#define MAX_BUF 2048
#define SPE "#$#"
#define MAPPINGS_FILENAME "mappings_cache"
#define LAST_DEVICE_FILENAME "last_device"
#define USER_MAPPINGS_DIR "userMappings/"
#define MAPPING_FILE_SUFFIX ".mappings_config"


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
    ~MainWindow(){};

protected:
    MappingRelation* getDevBtnData();

    int openDevice(short vid, short pid);

    int closeDevice();

    int listAllDevice();

    // 上一次使用的设备是否在当前设备列表
    bool hasLastDevInCurrentDeviceList(short lastDevVid, short lastDevPid);

    QComboBox* createAKeyBoardComboBox();

    void showErrorMessage(string *text);

    bool hasAddToMappingList(int btn_pos, int btn_value);

    void saveMappingsToFile(string filename);
    void saveLastDeviceToFile();

    void loadMappingsFile(string filename);

    void loadLastDeviceFile();

    void repaintMappings();

    void paintOneLineMapping(MappingRelation *mapping, int index);

    // 清空配置列表界面
    void clearMappingsArea();

    // 获取配置列表实际映射数量
    int getMappingListActualSize();

private slots:
    // 开启全局映射 按钮的槽函数
    void on_pushButton_2_clicked();

    // 停止映射 按钮的槽函数
    void on_pushButton_3_clicked();

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

    // 保存配置按钮槽函数
    void on_pushButton_4_clicked();

    void on_comboBox_2_activated(int index);

private:
    Ui::MainWindow *ui;

    vector<MappingRelation*> mappingList;// 已配置的按键映射列表
    vector<DeviceInfo*> deviceList;// 设备列表
    short vid=0,pid=0;// 当前设备的vid,pid
    hid_device *handle;// 当前设备的连接句柄
    bool isDeviceOpen = false;// 当前设备是否打开
    string deviceDesc; // 设备信息
    string deviceName; // 设备名称
    string currentMappingFileName;// 当前配置文件的文件名

    unsigned char buf[MAX_BUF];
    wchar_t wstr[MAX_STR];
    int res;


    QLabel *label;
};
#endif // MAINWINDOW_H
