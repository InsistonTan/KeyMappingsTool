#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "key_map.h"
#include "BtnTriggerTypeEnum.h"
#include "simulate_task.h"

#include<QApplication>
#include<QMainWindow>
#include<QLabel>
#include<QVBoxLayout>
#include<QKeyEvent>
#include<QDebug>
#include<QMessageBox>
#include<QLineEdit>
#include<global.h>
#include<QFile>
#include<QInputDialog>
#include<QDir>
#include<QSettings>
#include<QProcess>
#include<cmath>
#include<QCheckBox>
#include<QTimer>
#include<QScrollBar>
#include<QJsonDocument>
#include<QJsonObject>
#include<QJsonArray>
#include<QNetworkAccessManager>
#include<QNetworkRequest>
#include<QNetworkReply>
#include<QRegularExpression>
#include<QHostInfo>
#include<QDate>
#include <QDesktopServices>
#include <QUrl>
#include <QStyleHints>

QList<QString> MainWindow::currentSelectedDeviceList = {}; // 当前选择的设备列表

MainWindow::~MainWindow(){
    cleanupDirectInput();
}

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(972, 643);
    this->setWindowTitle(QString("KeyMappingsTool v").append(CURRENT_VERSION));

    // 初始化
    init();

    // 当前exe程序所在路径
    QString exePath = QCoreApplication::applicationDirPath();
    // 发送使用记录, 开发环境不发送
    if(exePath.contains("build") && (exePath.contains("release") || exePath.contains("debug"))){
        qDebug() << "当前为开发环境, 不发送使用统计";
    }else{
        sendUsageCount();
    }
    // 检查更新
    checkUpdate();
}


void MainWindow::init(){
    // 获取软件本地数据目录
    appDataDirPath = getAppDataDirStr();
    QDir dir = QDir(QDir::homePath() + "/AppData/Local/");
    // 使用 QDir 创建不存在的目录
    if (!dir.exists("KeyMappingToolData")) {
        if (!dir.mkdir("KeyMappingToolData")) {
            showErrorMessage(new std::string("创建存放用户配置的文件夹失败"));
            //return;
        }
    }
    // 文件夹创建失败
    if(!dir.exists("KeyMappingToolData")){
        appDataDirPath = "";
    }


    // 初始化directInput并扫描设备
    initDirectInput();

    QComboBox *comboBox  = ui->comboBox;
    comboBox->setMaximumHeight(30);
    // 使用迭代器遍历
    for (int i=0; i< (int)diDeviceList.size(); i++) {
        comboBox->addItem(diDeviceList[i].name.data());
    }
    comboBox->setCurrentIndex(-1);
    if(diDeviceList.empty()){
        comboBox->setPlaceholderText("未扫描到设备");
    }else{
        comboBox->setPlaceholderText("请选择设备");
    }


    // 设置滚动条的样式
    ui->scrollArea->setStyleSheet(R"(
        QScrollBar:vertical {
            border: none;
            background: #f2f2f2;
            width: 5px;
            margin: 0;
        }
        QScrollBar::handle:vertical {
            background: rgb(190, 190, 190);
            border-radius: 8px;
        }
        QScrollBar::sub-line:vertical {
            height: 0px;
        }
        QScrollArea{border:1px solid rgb(108, 108, 108);}
    )");

    ui->scrollAreaContents->setMaximumWidth(750);
    ui->scrollAreaContents->setLayout(ui->gridLayout);

    // 默认映射键盘
    //ui->radioButton->setChecked(true);

    // 加载上一次使用的设备
    loadLastDeviceFile();

    // 加载上一次的映射配置
    loadMappingsFile("");

    // 绘制出配置列表
    repaintMappings();

    if(getMappingListActualSize() > 0){
        ui->comboBox_2->setPlaceholderText("上一次使用的配置");
    }

    // 配置下拉添加一个空白配置
    ui->comboBox_2->addItem("空白配置");

    // 扫描用户保存的配置文件
    scanMappingFile();

    // 日志窗口
    this->logWindow = new LogWindow();

    //loadSettings();
    //this->settings = new DeadAreaSettings();

    this->assistWindow = new AssistFuncWindow();
    connect(this->assistWindow, &AssistFuncWindow::saveLastDeviceToFileSignal, this, &MainWindow::saveLastDeviceToFileSlot);

    if(AssistFuncWindow::getEnableMappingAfterOpening()){
        on_pushButton_2_clicked();
    }

    this->deadareaSettings = new DeadAreaSettings();

    g_mainWindow = this;
}

void MainWindow::scanMappingFile(){
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem("空白配置");
    mappingFileNameMap.clear();

    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    if(dir.exists(USER_MAPPINGS_DIR)){
        dir.cd(USER_MAPPINGS_DIR);
        // 获取文件列表，并筛选指定后缀的文件
        QFileInfoList list = dir.entryInfoList();

        for (const QFileInfo &fileInfo : list) {
            // 匹配指定后缀的文件，加入下拉列表
            if ("." + fileInfo.suffix() == MAPPING_FILE_SUFFIX ||
                "." + fileInfo.suffix() == MAPPING_FILE_SUFFIX_XBOX) {

                QString shortName = fileInfo.completeBaseName();

                // 当前选择了多个设备, 配置文件名需要跟当前选择的设备匹配
                if(currentSelectedDeviceList.size() > 1){
                    bool isAccept = true;
                    for(auto deviceName : currentSelectedDeviceList){
                        if(!fileInfo.completeBaseName().contains(deviceName)){
                            isAccept = false;
                        }
                    }
                    if(isAccept){
                        int lastUnderlinePos = shortName.lastIndexOf('_'); // 查找最后一个下划线的位置
                        if(lastUnderlinePos >= 0){
                            shortName = shortName.left(lastUnderlinePos); // 获取从开始到最后一个下划线之前的部分
                        }
                    }
                }else if(currentSelectedDeviceList.size() == 1){
                    // 当前为单设备, 只显示单设备的配置
                    // 过滤掉多设备的配置文件
                    if(fileInfo.completeBaseName().contains("_[") && fileInfo.completeBaseName().contains(",]")){
                        continue;
                    }
                }else if(currentSelectedDeviceList.size() == 0){
                    // 当前选择的设备为空, 显示所有配置
                    // 下拉框显示的配置名
                    QString shortName = fileInfo.completeBaseName();

                    int lastUnderlinePos = shortName.lastIndexOf('_'); // 查找最后一个下划线的位置
                    if(lastUnderlinePos >= 0){
                        shortName = shortName.left(lastUnderlinePos); // 获取从开始到最后一个下划线之前的部分
                    }
                }

                ui->comboBox_2->addItem(shortName);
                mappingFileNameMap[shortName] = fileInfo.absoluteFilePath();
            }
        }

        //ui->comboBox_2->setCurrentIndex(-1);
    }
}

bool MainWindow::hasLastDevInCurrentDeviceList(std::string lastDeviceNameListStr){
    if(lastDeviceNameListStr.empty()){
        return false;
    }

    // 按分隔符分割出设备列表
    auto lastDeviceNameList = QString(lastDeviceNameListStr.data()).split(SPE);

    int count = 0;
    for(auto deviceName : lastDeviceNameList){
        for(auto item : diDeviceList){
            if(deviceName.toStdString() == item.name){
                count++;
            }
        }
    }

    return count == lastDeviceNameList.size();
}

void MainWindow::repaintMappings(){
    qDebug("mappingList.size() : %d", getMappingListActualSize());

    if(getMappingListActualSize() > 0){
        // 清空配置页面
        clearMappingsArea();
        // 重新添加
        for(int i =0; i < (int)mappingList.size(); i++){
            if(mappingList[i] != nullptr){
                paintOneLineMapping(mappingList[i], i);
            }
        }
    }
}

