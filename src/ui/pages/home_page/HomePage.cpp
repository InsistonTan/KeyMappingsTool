#include "HomePage.h"
#include "qboxlayout.h"
#include "qcoreapplication.h"
#include "qdialog.h"
#include "qdir.h"
#include "qheaderview.h"
#include "qjsondocument.h"
#include "qlabel.h"
#include "qlineedit.h"
#include "qmutex.h"
#include "qobject.h"
#include "qpushbutton.h"
#include "qwidget.h"
#include "common/KeyMap.h"
#include "models/MappingRelation.h"
#include "services/MessageBoxService.h"
#include "ui/mainwindow/MainWindow.h"
#include "ui/pages/forcefeedback_simulate_page/ForceFeedbackSimulatePage.h"
#include "ui/pages/settings_page/SettingsPage.h"
#include "ui/widgets/CardMessageDialog.h"
#include "ui/widgets/NoWheelComboBox.h"

#include <QComboBox>
#include <QDir>
#include <QFrame>
#include <QLabel>
#include <QPushButton>
#include <QStandardItemModel>
#include <QTableView>
#include <QVBoxLayout>
#include <QFontMetrics>
#include <QCheckBox>
#include <QSettings>
#include <QProcess>
#include <QInputDialog>
#include <QScrollBar>
#include <QTimer>

#include <services/ConfigService.h>
#include <services/DirectInputService.h>
#include <services/LogService.h>

#include <common/Global.h>
#include <common/StringConstants.h>
#include <common/Theme.h>

#include <ui/widgets/MultiSelectCombobox.h>

#include <workers/SimulateTask.h>
#include <winsock.h>

HomePage::HomePage(QWidget *parent)
    : QWidget{parent}
{
    initUI();
    QTimer::singleShot(0,this,[this](){
        initData();
    });
}

QVector<QString> HomePage::getCurrentSelectedDeviceList()
{
    return deviceBox->selectedItems().toList();
}

