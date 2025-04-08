#include "AssistFuncWindow.h"
#include "ui_AssistFuncWindow.h"
#include "AssistFuncWorker.h"
#include<QThread>
#include"global.h"
#include<QDir>
#include<QJsonDocument>
#include<QJsonObject>
#include<QSettings>

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
    // 开启线程
    if(ETS2_enbaleAutoCancelHandbreak){
        pushToQueue("开启 欧卡2自动解除手刹");
        ui->checkBox->setChecked(true);
        startAssistFuncWork();
    }
}

AssistFuncWindow::~AssistFuncWindow()
{
    delete ui;
}

void AssistFuncWindow::saveSettings(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + ASSIST_FUNC_SETTINGS);  // 文件路径可以是绝对路径或相对路径
    QString text2;
    text2.append("{");
    text2.append("\n\t\"ETS2_enbaleAutoCancelHandbreak\":").append(ETS2_enbaleAutoCancelHandbreak ? "true" : "false").append("\n");
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
    bool enableETS2AutoCancelHandbreak = jsonObj["ETS2_enbaleAutoCancelHandbreak"].toBool();
    this->ETS2_enbaleAutoCancelHandbreak = enableETS2AutoCancelHandbreak;
}

void AssistFuncWindow::on_checkBox_stateChanged(int state){}

void copyETS2PluginDll(QString ets2Path, QString pluginDllFile){
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
                    return;
                }else{
                    dllPluginFile->copy(ETS2Dir->absolutePath() + "/" + pluginDllFile);
                    pushToQueue(parseSuccessLog("复制插件成功!"));
                }
            }
        }
    }
}

// 检查欧卡2的遥测数据共享内存dll是否存在
bool checkETS2Plugin(){
    // 从注册表读取steam路径
    QSettings *settings = new QSettings("HKEY_CURRENT_USER\\Software\\Valve\\Steam", QSettings::NativeFormat);
    // 读取 "SteamPath" 的值
    QString steamPath = settings->value("SteamPath").toString();
    // steam路径为空
    if(steamPath.isEmpty()){
        pushToQueue(parseErrorLog("从注册表读取steam安装路径失败, 检测欧卡2遥测数据插件失败!"));
        return false;
    }

    pushToQueue("读取steam路径成功!");

    QDir *ETS2Dir = new QDir(steamPath + ETS2_BIN_PATH);
    if(!ETS2Dir->exists()){
        pushToQueue(parseErrorLog("读取欧卡2安装路径失败!"));
        return false;
    }

    pushToQueue("读取欧卡2安装路径成功!");

    copyETS2PluginDll(QString(steamPath + ETS2_BIN_PATH + "win_x64"), "ETS2SharedMemoryMapPlugin64.dll");
    copyETS2PluginDll(QString(steamPath + ETS2_BIN_PATH + "win_x86"), "ETS2SharedMemoryMapPlugin32.dll");

    return true;
}

void AssistFuncWindow::startAssistFuncWork(){
    // 检查欧卡2的遥测数据共享内存dll是否存在
    if(!checkETS2Plugin()){
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
    // 开启欧卡2自动解除手刹
    if(ui->checkBox->isChecked()){
        if(ETS2_enbaleAutoCancelHandbreak == false){
            pushToQueue("开启 欧卡2自动解除手刹");

            ETS2_enbaleAutoCancelHandbreak = true;

            // 开启辅助功能任务
            startAssistFuncWork();
        }
    }else{
        // 关闭自动解除手刹
        if(ETS2_enbaleAutoCancelHandbreak){
            pushToQueue("关闭 欧卡2自动解除手刹");

            ETS2_enbaleAutoCancelHandbreak = false;
            emit stopWork();
        }
    }

    saveSettings();

    this->hide();
}

