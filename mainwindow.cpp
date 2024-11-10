#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "utils.h"
#include "key_map.h"
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

MainWindow::MainWindow(QMainWindow *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setFixedSize(800, 600);
    this->setWindowTitle("KeyMappingsTool v0.0.3");
    this->setWindowIcon(QIcon(":/icon/wheel_icon.png"));

    // 遍历所有设备
    listAllDevice();

    QComboBox *comboBox  = ui->comboBox;
    comboBox->setMaximumHeight(30);
    // comboBox->setStyleSheet("QComboBox { "
    //                         "border: 1px solid black; "  // 边框宽度和颜色
    //                         "border-radius: 1px; "       // 圆角半径
    //                         "} ");

    // 使用迭代器遍历
    for (int i=0; i< (int)deviceList.size(); i++) {
        comboBox->addItem(deviceList[i]->getDeviceName().data());
    }
    comboBox->setCurrentIndex(-1);


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

    // 加载上一次使用的设备
    loadLastDeviceFile();

    if(pid > 0 && vid > 0){
        comboBox->setCurrentText(deviceName.data());
        ui->label_2->setText(deviceDesc.data());
    }

    // 加载上一次的映射配置
    loadMappingsFile("");

    // 绘制出配置列表
    repaintMappings();

    // 配置下拉添加一个空白配置
    ui->comboBox_2->addItem("空白配置");

    // 扫描用户保存的配置文件
    QDir dir = QDir::current();
    if(dir.exists(USER_MAPPINGS_DIR)){
        dir.cd(USER_MAPPINGS_DIR);
        // 获取文件列表，并筛选指定后缀的文件
        QFileInfoList list = dir.entryInfoList();
        for (const QFileInfo &fileInfo : list) {
            if ("." + fileInfo.suffix() == MAPPING_FILE_SUFFIX) {
                // 匹配指定后缀的文件，加入下拉列表
                ui->comboBox_2->addItem(fileInfo.completeBaseName());
            }
        }

        ui->comboBox_2->setCurrentIndex(-1);
    }

}

bool MainWindow::hasLastDevInCurrentDeviceList(short lastDevVid, short lastDevPid){
    for(auto item : deviceList){
        if(item->getVid() == lastDevVid && item->getPid() == lastDevPid){
            return true;
        }
    }

    return false;
}

void MainWindow::repaintMappings(){
    qDebug("mappingList.size() : %d", (int)mappingList.size());

    if(mappingList.size() > 0){
        // 清空配置页面
        clearMappingsArea();
        // 重新添加
        for(int i =0; i < (int)mappingList.size(); i++){
            paintOneLineMapping(mappingList[i], i);
        }
    }
}


int MainWindow::listAllDevice(){
    // 初始化 HIDAPI 库
    if (hid_init()) {
        qDebug() << "Unable to initialize HIDAPI!";
        return 1;
    }

    // 获取所有连接的 HID 设备
    struct hid_device_info *devs, *cur_dev;
    devs = hid_enumerate(0x0, 0x0); // 获取所有设备，传入两个零表示不限制 VID 和 PID

    if (devs == NULL) {
        qDebug() << "No HID devices found!" ;
        hid_exit();
        return 1;
    }

    // 遍历所有设备并打印信息
    cur_dev = devs;
    while (cur_dev && cur_dev->product_string) {
        // qDebug() << "设备路径: " << cur_dev->path;
        // qDebug() << "设备的 VID:PID: " << cur_dev->vendor_id << ":" << cur_dev->product_id;
        // qDebug() << "设备的接口编号: " << cur_dev->interface_number;
        // qDebug("设备的产品字符串: %s", Utils::convertWcharToString(cur_dev->product_string).data());
        // qDebug("设备的制造商字符串: %s" , Utils::convertWcharToString(cur_dev->manufacturer_string).data());

        deviceList.push_back(new DeviceInfo(Utils::convertWcharToString(cur_dev->product_string),
                                            cur_dev->path,
                                            cur_dev->vendor_id,
                                            cur_dev->product_id));

        cur_dev = cur_dev->next;
    }

    // 释放设备信息内存
    hid_free_enumeration(devs);

    return 0;
}