void MainWindow::on_pushButton_2_clicked()
{
    if(!getIsRunning()){
        if(currentSelectedDeviceList.isEmpty()){
            showErrorMessage(nullptr);
            return;
        }

        if(getMappingListActualSize() <= 0){
            showErrorMessage(new std::string("映射列表为空!"));
            return;
        }

        // 映射列表有映射xbox的配置, 但虚拟xbox驱动未安装, 无法启动
        if(hasXboxMappingInMappingList(this->mappingList) && !checkDriverInstalled()){
            qDebug("驱动未安装, 无法启动!");
            return;
        }

        // 初始化directInput
        if(!initDirectInput()){
            return;
        }
        // 连接设备
        if(!openDiDevice(getCurrentSelectedDeviceList())){
            return;
        }

        // 当前为单设备映射, 需要给映射关系补上设备名称
        if (currentSelectedDeviceList.size() == 1) {
            for(auto mapping : mappingList){
                if(mapping != nullptr){
                    mapping->deviceName = currentSelectedDeviceList[0];
                }
            }
        }

        // 创建监听设备输入数据的任务
        SimulateTask *task = new SimulateTask(mappingList);
        QThread *thread = new QThread();

        // 将 worker 移到新线程
        task->moveToThread(thread);

        // 当线程开始时，启动工作任务
        QObject::connect(thread, &QThread::started, task, &SimulateTask::doWork);

        // 任务完成后，退出线程并清理
        QObject::connect(task, &SimulateTask::workFinished, thread, &QThread::quit);
        QObject::connect(task, &SimulateTask::workFinished, task, &SimulateTask::deleteLater);
        // masgbox消息传递的信号
        QObject::connect(task, &SimulateTask::msgboxSignal, this, &MainWindow::simulateMsgboxSlot);
        // 映射任务开启信号
        QObject::connect(task, &SimulateTask::startedSignal, this, &MainWindow::simulateStartedSlot);
        // 暂停按键被按下的信号
        QObject::connect(task, &SimulateTask::pauseClickSignal, this, &MainWindow::pauseClickSlot);
        // 线程结束信号
        QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

        // 启动线程
        thread->start();

        // 显示信息框
        //QMessageBox::information(this, "提醒", "启动全局映射成功!\n如果游戏里不生效, 请使用管理员身份重新运行本程序 ");

        // 保存当前配置的映射到本地文件
        saveMappingsToFile("");

        // 保存上次使用的设备到本地
        saveLastDeviceToFile();

        ui->pushButton_2->setText("停止全局映射");
        ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(255, 170, 127);color: rgb(0, 0, 0);}");
    }
    else{
        // 如果处于暂停状态, 将其重置
        if(getIsPause()){
            clickPauseBtn();
        }

        ui->label_4->setText("未启动");
        ui->label_4->setStyleSheet("QLabel{color: rgb(255, 85, 0);}");
        ui->pushButton_2->setText("启动全局映射");
        ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(170, 255, 255);color: rgb(0, 0, 0);}");
        setIsRuning(false);

        //enableUiAfterStopMapping();
    }
}

void MainWindow::pauseClickSlot(){
    if(getIsRunning()){
        if(getIsPause()){
            ui->label_4->setText("已暂停");
            ui->label_4->setStyleSheet("QLabel{color: rgb(248, 201, 19);}");
        }else{
            ui->label_4->setText("已启动");
            ui->label_4->setStyleSheet("QLabel{color: rgb(0, 160, 0);}");
        }
    }
}

void MainWindow::showErrorMessage(std::string *text) {
    // 使用定时器来模拟非阻塞的消息框
    QTimer* timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, [=]() {
        QMessageBox::critical(this, "错误", text == nullptr ? "还未选择任何设备!" : text->data());
    });
    // 启动定时器
    timer->start(100);  // 500毫秒后触发

}

bool MainWindow::hasAddToMappingList(MappingRelation* mapping){
    for(int i=0; i < (int)mappingList.size(); i++){
        if(mappingList[i] != nullptr && mapping != nullptr
            && mappingList[i]->dev_btn_name == mapping->dev_btn_name
            && mappingList[i]->deviceName == mapping->deviceName){

            return true;
        }
    }

    return false;
}
bool MainWindow::hasAddToMappingList(QString devBtnName, QString deviceName){
    for(int i=0; i < (int)mappingList.size(); i++){
        if(mappingList[i] != nullptr && mappingList[i]->dev_btn_name == devBtnName.toStdString() && mappingList[i]->deviceName == deviceName){
            return true;
        }
    }

    return false;
}

bool isClickNewLineBtn(MappingRelation *mapping){
    return mapping == nullptr;
}

void MainWindow::onTriggerTypeComboBoxActivated(int index){
    qDebug() << "按键触发模式选择: " << index;

    // 获取触发信号的对象
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());

    // 获取到该映射的位置
    int rowIndex = comboBox->objectName().toInt();

    MappingRelation *mapping = mappingList[rowIndex];
    // 更新选择的触发模式
    mapping->btnTriggerType = static_cast<TriggerTypeEnum>(index);
}