void HomePage::initUI()
{
    // =========================
    // 主内容区
    // =========================
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(Theme::contentMargin,Theme::contentMargin,Theme::contentMargin,Theme::contentMargin);

    // =========================
    // TopBar 顶部区域
    // =========================
    int fixedHeight = 32;
    QFrame *topBar = new QFrame;
    Theme::setQFrameStyleSheet(topBar);
    QVBoxLayout *topLayout = new QVBoxLayout(topBar);

    // ===============
    // 顶部区域, 第一行
    // ===============
    QWidget *top_line1 = new QWidget;
    Theme::setQWidgetStyleSheet(top_line1);

    QHBoxLayout *top_line1_layout = new QHBoxLayout(top_line1);

    QLabel *deviceLabel = new QLabel(StringConstants::device + ":");

    // 设备选择下拉框
    deviceBox = new MultiSelectComboBox();
    // 设置样式
    Theme::setComboBoxStyleSheet(deviceBox, 200);

    // 刷新按钮
    refreshDeviceListBtn = new QPushButton(StringConstants::refresh);
    Theme::setButtonStyleSheet(refreshDeviceListBtn, ButtonLevel::normal);
    connect(refreshDeviceListBtn, &QPushButton::clicked, this, [=](){
        // 清空下拉
        deviceBox->clear();

        // 重新扫描设备
        DirectInputService::scanDevice();

        // 新的设备信息列表
        auto diDeviceList = DirectInputService::getDeviceInfoListSnapshot();

        // 设备不为空
        if(!diDeviceList.empty()){
            deviceBox->setPlaceholderText(StringConstants::pleaseSelectDevice);
            for(const auto& item : diDeviceList){
                deviceBox->addItem(item.name.data());
            }
        }else{
            deviceBox->setPlaceholderText(StringConstants::noDeviceScan);
        }
    });

    QLabel *cfgLabel = new QLabel(StringConstants::config + ":");

    // 配置选择 下拉框
    cfgBox = new NoWheelComboBox();
    Theme::setComboBoxStyleSheet(cfgBox, 130);
    cfgBox->addItem(StringConstants::blankMappings);
    connect(cfgBox, &QComboBox::activated, this, [=](){
        // 全局映射正在运行, 不能切换映射配置
        if(RunningStatus::getIsRunning()){
            if(ConfigService::currentMappingFileName.isEmpty()){
                cfgBox->setCurrentIndex(-1);
            }else{
                cfgBox->setCurrentText(ConfigService::currentMappingFileName);
            }

            MessageBoxService::showError(StringConstants::error_pleaseStopKeyMappingRunning);
            return;
        }

        {
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            // 清空
            ConfigService::currentMappingConfig.clear();
        }

        // 清空
        clearMappingsArea();

        // 重绘表头
        drawHeaders();

        // 清空
        ConfigService::currentMappingFileName = "";

        // 选择了空白配置 (下拉框第一项永远是空白配置)
        if(cfgBox->currentIndex() == 0){
            // 发送信号
            emit currentSelectedMappingFileChanged();
            return;
        }

        // 获取配置文件名称
        auto filename = cfgBox->currentText();
        // 配置没变, 不需要做后续操作
        if(ConfigService::currentMappingFileName == filename){
            return;
        }

        // 设置当前配置文件名
        ConfigService::currentMappingFileName = filename;
        // 加载配置
        loadMappingsFile(filename);
        // 重画配置映射界面
        repaintMappings();
        // 发送信号
        emit currentSelectedMappingFileChanged();
    });


    //固定高度
    deviceLabel->setFixedHeight(fixedHeight);
    deviceBox->setFixedHeight(fixedHeight);
    refreshDeviceListBtn->setFixedHeight(fixedHeight);
    cfgLabel->setFixedHeight(fixedHeight);
    cfgBox->setFixedHeight(fixedHeight);

    top_line1_layout->addWidget(deviceLabel);
    top_line1_layout->addWidget(deviceBox);
    top_line1_layout->addWidget(refreshDeviceListBtn);
    top_line1_layout->addStretch();
    top_line1_layout->addWidget(cfgLabel);
    top_line1_layout->addWidget(cfgBox);

    topLayout->addWidget(top_line1);

    // =========================
    // 中间 Table
    // =========================
    QFrame *centerFrame = new QFrame;
    Theme::setQFrameStyleSheet(centerFrame);
    QVBoxLayout *centerLayout = new QVBoxLayout(centerFrame);

    // 映射列表的滚动区域
    scrollArea = new QScrollArea(this);
    scrollArea->setWidgetResizable(true);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setAlignment(Qt::AlignTop);
    Theme::setScrollBarStyleSheet(scrollArea);

    // 滚动区域容器
    QWidget *scrollAreaWidget = new QWidget;
    Theme::setQWidgetStyleSheet(scrollAreaWidget);

    // 滚动区域容器的布局
    scrollAreaWidgetLayout = new QGridLayout(scrollAreaWidget);
    scrollAreaWidgetLayout->setHorizontalSpacing(8);
    scrollAreaWidgetLayout->setVerticalSpacing(6);
    scrollAreaWidgetLayout->setAlignment(Qt::AlignTop);
    scrollAreaWidgetLayout->setContentsMargins(12, 12, 12, 12);

    // 绘制表头
    drawHeaders();

    // 滚动区域设置容器
    scrollArea->setWidget(scrollAreaWidget);

    // 把滚动区域添加到中间布局
    centerLayout->addWidget(scrollArea);

    // ===========================
    // 在底部区域上方新增一行
    // ===========================
    QFrame* bottomAreaAheadFrame = new QFrame;
    Theme::setQFrameStyleSheet(bottomAreaAheadFrame);
    QHBoxLayout* bottomAreaAheadLayout = new QHBoxLayout(bottomAreaAheadFrame);
    bottomAreaAheadLayout->setSpacing(8);
    bottomAreaAheadLayout->setContentsMargins(16,16,16,16);

    // 新增按键映射 按钮
    addMappingBtn = new QPushButton(StringConstants::addMapping);
    Theme::setButtonStyleSheet(addMappingBtn, ButtonLevel::normal);
    connect(addMappingBtn, &QPushButton::clicked, this, [=](){
        // 未选择设备
        if(getCurrentSelectedDeviceList().isEmpty()){
            MessageBoxService::showError(StringConstants::error_deviceNotSelect);
            return;
        }

        if(RunningStatus::getIsRunning()){
            MessageBoxService::showError(StringConstants::error_pleaseStopKeyMappingRunning);
            return;
        }

        // 禁用按钮，防止重复点击
        addMappingBtn->setEnabled(false);
        addMappingBtn->setText(StringConstants::btnText_addNewMapping_waitingDeviceInput);

        MappingRelation invalidMapping(false);
        // 新增一条映射
        paintOneLineMapping(invalidMapping, true);

        // 处理一下qt事件, 保证后面的滚动条能正常滚到底部
        QCoreApplication::processEvents();

        // 滑动条滑到最底部
        QScrollBar *sbar = scrollArea->verticalScrollBar();
        sbar->setValue(sbar->maximum());

        // 恢复按钮状态
        addMappingBtn->setText(StringConstants::btnText_addNewMapping_default);
        addMappingBtn->setEnabled(true);
    });
    // 保存当前映射
    QPushButton *saveBtn = new QPushButton(StringConstants::saveCurrentConfig);
    Theme::setButtonStyleSheet(saveBtn, ButtonLevel::normal);
    connect(saveBtn, &QPushButton::clicked, this, [=](){
        if(getMappingListActualSize() <= 0){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::error_mappingsListIsEmpty);
            return;
        }

        QString srcFileName;
        // 如果当前选择的配置不是下拉框的第一项 "空白配置"
        if(cfgBox->currentIndex() != 0){
            // 当前选择的配置文件名(无后缀)
            srcFileName = cfgBox->currentText();
            // if(!selectedFileShortName.isEmpty()){
            //     auto filePath = ConfigService::mappingFileNameMap[selectedFileShortName];
            //     QFile file(filePath);
            //     if(file.exists()){
            //         QFileInfo fileInfo(file);
            //         srcFileName = fileInfo.completeBaseName();
            //     }
            // }
        }

        // 输入框
        QLineEdit* lineEdit = new QLineEdit(srcFileName, this);
        Theme::setLineEditStyleSheet(lineEdit);

        // 自定义的卡片式弹窗
        CardMessageDialog inputDialog(CardMessageDialog::Type::Info,
                                StringConstants::saveMappings,
                                StringConstants::pleaseEnterMappingsName,
                                lineEdit,
                                {},
                                nullptr);
        if(inputDialog.exec() == QDialog::Accepted){
            QString text = lineEdit->text();
            if(!text.isEmpty()){
                // 保存配置
                QString saveFileShortName = ConfigService::saveCurrentMappingConfigToFile(text);
                // 保存失败
                if(saveFileShortName.isEmpty()){
                    return;
                }

                // 重新扫描配置文件
                scanMappingFile();
                // 设置本配置为当前选择的配置
                cfgBox->setCurrentText(saveFileShortName);
                // 重置当前配置文件名
                ConfigService::currentMappingFileName = text;
                // 保存成功提醒
                MessageBoxService::showSuccess(StringConstants::saveMappingsSuccess);
            }else {
                Global::showErrorMsgBoxAndPushToLog(StringConstants::mappingsNameNotAllowEmpty);
                return;
            }
        }
    });
    // 管理配置
    QPushButton *manageBtn = new QPushButton(StringConstants::manageConfig);
    Theme::setButtonStyleSheet(manageBtn, ButtonLevel::normal);
    connect(manageBtn, &QPushButton::clicked, this, [=](){
        //获取用户映射配置文件目录
        QDir dir(Global::getUserMappingsFileDir());

        std::string dirPath = QDir::toNativeSeparators(dir.absolutePath()).toStdString();
        std::string command = "explorer \"" + dirPath + "\"";  // 使用双引号包裹路径，处理空格

        // 将窗口最小化
        showMinimized();

        // 使用 QProcess 执行命令, 打开资源管理器
        QProcess::startDetached(command.data());
    });

    bottomAreaAheadLayout->addWidget(addMappingBtn);
    bottomAreaAheadLayout->addWidget(saveBtn);
    bottomAreaAheadLayout->addWidget(manageBtn);
    bottomAreaAheadLayout->addStretch();


    // =========================
    // BottomBar 底部区域
    // =========================
    QFrame *bottomBar = new QFrame;
    Theme::setQFrameStyleSheet(bottomBar);

    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(16,16,16,16);

    // 当前运行状态
    QLabel *runningStateHeader = new QLabel(StringConstants::currentState + ": ");
    runningStateLabel = new QLabel("●");
    runningStateTextLabel = new QLabel(StringConstants::notStart);

    // 左边分组, 用于显示运行状态
    QWidget* leftGroupWidget = new QWidget;
    leftGroupWidget->setStyleSheet(QString(R"(
        QWidget{
            padding: 0px;
            background-color: %1;
            color: %2;
        }
    )").arg(Theme::panelBg(),Theme::textColor()));

    QHBoxLayout *leftLayout = new QHBoxLayout(leftGroupWidget);
    leftLayout->setContentsMargins(0,0,0,0);
    leftLayout->setSpacing(2);
    leftLayout->addWidget(runningStateHeader);
    leftLayout->addWidget(runningStateLabel);
    leftLayout->addWidget(runningStateTextLabel);

    // 开启全局映射 按钮
    startBtn = new QPushButton(StringConstants::startKeyMappingsRunning);
    Theme::setButtonStyleSheet(startBtn, ButtonLevel::success);
    connect(startBtn, &QPushButton::clicked, this, &HomePage::on_startBtn_clicked);

    // 添加组件
    bottomLayout->addWidget(leftGroupWidget);
    bottomLayout->addStretch();
    bottomLayout->addWidget(startBtn);

    // =========================
    // 组装 mainArea
    // =========================
    mainLayout->addWidget(topBar);
    mainLayout->addWidget(centerFrame, 1);
    mainLayout->addWidget(bottomAreaAheadFrame);
    mainLayout->addWidget(bottomBar);

    // 更新 当前运行状态 的样式
    updateRunningStateText();
}