int MainWindow::openDevice(short vid, short pid){
    // Open the device using the VID, PID,
    // and optionally the Serial number.
    handle = hid_open(vid, pid, NULL);
    if (!handle) {
        qDebug("Unable to open device");
        return -1;
    }

    isDeviceOpen = true;
    return 0;
}

int MainWindow::closeDevice(){
    // Close the device
    hid_close(handle);

    isDeviceOpen = false;

    // Finalize the hidapi library
    hid_exit();

    return 0;
}


void MainWindow::on_pushButton_2_clicked()
{
    if(vid == 0 || pid == 0){
        showErrorMessage(nullptr);
        return;
    }

    if(getIsRunning()){
        showErrorMessage(new string("已经启动了!"));
        return;
    }

    int trueMappingListSize = 0;
    for(auto item : mappingList){
        if(item != nullptr){
            trueMappingListSize++;
        }
    }

    if(trueMappingListSize <= 0){
        showErrorMessage(new string("映射列表为空!"));
        return;
    }

    if(!isDeviceOpen){
        openDevice(vid, pid);
    }

    ui->label_4->setText("已启动");
    ui->label_4->setStyleSheet("QLabel{color: rgb(0, 170, 0);}");
    setIsRuning(true);

    // 创建监听设备输入数据的任务
    SimulateTask *task = new SimulateTask(handle, &mappingList);
    QThread *thread = new QThread();

    // 将 worker 移到新线程
    task->moveToThread(thread);

    // 当线程开始时，启动工作任务
    QObject::connect(thread, &QThread::started, task, &SimulateTask::doWork);

    // 任务完成后，退出线程并清理
    QObject::connect(task, &SimulateTask::workFinished, thread, &QThread::quit);
    QObject::connect(task, &SimulateTask::workFinished, task, &SimulateTask::deleteLater);
    QObject::connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    // 启动线程
    thread->start();

    // 显示信息框
    QMessageBox::information(nullptr, "提醒", "启动全局映射成功!\n如果游戏里不生效, 请使用管理员身份重新运行本程序 ");

    // 保存当前配置的映射到本地文件
    saveMappingsToFile("");

    // 保存上次使用的设备到本地
    saveLastDeviceToFile();
}


void MainWindow::on_pushButton_3_clicked()
{
    if(!getIsRunning()){
        showErrorMessage(new string("全局映射未启动!"));
        return;
    }

    if(vid == 0 || pid == 0){
        showErrorMessage(nullptr);
        return;
    }

    ui->label_4->setText("未启动");
    ui->label_4->setStyleSheet("QLabel{color: rgb(255, 85, 0);}");
    setIsRuning(false);

    // 显示信息框
    QMessageBox::information(nullptr, "提醒", "已停止全局映射!");
}

void MainWindow::showErrorMessage(string *text) {
    QMessageBox::critical(nullptr, "错误", text == nullptr ? "还未选择任何设备!" : text->data());
}

bool MainWindow::hasAddToMappingList(int btn_pos, int btn_value){
    for(int i=0; i < (int)mappingList.size(); i++){
        if(mappingList[i] && (mappingList[i]->dev_btn_pos == btn_pos && mappingList[i]->dev_btn_value == btn_value)){
            return true;
        }
    }

    return false;
}

bool isClickNewLineBtn(MappingRelation *mapping){
    return mapping == nullptr;
}