void MainWindow::paintOneLineMapping(MappingRelation *mapping, int index){
    // isClickNewLineBtn()为true 代表是新增加的一行, 不是读取历史配置
    bool isAddNewMapping = isClickNewLineBtn(mapping);
    if(isAddNewMapping){

        // 获取设备按下的按键位置和值
        mapping = getDevBtnData();

        if(mapping == nullptr){
            showErrorMessage(new std::string("未检测到按键被按下!"));
            return;
        }

        if(mapping->dev_btn_name.empty()){
            return;
        }


        // 检查该设备按键是否已经添加
        if(hasAddToMappingList(mapping)){
            showErrorMessage(new std::string("设备[" + mapping->deviceName.toStdString() + "]的 \"" + mapping->dev_btn_name + "\" 已经配置了映射!"));
            return;
        }

        // 将设备按键数据添加进新的映射关系
        mappingList.push_back(mapping);
    }

    // 该映射无效
    if(mapping == nullptr || mapping->dev_btn_name.empty()){
        return;
    }

    // 记录当前行的index值
    auto currentRowIndex = isAddNewMapping ? std::to_string(mappingList.size() - 1) : std::to_string(index);

    QGridLayout *layout = ui->gridLayout;
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(7,7,7,7);

    // 画出表头
    if(index == 0 || getMappingListActualSize() == 1){
        QLabel *h1 = new QLabel("设备按键");
        h1->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h2 = new QLabel("映射成");
        h2->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h3 = new QLabel("映射模式");
        h3->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h4 = new QLabel("映射按键");
        h4->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h5 = new QLabel("按键触发模式");
        h5->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h6 = new QLabel("备注");
        h6->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        int col = -1;
        layout->addWidget(h1, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h2, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h3, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h4, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h5, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h6, 0, ++col, Qt::AlignLeft);
    }

    QString label1Text = mapping->dev_btn_name.data();

    QLabel *label1 = new QLabel(label1Text.length() > 14 ? label1Text.left(14) + "..." : label1Text);
    label1->setMaximumHeight(30);
    label1->setMinimumHeight(30);
    label1->setMaximumWidth(160);
    // 根据系统主题设置背景色
    QStyleHints *styleHints = qApp->styleHints();
    if (styleHints->colorScheme() == Qt::ColorScheme::Dark) {
        label1->setStyleSheet("QLabel{color:rgb(170,255,255);padding-right:10px;}");
    } else {
        label1->setStyleSheet("QLabel{color:blue;padding-right:10px;}");
    }
    label1->setObjectName(currentRowIndex);
    label1->setToolTip(mapping->deviceName.isEmpty() ? label1Text : mapping->deviceName + ": " +label1Text);

    QLabel *label2 = new QLabel("->");
    label2->setMaximumHeight(30);
    label2->setMinimumHeight(30);
    label2->setMaximumWidth(50);
    label2->setObjectName(currentRowIndex);


    // 键盘按键下拉框
    QComboBox *comboBox = createAKeyBoardComboBox(mapping->dev_btn_type, mapping->mappingType); //// 重构代码优先处理
    // 设置一个序号, 为后续操作提供一个位置
    comboBox->setObjectName(currentRowIndex);
    // 历史配置展示
    if(!isAddNewMapping){
        comboBox->setCurrentText(mapping->keyboard_name.data());
    }
    // 连接信号和槽
    connect(comboBox, &QComboBox::activated, this, &MainWindow::onKeyBoardComboBoxActivated);


    // 映射模式下拉框
    QComboBox *mappingTypeComboBox = new QComboBox();
    mappingTypeComboBox->setMaximumHeight(36);
    mappingTypeComboBox->setMinimumHeight(36);
    mappingTypeComboBox->setMinimumWidth(90);
    mappingTypeComboBox->setMaximumWidth(90);
    mappingTypeComboBox->setStyleSheet("QComboBox{padding-left:10px;}"
                                       "QComboBox:disabled {"
                                       "   background-color: #f0f0f0;"
                                       "   color: #888888;"
                                       "}"
                                       );
    // 设置一个序号, 为后续操作提供一个位置
    mappingTypeComboBox->setObjectName(currentRowIndex);
    mappingTypeComboBox->addItem("映射键盘");
    mappingTypeComboBox->addItem("映射Xbox");

    if (QString(mapping->dev_btn_name.data()).contains("右转") || QString(mapping->dev_btn_name.data()).contains("左转")) {
        mappingTypeComboBox->setEnabled(false);
    }

    if(mapping->mappingType == MappingType::Keyboard){
        mappingTypeComboBox->setCurrentIndex(0);
    }else if(mapping->mappingType == MappingType::Xbox){
        mappingTypeComboBox->setCurrentIndex(1);
    }

    // 连接信号和槽
    connect(mappingTypeComboBox, &QComboBox::activated, this, [=]{
        if (mappingTypeComboBox->currentIndex() == 1){
            mapping->setMappingType(MappingType::Xbox);
        }else if (mappingTypeComboBox->currentIndex() == 0){
            mapping->setMappingType(MappingType::Keyboard);
        }
        //updateASwitchPushButton(btn, mapping->mappingType);
        updateAKeyBoardComboBox(comboBox, mapping->dev_btn_type, mapping->mappingType);
        mapping->keyboard_name = comboBox->currentText().toStdString();
        mapping->keyboard_value = 0;
    });


    // 按键触发模式下拉框
    QComboBox *triggerTypeComboBox = new QComboBox();
    triggerTypeComboBox->setMaximumHeight(36);
    triggerTypeComboBox->setMinimumHeight(36);
    triggerTypeComboBox->setMinimumWidth(160);
    triggerTypeComboBox->setMaximumWidth(160);
    triggerTypeComboBox->setStyleSheet("QComboBox{padding-left:10px;}"
                                       "QComboBox:disabled {"
                                       "   background-color: #f0f0f0;"
                                       "   color: #888888;"
                                       "}"
                                       );
    // 添加下拉框选择项
    for(int i = 0; i < TriggerTypeEnum::End; i++){
        TriggerTypeEnum enumItem = static_cast<TriggerTypeEnum>(i);
        triggerTypeComboBox->addItem(TRIGGER_TYPE_ENUM_MAP[enumItem].data());
    }
    // 设置一个序号, 为后续操作提供一个位置
    triggerTypeComboBox->setObjectName(currentRowIndex);
    // 历史配置展示
    if(!isAddNewMapping){
        triggerTypeComboBox->setCurrentText(TRIGGER_TYPE_ENUM_MAP[mapping->btnTriggerType].data());
    }
    // 连接信号和槽
    connect(triggerTypeComboBox, &QComboBox::activated, this, &MainWindow::onTriggerTypeComboBoxActivated);


    QLineEdit *lineEdit = new QLineEdit(isAddNewMapping ? "" : mapping->remark.data());
    lineEdit->setMaximumHeight(30);
    lineEdit->setMinimumHeight(30);
    lineEdit->setMinimumWidth(150);
    lineEdit->setStyleSheet("QLineEdit{margin:0 0 0 0; padding-left:5px;}");
    lineEdit->setObjectName(currentRowIndex);
    // 连接信号和槽
    QObject::connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::onLineEditTextChanged);

    // 删除按钮
    QPushButton *deleteBtn = new QPushButton("删除该映射");
    deleteBtn->setMaximumHeight(30);
    deleteBtn->setMaximumWidth(80);
    deleteBtn->setMinimumWidth(80);
    deleteBtn->setStyleSheet("QPushButton{background-color:rgb(255, 157, 157);margin-left:7;color: rgb(0, 0, 0);}");
    deleteBtn->setObjectName(currentRowIndex);
    // 绑定信号和槽
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteBtnClicked);


    int columnIndex = -1;
    int row = (index < 0 ? mappingList.size() : index) + 1;
    layout->addWidget(label1, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(label2, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(mappingTypeComboBox, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(comboBox, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(triggerTypeComboBox, row, ++columnIndex, Qt::AlignLeft); // triggerTypeComboBox
    layout->addWidget(lineEdit, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(deleteBtn, row, ++columnIndex, Qt::AlignLeft);

    if((mapping != nullptr && mapping->dev_btn_type == (std::string)WHEEL_AXIS)){
        // 轴隐藏按键触发模式的下拉框
        triggerTypeComboBox->setDisabled(true);

        // 反转轴的勾选
        QCheckBox *checkBox = new QCheckBox("反转该轴", this);
        checkBox->setMaximumHeight(30);
        checkBox->setMinimumHeight(30);
        checkBox->setMaximumWidth(80);
        checkBox->setObjectName(currentRowIndex);
        // 保存的配置, 恢复
        if(!isAddNewMapping && mapping->rotateAxis == 1){
            checkBox->setChecked(true);
        }

        // 绑定信号和槽
        connect(checkBox, &QCheckBox::toggled, this, &MainWindow::onCheckBoxToggle);

        layout->addWidget(checkBox, row, ++columnIndex, Qt::AlignLeft);
    }
}

void MainWindow::onCheckBoxToggle(bool checked){
    // 获取触发信号的对象
    QCheckBox *checkBox = qobject_cast<QCheckBox*>(sender());

    // 更新改动到mapping列表
    mappingList[checkBox->objectName().toInt()]->rotateAxis = checkBox->isChecked() ? 1 : 0;

    qDebug("checkBox clicked, index: %d", checkBox->objectName().toInt());
}

void MainWindow::on_pushButton_clicked()
{
    // 未选择设备
    if(currentSelectedDeviceList.isEmpty()){
        showErrorMessage(nullptr);
        return;
    }

    //qDebug(("deviceName: " + deviceName).data());

    if(getIsRunning()){
        showErrorMessage(new std::string("请先停止全局映射!"));
        return;
    }

    // 禁用按钮，防止重复点击
    ui->pushButton->setEnabled(false); 
    ui->pushButton->setText("等待输入...");

    paintOneLineMapping(nullptr, -1);

    QTimer::singleShot(50, [=](){
        QScrollBar *sbar = ui->scrollArea->verticalScrollBar();
        sbar->setValue(sbar->maximum());

        // 恢复按钮状态
        ui->pushButton->setText("新增按键映射");
        ui->pushButton->setEnabled(true);
    });
}

void MainWindow::saveLastDeviceToFile(bool isOnlySaveLastDevice){
    // 映射模式
    // QString mappingMode;
    // if(isOnlySaveLastDevice){
    //     mappingMode = getMappingModeFromFile();
    // }else{
    //     mappingMode = ui->radioButton->isChecked() ? KEYBOARD : XBOX;
    // }

    QString lastDeviceName = "";
    for(auto deviceName : currentSelectedDeviceList){
        lastDeviceName.append(deviceName).append(SPE);
    }
    // 去掉最后的分隔符
    if(lastDeviceName.contains(SPE)){
        lastDeviceName = lastDeviceName.left(lastDeviceName.length() - QString(SPE).length());
    }

    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + LAST_DEVICE_FILENAME);
    QString text2;
    text2.append(lastDeviceName + "\n");
    //text2.append(mappingMode);

    if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file2);  // 创建一个文本流对象
        out << text2;             // 写入文本

        // 关闭文件
        file2.close();
        qDebug() << "last device saved successfully!";
    } else {
        qDebug() << "Error opening file!";
    }
}

QString MainWindow::saveMappingsToFile(std::string filename){
    // 要保存的文本内容
    QString text;
    text.append("[\n\t");

    // 生成内容
    for(auto item: mappingList){
        if(item != nullptr && !item->dev_btn_name.empty()){
            text.append("{");

            text.append("\"dev_btn_name\":\"" + item->dev_btn_name + "\"").append(", ");
            text.append("\"dev_btn_type\":\"" + item->dev_btn_type + "\"").append(", ");
            text.append("\"keyboard_name\":\"" + (item->keyboard_name == "\\" ? "\\\\" : item->keyboard_name) + "\"").append(", ");
            text.append("\"keyboard_value\":" + std::to_string(item->keyboard_value)).append(", ");
            text.append("\"remark\":\"" + item->remark + "\"").append(", ");
            text.append("\"rotateAxis\":" + std::to_string(item->rotateAxis)).append(", ");
            text.append("\"btnTriggerType\":" + std::to_string(item->btnTriggerType)).append(", ");
            text.append("\"mappingType\":" + QString::number((int)item->mappingType)).append(", ");
            text.append("\"deviceName\":\"" + item->deviceName + "\"");// 最后一个, 后面不用加逗号
          
            text.append("},\n\t");
        }
    }
    if (text.endsWith(",\n\t")) {
        text.chop(3); // 删除最后多余的 ",\n\t"
    }
    text.append("\n]");

    // 获取当前目录
    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    // 如果filename不为空, 则检查配置文件文件夹是否存在,不存在就创建
    if(filename.size() > 0){
        // 使用 QDir 创建不存在的目录
        if (!dir.exists(USER_MAPPINGS_DIR)) {
            if (!dir.mkdir(USER_MAPPINGS_DIR)) {
                showErrorMessage(new std::string("创建存放用户配置的文件夹失败"));
                return "";
            }
        }
    }


    QString oldFileName;

    // 删除旧文件
    if(!filename.empty() && mappingFileNameMap.contains(ui->comboBox_2->currentText())){
        QFile oldFile(mappingFileNameMap[ui->comboBox_2->currentText()]);
        if(oldFile.exists()){
            oldFile.remove();
        }
    }

    // 最终保存的绝对路径文件名
    QString finalFileName = "";
    // 最终保存的无后缀文件名
    QString finalFileBaseName = "";

    // 如果filename不为空, 则使用USER_MAPPINGS_DIR + filename + MAPPING_FILE_SUFFIX后缀为文件名
    if(filename.size() > 0){
        finalFileBaseName.append(filename);

        QString multiDeviceName = "";
        // 当前为多设备
        if(currentSelectedDeviceList.size() > 1){
            multiDeviceName.append("_[");
            for(auto deviceName : currentSelectedDeviceList){
                multiDeviceName.append(deviceName).append(",");
            }
            multiDeviceName.append("]");
        }

        QFile file(appDataDirPath + USER_MAPPINGS_DIR + finalFileBaseName + multiDeviceName + MAPPING_FILE_SUFFIX);
        // 有重名文件
        if(file.exists()){
            // 当前文件名增加一个序号, 直到不重名
            for(int i = 1; ;i++){
                QString tempFileBaseName = finalFileBaseName + " (" + std::to_string(i).data() + ")";
                QFile tempFile(appDataDirPath + USER_MAPPINGS_DIR + tempFileBaseName + multiDeviceName + MAPPING_FILE_SUFFIX);
                // 该文件名不存在
                if(!tempFile.exists()){
                    finalFileBaseName = tempFileBaseName;
                    break;
                }
            }
        }

        // 生成最终文件绝对路径名
        finalFileName.append(appDataDirPath).append(USER_MAPPINGS_DIR).append(finalFileBaseName).append(multiDeviceName).append(MAPPING_FILE_SUFFIX);
    }else{
        finalFileName.append(MAPPINGS_FILENAME);
    }

    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file(finalFileName);

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);  // 创建一个文本流对象
        out << text;             // 写入文本

        // 关闭文件
        file.close();

        qDebug() << "mappings saved successfully!";

        if(!filename.empty()){
            qDebug() << "fianlFileBaseName: " << finalFileBaseName;
            return finalFileBaseName;
        }

    } else {
        qDebug() << "Error opening file!";
    }

    return "";
}

