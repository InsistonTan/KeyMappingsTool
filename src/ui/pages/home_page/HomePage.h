#pragma once

#include "qcontainerfwd.h"
#include "qgridlayout.h"
#include "qpushbutton.h"
#include "qtmetamacros.h"
#include "qwidget.h"
#include "models/MappingRelation.h"
#include <dinput.h>
#include <QComboBox>
#include <QTableView>
#include <QWidget>

#include <models/MappingConfig.h>

#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

#include <common/StringConstants.h>

#include <ui/widgets/MultiSelectCombobox.h>
#include <ui/widgets/NoWheelComboBox.h>

class HomePage : public QWidget
{
    Q_OBJECT
public:
    explicit HomePage(QWidget *parent = nullptr);


private:
    // ================================================================
    // 中间的映射列表滚动区域
    QScrollArea *scrollArea;
    // 滚动区域的组件的布局
    QGridLayout *scrollAreaWidgetLayout;
    // 设备选择下拉框
    MultiSelectComboBox *deviceBox;
    // 配置选择下拉框
    NoWheelComboBox *cfgBox;
    // 已选择设备的label
    QLabel *selectedDevicesLabelHeader;
    // 已选择设备的label
    QLabel *selectedDevicesLabel;
    // 开启全局映射按钮
    QPushButton *startBtn;
    // 当前运行状态 的 "●" label
    QLabel* runningStateLabel;
    // 当前运行状态 的 文字描述 label
    QLabel* runningStateTextLabel;
    // 新增映射按钮
    QPushButton* addMappingBtn;
    // 刷新设备列表按钮
    QPushButton* refreshDeviceListBtn;
    // ================================================================


    // ================================================================
    // 映射列表表头
    inline static const QVector<QString> tableHeaders =
        {
            StringConstants::deviceButton,
            StringConstants::mappingMode,
            StringConstants::mappingKey,
            StringConstants::keyTriggerMode,
            StringConstants::remark,
            StringConstants::action
        };
    // ================================================================


    // ================================================================
    void initUI();
    void initData();
    // 加载上一次使用的设备
    void loadLastDeviceFile();
    // 扫描用户保存的映射配置文件
    void scanMappingFile();
    // 加载映射配置文件;
    // filename: 文件无后缀名 / 文件绝对路径
    // isAbsolutePath: false代表filename是文件无后缀名, true代表filename是文件绝对路径
    void loadMappingsFile(QString filename, bool isAbsolutePath = false);
    // 重绘映射列表
    void repaintMappings();
    // 获取配置列表实际映射数量
    int getMappingListActualSize();
    // lastDeviceName 是否在当前设备列表
    bool hasLastDevInCurrentDeviceList(QString lastDeviceName);
    // (已弃用)更新已选择设备的label
    //void updateSelectedDeviceLabel();
    // 清空配置列表界面
    void clearMappingsArea();
    // 映射列表区域新增一条映射;
    // isAddNewMapping == true 时, 说明是新增一条映射, 而不是加载历史配置
    void paintOneLineMapping(MappingRelation& srcMapping, bool isAddNewMapping);
    // 获取设备数据
    MappingRelation getDevBtnData();
    // 映射是否在映射列表里
    bool hasAddToMappingList(const MappingRelation& mapping);
    bool hasAddToMappingList(QString devBtnName, QString deviceName);
    // 创建一个 映射按键 的下拉框
    MultiSelectComboBox* createAKeyBoardComboBox(DeviceDataTypeEnum dev_btn_type,
                                                 MappingType mappingType);
    // 更新 映射按键 的下拉框
    void updateAKeyBoardComboBox(MultiSelectComboBox* comboBox,
                                 DeviceDataTypeEnum dev_btn_type,
                                 MappingType mappingType);
    // 准备设备, 根据选择的设备, 初始化设备, 获得已初始化的设备实例列表
    QVector<LPDIRECTINPUTDEVICE8> prepareDiDevices();
    // 获取键盘按键或xbox按键 的 按键名称与按键值映射表
    std::map<std::string, short> getConstKeyMap(DeviceDataTypeEnum dev_btn_type,
                                                MappingType mappingType);
    // 检查驱动
    bool checkDriverInstalled();
    // 保存当前使用的映射 和 设备 到配置文件
    void saveCurrentMappingsAndDeviceToFile();
    // 获取当前选择的设备名称列表
    QVector<QString> getCurrentSelectedDeviceList();
    // 保存映射列表到文件
    //QString saveMappingsToFile(QString filename);
    // ================================================================



    // ================================================================
    // 根据列的组件, 创建行组件.......
    QWidget* createRow(QVector<QWidget*> colWidgets);
    // 更新运行状态
    void updateRunningStateText();
    // 绘制表头
    void drawHeaders();
    // QGridLayout删除某一行
    bool deleteRowFromGridLayout(QGridLayout* grid, QString objectName);
    // 获取QGridLayout行数
    int getGridLayoutRowCount(QGridLayout* grid);
    // ================================================================

signals:
    void settingsChangedSignal();
    void currentSelectedMappingFileChanged();

public slots:
    // 开启全局映射 按钮的槽函数
    void on_startBtn_clicked();
    // 用户设置改变的slot
    void settingsChangedSlot();
};
