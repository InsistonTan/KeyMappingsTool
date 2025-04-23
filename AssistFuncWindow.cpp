#include "AssistFuncWindow.h"
#include "ui_AssistFuncWindow.h"
#include "AssistFuncWorker.h"
#include "simulate_task.h"
#include "mainwindow.h"
#include "ForceFeedbackWorker.h"
#include<QThread>
#include"global.h"
#include<QDir>
#include<QJsonDocument>
#include<QJsonObject>
#include<QSettings>
#include<QFileDialog>
#include<QTimer>
#include<QMessageBox>

bool AssistFuncWindow::ETS2_enableAutoCancelHandbrake = false;
bool AssistFuncWindow::SYSTEM_enableMappingAfterOpening = false;
bool AssistFuncWindow::SYSTEM_enableOnlyLongestMapping = false;

AssistFuncWindow::AssistFuncWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AssistFuncWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("辅助功能");

    // 获取软件本地数据目录
    appDataDirPath = QDir::homePath() + "/AppData/Local/KeyMappingToolData/";

    // 方向盘力反馈模拟设置
    forceFeedbackSettingsWindow = new ForceFeedbackSettingsWindow();
    // 力反馈模拟设置改动信号
    connect(forceFeedbackSettingsWindow, &ForceFeedbackSettingsWindow::settingsChangeSignal, this, &AssistFuncWindow::onForceFeedbackSettingsChange);


    // 加载设置
    loadSettings();

    // 扫描欧卡目录
    if(ETS2InstallPath.isEmpty()){
        scanETS2InstallPath();
    }

    if(ETS2InstallPath.isEmpty()){
        ui->label_2->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_2->setStyleSheet("QLabel{color:black;}");
        ui->label_2->setToolTip(ETS2InstallPath);
        ui->label_2->setText(ETS2InstallPath.size() > 40 ? ETS2InstallPath.left(40) + "..." : ETS2InstallPath);
    }

    // 开启欧卡2自动解除手刹
    if(ETS2_enableAutoCancelHandbrake){
        pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 欧卡2自动解除手刹");
        ui->checkBox->setChecked(true);
        startAssistFuncWork();
    }

    if(SYSTEM_enableMappingAfterOpening){
        pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 打开软件后立即开启映射");
        ui->checkBox_2->setChecked(true);
    }

    if(SYSTEM_enableOnlyLongestMapping){
        pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 最长组合键优先模式");
        ui->checkBox_3->setChecked(true);
    }

    if(SYSTEM_enableForceFeedback && validateForceFeedbackParams()){
        pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 方向盘力反馈模拟");
        ui->checkBox_4->setChecked(true);

        QTimer::singleShot(1500, [=](){
            // 开启方向盘力反馈
            startForceFeedback();
        });

    }


}

AssistFuncWindow::~AssistFuncWindow()
{
    delete ui;
}

bool AssistFuncWindow::getEnableMappingAfterOpening()
{
    return SYSTEM_enableMappingAfterOpening;
}

bool AssistFuncWindow::getEnableOnlyLongestMapping()
{
    return SYSTEM_enableOnlyLongestMapping;
}