void MainWindow::paintOneLineMapping(MappingRelation *mapping, int index){
    // isClickNewLineBtn()为true 代表是新增加的一行, 不是读取历史配置
    MappingRelation *mappingDevBtnData;
    if(isClickNewLineBtn(mapping)){

        // 获取设备按下的按键位置和值
        mappingDevBtnData = getDevBtnData();

        if(mappingDevBtnData == nullptr){
            showErrorMessage(new string("未检测到按键被按下!"));
            return;
        }


        // 检查该设备按键是否已经添加
        if(hasAddToMappingList(mappingDevBtnData->dev_btn_pos, mappingDevBtnData->dev_btn_value)){
            showErrorMessage(new string("该设备按键已经配置了映射!"));
            return;
        }
        // 将设备按键数据添加进新的映射关系
        mappingList.push_back(new MappingRelation(mappingDevBtnData->dev_btn_pos, mappingDevBtnData->dev_btn_value, 0, ""));
    }


    QGridLayout *layout = ui->gridLayout;
    layout->setAlignment(Qt::AlignTop);
    layout->setContentsMargins(7,7,7,7);

    QLabel *label1 = new QLabel(isClickNewLineBtn(mapping)
            ? ("devbtn#" + to_string(mappingDevBtnData->dev_btn_pos) + "#" + to_string(mappingDevBtnData->dev_btn_value)).data()
            // 历史配置展示
            : ("devbtn#" + to_string(mapping->dev_btn_pos) + "#" + to_string(mapping->dev_btn_value)).data());
    label1->setMaximumHeight(30);
    label1->setMinimumHeight(30);
    label1->setMaximumWidth(100);
    label1->setStyleSheet("QLabel{color:blue;}");
    label1->setObjectName(isClickNewLineBtn(mapping) ? to_string(mappingList.size() - 1) : to_string(index));

    QLabel *label2 = new QLabel("->");
    label2->setMaximumHeight(30);
    label2->setMinimumHeight(30);
    label2->setMaximumWidth(60);
    label2->setObjectName(isClickNewLineBtn(mapping) ? to_string(mappingList.size() - 1) : to_string(index));

    QComboBox *comboBox = createAKeyBoardComboBox();
    // 设置一个序号, 为后续操作提供一个位置
    comboBox->setObjectName(isClickNewLineBtn(mapping) ? to_string(mappingList.size() - 1) : to_string(index));
    // 历史配置展示
    if(!isClickNewLineBtn(mapping)){
        comboBox->setCurrentText(mapping->keyboard_name.data());
    }

    // 连接信号和槽
    connect(comboBox, &QComboBox::activated, this, &MainWindow::onKeyBoardComboBoxActivated);

    QLineEdit *lineEdit = new QLineEdit(isClickNewLineBtn(mapping) ? "" : mapping->remark.data());
    lineEdit->setMaximumHeight(30);
    lineEdit->setMinimumHeight(30);
    lineEdit->setMinimumWidth(150);
    lineEdit->setStyleSheet("QLineEdit{margin:0 0 0 50;}");
    lineEdit->setObjectName(isClickNewLineBtn(mapping) ? to_string(mappingList.size() - 1) : to_string(index));
    // 连接信号和槽
    QObject::connect(lineEdit, &QLineEdit::textChanged, this, &MainWindow::onLineEditTextChanged);

    // 删除按钮
    QPushButton *deleteBtn = new QPushButton("删除该映射");
    deleteBtn->setMaximumHeight(30);
    deleteBtn->setMaximumWidth(80);
    deleteBtn->setMinimumWidth(80);
    deleteBtn->setStyleSheet("QPushButton{background-color:rgb(255, 157, 157);margin-left:7;}");
    deleteBtn->setObjectName(isClickNewLineBtn(mapping) ? to_string(mappingList.size() - 1) : to_string(index));
    // 绑定信号和槽
    connect(deleteBtn, &QPushButton::clicked, this, &MainWindow::onDeleteBtnClicked);

    layout->addWidget(label1, index < 0 ? mappingList.size() : index, 0, Qt::AlignLeft);
    layout->addWidget(label2, index < 0 ? mappingList.size() : index, 1, Qt::AlignLeft);
    layout->addWidget(comboBox, index < 0 ? mappingList.size() : index, 2, Qt::AlignLeft);
    layout->addWidget(lineEdit, index < 0 ? mappingList.size() : index, 3, Qt::AlignLeft);
    layout->addWidget(deleteBtn, index < 0 ? mappingList.size() : index, 4, Qt::AlignLeft);
}

void MainWindow::on_pushButton_clicked()
{
    // 未选择设备
    if(vid == 0 || pid == 0){
        showErrorMessage(nullptr);
        return;
    }

    if(getIsRunning()){
        showErrorMessage(new string("请先停止全局模拟"));
        return;
    }

    paintOneLineMapping(nullptr, -1);
}

