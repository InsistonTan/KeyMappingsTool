#include "deadareasettings.h"
#include "ui_deadareasettings.h"
#include "global.h"
#include<QFile>
#include<QDoubleValidator>
#include<QMessageBox>
#include<QDir>

DeadAreaSettings::DeadAreaSettings(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::DeadAreaSettings)
{
    ui->setupUi(this);

    this->setWindowTitle("死区设置");

    // 获取软件本地数据目录
    appDataDirPath = QDir::homePath() + "/AppData/Local/KeyMappingToolData/";

    // 输入限制, 只能输入浮点数
    QDoubleValidator* validator = new QDoubleValidator(0.000000, 1.000000, 6, this);  // 设置有效数字范围为 0 到 1，最多保留 6 位小数
    ui->lineEdit->setValidator(validator);
    ui->lineEdit_2->setValidator(validator);

    // 获取死区设置的值
    ui->lineEdit->setText(std::to_string(innerDeadAreaPanti).data());
    ui->lineEdit_2->setText(std::to_string(innerDeadAreaTaban).data());


}

DeadAreaSettings::~DeadAreaSettings()
{
    delete ui;
}

void DeadAreaSettings::on_pushButton_clicked()
{
    bool ok1, ok2;

    // 获取死区输入框的值
    double var1 = ui->lineEdit->text().toDouble(&ok1);
    double var2 = ui->lineEdit_2->text().toDouble(&ok2);

    if(!ok1 || var1 > 1 || var1 < 0){
        QMessageBox::critical(this, "错误", "盘面轴内部死区值不符合规范!");
        return;
    }
    if(!ok2 || var2 > 1 || var2 < 0){
        QMessageBox::critical(this, "错误", "踏板轴内部死区值不符合规范!");
        return;
    }


    // 创建一个 QFile 对象，并打开文件进行写入
    QFile file2(appDataDirPath + "settings");
    QString text2;
    text2.append(ui->lineEdit->text()).append("\n").append(ui->lineEdit_2->text());

    if (file2.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file2);  // 创建一个文本流对象
        out << text2;             // 写入文本

        // 关闭文件
        file2.close();
        qDebug() << "last device saved successfully!";
    } else {
        qDebug() << "Error opening file!";
    }

    // 设置到全局变量
    setInnerDeadAreaPanti(var1);
    setInnerDeadAreaTaban(var2);

    QMessageBox::information(this, "提醒", "保存成功!");
    this->hide();
}