void AssistFuncWindow::saveSettings(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + ASSIST_FUNC_SETTINGS);  // 文件路径可以是绝对路径或相对路径
    QString text2;
    text2.append("{\n\t");

    text2.append("\"ETS2_enableAutoCancelHandbrake\":").append(ui->checkBox->isChecked() ? "true" : "false").append(",\n\t");
    text2.append("\"SYSTEM_enableMappingAfterOpening\":").append(ui->checkBox_2->isChecked() ? "true" : "false").append(",\n\t");
    text2.append("\"SYSTEM_enableOnlyLongestMapping\":").append(ui->checkBox_3->isChecked() ? "true" : "false").append(",\n\t");
    text2.append("\"ETS2_installPath\":").append("\"" + ETS2InstallPath + "\"").append(",\n\t");

    text2.append("\"SYSTEM_enableForceFeedback\":").append(ui->checkBox_4->isChecked() ? "true" : "false").append(",\n\t");

    text2.append("\"SYSTEM_forceFeedbackSettings\":{\n\t\t");
    text2.append("\"throttleAxis\":").append("\"" + forceFeedbackSettingsWindow->throttleAxis + "\"").append(",\n\t\t");
    text2.append("\"brakeAxis\":").append("\"" + forceFeedbackSettingsWindow->brakeAxis + "\"").append(",\n\t\t");
    text2.append("\"steeringWheelAxis\":").append("\"" + forceFeedbackSettingsWindow->steeringWheelAxis + "\"").append(",\n\t\t");
    text2.append("\"isThrottleReverse\":").append(forceFeedbackSettingsWindow->isThrottleReverse ? "true" : "false").append(",\n\t\t");
    text2.append("\"isBrakeReverse\":").append(forceFeedbackSettingsWindow->isBrakeReverse ? "true" : "false").append(",\n\t\t");
    text2.append("\"acceleration_100km_time_s\":").append(std::to_string(forceFeedbackSettingsWindow->acceleration_100km_time_s)).append(",\n\t\t");
    text2.append("\"stop_100km_dis_m\":").append(std::to_string(forceFeedbackSettingsWindow->stop_100km_dis_m)).append(",\n\t\t");
    text2.append("\"maxSpeed_km_h\":").append(std::to_string(forceFeedbackSettingsWindow->maxSpeed_km_h)).append(",\n\t\t");
    text2.append("\"maxForceFeedbackGain\":").append(std::to_string(forceFeedbackSettingsWindow->maxForceFeedbackGain)).append("\n\t");
    text2.append("}\n");

    text2.append("}");
    if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file2);  // 创建一个文本流对象
        out << text2;             // 写入文本

        // 关闭文件
        file2.close();
        qDebug() << "保存辅助功能设置成功!";
        //pushToQueue(parseSuccessLog("保存辅助功能设置成功!"));
    } else {
        qDebug() << "打开辅助功能设置文件失败!";
        QMessageBox::critical(this, "保存失败", "打开/创建辅助功能设置文件失败!");
        pushToQueue(parseErrorLog("打开/创建辅助功能设置文件失败!"));
    }
}
void AssistFuncWindow::loadSettings(){
    // 打开 JSON 文件
    QFile file(appDataDirPath + ASSIST_FUNC_SETTINGS);
    if (!file.open(QIODevice::ReadOnly)) {
        pushToQueue(parseWarningLog("打开辅助功能设置文件失败!"));
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        pushToQueue(parseWarningLog("解析辅助功能设置文件的json内容失败!"));
        return;
    }

    // 获取 JSON 对象
    QJsonObject jsonObj = doc.object();

    // 读取信息
    bool enableETS2AutoCancelHandbrake = (jsonObj.contains("ETS2_enableAutoCancelHandbrake")) ? jsonObj["ETS2_enableAutoCancelHandbrake"].toBool() : false;
    bool enableAutoStartMapping = (jsonObj.contains("SYSTEM_enableMappingAfterOpening")) ? jsonObj["SYSTEM_enableMappingAfterOpening"].toBool() : false;
    bool enableOnlyLongestMapping = (jsonObj.contains("SYSTEM_enableOnlyLongestMapping")) ? jsonObj["SYSTEM_enableOnlyLongestMapping"].toBool() : false;
    this->ETS2_enableAutoCancelHandbrake = enableETS2AutoCancelHandbrake;
    this->SYSTEM_enableMappingAfterOpening = enableAutoStartMapping;
    this->SYSTEM_enableOnlyLongestMapping = enableOnlyLongestMapping;
    this->SYSTEM_enableForceFeedback = (jsonObj.contains("SYSTEM_enableForceFeedback")) ? jsonObj["SYSTEM_enableForceFeedback"].toBool() : false;
    // 欧卡2安装路径
    QString ets2Path = (jsonObj.contains("ETS2_installPath")) ? jsonObj["ETS2_installPath"].toString() : "";
    if(!ets2Path.isEmpty()){
        this->ETS2InstallPath = ets2Path;
    }

    // 力反馈模拟设置
    if(jsonObj.contains("SYSTEM_forceFeedbackSettings") && jsonObj["SYSTEM_forceFeedbackSettings"].isObject()){
        QJsonObject settingsObj = jsonObj["SYSTEM_forceFeedbackSettings"].toObject();
        this->forceFeedbackSettingsWindow->throttleAxis = (settingsObj.contains("throttleAxis")) ? settingsObj["throttleAxis"].toString() : "";
        this->forceFeedbackSettingsWindow->brakeAxis = (settingsObj.contains("brakeAxis")) ? settingsObj["brakeAxis"].toString() : "";
        this->forceFeedbackSettingsWindow->steeringWheelAxis = (settingsObj.contains("steeringWheelAxis")) ? settingsObj["steeringWheelAxis"].toString() : "";


        this->forceFeedbackSettingsWindow->isThrottleReverse = (settingsObj.contains("isThrottleReverse")) ? settingsObj["isThrottleReverse"].toBool() : false;
        this->forceFeedbackSettingsWindow->isBrakeReverse = (settingsObj.contains("isBrakeReverse")) ? settingsObj["isBrakeReverse"].toBool() : false;


        if(settingsObj.contains("acceleration_100km_time_s")){
            this->forceFeedbackSettingsWindow->acceleration_100km_time_s = settingsObj["acceleration_100km_time_s"].toDouble();
        }
        if(settingsObj.contains("stop_100km_dis_m")){
            this->forceFeedbackSettingsWindow->stop_100km_dis_m = settingsObj["stop_100km_dis_m"].toInt();
        }
        if(settingsObj.contains("maxSpeed_km_h")){
            this->forceFeedbackSettingsWindow->maxSpeed_km_h = settingsObj["maxSpeed_km_h"].toInt() > 0 ? settingsObj["maxSpeed_km_h"].toInt() : default_maxSpeed_km_h;
        }
        if(settingsObj.contains("maxForceFeedbackGain")){
            this->forceFeedbackSettingsWindow->maxForceFeedbackGain = settingsObj["maxForceFeedbackGain"].toDouble();
        }
    }
}