void HomePage::initData()
{
    // 初始化directInput并扫描设备
    DirectInputService::initDirectInput();
    DirectInputService::scanDevice();

    // 获取设备信息列表
    auto diDeviceList = DirectInputService::getDeviceInfoListSnapshot();

    // 使用迭代器遍历 设备信息列表
    for (int i=0; i< (int)diDeviceList.size(); i++) {
        deviceBox->addItem(diDeviceList[i].name.data());
    }
    deviceBox->setCurrentIndex(-1);
    if(diDeviceList.empty()){
        deviceBox->setPlaceholderText(StringConstants::noDeviceScan);
    }else{
        deviceBox->setPlaceholderText(StringConstants::pleaseSelectDevice);
    }

    // 加载上一次使用的设备
    loadLastDeviceFile();

    // 扫描用户保存的映射配置文件
    scanMappingFile();
    // 默认选择空白配置
    cfgBox->setCurrentIndex(0);

    // 用户软件设置
    auto cfg = ConfigService::getGlobalUserConfig();
    // 如果 上一次使用的映射配置文件路径为空, 则加载映射列表缓存
    // 否则 加载上一次的映射配置文件
    if(cfg.SYSTEM_lastUsedMappingConfigPath.isEmpty()){
        {
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            ConfigService::currentMappingConfig.mappingList = cfg.SYSTEM_lastUsedMappingCache;
            // 显示 "上一次使用的配置"
            cfgBox->setPlaceholderText(StringConstants::lastUsedMappings);
            cfgBox->setCurrentIndex(-1);
        }

        // 上一次使用的配置也为空
        if(ConfigService::getCurrentMappingConfig().mappingList.isEmpty()){
            // 选择默认的 "空白配置"
            cfgBox->setCurrentIndex(0);
        }
    }else{
        // 加载上一次使用的映射配置文件
        loadMappingsFile(cfg.SYSTEM_lastUsedMappingConfigPath, true);

        // 无后缀文件名
        QString baseName = QFileInfo(cfg.SYSTEM_lastUsedMappingConfigPath).baseName();
        // 下拉框有这个文件, 并且加载的配置不为空, 自动选择这个配置文件
        if(cfgBox->findText(baseName) != -1 && getMappingListActualSize() > 0){
            cfgBox->setCurrentText(baseName);
            ConfigService::currentMappingFileName = baseName;
        }
    }

    // 绘制出配置列表
    repaintMappings();

    // 下拉框默认
    if(getMappingListActualSize() > 0){
        cfgBox->setPlaceholderText(StringConstants::lastUsedMappings);
    }

    // 如果开启了 "打开软件后立即开启映射", 开启全局映射
    if(cfg.SYSTEM_enableMappingAfterOpening){
        // 1秒后启动全局映射
        QTimer::singleShot(1000, [this](){
            QCoreApplication::processEvents();
            on_startBtn_clicked();
        });
    }

}

void HomePage::loadLastDeviceFile()
{
    auto lastUsedDeviceNameList = ConfigService::get().SYSTEM_lastUsedDeviceNameList;
    for(const auto& lastDeviceName : lastUsedDeviceNameList){
        // lastDeviceName 在当前设备列表中, 自动选择该设备
        if(hasLastDevInCurrentDeviceList(lastDeviceName)){
            // 输出日志
            LogService::pushToLogQueue(StringConstants::lastDeviceInCurrentDeviceList.arg(lastDeviceName));

            // 将上一次使用的设备添加到当前选择的设备列表
            deviceBox->setItemSelected(lastDeviceName);

            auto list = deviceBox->selectedItems();
            qDebug() << "1";
        }else{
            LogService::pushToLogQueue(StringConstants::lastDeviceNotInCurrentDeviceList.arg(lastDeviceName));
        }
    }

    // 更新已选择设备label
    updateSelectedDeviceLabel();
}

void HomePage::scanMappingFile()
{
    cfgBox->clear();
    cfgBox->addItem(StringConstants::blankMappings);
    //ConfigService::mappingFileNameMap.clear();

    QDir dir(Global::getUserMappingsFileDir());

    // 获取文件列表，并筛选指定后缀的文件
    QFileInfoList list = dir.entryInfoList();

    for (const QFileInfo &fileInfo : list) {
        // 匹配指定后缀的文件，加入下拉列表
        if ("." + fileInfo.suffix() == MAPPING_FILE_SUFFIX) {

            // 无后缀的文件名
            QString shortName = fileInfo.completeBaseName();

            if(!shortName.isEmpty()){
                cfgBox->addItem(shortName);
                //ConfigService::mappingFileNameMap[shortName] = fileInfo.absoluteFilePath();
            }
        }
    }

}

void HomePage::loadMappingsFile(QString filename, bool isAbsolutePath)
{
    // 映射配置文件名/路径为空
    if(filename.isEmpty()){
        LogService::parseErrorLog(StringConstants::error_mappingFilenameEmpty);
        return;
    }

    // 文件的绝对路径
    QString filePath;

    // filename是否是绝对路径
    if(isAbsolutePath){
        filePath = filename;
    }else{
        filePath = Global::getUserMappingFilePath(filename);
        // // 从map中根据文件名获取文件的绝对路径
        // if(ConfigService::mappingFileNameMap.contains(filename)){
        //     filePath = ConfigService::mappingFileNameMap[filename];
        // }else{
        //     //获取该文件绝对路径失败
        //     MessageBoxService::showError(StringConstants::error_loadConfigFileFailed_getPathFailed.arg(filename));
        //     return;
        // }
    }


    QFile file(filePath);

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在:" << filePath;
        return;
    }

    // 新版配置文件以json格式存储
    // 尝试以json文件格式读取, 如果读取失败再使用旧版配置文件的格式读取
    // 打开 JSON 文件
    if (!file.open(QIODevice::ReadOnly)) {
        LogService::parseWarningLog(StringConstants::openFileFailed.arg(filePath));
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();

    QJsonDocument doc = QJsonDocument::fromJson(jsonData);

    // 解析json, 生成映射配置对象
    auto newConfig = MappingConfig::fromJson(doc);

    QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
    // 设置新配置
    ConfigService::currentMappingConfig = newConfig;
}

void HomePage::repaintMappings()
{
    //qDebug("mappingList.size() : %d", getMappingListActualSize());

    if(getMappingListActualSize() > 0){
        // 清空配置页面
        clearMappingsArea();

        // 重绘表头
        drawHeaders();

        auto mappingList = ConfigService::getCurrentMappingList();

        // 重新添加
        for(int i =0; i < mappingList.size(); i++){
            if(mappingList[i].valid){
                paintOneLineMapping(mappingList[i], false);
            }
        }
    }
}

int HomePage::getMappingListActualSize()
{
    int size = 0;
    for(const auto& item : ConfigService::getCurrentMappingList()){
        if(item.valid){
            size++;
        }
    }

    return size;
}

bool HomePage::hasLastDevInCurrentDeviceList(QString lastDeviceName)
{
    if(lastDeviceName.isEmpty()){
        return false;
    }

    // 获取设备信息列表
    auto diDeviceList = DirectInputService::getDeviceInfoListSnapshot();

    for(const auto& item : diDeviceList){
        if(lastDeviceName.toStdString() == item.name){
            return true;
        }
    }

    return false;
}