void MainWindow::loadSettings(){
    // 要读取的文件路径
    QFile file(appDataDirPath + "settings");

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在: settings";
        return;
    }

    // 打开文件进行读取
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // 逐行读取文件
        // 逐行读取文件
        if(!in.atEnd()) {
            QString line0 = in.readLine(); // 读取一行
            bool ok1, ok2;
            double var1 = line0.toDouble(&ok1);
            if(ok1 && var1 >= 0 && var1 <= 1){
                innerDeadAreaPanti = var1;
            }

            if(!in.atEnd()) {
                QString line1 = in.readLine(); // 读取一行
                double var2 = line1.toDouble(&ok2);
                if(ok2 && var2 >= 0 && var2 <= 1){
                    innerDeadAreaTaban = var2;
                }
            }
        }

        // 关闭文件
        file.close();
    } else {
        qDebug() << "Error opening file!";
    }
}

// 从文件中获取上一次运行选择的映射模式
QString MainWindow::getMappingModeFromFile(){
    // 要读取的文件路径
    QFile file(appDataDirPath + LAST_DEVICE_FILENAME);

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在: " << LAST_DEVICE_FILENAME;
        pushToQueue(parseWarningLog("记录上一次使用的设备的缓存文件不存在!"));
        return KEYBOARD;
    }

    // 打开文件进行读取
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // 逐行读取文件
        if(!in.atEnd()) {
            QString line0 = in.readLine(); // 读取一行
                if(!in.atEnd()) {
                    if(in.readLine().toStdString() == XBOX){
                        return XBOX;
                    }
                }
            }

        // 关闭文件
        file.close();
    } else {
        qDebug() << "Error opening file!";
        pushToQueue(parseErrorLog("打开记录上一次使用的设备的缓存文件失败!"));
    }

    return KEYBOARD;
}

void MainWindow::loadLastDeviceFile(){
    // 要读取的文件路径
    QFile file(appDataDirPath + LAST_DEVICE_FILENAME);

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在: " << LAST_DEVICE_FILENAME;
        pushToQueue(parseWarningLog("记录上一次使用的设备的缓存文件不存在!"));
        return;
    }

    // 打开文件进行读取
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // 逐行读取文件
        // 逐行读取文件
        if(!in.atEnd()) {
            QString line0 = in.readLine(); // 读取一行

            qDebug("读取上一次使用的设备成功");
            pushToQueue(parseSuccessLog("读取上一次使用的设备成功!"));

            if(hasLastDevInCurrentDeviceList(line0.toStdString())){
                qDebug("上一次使用的设备在当前设备列表中");
                pushToQueue("上一次使用的设备在当前设备列表中, 自动选择该设备");

                auto deviceList = line0.split(SPE);
                // 将上一次使用的设备添加到当前选择的设备列表
                currentSelectedDeviceList.append(deviceList);

                // 更新已选择设备label
                updateSelectedDeviceLabel();
            }

            // if(!in.atEnd()) {
            //     if(in.readLine().toStdString() == XBOX){
            //         ui->radioButton_2->setChecked(true);
            //         //ui->label_7->setText("Xbox按键");
            //         setDefaultMappingType(MappingType::Xbox);
            //     }
            // }

        }

        // 关闭文件
        file.close();
    } else {
        qDebug() << "Error opening file!";
        pushToQueue(parseErrorLog("打开记录上一次使用的设备的缓存文件失败!"));
    }
}

