#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "key_map.h"
#include "BtnTriggerTypeEnum.h"
#include <QApplication>
#include <QMainWindow>
#include <QLabel>
#include <QVBoxLayout>
#include <QKeyEvent>
#include <QDebug>
#include <QMessageBox>
#include <QLineEdit>
#include<global.h>
#include<simulate_task.h>
#include<QFile>
#include<QInputDialog>
#include<QDir>
#include <QSettings>
#include<QProcess>
#include<cmath>
#include <QCheckBox>
#include<QTimer>
#include<QScrollBar>



MainWindow::~MainWindow(){
    cleanupDirectInput();
}

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(910, 584);
    this->setWindowTitle("KeyMappingsTool v1.0.8");
    //this->setWindowIcon(QIcon(":/icon/wheel_icon.png"));

    // 遍历所有设备
    // listAllDevice();

    // 获取软件本地数据目录
    appDataDirPath = QDir::homePath() + "/AppData/Local/KeyMappingToolData/";
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
    // comboBox->setStyleSheet("QComboBox { "
    //                         "border: 1px solid black; "  // 边框宽度和颜色
    //                         "border-radius: 1px; "       // 圆角半径
    //                         "} ");

    // 使用迭代器遍历
    for (int i=0; i< (int)diDeviceList.size(); i++) {
        comboBox->addItem(diDeviceList[i].name.data());
    }
    comboBox->setCurrentIndex(-1);
    if(diDeviceList.empty()){
        comboBox->setPlaceholderText("未扫描到设备");
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
    ui->radioButton->setChecked(true);

    // 加载上一次使用的设备
    loadLastDeviceFile();

    // 加载上一次的映射配置
    loadMappingsFile("");

    // 绘制出配置列表
    repaintMappings();

    // 配置下拉添加一个空白配置
    ui->comboBox_2->addItem("空白配置");

    // 扫描用户保存的配置文件
    scanMappingFile();

    // 日志窗口
    this->logWindow = new LogWindow();

    loadSettings();
    this->settings = new DeadAreaSettings();

    this->assistWindow = new AssistFuncWindow();

    if(this->assistWindow->getEnableMappingAfterOpening()){
        on_pushButton_2_clicked();
    }
}

void MainWindow::scanMappingFile(){
    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    if(dir.exists(USER_MAPPINGS_DIR)){
        dir.cd(USER_MAPPINGS_DIR);
        // 获取文件列表，并筛选指定后缀的文件
        QFileInfoList list = dir.entryInfoList();
        for (const QFileInfo &fileInfo : list) {
            if ("." + fileInfo.suffix() == (getIsXboxMode() ? MAPPING_FILE_SUFFIX_XBOX :MAPPING_FILE_SUFFIX)) {
                // 匹配指定后缀的文件，加入下拉列表
                ui->comboBox_2->addItem(fileInfo.completeBaseName());
            }
        }

        ui->comboBox_2->setCurrentIndex(-1);
    }
}

bool MainWindow::hasLastDevInCurrentDeviceList(std::string lastDeviceName){
    for(auto item : diDeviceList){
        if(item.name == lastDeviceName){
            return true;
        }
    }

    return false;
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
        if(deviceName.empty()){
            showErrorMessage(nullptr);
            return;
        }

        if(getMappingListActualSize() <= 0){
            showErrorMessage(new std::string("映射列表为空!"));
            return;
        }

        // xbox模式, 但驱动未安装, 无法启动
        if(getIsXboxMode() && !checkDriverInstalled()){
            qDebug("驱动未安装, 无法启动!");
            return;
        }

        // 初始化directInput
        if(!initDirectInput()){
            return;
        }
        // 连接设备
        if(!openDiDevice(ui->comboBox->currentIndex())){
            return;
        }

        // 创建监听设备输入数据的任务
        SimulateTask *task = new SimulateTask(&mappingList);
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
        ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(255, 170, 127);}");
    }
    else{
        if(deviceName.empty()){
            showErrorMessage(nullptr);
            return;
        }

        // 如果处于暂停状态, 将其重置
        if(getIsPause()){
            clickPauseBtn();
        }

        ui->label_4->setText("未启动");
        ui->label_4->setStyleSheet("QLabel{color: rgb(255, 85, 0);}");
        ui->pushButton_2->setText("启动全局映射");
        ui->pushButton_2->setStyleSheet("QPushButton{background-color:rgb(170, 255, 255);}");
        setIsRuning(false);

        if(!getIsXboxMode()){
            ui->pushButton_8->show();
        }

        // 显示信息框
        //QMessageBox::information(this, "提醒", "已停止全局映射!");
    }
}