void HomePage::updateSelectedDeviceLabel()
{
    //QString labelText = "";

    // int num = currentSelectedDeviceList.size();

    // selectedDevicesLabelHeader->setText(
    //     StringConstants::selectedDeviceCount
    //     + ": "
    //     + (num == 0
    //         ? "<b style='color: rgb(160, 0, 0);'>0</b>"
    //         : "<b style='color: rgb(0, 160, 0);'>"+QString::number(num)+"</b>")
    //     );


    // if(currentSelectedDeviceList.size() == 0){
    //     selectedDevicesLabelHeader->setText(StringConstants::selectedDevice + "(<b style='color: rgb(160, 0, 0);'>0</b>):");
    //     selectedDevicesLabel->setText(StringConstants::hasNone);
    //     selectedDevicesLabel->setStyleSheet("");
    //     selectedDevicesLabel->setToolTip("");
    // }else{
    //     for(const auto& str : currentSelectedDeviceList){
    //         labelText.append(str).append(", ");
    //     }
    //     int index = labelText.lastIndexOf(", ");
    //     if(index >= 0){
    //         labelText = labelText.left(index);
    //     }

    //     selectedDevicesLabelHeader->setText(QString(StringConstants::selectedDevice + "(<b style='color: rgb(0, 160, 0);'>") + std::to_string(currentSelectedDeviceList.size()).data() + "</b>):");

    //     // 获取 QFontMetrics 对象
    //     QFontMetrics metrics(selectedDevicesLabel->font());
    //     int labelHeight = selectedDevicesLabel->height(); // 标签高度（像素）
    //     int lineHeight = metrics.height(); // 单行高度
    //     int maxLines = labelHeight/lineHeight; // 计算最大行数
    //     // 设置省略模式
    //     selectedDevicesLabel->setText(
    //         metrics.elidedText(labelText, Qt::ElideLeft,
    //                            selectedDevicesLabel->width() * maxLines));

    //     selectedDevicesLabel->setStyleSheet("QLabel{color: rgb(0, 160, 0);}");
    //     selectedDevicesLabel->setToolTip(labelText.replace(", ", "<br>"));
    // }

}

void HomePage::clearMappingsArea()
{
    // 遍历滚动区域容器的布局中的所有项
    while (QLayoutItem *item = scrollAreaWidgetLayout->takeAt(0)) {
        // 如果该项是一个 QWidget，删除它
        if (QWidget *widget = item->widget()) {
            // 异步删除以防止立即删除导致问题
            widget->deleteLater();
        }
        // 删除布局项
        delete item;
    }
}