void MainWindow::loadMappingsFile(std::string filename){
    // 未选择设备, 但是要求加载上一次使用的配置
    if(currentSelectedDeviceList.isEmpty() && filename.size() <= 0 ){
        qDebug() << "未选择设备, 无法加载历史映射文件!";
        return;
    }

    QString finalFilePath;
    if(!filename.empty()){
        if(mappingFileNameMap.contains(filename.data())){
            finalFilePath = mappingFileNameMap[filename.data()];
        }else{
            QMessageBox::critical(this, "错误", ("加载配置文件[" + filename + "]失败:\n\n获取该文件绝对路径失败!").data());
            return;
        }
    }else{
        finalFilePath = appDataDirPath + MAPPINGS_FILENAME;
    }

    // 要读取的文件路径
    QString filePath = finalFilePath;
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
        pushToQueue(parseWarningLog("打开配置文件["+filePath+"]失败!"));
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &error);
    // json格式错误
    if (error.error != QJsonParseError::NoError) {
        qDebug() << "JSON Parse Error:" << error.errorString();
        //return;
    }

    if (!doc.isNull() && doc.isArray()) {
        // 需要重新保存一次映射, 将其转换成新版配置
        bool needReSaveMappingsToFile = false;

        // 配置文件统一使用 ".di_mappings_config", ".di_xbox_mappings_config"需要转换成这个
        if (mappingFileNameMap[filename.data()].endsWith(MAPPING_FILE_SUFFIX_XBOX)) {
            // 需要重新保存一次映射, 将其转换成新版配置
            needReSaveMappingsToFile = true;
        }

        // 获取 JSON 对象数组
        QJsonArray jsonArray = doc.array();

        // 清空映射列表
        mappingList.clear();

        // 遍历json数组
        for (const QJsonValue &value : jsonArray) {
            if(value.isObject()){
                QJsonObject jsonObj = value.toObject();
                // 从json对象读取信息
                MappingRelation *mapping = new MappingRelation();
              
                mapping->dev_btn_name = (jsonObj.contains("dev_btn_name")) ? jsonObj["dev_btn_name"].toString().toStdString() : "";
                mapping->dev_btn_type = (jsonObj.contains("dev_btn_type")) ? jsonObj["dev_btn_type"].toString().toStdString() : "";
                mapping->keyboard_name = (jsonObj.contains("keyboard_name")) ? jsonObj["keyboard_name"].toString().toStdString() : "";
                mapping->keyboard_value = (jsonObj.contains("keyboard_value")) ? jsonObj["keyboard_value"].toInt() : 0;
                mapping->remark = (jsonObj.contains("remark")) ? jsonObj["remark"].toString().toStdString() : "";
                mapping->rotateAxis = (jsonObj.contains("rotateAxis") && jsonObj["rotateAxis"].toInt() == 1) ? 1 : 0;
                mapping->btnTriggerType =
                    (jsonObj.contains("btnTriggerType") && jsonObj["btnTriggerType"].toInt() > 0 && jsonObj["btnTriggerType"].toInt() < TriggerTypeEnum::End)
                                              ? static_cast<TriggerTypeEnum>(jsonObj["btnTriggerType"].toInt())
                                              : TriggerTypeEnum::Normal;

                if (jsonObj.contains("mappingType")) {
                    // 读取映射类型
                    if ((MappingType)jsonObj["mappingType"].toInt() == MappingType::Xbox) {
                        mapping->mappingType = MappingType::Xbox;
                    } else if ((MappingType)jsonObj["mappingType"].toInt() == MappingType::Keyboard) {
                        mapping->mappingType = MappingType::Keyboard;
                    }
                }else{
                    // 配置文件没有mappingType信息, 说明需要重新保存一次映射, 将其转换成新版配置
                    needReSaveMappingsToFile = true;

                    if (mappingFileNameMap[filename.data()].endsWith(MAPPING_FILE_SUFFIX_XBOX)) {
                        mapping->mappingType = MappingType::Xbox;
                    }
                }

                mapping->deviceName = (jsonObj.contains("deviceName") ? jsonObj["deviceName"].toString() : "");

                // 按键名称不为空才添加进列表
                if(!mapping->dev_btn_name.empty()){
                    mappingList.push_back(mapping);
                }
            }
        }

        // 关闭文件
        file.close();

        // 重新保存一次配置
        if(needReSaveMappingsToFile && !filename.empty()){
            QString saveFileShortName = saveMappingsToFile(filename);

            // 重新扫描一遍
            scanMappingFile();
            ui->comboBox_2->setCurrentText(saveFileShortName);
        }

        return;
    }else{
        // 读取旧版配置文件

        // 关闭文件
        if(file.isOpen()){
            file.close();
        }

        // 打开文件进行读取
        if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            QTextStream in(&file);

            // 清空映射列表
            mappingList.clear();

            // 逐行读取文件
            while (!in.atEnd()) {
                QString line = in.readLine(); // 读取一行
                QStringList list = line.split(SPE);
                if(list.size() >= 5){
                    MappingRelation *mapping = new MappingRelation();
                    mapping->dev_btn_name = list[0].toStdString();
                    mapping->dev_btn_type = list[1].toStdString();
                    mapping->keyboard_name = list[2].toStdString();
                    mapping->keyboard_value = list[3].toShort();
                    mapping->remark = list[4].toStdString();
                    // 是否旋转轴
                    if(list.size() >= 6 && list[5] != nullptr && !list[5].isEmpty() && list[5].toInt() == 1){
                        mapping->rotateAxis = 1;
                    }
                    // 按键触发模式
                    if(list.size() >= 7 && list[6] != nullptr && !list[6].isEmpty() && list[6].toInt() > 0){
                        mapping->btnTriggerType = static_cast<TriggerTypeEnum>(list[6].toInt());
                    }else{
                        mapping->btnTriggerType = TriggerTypeEnum::Normal;
                    }
                    mappingList.push_back(mapping);
                }
            }

            // 关闭文件
            file.close();
        } else {
            qDebug() << "Error opening file!";
        }

        // 将旧版配置文件转为新版配置文件
        QString saveFileShortName = saveMappingsToFile(filename);

        // 重新扫描一遍
        scanMappingFile();
        ui->comboBox_2->setCurrentText(saveFileShortName);
    }
}

void MainWindow::onDeleteBtnClicked(){
    // 获取触发信号的对象
    QPushButton *deleteBtn = qobject_cast<QPushButton*>(sender());

    // 获取到该映射的位置
    int rowIndex = deleteBtn->objectName().toInt();

    if(rowIndex < 0){
        return;
    }

    // 释放该位置的指针
    //delete mappingList[rowIndex];
    // 替换为空指针
    //mappingList[rowIndex] = nullptr;

    // 将该映射的按键名称置空
    mappingList[rowIndex]->dev_btn_name = "";
    mappingList[rowIndex]->keyboard_value = 0;

    // 删除该行所有组件
    QList<QWidget*> widgets = this->findChildren<QWidget*>(deleteBtn->objectName());
    for (QWidget *widget : widgets) {
        //widget->hide();
        widget->deleteLater();
    }
}

MappingRelation* MainWindow::getDevBtnData(){
    // 初始化DirectInput
    if(!initDirectInput()){
        return nullptr;
    }
    // 连接设备
    if(!openDiDevice(getCurrentSelectedDeviceList())){
        return nullptr;
    }

    // 设置一个3s定时器，时间到后停止监听
    bool isListening = true;
    QTimer timer;
    timer.setSingleShot(true);
    timer.start(3000);
    connect(&timer, &QTimer::timeout, [&isListening](){ isListening = false;});
    
    // 获取第一次数据
    bool isFirstData = true;
    qint64 timeTick = QDateTime::currentMSecsSinceEpoch();
    std::map<QString, BUTTONS_VALUE_TYPE> firstKeyStateMap;
    std::map<std::string, int> tempRecord;

    bool enableLogs = getEnablePovLog() || getEnableBtnLog() || getEnableAxisLog();

    // 监听设备按键状态
    while(isListening){
        QApplication::processEvents(); // 处理事件队列，防止界面卡死
        // 获取设备状态数据
        auto res = getInputState(enableLogs);

        // 获取初始按键状态
        if (isFirstData) {
            // 跳过前50ms的数据，第一次采集的数据为空，可能是BUG
            if (QDateTime::currentMSecsSinceEpoch() - timeTick > 50) {
                isFirstData = false;
                auto firstKeyRes = getInputState(enableLogs); // 获取按键状态，第一次获取为0，应该是BUG
                for (auto item : firstKeyRes) {
                    if (item->dev_btn_type == WHEEL_BUTTON) {
                        QString deviceName = item->deviceName;
                        if (firstKeyStateMap.find(deviceName) == firstKeyStateMap.end()) {
                            // 记录第一次数据
                            firstKeyStateMap.insert_or_assign(deviceName, item->dev_btn_bit_value);
                        } else {
                            // 不是第一次读到该按键的值, 叠加
                            firstKeyStateMap[deviceName] |= item->dev_btn_bit_value;
                        }
                    }
                }
            } else {
                continue;
            }
        }

        if(res.size() > 0){
            for(auto item : res){
                // 方向盘的轴
                auto btnOrAxisStr = item->deviceName.toStdString() + "-" + item->dev_btn_name;
                if(item->dev_btn_type == (std::string)WHEEL_AXIS){
                    auto tmpAxis = tempRecord.find(btnOrAxisStr);
                    // 记录第一次数据
                    if(tmpAxis == tempRecord.end()){
                        tempRecord.insert_or_assign(btnOrAxisStr, item->dev_btn_value);
                    }else{
                        // 不是第一次读到该轴的值, 与第一次的值比较, 大于一定量才能确定是该轴要新建映射
                        if(std::abs(item->dev_btn_value - tmpAxis->second) > AXIS_CHANGE_VALUE){
                            // 确定该轴是映射键盘还是xbox
                            // 创建一个 QMessageBox
                            QMessageBox confirm1;
                            confirm1.setWindowTitle("请确认");
                            confirm1.setText("检测到设备[" + item->deviceName + "]的轴\n\n请确认该轴是 映射键盘按键 还是 映射xbox手柄");
                            // 设置图标
                            confirm1.setIcon(QMessageBox::Information);
                            // 创建并添加自定义按钮
                            QPushButton* mappingToKeyBoard = confirm1.addButton("映射键盘按键", QMessageBox::ActionRole);
                            QPushButton* mappingToXbox = confirm1.addButton("映射xbox手柄", QMessageBox::ActionRole);
                            QPushButton* cancel1 = confirm1.addButton("取消", QMessageBox::RejectRole);
                            // 设置默认按钮
                            confirm1.setDefaultButton(cancel1);
                            confirm1.exec();

                            // 映射键盘按键
                            if(confirm1.clickedButton() == mappingToKeyBoard){
                                // 如果已经存在该轴的映射, 直接返回
                                if(hasAddToMappingList(QString(item->dev_btn_name.data()), item->deviceName)){
                                    showErrorMessage(new std::string("设备[" + item->deviceName.toStdString() + "]的 \"" + item->dev_btn_name + "\" 已经配置了映射!"));
                                    // 点击取消, 清空按键名称
                                    item->dev_btn_name = "";
                                    return item;
                                }

                                // 确定是方向盘转向轴还是踏板轴
                                // 创建一个 QMessageBox
                                QMessageBox msgBox;
                                msgBox.setWindowTitle("请确认");
                                msgBox.setText("轴映射键盘按键需要区分转向轴和踏板轴\n\n请确认是转向轴, 还是踏板轴");
                                // 设置图标
                                msgBox.setIcon(QMessageBox::Information);
                                // 创建并添加自定义按钮
                                QPushButton* panti = msgBox.addButton("是转向轴", QMessageBox::ActionRole);
                                QPushButton* taban = msgBox.addButton("是踏板轴", QMessageBox::ActionRole);
                                QPushButton* cancel = msgBox.addButton("取消", QMessageBox::RejectRole);
                                // 设置默认按钮
                                msgBox.setDefaultButton(cancel);
                                // 显示消息框
                                msgBox.exec();

                                item->mappingType = MappingType::Keyboard;

                                // 判断用户点击'是转向轴'按钮
                                if (msgBox.clickedButton() == panti){
                                    // 值增大, 说明是转向轴右转
                                    if(item->dev_btn_value > tmpAxis->second){
                                        item->dev_btn_name += "右转";
                                        return item;
                                    }else if(item->dev_btn_value < tmpAxis->second){
                                        item->dev_btn_name += "左转";
                                        return item;
                                    }
                                }else if(msgBox.clickedButton() == taban){
                                    // 如果已经存在该轴的左转或右转映射, 直接返回
                                    if(hasAddToMappingList(QString(item->dev_btn_name.data()) + "左转", item->deviceName)
                                        || hasAddToMappingList(QString(item->dev_btn_name.data()) + "右转", item->deviceName)){
                                        showErrorMessage(new std::string("设备[" + item->deviceName.toStdString() + "]的 \""
                                                                         + item->dev_btn_name + "左转\" 或 "
                                                                         + item->dev_btn_name + "右转\"" + " 已经配置了映射! \n\n 不能再重复配置!"));
                                        // 点击取消, 清空按键名称
                                        item->dev_btn_name = "";
                                        return item;
                                    }

                                    // 踏板
                                    return item;
                                }else{
                                    // 点击取消, 清空按键名称
                                    item->dev_btn_name = "";
                                    return item;
                                }
                            }else if(confirm1.clickedButton() == mappingToXbox){
                                // 如果已经存在该轴的左转或右转映射, 直接返回
                                if(hasAddToMappingList(QString(item->dev_btn_name.data()) + "左转", item->deviceName)
                                   || hasAddToMappingList(QString(item->dev_btn_name.data()) + "右转", item->deviceName)){
                                    showErrorMessage(new std::string("设备[" + item->deviceName.toStdString() + "]的 \""
                                                                     + item->dev_btn_name + "左转\" 或 "
                                                                     + item->dev_btn_name + "右转\"" + " 已经配置了映射! \n\n 不能再重复配置!"));
                                    // 点击取消, 清空按键名称
                                    item->dev_btn_name = "";
                                    return item;
                                }

                                item->mappingType = MappingType::Xbox;
                                return item;
                            }else{
                                // 点击取消, 清空按键名称
                                item->dev_btn_name = "";
                                return item;
                            }
                        }
                    }
                }else{
                    // 方向盘按键
                    if(firstKeyStateMap.size() > 0){
                        QString deviceName = item->deviceName;
                        BUTTONS_VALUE_TYPE nowValue = item->dev_btn_bit_value;
                        for (auto firstKey : firstKeyStateMap) {
                            if (firstKey.first == deviceName && (firstKey.second != nowValue)) {
                                // 找出不同的按键
                                BUTTONS_VALUE_TYPE btnValue = nowValue ^ firstKey.second;
                                item->dev_btn_name = ButtonsValueTypeToString(btnValue);
                                item->dev_btn_bit_value = btnValue;
                                return item;
                            }
                        }
                    } else {
                        return item;
                    }
                }
            }
        }

        // 释放res内存
        qDeleteAll(res);  // 删除所有指针指向的对象
        res.clear();      // 清空列表
    }

    return nullptr;
}