void MainWindow::saveLastDeviceToFile(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(LAST_DEVICE_FILENAME);  // 文件路径可以是绝对路径或相对路径
    QString text2;
    text2.append(ui->comboBox->currentText().toStdString() + "\n"
                 + ui->label_2->text().toStdString() + "\n"
                 + to_string(vid) + "\n"
                 + to_string(pid));
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

void MainWindow::saveMappingsToFile(string filename){
    // 要保存的文本内容
    QString text;

    // 生成内容
    for(auto item: mappingList){
        if(item != nullptr){
            text.append(to_string(item->dev_btn_pos) + SPE
                        + to_string(item->dev_btn_value) + SPE
                        + item->keyboard_name + SPE
                        + to_string(item->keyboard_value) + SPE
                        + item->remark + "\n");
        }
    }

    // 如果filename不为空, 则检查配置文件文件夹是否存在,不存在就创建
    if(filename.size() > 0){
        // 获取当前目录
        QDir dir = QDir::current();

        // 使用 QDir 创建不存在的目录
        if (!dir.exists(USER_MAPPINGS_DIR)) {
            if (!dir.mkdir(USER_MAPPINGS_DIR)) {
                showErrorMessage(new string("创建存放用户配置的文件夹失败"));
                return;
            }
        }
    }

    // 创建一个 QFile 对象，并打开文件进行写入
    // 如果filename不为空, 则使用USER_MAPPINGS_DIR + filename + MAPPING_FILE_SUFFIX后缀为文件名
    QFile file(filename.size() > 0
                   ? (USER_MAPPINGS_DIR + filename + MAPPING_FILE_SUFFIX).data()
                   : (MAPPINGS_FILENAME + to_string(vid) + "_" + to_string(pid)).data());

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

void MainWindow::loadLastDeviceFile(){
    // 要读取的文件路径
    QFile file(LAST_DEVICE_FILENAME);

    // 文件不存在, 操作结束
    if(!file.exists()){
        qDebug() << "文件不存在: " << LAST_DEVICE_FILENAME;
        return;
    }

    // 打开文件进行读取
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);

        // 逐行读取文件
        // 逐行读取文件
        if(!in.atEnd()) {
            QString line0 = in.readLine(); // 读取一行
            deviceName = line0.toStdString();
            if(!in.atEnd()) {
                QString line1 = in.readLine(); // 读取一行
                deviceDesc = line1.toStdString();
                if(!in.atEnd()) {
                    QString line2 = in.readLine(); // 读取一行
                    int t_vid = line2.toInt();
                    int t_pid;
                    if(!in.atEnd()) {
                        QString line3 = in.readLine(); // 读取第一行
                        t_pid = line3.toInt();
                    }

                    // 当前设备列表包含这个设备, 自动将此设备设为已选择的设备
                    if(t_vid >0 && t_pid >0 && hasLastDevInCurrentDeviceList(t_vid, t_pid)){
                        vid = t_vid, pid = t_pid;
                    }
                }
            }
        }

        // 关闭文件
        file.close();
    } else {
        qDebug() << "Error opening file!";
    }
}

