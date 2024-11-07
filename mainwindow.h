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

    QComboBox* createAKeyBoardComboBox();

    void showErrorMessage(string *text);

    bool hasAddToMappingList(int btn_pos, int btn_value);

    void saveMappingsToFile();

    void loadMappingsFile();

    void loadLastDeviceFile();

    void repaintMappings();

    void paintOneLineMapping(MappingRelation *mapping, int index);

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

private:
    Ui::MainWindow *ui;

    vector<MappingRelation*> mappingList;// 已配置的按键映射列表
    vector<DeviceInfo*> deviceList;// 设备列表
    short vid,pid;// 当前设备的vid,pid
    hid_device *handle;// 当前设备的连接句柄
    bool isDeviceOpen = false;// 当前设备是否打开
    string deviceDesc; // 设备信息
    string deviceName; // 设备名称

    unsigned char buf[MAX_BUF];
    wchar_t wstr[MAX_STR];
    int res;


    QLabel *label;
};
#endif // MAINWINDOW_H