void MainWindow::pauseClickSlot(){
    if(getIsRunning()){
        if(getIsPause()){
            ui->label_4->setText("已暂停");
            ui->label_4->setStyleSheet("QLabel{color: rgb(248, 201, 19);}");
        }else{
            ui->label_4->setText("已启动");
            ui->label_4->setStyleSheet("QLabel{color: rgb(0, 170, 0);}");
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

bool MainWindow::hasAddToMappingList(std::string btn_name){
    for(int i=0; i < (int)mappingList.size(); i++){
        if(mappingList[i] && (mappingList[i]->dev_btn_name == btn_name)){
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
    MappingRelation *mappingDevBtnData = nullptr;
    if(isClickNewLineBtn(mapping)){

        // 获取设备按下的按键位置和值
        mappingDevBtnData = getDevBtnData();

        if(mappingDevBtnData == nullptr){
            showErrorMessage(new std::string("未检测到按键被按下!"));
            return;
        }

        if(mappingDevBtnData->dev_btn_name.empty()){
            return;
        }


        // 检查该设备按键是否已经添加
        if(hasAddToMappingList(mappingDevBtnData->dev_btn_name)){
            showErrorMessage(new std::string("该设备按键已经配置了映射!"));
            return;
        }

        // 将设备按键数据添加进新的映射关系
        mappingList.push_back(mappingDevBtnData);
    }

    // 记录当前行的index值
    auto currentRowIndex = isClickNewLineBtn(mapping) ? std::to_string(mappingList.size() - 1) : std::to_string(index);

    QGridLayout *layout = ui->gridLayout;
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(7,7,7,7);

    // 画出表头
    if(index == 0 || getMappingListActualSize() == 1){
        QLabel *h1 = new QLabel("设备按键");
        h1->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h2 = new QLabel("映射成");
        h2->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h3 = new QLabel(getIsXboxMode() ? "Xbox按键" : "键盘按键");
        h3->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h4 = new QLabel("按键触发模式");
        h4->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        QLabel *h5 = new QLabel("备注");
        h5->setStyleSheet("font-weight: bold;"); // 设置样式表实现加粗

        int col = -1;
        layout->addWidget(h1, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h2, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h3, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h4, 0, ++col, Qt::AlignLeft);
        layout->addWidget(h5, 0, ++col, Qt::AlignLeft);
    }

    QLabel *label1 = new QLabel(isClickNewLineBtn(mapping)
            ? mappingDevBtnData->dev_btn_name.data()
            // 历史配置展示
            : mapping->dev_btn_name.data());
    label1->setMaximumHeight(30);
    label1->setMinimumHeight(30);
    label1->setMaximumWidth(90);
    label1->setStyleSheet("QLabel{color:blue;}");
    label1->setObjectName(currentRowIndex);

    QLabel *label2 = new QLabel("->");
    label2->setMaximumHeight(30);
    label2->setMinimumHeight(30);
    label2->setMaximumWidth(60);
    label2->setObjectName(currentRowIndex);


    // 键盘按键下拉框
    QComboBox *comboBox = createAKeyBoardComboBox(
        isClickNewLineBtn(mapping) ? mappingDevBtnData->dev_btn_type : mapping->dev_btn_type
        );
    // 设置一个序号, 为后续操作提供一个位置
    comboBox->setObjectName(currentRowIndex);
    // 历史配置展示
    if(!isClickNewLineBtn(mapping)){
        comboBox->setCurrentText(mapping->keyboard_name.data());
    }
    // 连接信号和槽
    connect(comboBox, &QComboBox::activated, this, &MainWindow::onKeyBoardComboBoxActivated);


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
    if(!isClickNewLineBtn(mapping)){
        triggerTypeComboBox->setCurrentText(TRIGGER_TYPE_ENUM_MAP[mapping->btnTriggerType].data());
    }
    // 连接信号和槽
    connect(triggerTypeComboBox, &QComboBox::activated, this, &MainWindow::onTriggerTypeComboBoxActivated);


    QLineEdit *lineEdit = new QLineEdit(isClickNewLineBtn(mapping) ? "" : mapping->remark.data());
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
    deleteBtn->setStyleSheet("QPushButton{background-color:rgb(255, 157, 157);margin-left:7;}");
    deleteBtn->setObjectName(currentRowIndex);
    // 绑定信号和槽
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteBtnClicked);


    int columnIndex = -1;
    int row = (index < 0 ? mappingList.size() : index) + 1;
    layout->addWidget(label1, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(label2, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(comboBox, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(triggerTypeComboBox, row, ++columnIndex, Qt::AlignLeft); // triggerTypeComboBox
    layout->addWidget(lineEdit, row, ++columnIndex, Qt::AlignLeft);
    layout->addWidget(deleteBtn, row, ++columnIndex, Qt::AlignLeft);

    if((mapping != nullptr && mapping->dev_btn_type == (std::string)WHEEL_AXIS)
        || (mappingDevBtnData != nullptr && mappingDevBtnData->dev_btn_type == (std::string)WHEEL_AXIS)){

        // 轴隐藏按键触发模式的下拉框
        triggerTypeComboBox->setDisabled(true);

        // 反转轴的勾选
        QCheckBox *checkBox = new QCheckBox("反转该轴", this);
        checkBox->setMaximumHeight(30);
        checkBox->setMinimumHeight(30);
        checkBox->setMaximumWidth(80);
        checkBox->setObjectName(currentRowIndex);
        // 保存的配置, 恢复
        if(!isClickNewLineBtn(mapping) && mapping->rotateAxis == 1){
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
    if(deviceName.empty()){
        showErrorMessage(nullptr);
        return;
    }

    //qDebug(("deviceName: " + deviceName).data());

    if(getIsRunning()){
        showErrorMessage(new std::string("请先停止全局模拟"));
        return;
    }

    // 禁用按钮，防止重复点击
    ui->pushButton->setEnabled(false); 
    ui->pushButton->setText("等待输入...");

    // 下面的函数会导致界面卡死, 需要重绘一下
    this->repaint ();

    paintOneLineMapping(nullptr, -1);

    QTimer::singleShot(50, [=](){
        QScrollBar *sbar = ui->scrollArea->verticalScrollBar();
        sbar->setValue(sbar->maximum());

        // 恢复按钮状态
        ui->pushButton->setText("新增按键映射");
        ui->pushButton->setEnabled(true);
    });
}

void MainWindow::saveLastDeviceToFile(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + LAST_DEVICE_FILENAME);  // 文件路径可以是绝对路径或相对路径
    QString text2;
    text2.append(ui->comboBox->currentText().toStdString() + "\n" + (ui->radioButton->isChecked() ? KEYBOARD : XBOX));
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

void MainWindow::saveMappingsToFile(std::string filename){
    // 要保存的文本内容
    QString text;

    // 生成内容
    for(auto item: mappingList){
        if(item != nullptr){
            text.append(item->dev_btn_name + SPE
                        + item->dev_btn_type + SPE
                        + item->keyboard_name + SPE
                        + std::to_string(item->keyboard_value) + SPE
                        + item->remark + SPE
                        + std::to_string(item->rotateAxis) + SPE
                        + std::to_string(item->btnTriggerType)
                        + "\n");
        }
    }

    // 获取当前目录
    QDir dir = appDataDirPath.isEmpty() ? QDir::current() : QDir(appDataDirPath);

    // 如果filename不为空, 则检查配置文件文件夹是否存在,不存在就创建
    if(filename.size() > 0){
        // 使用 QDir 创建不存在的目录
        if (!dir.exists(USER_MAPPINGS_DIR)) {
            if (!dir.mkdir(USER_MAPPINGS_DIR)) {
                showErrorMessage(new std::string("创建存放用户配置的文件夹失败"));
                return;
            }
        }
    }

    // 创建一个 QFile 对象，并打开文件进行写入
    // 如果filename不为空, 则使用USER_MAPPINGS_DIR + filename + MAPPING_FILE_SUFFIX后缀为文件名
    QFile file(filename.size() > 0
                   ? (appDataDirPath.toStdString() + USER_MAPPINGS_DIR + filename + (getIsXboxMode() ? MAPPING_FILE_SUFFIX_XBOX : MAPPING_FILE_SUFFIX)).data()
                   : (appDataDirPath.toStdString() + MAPPINGS_FILENAME).data());

    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);  // 创建一个文本流对象
        out << text;             // 写入文本

        // 关闭文件
        file.close();
        qDebug() << "mappings saved successfully!";
    } else {
        qDebug() << "Error opening file!";
    }
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
                pushToQueue(parseSuccessLog("上一次使用的设备在当前设备列表中, 自动选择该设备"));

                deviceName = line0.toStdString();
                ui->comboBox->setCurrentText(deviceName.data());

                if(!in.atEnd()) {
                    if(in.readLine().toStdString() == XBOX){
                        ui->radioButton_2->setChecked(true);
                        //ui->label_7->setText("Xbox按键");
                        setIsXboxMode(true);

                        // 隐藏死区设置
                        this->ui->pushButton_8->hide();
                    }
                }
            }

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
    if(deviceName.empty() && filename.size() <= 0 ){
        qDebug() << "未选择设备, 无法加载历史映射文件!";
        return;
    }

    // 要读取的文件路径
    QString filePath = filename.size() > 0
                           ? (appDataDirPath.toStdString() + USER_MAPPINGS_DIR + filename + (getIsXboxMode() ? MAPPING_FILE_SUFFIX_XBOX : MAPPING_FILE_SUFFIX)).data()
                           : (appDataDirPath.toStdString() + MAPPINGS_FILENAME).data();
    QFile file(filePath);

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在:" << filePath;
        return;
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
}

void MainWindow::onDeleteBtnClicked(){
    // 获取触发信号的对象
    QPushButton *deleteBtn = qobject_cast<QPushButton*>(sender());

    // 获取到该映射的位置
    int rowIndex = deleteBtn->objectName().toInt();

    // 释放该位置的指针
    delete mappingList[rowIndex];
    // 替换为空指针
    mappingList[rowIndex] = nullptr;

    // 删除该行所有组件
    QList<QWidget*> widgets = this->findChildren<QWidget*>(deleteBtn->objectName());
    for (QWidget *widget : widgets) {
        //widget->hide();
        widget->deleteLater();
    }

    // for(auto item : mappingList){
    //     if(item!=nullptr)
    //         qDebug(to_string(item->dev_btn_pos).data());
    // }
}

MappingRelation* MainWindow::getDevBtnData(){
    // 初始化DirectInput
    if(!initDirectInput()){
        return nullptr;
    }
    // 连接设备
    if(!openDiDevice(ui->comboBox->currentIndex())){
        return nullptr;
    }

    std::map<std::string, int> tempRecord;


    // 监听设备按键状态
    for(int i=0; i<60; i++){
        // 获取设备状态数据
        auto res = getInputState((getEnablePovLog() || getEnableBtnLog() || getEnableAxisLog())? true : false);
        if(res.size() > 0){
            for(auto item : res){
                // 方向盘的轴
                if(item->dev_btn_type == (std::string)WHEEL_AXIS){
                    auto tmpAxis = tempRecord.find(item->dev_btn_name);
                    // 第一次读到该轴的值
                    if(tmpAxis == tempRecord.end()){
                        tempRecord.insert_or_assign(item->dev_btn_name, item->dev_btn_value);
                    }else{
                        // 不是第一次读到该轴的值, 与第一次的值比较, 大于一定量才能确定是该轴要新建映射
                        if(std::abs(item->dev_btn_value - tmpAxis->second) > 500){
                            // 映射xbox模式
                            if(getIsXboxMode()){
                                return item;
                            }else{
                                // 映射键盘模式
                                // 确定是方向盘盘面转动轴还是踏板轴
                                // 创建一个 QMessageBox
                                QMessageBox msgBox;
                                msgBox.setWindowTitle("请确认");
                                msgBox.setText("检测到轴, 请确认是方向盘盘面转动, 还是踏板踩下");
                                // 设置图标
                                msgBox.setIcon(QMessageBox::Information);
                                // 创建并添加自定义按钮
                                QPushButton* panti = msgBox.addButton("是盘面轴", QMessageBox::ActionRole);
                                QPushButton* taban = msgBox.addButton("是踏板轴", QMessageBox::ActionRole);
                                QPushButton* cancel = msgBox.addButton("取消", QMessageBox::RejectRole);
                                // 设置默认按钮
                                msgBox.setDefaultButton(cancel);
                                // 显示消息框
                                msgBox.exec();
                                // 判断用户点击'是盘面轴'按钮
                                if (msgBox.clickedButton() == panti){
                                    // 值增大, 说明是盘面轴右转
                                    if(item->dev_btn_value > tmpAxis->second){
                                        item->dev_btn_name += "右转";
                                        return item;
                                    }else if(item->dev_btn_value < tmpAxis->second){
                                        item->dev_btn_name += "左转";
                                        return item;
                                    }
                                }else if(msgBox.clickedButton() == taban){
                                    // 踏板
                                    return item;
                                }else{
                                    // 点击取消, 清空按键名称
                                    item->dev_btn_name = "";
                                    return item;
                                }
                            }
                        }
                    }
                }else{
                    // 方向盘按键, 直接返回
                    return item;
                }
            }
        }

        // 释放res内存
        qDeleteAll(res);  // 删除所有指针指向的对象
        res.clear();      // 清空列表

        Sleep(50);
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

std::map<std::string, short> MainWindow::getConstKeyMap(std::string dev_btn_type){
    if(getIsXboxMode()){
        if(dev_btn_type == (std::string)WHEEL_BUTTON){
            return VK_XBOX_BTN_MAP;
        }

        return VK_XBOX_AXIS_MAP;
    }

    return VK_MAP;

}

// 创建一个键盘按键下拉选择框
QComboBox* MainWindow::createAKeyBoardComboBox(std::string dev_btn_type){
    QComboBox *comboBox = new QComboBox();

    std::map<std::string, short> map = getConstKeyMap(dev_btn_type);

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
    comboBox->setMaximumHeight(36);
    comboBox->setMinimumHeight(36);
    comboBox->setMinimumWidth(150);
    comboBox->setMaximumWidth(150);
    comboBox->setStyleSheet("QComboBox{padding-left:10px;}");

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
    //qDebug("index:%d", index);
    if(index >= 0){
        // 如果选择的是新的设备, 需要清空列表
        if(ui->comboBox->currentText().toStdString() != deviceName){
            deviceName = ui->comboBox->currentText().toStdString();

            // 清空数组
            mappingList.clear();
            // 清空界面
            clearMappingsArea();

            // 重置配置选择下拉为空白配置
            ui->comboBox_2->setCurrentIndex(0);
            // 重置当前配置文件名
            currentMappingFileName = "";
        }
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

    // 创建一个输入对话框
    bool ok;
    QString text = QInputDialog::getText(nullptr, "保存配置", "请输入配置名称:", QLineEdit::Normal, currentMappingFileName.data(), &ok);

    // 如果用户点击了OK，并且输入有效
    if (ok && !text.isEmpty()) {
        // 保存配置
        saveMappingsToFile(text.toStdString());

        // 清空映射列表配置页面
        //clearMappingsArea();

        // 清空映射列表
        //mappingList.clear();

        // 判断是否需要加进配置文件下拉列表
        if(ui->comboBox_2->findText(text.toStdString().data()) == -1){
            ui->comboBox_2->addItem(text);
        }

        // 切换到空白配置
        //ui->comboBox_2->setCurrentIndex(0);
        ui->comboBox_2->setCurrentText(text);

        // 重置当前配置文件名
        //currentMappingFileName = "";
        currentMappingFileName = text.toStdString();

        QMessageBox::information(this, "提醒", "配置保存成功!");
    } else {
        showErrorMessage(new std::string("配置名称不能为空!"));
        return;
    }
}


void MainWindow::on_comboBox_2_activated(int index)
{
    // // 未选择设备
    // if(vid == 0 || pid == 0){
    //     showErrorMessage(nullptr);
    //     return;
    // }

    // if(getIsRunning()){
    //     setIsRuning(false);
    // }

    // 选择了空白配置
    if(index == 0){
        // 清空一切
        mappingList.clear();
        clearMappingsArea();
        currentMappingFileName = "";

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

void MainWindow::on_radioButton_clicked()
{
    if(!getIsXboxMode()){
        return;
    }

    //ui->label_7->setText("键盘按键");

    // 设置为键盘模式
    setIsXboxMode(false);

    // 重置映射数据
    mappingList.clear();
    clearMappingsArea();
    currentMappingFileName = "";

    // 重置配置文件下拉
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem("空白配置");


    // 重新寻找保存的配置文件(xbox)
    scanMappingFile();

    // 显示死区设置
    this->ui->pushButton_8->show();
}

void MainWindow::on_radioButton_2_clicked()
{
    if(getIsXboxMode()){
        return;
    }

    //ui->label_7->setText("Xbox按键");

    // 检查驱动
    checkDriverInstalled();

    // 设置为xbox模式
    setIsXboxMode(true);

    // 重置映射数据
    mappingList.clear();
    clearMappingsArea();
    currentMappingFileName = "";

    // 重置配置文件下拉
    ui->comboBox_2->clear();
    ui->comboBox_2->addItem("空白配置");


    // 重新寻找保存的配置文件(xbox)
    scanMappingFile();

    // 隐藏死区设置
    this->ui->pushButton_8->hide();
}





void MainWindow::on_pushButton_5_clicked()
{
    QMainWindow *window = new QMainWindow();
    window->setWindowTitle("请作者喝杯奶茶, 感谢您的支持和鼓励");
    window->setFixedSize(400, 300);

    // 创建一个 QLabel 来显示图片
    QLabel* label = new QLabel();
    QPixmap pixmap(":/icon/shoukuanma.jpg");  // 替换为你的图片路径
    pixmap = pixmap.scaled(400, 300, Qt::KeepAspectRatio);
    label->setPixmap(pixmap);
    label->setAlignment(Qt::AlignCenter);  // 设置图片居中显示

    // 创建一个 QWidget 作为主窗口的中央部件
    QWidget* centralWidget = new QWidget();

    // 创建一个 QVBoxLayout 布局管理器
    QVBoxLayout* layout = new QVBoxLayout(centralWidget);

    // 将 QLabel 添加到布局中
    layout->addWidget(label);

    // 设置布局为中央部件的布局
    centralWidget->setLayout(layout);

    // 设置中央部件
    window->setCentralWidget(centralWidget);

    window->show();


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
        ui->comboBox->setPlaceholderText("");
        for(auto item : diDeviceList){
            ui->comboBox->addItem(item.name.data());
        }

        if(hasLastDevInCurrentDeviceList(deviceName)){
            ui->comboBox->setCurrentText(deviceName.data());
        }else{
            ui->comboBox->setCurrentIndex(-1);
        }
    }
}


void MainWindow::on_pushButton_7_clicked()
{
    this->logWindow->show();
}

// 模拟服务报错的slot
void MainWindow::simulateMsgboxSlot(bool isError, QString text){
    if(isError){
        QMessageBox::critical(this, "错误", text);
    }else{
        QMessageBox::information(this, "提醒", text);
    }

}

void MainWindow::simulateStartedSlot(){
    ui->label_4->setText("已启动");
    ui->label_4->setStyleSheet("QLabel{color: rgb(0, 170, 0);}");
    if(!getIsXboxMode()){
        ui->pushButton_8->hide();
    }

}


void MainWindow::on_pushButton_8_clicked()
{
    this->settings->show();
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
    this->assistWindow->show();
}