void AssistFuncWindow::on_checkBox_stateChanged(int state){}

void AssistFuncWindow::on_checkBox_2_stateChanged(int arg1){}

void AssistFuncWindow::scanETS2InstallPath(){
    // 从注册表读取steam路径
    QSettings *settings = new QSettings("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
    // 读取 "SteamPath" 的值
    QString steamPath = settings->value("SteamPath").toString();
    // steam路径为空
    if(steamPath.isEmpty()){
        pushToQueue(parseErrorLog("从注册表读取steam安装路径失败, 请在 首页-辅助功能-选择欧卡目录 手动选择欧卡安装目录!"));
        return;
    }

    pushToQueue("读取steam目录成功: " + steamPath);

    QDir *ETS2Dir = new QDir(steamPath + ETS2_PATH);
    if(!ETS2Dir->exists()){
        pushToQueue(parseErrorLog("从steam安装目录读取欧卡2安装目录失败, 请在 首页-辅助功能-选择欧卡目录 手动选择欧卡安装目录!"));
        return;
    }

    this->ETS2InstallPath = ETS2Dir->absolutePath();

    pushToQueue("读取欧卡2安装目录成功: " + ETS2Dir->absolutePath());
}

bool copyETS2PluginDll(QString ets2Path, QString pluginDllPath, QString pluginDllFile){
    QDir *ETS2Dir = new QDir(ets2Path);
    if(ETS2Dir->exists()){
        if(!ETS2Dir->exists(ETS2_PLUGINS_DIR)){
            ETS2Dir->mkdir(ETS2_PLUGINS_DIR);
        }

        if(ETS2Dir->cd(ETS2_PLUGINS_DIR)){
            // plugins目录不存在dll
            if(!ETS2Dir->exists(pluginDllFile)){
                pushToQueue(parseWarningLog("欧卡2目录未检测到遥测数据插件[" + pluginDllFile + "], 将复制插件到欧卡2 plugins 目录[" + ETS2Dir->absolutePath() + "]..."));
                QString pluginDllFilePath = pluginDllPath + "/" + pluginDllFile;
                QFile *dllPluginFile = new QFile("plugins/" + pluginDllFilePath);
                if(!dllPluginFile->exists()){
                    pushToQueue(parseErrorLog("软件安装目录/plugins/" + pluginDllFilePath + " 不存在, 无法将该插件复制到欧卡2 plugins 目录!"));
                    return false;
                }else{
                    if(!dllPluginFile->copy(ETS2Dir->absolutePath() + "/" + pluginDllFile)){
                        pushToQueue(parseErrorLog("复制插件失败!"));
                        return false;
                    }
                    pushToQueue(parseSuccessLog("复制插件成功, 重启游戏后生效"));
                }
            }
        }
    }else{
        pushToQueue(parseErrorLog("文件夹不存在: " + ets2Path + ", 请确保欧卡2的安装目录正确!"));
        return false;
    }

    return true;
}

// 检查欧卡2的遥测数据共享内存dll是否存在
bool AssistFuncWindow::checkETS2Plugin(){
    if(this->ETS2InstallPath.isEmpty()){
        pushToQueue(parseErrorLog("欧卡2安装路径为空!"));
        return false;
    }

    if(!copyETS2PluginDll(QString(ETS2InstallPath + "/bin/win_x64"), "Win64" , "scs-telemetry.dll")){
        return false;
    }
    copyETS2PluginDll(QString(ETS2InstallPath + "/bin/win_x86"), "Win32" , "scs-telemetry.dll");

    return true;
}

void AssistFuncWindow::startAssistFuncWork(){
    // 检查欧卡2的遥测数据共享内存dll是否存在
    if(!checkETS2Plugin()){
        pushToQueue(parseErrorLog("欧卡2自动解除手刹功能启动失败!"));
        QMessageBox::critical(this, "错误", "欧卡2自动解除手刹功能启动失败! \n\n详情请看日志");
        // 启动失败, 重置状态为 未启用
        this->ui->checkBox->setChecked(false);
        ETS2_enableAutoCancelHandbrake = false;
        return;
    }

    AssistFuncWorker *worker = new AssistFuncWorker();
    QThread *thread = new QThread;
    worker->moveToThread(thread);

    // 连接信号槽
    connect(thread, &QThread::started, worker, &AssistFuncWorker::doWork);
    // 停止任务信号
    connect(this, &AssistFuncWindow::stopWork, worker, &AssistFuncWorker::cancelWorkSlot);
    // 任务结束信号
    connect(worker, &AssistFuncWorker::workFinished, thread, &QThread::quit);
    connect(worker, &AssistFuncWorker::workFinished, worker, &AssistFuncWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}

void AssistFuncWindow::on_pushButton_clicked()
{
    // 保存设置
    saveSettings();
    save();

    // 开启欧卡2自动解除手刹
    if(ui->checkBox->isChecked()){
        if(ETS2_enableAutoCancelHandbrake == false){
            pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 欧卡2自动解除手刹");

            ETS2_enableAutoCancelHandbrake = true;

            // 开启辅助功能任务
            startAssistFuncWork();
        }
    }else{
        // 关闭自动解除手刹
        if(ETS2_enableAutoCancelHandbrake){
            pushToQueue("<b style='color:red;'>关闭</b> 欧卡2自动解除手刹");
            emit stopWork();

            ETS2_enableAutoCancelHandbrake = false;
        }
    }

    // SYSTEM_enableOnlyLongestMapping值发生变化
    if(SYSTEM_enableOnlyLongestMapping != ui->checkBox_3->isChecked()){

        // SYSTEM_enableOnlyLongestMapping变量值更新
        SYSTEM_enableOnlyLongestMapping = ui->checkBox_3->isChecked();
        SYSTEM_enableOnlyLongestMapping ? pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 最长组合键优先模式")
                                        : pushToQueue("<b style='color:red;'>关闭</b> 最长组合键优先模式");

        // 提交信号
        SimulateTask::changeEnableOnlyLongestMapping();
    }


    // 开启方向盘力反馈模拟
    if(ui->checkBox_4->isChecked()){
        // 力反馈选项由关闭到开启
        if(!SYSTEM_enableForceFeedback){
            // 校验力反馈设置项是否正确
            // 校验不通过
            if(!validateForceFeedbackParams()){
                return;
            }
            pushToQueue("<b style='color:rgb(0, 151, 144);'>开启</b> 方向盘力反馈模拟");
            SYSTEM_enableForceFeedback = true;
            startForceFeedback();
        }
    }else{
        if(SYSTEM_enableForceFeedback){
            pushToQueue("<b style='color:red;'>关闭</b> 方向盘力反馈模拟");
            emit stopForceFeedbackSignal();

            SYSTEM_enableForceFeedback = false;
        }
    }

    //this->hide();
}


void AssistFuncWindow::on_pushButton_2_clicked()
{
    // 打开文件夹选择对话框
    QString folderPath = QFileDialog::getExistingDirectory(
        this,                       // 父窗口
        tr("选择欧卡2文件夹"),            // 对话框标题
        QDir::homePath(),            // 默认打开的目录（用户主目录）
        QFileDialog::ShowDirsOnly    // 只显示目录
            | QFileDialog::DontResolveSymlinks  // 不解析符号链接
        );

    // 检查用户是否选择了文件夹（没有点击取消）
    if (!folderPath.isEmpty()) {
        this->ETS2InstallPath = folderPath;

        ui->label_2->setStyleSheet("QLabel{color:black;}");
        ui->label_2->setToolTip(ETS2InstallPath);
        ui->label_2->setText(ETS2InstallPath.size() > 40 ? ETS2InstallPath.left(40) + "..." : ETS2InstallPath);

        unsave();
    }
}

void AssistFuncWindow::unsave(){
    this->setWindowTitle("辅助功能 *设置未保存");
}
void AssistFuncWindow::save(){
    this->setWindowTitle("辅助功能");
}


void AssistFuncWindow::on_checkBox_clicked()
{
    unsave();
}

void AssistFuncWindow::on_checkBox_2_clicked()
{
    unsave();
}

void AssistFuncWindow::on_checkBox_3_clicked()
{
    unsave();
}


void AssistFuncWindow::on_pushButton_3_clicked()
{
    this->forceFeedbackSettingsWindow->updateUI(true);
    // 如果窗口是最小化状态, 清除最小化
    if(this->forceFeedbackSettingsWindow->windowState() == Qt::WindowMinimized){
        this->forceFeedbackSettingsWindow->setWindowState(this->forceFeedbackSettingsWindow->windowState() & ~Qt::WindowMinimized);
    }
    this->forceFeedbackSettingsWindow->show();
    this->forceFeedbackSettingsWindow->activateWindow();
}

void AssistFuncWindow::onForceFeedbackSettingsChange(){
    // 保存设置
    saveSettings();

    // 只有在开启力反馈状态下, 才提交设置更新的信号
    if(SYSTEM_enableForceFeedback){
        // 力反馈设置
        emit forceFeedbackSettingsChangeSignal();
    }
}


void AssistFuncWindow::on_checkBox_4_stateChanged(int arg1)
{

}

void AssistFuncWindow::startForceFeedback(){
    if(MainWindow::getCurrentSelectedDeviceIndex() < 0){
        pushToQueue(parseErrorLog("启动力反馈模拟错误: 未选择设备!"));
        QMessageBox::critical(this, "错误", "启动力反馈模拟错误: 未选择设备!\n请选择设备后重新手动开启");
        this->ui->checkBox_4->setChecked(false);
        this->SYSTEM_enableForceFeedback = false;
        return;
    }

    // 记录本次使用设备到本地
    emit saveLastDeviceToFileSignal();

    // 初始化directInput
    if(!initDirectInput()){
        return;
    }
    // 连接设备
    if(!openDiDevice(MainWindow::getCurrentSelectedDeviceIndex())){
        return;
    }

    ForceFeedbackWorker *worker = new ForceFeedbackWorker(forceFeedbackSettingsWindow);
    QThread *thread = new QThread;
    worker->moveToThread(thread);

    // 连接信号槽
    connect(thread, &QThread::started, worker, &ForceFeedbackWorker::doWork);
    // 力反馈模拟设置更新信号
    connect(this, &AssistFuncWindow::forceFeedbackSettingsChangeSignal, worker, &ForceFeedbackWorker::settingsChangeSlot);
    // 停止任务信号
    connect(this, &AssistFuncWindow::stopForceFeedbackSignal, worker, &ForceFeedbackWorker::cancelWorkSlot);
    // 任务结束信号
    connect(worker, &ForceFeedbackWorker::workFinished, thread, &QThread::quit);
    connect(worker, &ForceFeedbackWorker::workFinished, worker, &ForceFeedbackWorker::deleteLater);
    connect(thread, &QThread::finished, thread, &QThread::deleteLater);

    thread->start();
}


void AssistFuncWindow::on_checkBox_4_clicked()
{
    unsave();
}

bool AssistFuncWindow::validateForceFeedbackParams(){
    if(ui->checkBox_4->isChecked() && MainWindow::getCurrentSelectedDeviceIndex() < 0){
        pushToQueue(parseErrorLog("开启模拟力反馈失败: 还未选择设备!"));
        QMessageBox::critical(this, "错误", "开启失败: 还未选择设备!");
        ui->checkBox_4->setChecked(false);
        return false;
    }

    // 检查设备是否支持力反馈
    if(ui->checkBox_4->isChecked() && !checkIsSupportForceFeedback()){
        pushToQueue(parseErrorLog("开启模拟力反馈失败: 当前设备不支持力反馈!"));
        QMessageBox::critical(this, "错误", "开启失败: 当前设备不支持力反馈");
        ui->checkBox_4->setChecked(false);
        return false;
    }

    if(ui->checkBox_4->isChecked() && (this->forceFeedbackSettingsWindow->throttleAxis.isEmpty()
                                        || this->forceFeedbackSettingsWindow->brakeAxis.isEmpty()
                                        || this->forceFeedbackSettingsWindow->steeringWheelAxis.isEmpty())){
        pushToQueue(parseErrorLog("开启模拟力反馈失败: 未设置盘面/油门踏板/刹车踏板!"));
        QMessageBox::critical(this, "错误", "开启失败: 未设置盘面/油门踏板/刹车踏板!");
        ui->checkBox_4->setChecked(false);
        return false;
    }

    // 获取不到油门踏板的数值范围
    if(ui->checkBox_4->isChecked() && axisValueRangeMap.find(this->forceFeedbackSettingsWindow->throttleAxis.toStdString()) == axisValueRangeMap.end()){
        pushToQueue(parseErrorLog("启动力反馈模拟失败: 获取油门踏板数值范围失败!"));
        QMessageBox::critical(nullptr, "错误", "启动失败: 获取油门踏板数值范围失败!");
        ui->checkBox_4->setChecked(false);
        return false;
    }

    // 获取不到油门踏板的数值范围
    if(ui->checkBox_4->isChecked() && axisValueRangeMap.find(this->forceFeedbackSettingsWindow->brakeAxis.toStdString()) == axisValueRangeMap.end()){
        pushToQueue(parseErrorLog("启动力反馈模拟失败: 获取刹车踏板数值范围失败!"));
        QMessageBox::critical(nullptr, "错误", "启动失败: 获取刹车踏板数值范围失败!");
        ui->checkBox_4->setChecked(false);
        return false;
    }

    return true;
}