void HomePage::paintOneLineMapping(MappingRelation &srcMapping, bool isAddNewMapping)
{
    // 新增映射
    if(isAddNewMapping){
        // 获取设备按下的按键位置和值
        MappingRelation newMapping = getDevBtnData();

        // 映射无效
        if(newMapping.valid == false){
            return;
        }

        // 检查该设备按键是否已经添加
        if(hasAddToMappingList(newMapping)){
            Global::showErrorMsgBoxAndPushToLog(
                StringConstants::error_alreadyExistsMapping.arg(
                    newMapping.deviceName,
                    newMapping.dev_btn_name.data()));
            return;
        }

        {
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            // 将设备按键数据添加进新的映射关系
            ConfigService::currentMappingConfig.mappingList.append(newMapping);
        }
    }

    // 确定绘制的 是新增的映射 还是 历史配置的映射
    // 当前映射的对象
    MappingRelation currentMapping = isAddNewMapping ? ConfigService::getCurrentMappingList().back() : srcMapping;

    // 该映射无效
    if(currentMapping.valid == false || currentMapping.dev_btn_name.isEmpty()){
        return;
    }

    // 当前行的所有列的组件指针列表
    QVector<QWidget*> cols;

    // 当前行号
    int currentRow = getGridLayoutRowCount(scrollAreaWidgetLayout) + 1;

    // 设备按键名称, "设备名称-按键名称"
    // 用于在映射列表查找当前映射进行修改, 以及用于在 QGridLayout 中查找当前行的组件进行删除
    QString deviceNameAndBtnName = Global::getBtnOrAxisFullName(currentMapping.deviceName,
                                                                currentMapping.dev_btn_name);

    // 设备按键名称
    QString devNameText = currentMapping.deviceName;
    QLabel *devNamelabel = new QLabel(devNameText.length() > 8
                                          ? devNameText.left(4) + "..." + devNameText.right(4)
                                          : devNameText);
    devNamelabel->setToolTip(devNameText);

    // 设备按键名称
    QString devBtnText = currentMapping.dev_btn_name;
    QLabel *devBtnlabel = new QLabel(devBtnText);
    devBtnlabel->setToolTip(devBtnText);


    // ===========================================================================
    // 键盘按键下拉框
    // ===========================================================================
    MultiSelectComboBox *keyBoardComboBox = createAKeyBoardComboBox(currentMapping.dev_btn_type,
                                                                    currentMapping.mappingType);
    Theme::setComboBoxStyleSheet(keyBoardComboBox, 140);
    // 历史配置展示
    if(!isAddNewMapping){
        auto keyNameListString = currentMapping.keyboard_name;
        auto keyNameList = keyNameListString.split(KEYBOARD_COMBINE_KEY_SPE);
        for(const auto& keyName : keyNameList){
            keyBoardComboBox->setItemSelected(keyName);
        }
    }

    // 连接信号和槽
    connect(keyBoardComboBox, &MultiSelectComboBox::selectionChanged, this, [=](){
        // 获取下拉框已选择的选项
        auto selectedItemList = keyBoardComboBox->selectedItems();

        QVector<std::map<std::string, short>> mapList = {VK_MAP, VK_XBOX_BTN_MAP, VK_XBOX_AXIS_MAP};

        // 已选择项对于的键盘扫描码列表
        QStringList keyboardValueList;
        for(auto& selected : selectedItemList){
            for(auto map : mapList){
                // 在键盘 按键名称与虚拟值 map中根据名称查找出值, 并更新到已配置的mapping中
                auto item = map.find(selected.toStdString());
                if (item != map.end()) {
                    keyboardValueList.append(std::to_string(item->second).data());
                    break;
                }
            }
        }

        {
            // 加锁修改
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            for(int i=0; i<ConfigService::currentMappingConfig.mappingList.size(); i++){
                auto item = ConfigService::currentMappingConfig.mappingList[i];
                auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,item.dev_btn_name);
                if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                    ConfigService::currentMappingConfig.mappingList[i].keyboard_name
                        = selectedItemList.size() > 0
                            ? selectedItemList.join(KEYBOARD_COMBINE_KEY_SPE)
                            : "";
                    ConfigService::currentMappingConfig.mappingList[i].keyboard_value
                        = keyboardValueList.size() > 0
                            ? keyboardValueList.join(KEYBOARD_COMBINE_KEY_SPE)
                            : "";
                }
            }
        }
    });


    // ===========================================================================
    // 按键触发模式下拉框
    // ===========================================================================
    NoWheelComboBox *triggerTypeComboBox = new NoWheelComboBox();
    Theme::setComboBoxStyleSheet(triggerTypeComboBox, 140);

    // 添加下拉框选择项
    for(int i = 0; i < TriggerTypeEnum::End; i++){
        TriggerTypeEnum enumItem = static_cast<TriggerTypeEnum>(i);
        triggerTypeComboBox->addItem(TRIGGER_TYPE_ENUM_MAP[enumItem].data());
    }

    // 历史配置展示
    if(!isAddNewMapping){
        triggerTypeComboBox->setCurrentText(TRIGGER_TYPE_ENUM_MAP[currentMapping.btnTriggerType].data());
    }

    // 连接信号和槽
    connect(triggerTypeComboBox, &QComboBox::activated, this, [=](){
        // 加锁修改
        QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
        for(int i=0; i<ConfigService::currentMappingConfig.mappingList.size(); i++){
            auto item = ConfigService::currentMappingConfig.mappingList[i];
            auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,
                                                                          item.dev_btn_name);
            if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                // 更新选择的触发模式
                ConfigService::currentMappingConfig.mappingList[i].btnTriggerType = static_cast<TriggerTypeEnum>(triggerTypeComboBox->currentIndex());
            }
        }
    });


    // ===========================================================================
    // 映射模式切换按钮
    // ===========================================================================
    QPushButton *mappingTypeBtn = new QPushButton;
    mappingTypeBtn->setCheckable(true);
    mappingTypeBtn->setCursor(Qt::PointingHandCursor);
    mappingTypeBtn->setIconSize(QSize(24, 24));
    mappingTypeBtn->setContentsMargins(0, 0, 0, 0);
    mappingTypeBtn->setStyleSheet(R"(
        QPushButton {
            margin: 0px;
            padding: 0px;
            background-color: transparent;
            border: 1px solid rgba(148, 163, 184, 0.18);
            border-radius: 6px;
        }

        QPushButton:hover {
            background-color: rgba(148, 163, 184, 0.12);
            border: 1px solid rgba(148, 163, 184, 0.30);
        }

        QPushButton:pressed {
            background-color: rgba(148, 163, 184, 0.18);
        }
    )");

    if (QString(currentMapping.dev_btn_name.data()).contains(StringConstants::text_wheelRight)
        || QString(currentMapping.dev_btn_name.data()).contains(StringConstants::text_wheelLeft)) {
        mappingTypeBtn->setEnabled(false);

    }

    // 显示图标
    Global::switchMappingTypeIcon(mappingTypeBtn, currentMapping);
    // 连接信号和槽
    connect(mappingTypeBtn, &QPushButton::clicked, this, [=]{
        // 加锁修改
        QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
        for(int i=0; i<ConfigService::currentMappingConfig.mappingList.size(); i++){
            auto item = ConfigService::currentMappingConfig.mappingList[i];
            auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,
                                                                          item.dev_btn_name);
            if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                auto newMappingType = ConfigService::currentMappingConfig.mappingList[i].mappingType == MappingType::Keyboard
                                          ? MappingType::Xbox
                                          : MappingType::Keyboard;

                ConfigService::currentMappingConfig.mappingList[i].setMappingType(newMappingType);

                // 切换图标
                Global::switchMappingTypeIcon(mappingTypeBtn, ConfigService::currentMappingConfig.mappingList[i]);

                // 轴映射 xbox, 隐藏按键触发模式的下拉框
                if(item.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS
                    && item.mappingType == MappingType::Xbox){
                    // 恢复默认选项
                    triggerTypeComboBox->setCurrentIndex(0);
                    ConfigService::currentMappingConfig.mappingList[i].btnTriggerType = TriggerTypeEnum::Normal;
                    // 设置为不可用
                    triggerTypeComboBox->setDisabled(true);

                    // 轴映射xbox, 映射按键下拉框只能单选
                    keyBoardComboBox->setSelectionMode(true);
                }

                // 轴映射 键盘按键, 显示按键触发模式的下拉框
                if(item.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS
                    && item.mappingType == MappingType::Keyboard){
                    triggerTypeComboBox->setDisabled(false);
                }

                // 更新映射按键的选择下拉框
                updateAKeyBoardComboBox(keyBoardComboBox,
                                        item.dev_btn_type,
                                        newMappingType);
                ConfigService::currentMappingConfig.mappingList[i].keyboard_name = keyBoardComboBox->currentText();
                ConfigService::currentMappingConfig.mappingList[i].keyboard_value = 0;
            }
        }

    });


    // ===========================================================================
    // 备注
    // ===========================================================================
    QLineEdit *lineEdit = new QLineEdit(isAddNewMapping ? "" : currentMapping.remark);
    Theme::setLineEditStyleSheet(lineEdit);
    // 连接信号和槽
    QObject::connect(lineEdit, &QLineEdit::textChanged, this, [=]{
        // 加锁修改
        QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
        for(int i=0; i<ConfigService::currentMappingConfig.mappingList.size(); i++){
            auto item = ConfigService::currentMappingConfig.mappingList[i];
            auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,
                                                                          item.dev_btn_name);
            if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                // 更新备注信息
                ConfigService::currentMappingConfig.mappingList[i].remark = lineEdit->text();
            }
        }

    });


    // ===========================================================================
    // 删除按钮
    // ===========================================================================
    QPushButton *deleteBtn = new QPushButton(StringConstants::deleteMappings);
    Theme::setButtonStyleSheet(deleteBtn, ButtonLevel::critical, "padding:6px;");
    QFont font = deleteBtn->font();
    font.setPointSize(8);
    deleteBtn->setFont(font);

    // 绑定信号和槽
    connect(deleteBtn, &QPushButton::clicked, this, [=](){
        {
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);

            // 删除当前行组件
            bool res = deleteRowFromGridLayout(scrollAreaWidgetLayout, deviceNameAndBtnName);
            // 删除失败
            if(!res)
                return;

            for(int i=0; i< ConfigService::currentMappingConfig.mappingList.size(); i++){
                auto item = ConfigService::currentMappingConfig.mappingList[i];
                auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,
                                                                              item.dev_btn_name);
                if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                    // 删除映射
                    ConfigService::currentMappingConfig.mappingList.remove(i);
                    break;
                }
            }
        }
    });

    // "反转该轴" 的 checkbox
    QCheckBox *checkBox = nullptr;
    if((currentMapping.valid && currentMapping.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS)){
        // 轴映射 xbox
        if(currentMapping.mappingType == MappingType::Xbox){
            //qDebug() << "轴映射xbox";

            // 轴隐藏按键触发模式的下拉框
            triggerTypeComboBox->setDisabled(true);

            // 当轴映射xbox时, 键盘按键下拉框设置为单选模式
            keyBoardComboBox->setSelectionMode(true);
        }

        // 反转轴的勾选
        checkBox = new QCheckBox(StringConstants::rotateAxis, this);
        checkBox->setToolTip(StringConstants::rotateAxisToolTips);
        Theme::setCheckBoxStyleSheet(checkBox);

        if(currentMapping.rotateAxis == 1){
            checkBox->setChecked(true);
        }
    }

    if(checkBox != nullptr){
        // ===========================================================================
        // 绑定信号和槽
        // ===========================================================================
        connect(checkBox, &QCheckBox::toggled, this, [=](){
            // 加锁修改
            QMutexLocker locker(&ConfigService::mutex_currentMappingConfig);
            for(int i=0; i<ConfigService::currentMappingConfig.mappingList.size(); i++){
                auto item = ConfigService::currentMappingConfig.mappingList[i];
                auto item_deviceNameAndBtnName = Global::getBtnOrAxisFullName(item.deviceName,
                                                                              item.dev_btn_name);
                if(deviceNameAndBtnName == item_deviceNameAndBtnName){
                    // 更新改动到mapping列表
                    ConfigService::currentMappingConfig.mappingList[i].rotateAxis = checkBox->isChecked() ? 1 : 0;
                }
            }
        });
    }

    // 最后一列, "操作"列
    QWidget* actionCol = new QWidget;
    actionCol->setStyleSheet("padding:0;margin:0;");
    QHBoxLayout* actionColLayout = new QHBoxLayout(actionCol);
    actionColLayout->setSpacing(8);
    actionColLayout->setContentsMargins(0, 0, 0, 0);
    // 添加 "反转该轴" 单选框
    if(checkBox != nullptr){
        actionColLayout->addWidget(checkBox);
    }
    // 添加删除按钮
    actionColLayout->addWidget(deleteBtn);


    // 把组件添加到 列表
    cols.append(Theme::createStyledDeviceNameBtnNameGroup(devNamelabel, devBtnlabel));
    cols.append(mappingTypeBtn);
    cols.append(keyBoardComboBox);
    cols.append(triggerTypeComboBox);
    cols.append(lineEdit);
    cols.append(actionCol);

    // 统一高度, 添加进布局
    for(int i=0; i<cols.size(); i++){
        cols[i]->setFixedHeight(32);
        // 设置ObjectName为 "设备名称+设备按键名称", 用于删除组件
        cols[i]->setObjectName(deviceNameAndBtnName);
        scrollAreaWidgetLayout->addWidget(cols[i], currentRow, i);
    }

}

