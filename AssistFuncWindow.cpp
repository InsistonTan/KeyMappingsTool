#include "AssistFuncWindow.h"
#include "ui_AssistFuncWindow.h"
#include "AssistFuncWorker.h"
#include<QThread>
#include"global.h"
#include<QDir>
#include<QJsonDocument>
#include<QJsonObject>
#include<QSettings>
#include<QFileDialog>
#include<QTimer>

AssistFuncWindow::AssistFuncWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::AssistFuncWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("辅助功能");

    // 获取软件本地数据目录
    appDataDirPath = QDir::homePath() + "/AppData/Local/KeyMappingToolData/";

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

    // 开启线程
    if(ETS2_enableAutoCancelHandbrake){
        pushToQueue("开启 欧卡2自动解除手刹");
        ui->checkBox->setChecked(true);
        startAssistFuncWork();
    }

    if(SYSTEM_enableMappingAfterOpening){
        pushToQueue("开启 打开软件后立即开启映射");
        ui->checkBox_2->setChecked(true);
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

void AssistFuncWindow::saveSettings(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + ASSIST_FUNC_SETTINGS);  // 文件路径可以是绝对路径或相对路径
    QString text2;
    text2.append("{");

    text2.append("\n\t\"ETS2_enableAutoCancelHandbrake\":").append(ui->checkBox->isChecked() ? "true" : "false").append(",").append("\n");
    text2.append("\n\t\"SYSTEM_enableMappingAfterOpening\":").append(ui->checkBox_2->isChecked() ? "true" : "false").append(",").append("\n");
    text2.append("\n\t\"ETS2_installPath\":").append("\"" + ETS2InstallPath + "\"").append("\n");
    text2.append("}");
    if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file2);  // 创建一个文本流对象
        out << text2;             // 写入文本

        // 关闭文件
        file2.close();
        qDebug() << "保存辅助功能设置成功!";
        pushToQueue(parseSuccessLog("保存辅助功能设置成功!"));
    } else {
        qDebug() << "打开辅助功能设置文件失败!";
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
    bool enableETS2AutoCancelHandbrake = jsonObj["ETS2_enableAutoCancelHandbrake"].toBool();
    bool enableAutoStartMapping = jsonObj["SYSTEM_enableMappingAfterOpening"].toBool();
    this->ETS2_enableAutoCancelHandbrake = enableETS2AutoCancelHandbrake;
    this->SYSTEM_enableMappingAfterOpening = enableAutoStartMapping;
    // 欧卡2安装路径
    QString ets2Path = jsonObj["ETS2_installPath"].toString();
    if(!ets2Path.isEmpty()){
        this->ETS2InstallPath = ets2Path;
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

bool copyETS2PluginDll(QString ets2Path, QString pluginDllFile){
    QDir *ETS2Dir = new QDir(ets2Path);
    if(ETS2Dir->exists()){
        if(!ETS2Dir->exists(ETS2_PLUGINS_DIR)){
            ETS2Dir->mkdir(ETS2_PLUGINS_DIR);
        }

        if(ETS2Dir->cd(ETS2_PLUGINS_DIR)){
            // plugins目录不存在dll
            if(!ETS2Dir->exists(pluginDllFile)){
                pushToQueue(parseWarningLog("欧卡2目录未检测到遥测数据插件[" + pluginDllFile + "], 将复制插件到欧卡2 plugins 目录[" + ETS2Dir->absolutePath() + "]..."));
                QFile *dllPluginFile = new QFile("plugins/" + pluginDllFile);
                if(!dllPluginFile->exists()){
                    pushToQueue(parseErrorLog("软件安装目录/plugins/" + pluginDllFile + " 不存在, 无法将该插件复制到欧卡2 plugins 目录!"));
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
        pushToQueue(parseErrorLog("文件夹不存在: " + ets2Path));
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

    if(!copyETS2PluginDll(QString(ETS2InstallPath + "/bin/win_x64"), "ETS2SharedMemoryMapPlugin64.dll")){
        return false;
    }
    copyETS2PluginDll(QString(ETS2InstallPath + "/bin/win_x86"), "ETS2SharedMemoryMapPlugin32.dll");

    return true;
}

void AssistFuncWindow::startAssistFuncWork(){
    // 检查欧卡2的遥测数据共享内存dll是否存在
    if(!checkETS2Plugin()){
        pushToQueue(parseErrorLog("欧卡2自动解除手刹功能启动失败!"));
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
            pushToQueue("开启 欧卡2自动解除手刹");
        }else{
            emit stopWork();
        }

        QTimer::singleShot(500, [=](){
            pushToQueue("启用 欧卡2自动解除手刹功能");
            // 开启辅助功能任务
            startAssistFuncWork();
        });

    }else{
        // 关闭自动解除手刹
        if(ETS2_enableAutoCancelHandbrake){
            pushToQueue("关闭 欧卡2自动解除手刹");
            emit stopWork();
        }
    }

    this->hide();
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