void MainWindow::onLineEditTextChanged(const QString &text){
    // 获取触发信号的对象
    QLineEdit *lineEdit = qobject_cast<QLineEdit*>(sender());

    // 获取到该映射的位置
    int rowIndex = lineEdit->objectName().toInt();

    // 更新备注信息
    mappingList[rowIndex]->remark = text.toStdString();
}

std::map<std::string, short> MainWindow::getConstKeyMap(std::string dev_btn_type, MappingType mappingType){
    if(mappingType == MappingType::Xbox){
        if(dev_btn_type == (std::string)WHEEL_BUTTON){
            return VK_XBOX_BTN_MAP;
        }

        return VK_XBOX_AXIS_MAP;
    }

    return VK_MAP;

}

void MainWindow::updateASwitchPushButton(QPushButton *btn, MappingType mappingType){
    if(mappingType == MappingType::Keyboard){
        btn->setText("> 键盘 <");
        btn->setStyleSheet("QPushButton{background-color:rgb(251, 251, 251);margin-left:7;color: black;}");
    }else if (mappingType == MappingType::Xbox){
        btn->setText("> Xbox <");
        btn->setStyleSheet("QPushButton{background-color:rgb(116, 224, 116);margin-left:7;color: black;}");
    }
}
    
void MainWindow::updateAKeyBoardComboBox(QComboBox *comboBox, std::string dev_btn_type, MappingType mappingType){
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

// 创建一个键盘按键下拉选择框
QComboBox* MainWindow::createAKeyBoardComboBox(std::string dev_btn_type, MappingType mappingType){
    QComboBox *comboBox = new QComboBox();
    comboBox->setCurrentIndex(-1);
    comboBox->setMaximumHeight(36);
    comboBox->setMinimumHeight(36);
    comboBox->setMinimumWidth(150);
    comboBox->setMaximumWidth(150);
    comboBox->setStyleSheet("QComboBox{padding-left:10px;}");

    // 添加下拉框选择项
    updateAKeyBoardComboBox(comboBox, dev_btn_type, mappingType);

    return comboBox;
}


void MainWindow::onKeyBoardComboBoxActivated(int index){
    // 获取触发信号的对象
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());

    // 获取到该映射的位置
    int rowIndex = comboBox->objectName().toInt();

    MappingRelation *mapping = mappingList[rowIndex];

    QList<std::map<std::string, short>> mapList = {VK_MAP, VK_XBOX_BTN_MAP, VK_XBOX_AXIS_MAP};
    for(auto map : mapList){
        // 在键盘 按键名称与虚拟值 map中根据名称查找出值, 并更新到已配置的mapping中
        auto item = map.find(comboBox->currentText().toStdString());  // 查找键为 key 的元素
        if (item != map.end()) {
            qDebug("key_name:%s, key_value:%d", item ->first.data(), item->second);

            mapping->keyboard_name = item ->first;
            mapping->keyboard_value = item->second;

            //break;
        }
    }
}


void MainWindow::on_comboBox_activated(int index)
{
    if(getIsRunning()){
        ui->comboBox->setCurrentIndex(-1);
        QMessageBox::critical(this, "错误", "请先停止全局映射!");
        return;
    }

    //qDebug("index:%d", index);
    if(index >= 0){
        // 添加选择的设备进列表
        if(!currentSelectedDeviceList.contains(ui->comboBox->currentText())){
            currentSelectedDeviceList.append(ui->comboBox->currentText());

            // 更新label
            updateSelectedDeviceLabel();

            // 重置配置文件下拉
            ui->comboBox_2->clear();
            ui->comboBox_2->addItem("空白配置");
            // 重置配置选择下拉为空白配置
            ui->comboBox_2->setCurrentIndex(0);
            // 重置当前配置文件名
            currentMappingFileName = "";
            // 重新扫描配置文件
            scanMappingFile();

            // 清空映射列表
            clearMappingsArea();
            mappingList.clear();
        }

        ui->comboBox->setCurrentIndex(-1);
    }
}

void MainWindow::clearMappingsArea(){
    // 遍历布局中的所有项
    while (QLayoutItem *item = ui->gridLayout->takeAt(0)) {
        // 如果该项是一个 QWidget，删除它
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();  // 异步删除以防止立即删除导致问题
        }
        // 删除布局项
        delete item;
    }
}


void MainWindow::on_pushButton_4_clicked()
{
    if(getMappingListActualSize() <= 0){
        showErrorMessage(new std::string("配置为空, 无法保存!"));
        return;
    }

    QString srcFileName = "";
    auto selectedFileShortName = ui->comboBox_2->currentText();
    if(!selectedFileShortName.isEmpty()){
        auto filePath = mappingFileNameMap[selectedFileShortName];
        QFile file(filePath);
        if(file.exists()){
            QFileInfo fileInfo(file);
            srcFileName = fileInfo.completeBaseName();
        }
    }

    // 创建一个输入对话框
    bool ok;
    QString text = QInputDialog::getText(this, "保存配置", "请输入配置名称:", QLineEdit::Normal, srcFileName, &ok);

    // 如果用户点击了OK，并且输入有效
    if (ok) {
        if(!text.isEmpty()){
            // 保存配置
            QString saveFileShortName = saveMappingsToFile(text.toStdString());

            // 重新扫描一遍
            scanMappingFile();

            ui->comboBox_2->setCurrentText(saveFileShortName);

            // 重置当前配置文件名
            //currentMappingFileName = "";
            currentMappingFileName = text.toStdString();

            QMessageBox::information(this, "提醒", "配置保存成功!");
        }else {
            showErrorMessage(new std::string("配置名称不能为空!"));
            return;
        }
    }
}