void MainWindow::loadMappingsFile(string filename){
    if((vid == 0 || pid == 0) && filename.size() <= 0 ){
        qDebug() << "设备vid == 0 或 pid == 0, 无法加载历史映射文件!";
        return;
    }

    // 要读取的文件路径
    QString filePath = filename.size() > 0
                           ? (USER_MAPPINGS_DIR + filename + MAPPING_FILE_SUFFIX).data()
                           :(MAPPINGS_FILENAME + to_string(vid) + "_" + to_string(pid)).data();
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
            if(list.size() == 5){
                MappingRelation *mapping = new MappingRelation();
                mapping->dev_btn_pos = list[0].toShort();
                mapping->dev_btn_value = list[1].toShort();
                mapping->keyboard_name = list[2].toStdString();
                mapping->keyboard_value = list[3].toShort();
                mapping->remark = list[4].toStdString();
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
    // 设备没打开, 需要手动打开
    if(!isDeviceOpen){
        openDevice(vid,pid);
    }

    // 先读一次数据, 用作后续对比
    res = hid_read(handle, buf, MAX_BUF);
    int temp[res];
    for(int k=0; k<res; k++){
        temp[k] = (int) buf[k];
    }

    for(int j=0; j< 60; j++){
        //qDebug("第%d次读取输入报告--------------------", j);

        // Read requested state
        res = hid_read(handle, buf, MAX_BUF);

        string str = "";
        // Print out the returned buffer.
        for (int i = 0; i < res; i++){
            str += to_string((int) buf[i]);
            str += " ";
        }

        for(int p=3;p<res;p++){
            if((int)buf[p]!=temp[p]){
                qDebug("操作对应数值:%d ,处于报告中的第%d位", (int)buf[p], p+1);
                MappingRelation *mapping = new MappingRelation(p, (int)buf[p], -1, "");

                return mapping;
            }

        }

        Sleep(50);
    }

    //qDebug("返回数据:%s", buf);

    closeDevice();

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

// 创建一个键盘按键下拉选择框
QComboBox* MainWindow::createAKeyBoardComboBox(){
    QComboBox *comboBox = new QComboBox();

    // 使用迭代器遍历 map
    for (map<string, short>::const_iterator item = VK_MAP.cbegin(); item != VK_MAP.cend(); ++item) {
        comboBox->addItem(item->first.data());
    }

    comboBox->setCurrentIndex(-1);
    comboBox->setMaximumHeight(30);
    comboBox->setMinimumHeight(30);
    comboBox->setMaximumWidth(200);

    return comboBox;
}


void MainWindow::onKeyBoardComboBoxActivated(int index){
    // 获取触发信号的对象
    QComboBox *comboBox = qobject_cast<QComboBox*>(sender());

    // 获取到该映射的位置
    int rowIndex = comboBox->objectName().toInt();

    MappingRelation *mapping = mappingList[rowIndex];

    // 在键盘 按键名称与虚拟值 map中根据名称查找出值, 并更新到已配置的mapping中
    auto item = VK_MAP.find(comboBox->currentText().toStdString());  // 查找键为 key 的元素
    if (item != VK_MAP.end()) {
        mapping->keyboard_name = item ->first;
        mapping->keyboard_value = item->second;
    }

}


void MainWindow::on_comboBox_activated(int index)
{
    //qDebug("index:%d", index);
    if(index >= 0){
        qDebug("设备路径:%s", deviceList[index]->getDevicePath().data());

        string str = "设备名称:" + deviceList[index]->getDeviceName() + " 设备路径:" + deviceList[index]->getDevicePath();

        ui->label_2->setText(QString::fromStdString(str));

        // 如果选择的是新的设备, 需要清空列表
        if((pid <= 0 && vid <=0) || (vid != deviceList[index]->getVid()) || pid != deviceList[index]->getPid()){
            if(isDeviceOpen){
                closeDevice();
            }

            // 清空数组
            mappingList.clear();
            // 清空界面
            clearMappingsArea();

            // 重置配置选择下拉为空白配置
            ui->comboBox_2->setCurrentIndex(0);
            // 重置当前配置文件名
            currentMappingFileName = "";
        }

        vid = deviceList[index]->getVid();
        pid = deviceList[index]->getPid();

        // 尝试加载历史配置文件
        //loadMappingsFile("");
        // 重画配置列表
        //repaintMappings();
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
    if(mappingList.size() <= 0){
        showErrorMessage(new string("配置为空, 无法保存!"));
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
        clearMappingsArea();

        // 清空映射列表
        mappingList.clear();

        // 判断是否需要加进配置文件下拉列表
        if(text.toStdString() != currentMappingFileName){
            ui->comboBox_2->addItem(text);
        }

        // 切换到空白配置
        ui->comboBox_2->setCurrentIndex(0);

        // 重置当前配置文件名
        currentMappingFileName = "";
    } else {
        showErrorMessage(new string("配置名称不能为空!"));
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
    string filename = comboBox->currentText().toStdString();

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

