#include "DeadAreaSettings.h"
#include "ui_DeadAreaSettings.h"
#include "global.h"
#include<QFile>
#include<QMessageBox>
#include<QJsonDocument>
#include<QJsonObject>

DeadAreaSettings::DeadAreaSettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeadAreaSettings)
{
    ui->setupUi(this);

    setWindowTitle("死区设置");

    loadSettingsFile();

    this->ui->lineEdit->setText(std::to_string(getXboxJoystickInnerDeadAreaValue()).data());
    this->ui->lineEdit_2->setText(std::to_string(getXboxTriggerInnerDeadAreaValue()).data());
    this->ui->lineEdit_4->setText(std::to_string(getInnerDeadAreaPanti()).data());
    this->ui->lineEdit_3->setText(std::to_string(getInnerDeadAreaTaban()).data());

    save();
}

DeadAreaSettings::~DeadAreaSettings()
{
    delete ui;
}

void DeadAreaSettings::unsave(){
    setWindowTitle("死区设置 *设置未保存");
}
void DeadAreaSettings::save(){
    setWindowTitle("死区设置");
}

void DeadAreaSettings::on_lineEdit_textChanged(const QString &arg1)
{
    unsave();
}


void DeadAreaSettings::on_lineEdit_editingFinished()
{
    bool ok;
    double value = ui->lineEdit->text().toDouble(&ok);

    // 输入超出范围 -1, 1
    if (!ok || value < -1 || value > 1) {
        ui->lineEdit->setText("0.000000");
    }
}


void DeadAreaSettings::on_pushButton_clicked()
{
    double value = ui->lineEdit->text().toDouble();
    if(value <= -1 || value >= 1){
        value = 0;
        ui->lineEdit->setText("0.000000");
    }
    setXboxJoystickInnerDeadAreaValue(value);

    double value2 = ui->lineEdit_2->text().toDouble();
    if(value2 <= -1 || value2 >= 1){
        value2 = 0;
        ui->lineEdit_2->setText("0.000000");
    }
    setXboxTriggerInnerDeadAreaValue(value2);

    // 设置轴映射键盘按键时, 转向轴的内部死区
    setInnerDeadAreaPanti(ui->lineEdit_4->text().toDouble());
    // 设置轴映射键盘按键时, 踏板轴的内部死区
    setInnerDeadAreaTaban(ui->lineEdit_3->text().toDouble());

    saveSettingsToFile();

    save();
}

void DeadAreaSettings::saveSettingsToFile(){
    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(getAppDataDirStr() + SETTINGS_FILE_NAME);
    QString text2;
    text2.append("{\n\t");

    // 映射键盘按键, 转向轴的内部死区
    text2.append("\"keyboardWheelAxisInnerDeadAreaValue\":").append(std::to_string(ui->lineEdit_4->text().toDouble())).append(",\n\t");
    // 映射键盘按键, 踏板轴的内部死区
    text2.append("\"keyboardPedalAxisInnerDeadAreaValue\":").append(std::to_string(ui->lineEdit_3->text().toDouble())).append(",\n\t");

    // 映射手柄, 手柄摇杆的内部死区
    text2.append("\"xboxJoystickInnerDeadAreaValue\":").append(std::to_string(ui->lineEdit->text().toDouble())).append(",\n\t");
    // 映射手柄, 手柄扳机的内部死区
    text2.append("\"xboxTriggerInnerDeadAreaValue\":").append(std::to_string(ui->lineEdit_2->text().toDouble())).append("");

    text2.append("\n}");
    if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file2);  // 创建一个文本流对象
        out << text2;             // 写入文本

        // 关闭文件
        file2.close();
    } else {
        qDebug() << "打开xbox死区设置文件失败!";
        QMessageBox::critical(this, "保存失败", "打开/创建xbox死区设置文件失败!");
        pushToQueue(parseErrorLog("打开/创建xbox死区设置文件失败!"));
    }
}

void DeadAreaSettings::loadSettingsFile(){
    // 打开 JSON 文件
    QFile file(getAppDataDirStr() + SETTINGS_FILE_NAME);
    if (!file.open(QIODevice::ReadOnly)) {
        pushToQueue(parseWarningLog("打开xbox死区设置文件失败!"));
        return;
    }

    // 读取文件内容并解析为 JSON 文档
    QByteArray jsonData = file.readAll();
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    if (doc.isNull()) {
        pushToQueue(parseWarningLog("解析xbox死区设置文件的json内容失败!"));
        return;
    }

    // 获取 JSON 对象
    QJsonObject jsonObj = doc.object();
    if(jsonObj.contains("xboxJoystickInnerDeadAreaValue")){
        double value = jsonObj["xboxJoystickInnerDeadAreaValue"].toDouble(0.0);
        // 输入超出范围 -1, 1
        if(value < -1 || value > 1){
            value = 0;
        }

        setXboxJoystickInnerDeadAreaValue(value);
        this->ui->lineEdit->setText(std::to_string(value).data());
    }

    if(jsonObj.contains("xboxTriggerInnerDeadAreaValue")){
        double value = jsonObj["xboxTriggerInnerDeadAreaValue"].toDouble(0.0);
        // 输入超出范围 -1, 1
        if(value < -1 || value > 1){
            value = 0;
        }

        setXboxTriggerInnerDeadAreaValue(value);
        this->ui->lineEdit_2->setText(std::to_string(value).data());
    }

    if(jsonObj.contains("keyboardWheelAxisInnerDeadAreaValue")){
        double value = jsonObj["keyboardWheelAxisInnerDeadAreaValue"].toDouble(DEFAULT_INNER_DEADAREA_VALUE);
        // 输入超出范围 -1, 1
        if(value < 0 || value > 1){
            value = DEFAULT_INNER_DEADAREA_VALUE;
        }

        setInnerDeadAreaPanti(value);
        this->ui->lineEdit_4->setText(std::to_string(value).data());
    }

    if(jsonObj.contains("keyboardPedalAxisInnerDeadAreaValue")){
        double value = jsonObj["keyboardPedalAxisInnerDeadAreaValue"].toDouble(DEFAULT_INNER_DEADAREA_VALUE);
        // 输入超出范围 -1, 1
        if(value < 0 || value > 1){
            value = DEFAULT_INNER_DEADAREA_VALUE;
        }

        setInnerDeadAreaTaban(value);
        this->ui->lineEdit_3->setText(std::to_string(value).data());
    }
}


void DeadAreaSettings::on_lineEdit_2_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_2->text().toDouble(&ok);

    // 输入超出范围 -1, 1
    if (!ok || value < -1 || value > 1) {
        ui->lineEdit_2->setText("0.000000");
    }
}


void DeadAreaSettings::on_lineEdit_2_textChanged(const QString &arg1)
{
    unsave();
}


void DeadAreaSettings::on_lineEdit_4_textChanged(const QString &arg1)
{
    unsave();
}


void DeadAreaSettings::on_lineEdit_3_textChanged(const QString &arg1)
{
    unsave();
}


void DeadAreaSettings::on_lineEdit_4_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_4->text().toDouble(&ok);

    // 输入超出范围 0, 1
    if (!ok || value < 0 || value > 1) {
        ui->lineEdit_4->setText(std::to_string(DEFAULT_INNER_DEADAREA_VALUE).data());
    }
}


void DeadAreaSettings::on_lineEdit_3_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_3->text().toDouble(&ok);

    // 输入超出范围 0, 1
    if (!ok || value < 0 || value > 1) {
        ui->lineEdit_3->setText(std::to_string(DEFAULT_INNER_DEADAREA_VALUE).data());
    }
}