void MainWindow::on_comboBox_2_activated(int index)
{
    if(getIsRunning()){
        if(currentMappingFileName.empty()){
            ui->comboBox_2->setCurrentIndex(-1);
        }else{
            ui->comboBox_2->setCurrentText(currentMappingFileName.data());
        }

        QMessageBox::critical(this, "错误", "请先停止全局映射!");
        return;
    }

    // 清空一切
    mappingList.clear();
    clearMappingsArea();
    currentMappingFileName = "";

    // 选择了空白配置
    if(index == 0){
        return;
    }

    // 获取触发信号的对象
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());

    // 获取配置文件名称
    std::string filename = comboBox->currentText().toStdString();

    // 配置没变, 不需要做后续操作
    if(currentMappingFileName == filename){
        return;
    }

    // 设置当前配置文件名
    currentMappingFileName = filename;

    qDebug() << "选择的配置文件名: " + filename;

    // 加载配置
    loadMappingsFile(filename);

    // 重画配置映射界面
    repaintMappings();
}

int MainWindow::getMappingListActualSize(){
    int size = 0;
    for(auto item : mappingList){
        if(item != nullptr){
            size++;
        }
    }

    return size;
}




// 检查驱动是否安装
bool MainWindow::checkDriverInstalled() {
    QSettings *settings = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\Nefarius Software Solutions e.U.\\ViGEm Bus Driver", QSettings::NativeFormat);
    if(settings->contains("Version")){
        qDebug("驱动已安装");
        return true;
    }else{
        //QMessageBox::information(nullptr, "", "");

        //获取当前目录
        QDir dir = QDir(QCoreApplication::applicationDirPath());
        dir.cd("driver");
        std::string dirPath = QDir::toNativeSeparators(dir.absolutePath()).toStdString();

        // 创建一个 QMessageBox 对象，带有 OK 和 Cancel 按钮
        QMessageBox msgBox;
        msgBox.setWindowTitle("提醒");
        msgBox.setText(("检测到虚拟手柄驱动未安装, 点击确认将前往资源管理器, 请手动安装驱动程序\n驱动安装包所在文件夹:\n" + dirPath).data());
        // 设置图标
        msgBox.setIcon(QMessageBox::Warning);
        // 创建并添加自定义按钮
        QPushButton* ok = msgBox.addButton("确认", QMessageBox::ActionRole);
        QPushButton* cancel = msgBox.addButton("取消", QMessageBox::RejectRole);

        // 设置默认按钮
        msgBox.setDefaultButton(ok);

        // 显示消息框
        msgBox.exec();

        // 判断用户点击确认按钮
        if (msgBox.clickedButton() == ok) {
            // 用户点击了 OK

            //QMessageBox::information(nullptr, "", QDir::toNativeSeparators(dir.absolutePath()).toStdString().data());

            std::string command = "explorer \"" + dirPath + "\"";  // 使用双引号包裹路径，处理空格

            // 将窗口最小化
            showMinimized();

            // 打开资源管理器
            //std::system(command.c_str());

            // 使用 QProcess 执行命令
            QProcess::startDetached(command.data());
        }

        return false;
    }
}

void MainWindow::on_pushButton_6_clicked()
{
    // 清空设备列表
    diDeviceList.clear();
    // 清空下拉
    ui->comboBox->clear();

    // 重新扫描
    scanDevice();

    // 设备不为空
    if(!diDeviceList.empty()){
        ui->comboBox->setPlaceholderText("请选择设备");
        for(auto item : diDeviceList){
            ui->comboBox->addItem(item.name.data());
        }

        // if(hasLastDevInCurrentDeviceList(deviceName)){
        //     ui->comboBox->setCurrentText(deviceName.data());
        // }else{
        //     ui->comboBox->setCurrentIndex(-1);
        // }
    }else{
        ui->comboBox->setPlaceholderText("未扫描到设备");
    }
}


void MainWindow::on_pushButton_7_clicked()
{
    // 如果窗口是最小化状态, 清除最小化
    if(this->logWindow->windowState() == Qt::WindowMinimized){
        this->logWindow->setWindowState(this->logWindow->windowState() & ~Qt::WindowMinimized);
    }
    this->logWindow->show();
    this->logWindow->activateWindow();
}

// 模拟服务报错的slot
void MainWindow::simulateMsgboxSlot(bool isError, QString text){
    if(isError){
        QMessageBox::critical(this, "错误", text);
    }else{
        // 主窗口最小化, 不显示提醒消息框
        if (this->isMinimized()) {
            return;
        }
        QMessageBox::information(this, "提醒", text);
    }
}

void MainWindow::simulateStartedSlot(){
    ui->label_4->setText("已启动");
    ui->label_4->setStyleSheet("QLabel{color: rgb(0, 160, 0);}");

    //disableUiAfterStartMapping();
}


void MainWindow::on_pushButton_8_clicked()
{
    // 合并映射键盘模式轴死区设置窗口 和 xbox死区设置窗口
    // 如果窗口是最小化状态, 清除最小化
    if(this->deadareaSettings->windowState() == Qt::WindowMinimized){
        this->deadareaSettings->setWindowState(this->deadareaSettings->windowState() & ~Qt::WindowMinimized);
    }
    this->deadareaSettings->show();
    this->deadareaSettings->activateWindow();
}


void MainWindow::on_pushButton_9_clicked()
{
    //获取配置文件目录
    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    // 用户配置文件夹不存在, 创建一个
    if(dir.exists() && !dir.exists(USER_MAPPINGS_DIR)){
        dir.mkdir(USER_MAPPINGS_DIR);
    }

    // 目录存在
    if(dir.exists() && dir.cd(USER_MAPPINGS_DIR)){
        std::string dirPath = QDir::toNativeSeparators(dir.absolutePath()).toStdString();
        std::string command = "explorer \"" + dirPath + "\"";  // 使用双引号包裹路径，处理空格

        // 将窗口最小化
        showMinimized();

        // 使用 QProcess 执行命令, 打开资源管理器
        QProcess::startDetached(command.data());
    }else{
        QMessageBox::information(this, "提醒", "还没保存过配置");
    }

}


void MainWindow::on_pushButton_10_clicked()
{
    // 如果窗口是最小化状态, 清除最小化
    if(this->assistWindow->windowState() == Qt::WindowMinimized){
        this->assistWindow->setWindowState(this->assistWindow->windowState() & ~Qt::WindowMinimized);
    }
    this->assistWindow->show();
    this->assistWindow->activateWindow();

}

// 获取当前选择的设备的下标
QList<QString> MainWindow::getCurrentSelectedDeviceList(){
    return currentSelectedDeviceList;
}

void MainWindow::saveLastDeviceToFileSlot(){
    saveLastDeviceToFile(true);
}

// 获取免费api LeanCloud 的访问域名
void MainWindow::getApiHost(bool isSendUsage){
    QNetworkAccessManager *manager = new QNetworkAccessManager();

    // 设置请求 URL
    QUrl url("https://wenku.baidu.com/wpeditor/getdocument?axios_original=1");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 构造 JSON 数据
    QJsonObject json;
    json["fsid"] = 1101408526343678;
    json["scene"] = "fcbshare";
    json["uk"] = 556868662;
    json["shareid"] = "5xkPedtvDL88S2vwJnIoPvQItKbzJWDVtUyeJN4Kl7g";

    QByteArray data = QJsonDocument(json).toJson();

    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, data);

    // 连接信号槽处理响应
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QString responseStr = QString(response);
            // 使用正则匹配出域名
            QRegularExpression hostReg(R"((?<=(LeanCloud_host=))[^\"]+)");
            QRegularExpressionMatch matchRes = hostReg.match(response);
            if(matchRes.hasMatch()){
                QString host = matchRes.captured(0);
                if(!host.isEmpty()){
                    qDebug() << "从百度笔记中读取到api host: " << host;
                    if(isSendUsage){
                        this->sendUsageCount(host);
                    }else{
                        this->checkUpdate(host);
                    }
                }
            }
        } else {
            qDebug() << "Http Request Error:" << reply->errorString();
        }
        reply->deleteLater();
        manager->deleteLater();
    });

}