MappingRelation HomePage::getDevBtnData()
{
    // 准备
    auto initedDeviceList = prepareDiDevices();
    if(initedDeviceList.empty()){
        return MappingRelation(false);
    }

    bool enableLogs = Global::getEnablePovLog() || Global::getEnableBtnLog() || Global::getEnableAxisLog();

    // 获取设备接下来操作的按键或轴
    auto item = DirectInputService::getNextActionBtnOrAxis(
                    initedDeviceList,
                    ConfigService::get().SYSTEM_enableOnlyChangeKeyWhenNew,
                    enableLogs);

    // 用户没有按下按键或踩下轴, 返回这个无效对象
    if(item.valid == false){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::error_btnNotPressed);
        return item;
    }

    // 用户操作了设备的轴, 需要确定该轴是映射 xbox 还是 键盘按键
    if(item.dev_btn_type == DeviceDataTypeEnum::WHEEL_AXIS){
        // 确定该轴是映射键盘还是xbox
        // 创建并添加自定义按钮
        QPushButton* mappingToKeyBoard = new QPushButton(StringConstants::btnText_mappingKeyboard, this);
        QPushButton* mappingToXbox = new QPushButton(StringConstants::btnText_mappingXbox, this);
        QPushButton* cancel = new QPushButton(StringConstants::btnText_cancel, this);
        Theme::setButtonStyleSheet(mappingToKeyBoard, ButtonLevel::primary);
        Theme::setButtonStyleSheet(mappingToXbox, ButtonLevel::primary);
        Theme::setButtonStyleSheet(cancel, ButtonLevel::normal);
        // 创建消息弹窗
        CardMessageDialog dialog(CardMessageDialog::Type::Info,
                                 StringConstants::text_pleaseConfirm,
                                 StringConstants::text_confirmAxisMappingMessageBoxText.arg(item.deviceName, item.dev_btn_name),
                                 nullptr,
                                 {mappingToKeyBoard, mappingToXbox, cancel});

        dialog.exec();

        // 轴-左转
        QString tempBtnNameLeft = item.dev_btn_name + StringConstants::text_wheelLeft;
        // 轴-右转
        QString tempBtnNameRight = item.dev_btn_name + StringConstants::text_wheelRight;
        // 已经配置了映射
        QString tempLeftOrRightErrorMsg = StringConstants::error_leftOrRightAlreadyExistsMapping.arg(item.deviceName, item.dev_btn_name.data());

        // 映射键盘按键
        if(dialog.clickedButton() == mappingToKeyBoard){
            // 如果已经存在该轴的映射, 直接返回
            if(hasAddToMappingList(QString(item.dev_btn_name.data()), item.deviceName)){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::error_alreadyExistsMapping.arg(item.deviceName, item.dev_btn_name.data()));
                // 将该映射设置为无效
                item.valid = false;
                return item;
            }

            // 创建并添加自定义按钮
            QPushButton* panti = new QPushButton(StringConstants::btnText_isWheelAxis, this);
            QPushButton* taban = new QPushButton(StringConstants::btnText_isPedalAxis, this);
            QPushButton* cancel2 = new QPushButton(StringConstants::btnText_cancel, this);
            Theme::setButtonStyleSheet(panti, ButtonLevel::primary);
            Theme::setButtonStyleSheet(taban, ButtonLevel::primary);
            Theme::setButtonStyleSheet(cancel2, ButtonLevel::normal);

            // 创建消息弹窗
            CardMessageDialog dialog2(CardMessageDialog::Type::Info,
                                     StringConstants::text_pleaseConfirm,
                                     StringConstants::text_confirmAxisTypeMessageBoxText,
                                     nullptr,
                                     {panti, taban, cancel2});

            dialog2.exec();

            item.mappingType = MappingType::Keyboard;

            // 判断用户点击'是转向轴'按钮
            if (dialog2.clickedButton() == panti){
                // 值增大, 说明是转向轴右转
                if(item.axisValueChange > 0){
                    item.dev_btn_name += StringConstants::text_wheelRight.toStdString();
                    return item;
                }else if(item.axisValueChange < 0){
                    item.dev_btn_name += StringConstants::text_wheelLeft.toStdString();
                    return item;
                }
            }else if(dialog2.clickedButton() == taban){
                // 如果已经存在该轴的左转或右转映射, 直接返回
                if(hasAddToMappingList(tempBtnNameLeft, item.deviceName) || hasAddToMappingList(tempBtnNameRight, item.deviceName)){
                    // 显示错误弹窗
                    Global::showErrorMsgBoxAndPushToLog(tempLeftOrRightErrorMsg);
                    // 将该映射设置为无效
                    item.valid = false;
                    return item;
                }

                // 对踏板轴 自动识别是否需要反转该轴, 值减小, 设置反转该轴
                if(item.axisValueChange < 0){
                    item.rotateAxis = 1;
                }

                // 踏板
                return item;
            }else{
                // 点击取消, 将该映射设置为无效
                item.valid = false;
                return item;
            }

        }else if(dialog.clickedButton() == mappingToXbox){
            // 如果已经存在该轴的左转或右转映射, 直接返回
            // 如果已经存在该轴的左转或右转映射, 直接返回
            if(hasAddToMappingList(tempBtnNameLeft, item.deviceName) || hasAddToMappingList(tempBtnNameRight, item.deviceName)){
                // 显示错误弹窗
                Global::showErrorMsgBoxAndPushToLog(tempLeftOrRightErrorMsg);
                // 将该映射设置为无效
                item.valid = false;
                return item;
            }

            // 对除了"X轴"之外的轴(因为通常"X轴"是转向轴), 自动识别是否需要反转该轴, 值减小, 设置反转该轴
            if(item.dev_btn_name.toLower() != (StringConstants::axisX.toLower()) && item.axisValueChange < 0){
                item.rotateAxis = 1;
            }

            item.mappingType = MappingType::Xbox;
            return item;
        }else{
            // 点击取消, 将该映射设置为无效
            item.valid = false;
            return item;
        }

    }

    // 返回
    return item;
}

bool HomePage::hasAddToMappingList(const MappingRelation &mapping)
{
    for(const auto& item : ConfigService::getCurrentMappingList()){
        if(item.valid
            && mapping.valid
            && item.dev_btn_name == mapping.dev_btn_name
            && item.deviceName == mapping.deviceName){

            return true;
        }
    }

    return false;
}

bool HomePage::hasAddToMappingList(QString devBtnName, QString deviceName)
{
    for(const auto& item : ConfigService::getCurrentMappingList()){
        if(item.valid
            && item.dev_btn_name == devBtnName
            && item.deviceName == deviceName){

            return true;
        }
    }

    return false;
}

MultiSelectComboBox *HomePage::createAKeyBoardComboBox(DeviceDataTypeEnum dev_btn_type, MappingType mappingType)
{
    MultiSelectComboBox *comboBox = new MultiSelectComboBox();
    comboBox->setCurrentIndex(-1);
    // comboBox->setMaximumHeight(36);
    // comboBox->setMinimumHeight(36);
    // comboBox->setMinimumWidth(150);
    // comboBox->setMaximumWidth(150);
    //comboBox->setStyleSheet("QComboBox{}");

    // 添加下拉框选择项
    updateAKeyBoardComboBox(comboBox, dev_btn_type, mappingType);

    return comboBox;
}

void HomePage::updateAKeyBoardComboBox(MultiSelectComboBox *comboBox, DeviceDataTypeEnum dev_btn_type, MappingType mappingType)
{
    // 清空下拉框
    comboBox->clear();

    std::map<std::string, short> map = getConstKeyMap(dev_btn_type, mappingType);

    // 手动置顶 暂停按键选项
    if(map.find(PAUSE_BTN_STR) != map.end()){
        comboBox->addItem(PAUSE_BTN_STR);
    }

    // 使用迭代器遍历 map
    for (std::map<std::string, short>::const_iterator item = map.cbegin(); item != map.cend(); ++item) {
        // 跳过暂停按键, 已手动将其置顶
        if(item->second == PAUSE_BTN_VAL){
            continue;
        }
        comboBox->addItem(item->first.data());
    }
    comboBox->setCurrentIndex(-1);
}

QVector<LPDIRECTINPUTDEVICE8> HomePage::prepareDiDevices()
{
    // 初始化directInput
    if(!DirectInputService::initDirectInput()){
        return {};
    }

    // 连接设备
    if(!DirectInputService::openDiDevice(getCurrentSelectedDeviceList())){
        return {};
    }

    // 已初始化的当前选择的设备
    QVector<LPDIRECTINPUTDEVICE8> initedSelectedDeviceList;

    for(auto &deviceName : getCurrentSelectedDeviceList()){
        auto initedDevice = DirectInputService::getInitedDevice(deviceName);
        if(initedDevice == nullptr){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::getInitedDeviceByDeviceNameFailed.arg(deviceName));
            return {};
        }else{
            initedSelectedDeviceList.append(initedDevice);
        }
    }

    return initedSelectedDeviceList;
}

std::map<string, short> HomePage::getConstKeyMap(DeviceDataTypeEnum dev_btn_type, MappingType mappingType)
{
    if(mappingType == MappingType::Xbox){
        if(dev_btn_type == DeviceDataTypeEnum::WHEEL_BUTTON){
            return VK_XBOX_BTN_MAP;
        }

        return VK_XBOX_AXIS_MAP;
    }

    return VK_MAP;

}

bool HomePage::checkDriverInstalled()
{
    QSettings *settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Nefarius Software Solutions e.U.\\ViGEm Bus Driver", QSettings::NativeFormat);
    if(settings->contains("Version")){
        //qDebug("驱动已安装");
        return true;
    }else{
        //QMessageBox::information(nullptr, "", "");

        //获取当前目录
        QDir dir(QCoreApplication::applicationDirPath());

        //std::string dirPath = QDir::toNativeSeparators(driverPath).toStdString();
        QString driverPath = dir.filePath("driver/ViGEmBus_1.22.0_x64_x86_arm64.exe");

        // 创建消息弹窗
        CardMessageDialog dialog(CardMessageDialog::Type::Warning, StringConstants::info, StringConstants::installXboxDriverTips.arg(driverPath.data()));

        // 判断用户点击确认按钮
        if (dialog.exec() == QDialog::Accepted) {
            //std::string command = "explorer \"" + driverPath + "\"";  // 使用双引号包裹路径，处理空格
            QString command = "\"" + driverPath + "\"";  // 使用双引号包裹路径，处理空格

            QFile driverFile(driverPath);
            // 安装包不存在
            if(!driverFile.exists()){
                Global::showErrorMsgBoxAndPushToLog(StringConstants::xboxDriverFileNotExistsTips.arg(driverPath));
                return false;
            }

            // 将窗口最小化
            //showMinimized();

            // 使用 QProcess 执行命令
            QProcess::startDetached(command);
        }

        return false;
    }

}

void HomePage::saveCurrentMappingsAndDeviceToFile()
{
    if(getCurrentSelectedDeviceList().size() < 1){
        return;
    }

    QString selectedMappingFilePath;
    // 当前选择的映射文件
    if(cfgBox->currentIndex() > 0){
        auto fileShortName = cfgBox->currentText();
        selectedMappingFilePath = Global::getUserMappingFilePath(fileShortName);
    }

    // 更新配置
    {
        QMutexLocker locker(&ConfigService::mutex_globalUserConfig);

        // 保存当前选择的设备
        ConfigService::globalUserConfig.SYSTEM_lastUsedDeviceNameList = getCurrentSelectedDeviceList();

        if(selectedMappingFilePath.isEmpty()){
            // 保存当前使用的映射列表
            ConfigService::globalUserConfig.SYSTEM_lastUsedMappingCache = ConfigService::getCurrentMappingList();
            ConfigService::globalUserConfig.SYSTEM_lastUsedMappingConfigPath = "";
        }else{
            ConfigService::globalUserConfig.SYSTEM_lastUsedMappingCache = {};
            // 保存当前使用的映射文件绝对路径
            ConfigService::globalUserConfig.SYSTEM_lastUsedMappingConfigPath = selectedMappingFilePath;
        }
    }

    // 保存到文件
    ConfigService::saveGlobalUserConfigToFile();
}

QWidget *HomePage::createRow(QVector<QWidget *> colWidgets)
{
    QWidget* row  = new QWidget;
    row->setObjectName("row");
    row->setStyleSheet(R"(
        QWidget#row{
            padding: 0px;
        }
    )");
    row->setFixedHeight(36);
    QHBoxLayout* rowLayout = new QHBoxLayout(row);
    rowLayout->setContentsMargins(10, 4, 10, 4);
    rowLayout->setSpacing(8);
    rowLayout->setAlignment(Qt::AlignVCenter);

    for(int i = 0; i < colWidgets.size(); i++){
        colWidgets[i]->setMaximumHeight(28);
        colWidgets[i]->setMinimumHeight(28);
        rowLayout->addWidget(colWidgets[i]);
    }

    // 添加空白填充
    rowLayout->addStretch();

    return row;
}