QString lastInvalidApiHost = ""; //上一次使用的无效api域名

// 发送软件使用统计
void MainWindow::sendUsageCount(QString apiHost){
    // 设置默认的api域名
    if(apiHost.isEmpty()){
        apiHost = DEFAULT_API_HOST;
    }
    // 当前域名无效
    if(apiHost == lastInvalidApiHost){
        return;
    }

    // 发送软件使用情况
    QNetworkAccessManager *manager = new QNetworkAccessManager();
    // 设置请求 URL
    QUrl url(apiHost + "/1.1/classes/usage_count");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("X-LC-Id", X_LC_Id);
    request.setRawHeader("X-LC-Key", X_LC_Key);

    // 获取主机名
    QString username = QHostInfo::localHostName();

    // 构造 JSON 数据
    QJsonObject json;
    json["username"] = username.isEmpty() ? "unknown" : username;
    json["date"] = QDate::currentDate().toString("yyyyMMdd");
    json["version"] = CURRENT_VERSION;

    QByteArray data = QJsonDocument(json).toJson();
    // 发送 POST 请求
    QNetworkReply *reply = manager->post(request, data);

    // 连接信号槽处理响应
    QObject::connect(reply, &QNetworkReply::finished, [=]() {
        if (reply->error() == QNetworkReply::NoError) {
            QByteArray response = reply->readAll();
            QString responseStr = QString(response);
            if(responseStr.contains("objectId")){
                qDebug() << "发送使用统计成功";
            }
        } else {
            qDebug() << "Http Request Error:" << reply->errorString();
            lastInvalidApiHost = apiHost;
            getApiHost();
        }
        reply->deleteLater();
        manager->deleteLater();
    });
}

// 发送软件使用统计
void MainWindow::checkUpdate(QString apiHost){
    // 设置默认的api域名
    if(apiHost.isEmpty()){
        apiHost = DEFAULT_API_HOST;
    }
    // 当前域名无效
    if(apiHost == lastInvalidApiHost){
        return;
    }

    // 检查软件更新
    QNetworkAccessManager *manager2 = new QNetworkAccessManager();
    // 设置请求 URL
    QUrl url2(apiHost + "/1.1/classes/key_mappings_tool_update/680a3dc5287e5660ffb108a0");
    QNetworkRequest request2(url2);
    request2.setRawHeader("X-LC-Id", X_LC_Id);
    request2.setRawHeader("X-LC-Key", X_LC_Key);

    // 发送 GET 请求
    QNetworkReply *reply2 = manager2->get(request2);

    // 连接信号槽处理响应
    QObject::connect(reply2, &QNetworkReply::finished, [=]() {
        if (reply2->error() == QNetworkReply::NoError) {
            QByteArray response = reply2->readAll();
            QString responseStr = QString(response);
            if(responseStr.contains("objectId")){
                QJsonDocument doc = QJsonDocument::fromJson(response);
                if(!doc.isNull() && doc.isObject()){
                    auto obj = doc.object();
                    // 获取最新版本号, 跟目前版本号比较
                    if(obj.contains("latest_version") && obj["latest_version"].toString() > CURRENT_VERSION){
                        qDebug() << "检查到有新版本";
                        QString text = "新版本: " + obj["latest_version"].toString()
                                        + "\n\n更新内容:\n" + (obj.contains("desc") && !obj["desc"].toString().isEmpty() ? obj["desc"].toString() : "暂无更新日志")
                                        + "";
                        // 创建一个 QMessageBox
                        QMessageBox msgBox;
                        msgBox.setWindowTitle("版本更新提醒");
                        msgBox.setText(text);
                        // 设置图标
                        msgBox.setIcon(QMessageBox::Information);
                        // 创建并添加自定义按钮
                        QPushButton* github = msgBox.addButton("github下载", QMessageBox::ActionRole);
                        QPushButton* lanzou = msgBox.addButton("蓝奏云下载", QMessageBox::ActionRole);
                        QPushButton* cancel = msgBox.addButton("取消", QMessageBox::RejectRole);
                        // 设置默认按钮
                        msgBox.setDefaultButton(cancel);
                        // 显示消息框
                        msgBox.exec();
                        // 判断用户点击'github下载'按钮
                        if (msgBox.clickedButton() == github){
                            // 打开网页链接
                            QDesktopServices::openUrl(QUrl(obj.contains("download_github") && !obj["download_github"].toString().isEmpty() ? obj["download_github"].toString() : ""));
                        }else if(msgBox.clickedButton() == lanzou){
                            // 打开网页链接
                            QDesktopServices::openUrl(QUrl(obj.contains("download_lanzou") && !obj["download_lanzou"].toString().isEmpty() ? obj["download_lanzou"].toString() : ""));
                        }
                    }
                }
            }
        } else {
            qDebug() << "Http Request Error:" << reply2->errorString();
            pushToQueue(parseWarningLog(QString("<b>检查版本更新</b>: api域名").append("[").append(apiHost).append("]访问失败, 错误信息: ").append(reply2->errorString())));

            lastInvalidApiHost = apiHost;
            getApiHost(false);
        }
        reply2->deleteLater();
        manager2->deleteLater();
    });
}



void MainWindow::on_pushButton_11_clicked()
{
    if(getIsRunning()){
        QMessageBox::critical(this, "错误", "请先停止全局映射!");
        return;
    }

    // 清空当前选择的设备
    currentSelectedDeviceList.clear();
    // 清空已初始化的设备
    clearInitedDeviceList();
    // 更新label
    updateSelectedDeviceLabel();

    // 重置配置文件下拉
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem("空白配置");
    // 重置配置选择下拉为空白配置
    ui->comboBox_2->setCurrentIndex(0);
    // 重置当前配置文件名
    currentMappingFileName = "";
    // 重新扫描配置文件
    scanMappingFile();

    // 清空一切
    mappingList.clear();
    clearMappingsArea();
}

// 启动全局映射后, 将部分控件设置为不可点击
void MainWindow::disableUiAfterStartMapping(){
    ui->comboBox->setEnabled(false);
    ui->pushButton_11->setEnabled(false);
    // ui->radioButton->setEnabled(false);
    // ui->radioButton_2->setEnabled(false);
    ui->comboBox_2->setEnabled(false);
    ui->pushButton->setEnabled(false);
}
// 停止全局映射后, 将部分控件恢复正常状态
void MainWindow::enableUiAfterStopMapping(){
    ui->comboBox->setEnabled(true);
    ui->pushButton_11->setEnabled(true);
    // ui->radioButton->setEnabled(true);
    // ui->radioButton_2->setEnabled(true);
    ui->comboBox_2->setEnabled(true);
    ui->pushButton->setEnabled(true);
}

// 更新已选择设备的label
void MainWindow::updateSelectedDeviceLabel(){
    QString labelText = "";

    if(currentSelectedDeviceList.size() == 0){
        ui->label_2->setText("已选择设备(<b style='color:red;'>0</b>):");
        ui->label_5->setText("暂无");
        ui->label_5->setStyleSheet("");
        ui->label_5->setToolTip("");
    }else{
        for(auto str : currentSelectedDeviceList){
            labelText.append(str).append(", ");
        }
        int index = labelText.lastIndexOf(", ");
        if(index >= 0){
            labelText = labelText.left(index);
        }

        ui->label_2->setText(QString("已选择设备(<b style='color: rgb(0, 160, 0);'>") + std::to_string(currentSelectedDeviceList.size()).data() + "</b>):");

        // 获取 QFontMetrics 对象
        QFontMetrics metrics(ui->label_5->font());
        int labelHeight = ui->label_5->height(); // 标签高度（像素）
        int lineHeight = metrics.height(); // 单行高度
        int maxLines = labelHeight/lineHeight; // 计算最大行数
        // 设置省略模式
        ui->label_5->setText(metrics.elidedText(labelText, Qt::ElideLeft,
                                          ui->label_5->width() * maxLines));

        ui->label_5->setStyleSheet("QLabel{color: rgb(0, 160, 0);}");
        ui->label_5->setToolTip(labelText.replace(", ", "<br>"));
    }
}

// 获取省略模式的文本(文本超出组件显示范围将显示省略号...)
QString MainWindow::getElidedText(QWidget* widget, QString srcText){
    return "";
}


bool MainWindow::hasSameNameMappingFile(QString newFileName){
    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    if(dir.exists(USER_MAPPINGS_DIR)){
        dir.cd(USER_MAPPINGS_DIR);
        // 获取文件列表，并筛选指定后缀的文件
        QFileInfoList list = dir.entryInfoList();

        for(auto fileInfo : list){
            if(fileInfo.fileName() == newFileName + MAPPING_FILE_SUFFIX){
                return true;
            }
        }
    }

    return false;
}