void HomePage::updateRunningStateText()
{
    // 当前正在运行
    if(RunningStatus::getIsRunning()){
        cfgBox->setDisabled(true);
        deviceBox->setDisabled(true);
        addMappingBtn->setDisabled(true);
        refreshDeviceListBtn->setDisabled(true);

        // 当前为暂停状态
        if(RunningStatus::getIsPause()){
            // "已暂停"
            runningStateTextLabel->setText(StringConstants::alreadyPause);
            // 黄色的小圆点
            runningStateLabel->setStyleSheet("QLabel{color: rgb(248, 201, 19);}");
        }else{
            // "已启动"
            runningStateTextLabel->setText(StringConstants::alreadyStart);
            // 绿色的小圆点
            runningStateLabel->setStyleSheet("QLabel{color: rgb(0, 160, 0);}");
        }

        // 按键文本 "停止全局映射"
        startBtn->setText(StringConstants::stopKeyMappingsRunning);
        Theme::setButtonStyleSheet(startBtn, ButtonLevel::critical);
    }else{
        cfgBox->setDisabled(false);
        deviceBox->setDisabled(false);
        addMappingBtn->setDisabled(false);
        refreshDeviceListBtn->setDisabled(false);

        // 运行状态 "未启动"
        runningStateTextLabel->setText(StringConstants::notStart);
        // 红色小圆点
        runningStateLabel->setStyleSheet("QLabel{color: rgb(255, 85, 0);}");
        // 按键文本 "启动全局映射"
        startBtn->setText(StringConstants::startKeyMappingsRunning);
        Theme::setButtonStyleSheet(startBtn, ButtonLevel::success);
    }
}

void HomePage::drawHeaders()
{
    int index = -1;
    for(const auto& header : tableHeaders){
        scrollAreaWidgetLayout->addWidget(new QLabel(header), 0, ++index);
    }
}

bool HomePage::deleteRowFromGridLayout(QGridLayout *grid, QString objectName)
{
    if(grid == nullptr || objectName.isEmpty())
        return false;

    // 列数, 即表头数量
    int colCount = tableHeaders.size();

    int findRow = -1;

    // 1. 根据objectName找到行, 再删除整行的 widget
    for (int row = 0; row < 9999; row++)
    {
        auto item = grid->itemAtPosition(row, 0);
        if (!item) continue;

        QWidget *w = item->widget();
        if (!w) continue;

        // 找到了!
        if(w->objectName() == objectName){
            findRow = row;
            break;
        }
    }

    if(findRow < 0){
        Global::showErrorMsgBoxAndPushToLog(StringConstants::error_deleteRowFailed);
        return false;
    }

    // 2.删除整行
    for(int col=0; col < colCount; col++){
        auto item = grid->itemAtPosition(findRow, col);
        if (!item) continue;

        QWidget *w = item->widget();
        if (!w) continue;

        grid->removeWidget(w);
        w->deleteLater();
    }

    // 3. 上移所有后续行
    for (int r = findRow + 1; r < grid->rowCount(); r++)
    {
        for (int c = 0; c < colCount; c++)
        {
            auto item = grid->itemAtPosition(r, c);
            if (!item) continue;

            QWidget *w = item->widget();
            if (!w) continue;

            grid->removeWidget(w);
            grid->addWidget(w, r - 1, c);
        }
    }

    return true;
}

int HomePage::getGridLayoutRowCount(QGridLayout *grid)
{
    if(grid == nullptr)
        return 0;

    int counter = -1;

    // 从第一行开始遍历, 直到出现空的第一列, 说明遍历结束
    for(int row = 0;; row++){
        if (grid->itemAtPosition(row, 0))
        {
            counter++;
        }else{
            break;
        }
    }

    return counter;
}

void HomePage::on_startBtn_clicked()
{
    if(!RunningStatus::getIsRunning()){
        if(getCurrentSelectedDeviceList().isEmpty()){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::startKeyMappingsRunningFailed + ": " +StringConstants::error_deviceNotSelect);
            return;
        }

        if(getMappingListActualSize() <= 0){
            Global::showErrorMsgBoxAndPushToLog(StringConstants::error_mappingsListIsEmpty);
            return;
        }

        // 映射列表有映射xbox的配置, 但虚拟xbox驱动未安装, 无法启动
        if(Global::hasXboxMappingInMappingList(ConfigService::getCurrentMappingList())
            && !checkDriverInstalled()){
            qDebug("驱动未安装, 无法启动!");
            return;
        }

        // 映射配置所需设备与当前选择的设备检查
        auto mappingList = ConfigService::getCurrentMappingList();
        auto cuurentSelectedDeviceList = getCurrentSelectedDeviceList();
        QSet<QString> unmatchDeviceSet;
        for(const auto& mapping : mappingList){
            // 该映射所属设备不再当前选择的设备中
            if(!cuurentSelectedDeviceList.contains(mapping.deviceName))
                unmatchDeviceSet.insert(mapping.deviceName);
        }
        // 所需设备与当前选择的设备不完全匹配
        if(!unmatchDeviceSet.isEmpty()){
            // 显示提示信息
            MessageBoxService::showInfo(StringConstants::devicesNotMatchTips.arg(unmatchDeviceSet.values().join("\n")));
        }

        // 根据读取选择的设备名称, 初始化设备, 获得已初始化后的设备实例
        auto initedSelectedDeviceList = prepareDiDevices();
        if(initedSelectedDeviceList.empty()){
            return;
        }


        // 创建监听设备输入数据的任务
        SimulateTask *task = new SimulateTask(ConfigService::getCurrentMappingList(), initedSelectedDeviceList);
        QThread *thread = new QThread();

        // 将 worker 移到新线程
        task->moveToThread(thread);

        // 用户设置发生改动, 需要通知 SimulateTask
        connect(this, &HomePage::settingsChangedSignal, task, &SimulateTask::settingsChangedSlot);

        // 当线程开始时，启动工作任务
        connect(thread, &QThread::started, task, &SimulateTask::doWork);

        // 任务完成后，退出线程并清理
        connect(task, &SimulateTask::workFinished, thread, &QThread::quit);
        connect(task, &SimulateTask::workFinished, task, &SimulateTask::deleteLater);

        // 映射任务开启信号
        connect(task, &SimulateTask::startedSignal, this, [=](){
            updateRunningStateText();
        });
        // 暂停按键被按下的信号
        connect(task, &SimulateTask::pauseClickSignal, this, [=](){
            updateRunningStateText();
        });
        // 线程结束信号
        connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        // 启动线程
        thread->start();

        // 保存当前使用的映射 和 设备 到配置文件
        saveCurrentMappingsAndDeviceToFile();

        updateRunningStateText();
    }
    else{
        // 如果处于暂停状态, 将其重置
        if(RunningStatus::getIsPause()){
            RunningStatus::clickPauseBtn();
        }

        RunningStatus::setIsRuning(false);

        updateRunningStateText();
    }

}

void HomePage::settingsChangedSlot()
{
    emit settingsChangedSignal();
}








