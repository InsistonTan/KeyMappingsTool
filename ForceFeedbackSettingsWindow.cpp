#include "ForceFeedbackSettingsWindow.h"
#include "ui_ForceFeedbackSettingsWindow.h"
#include "mainwindow.h"
#include "global.h"
#include<QMessageBox>
#include<QStyleHints>

ForceFeedbackSettingsWindow::ForceFeedbackSettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::ForceFeedbackSettingsWindow)
{
    ui->setupUi(this);

    setWindowTitle("力反馈模拟设置");
}

ForceFeedbackSettingsWindow::~ForceFeedbackSettingsWindow()
{
    delete ui;
}

void ForceFeedbackSettingsWindow::on_pushButton_clicked()
{
    if(MainWindow::getCurrentSelectedDeviceList().isEmpty()){
        QMessageBox::critical(this, "错误", "还未选择设备!");
        return;
    }

    ui->pushButton->setEnabled(false);
    ui->pushButton->setText("等待油门踩下...");
    this->repaint();

    auto mapping = getDevInputAxis();

    ui->pushButton->setText("点击后踩下油门");
    ui->pushButton->setEnabled(true);

    if(mapping != nullptr){
        if(!mapping->dev_btn_name.empty()){
            if(mapping->deviceName == this->brakeAxisDeviceName && mapping->dev_btn_name == this->brakeAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为刹车!");
                return;
            }
            if(mapping->deviceName == this->steeringWheelAxisDeviceName && mapping->dev_btn_name == this->steeringWheelAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为转向轴!");
                return;
            }

            this->throttleAxis = mapping->dev_btn_name.data();
            this->throttleAxisDeviceName = mapping->deviceName;
            unsave();
        }

        // 设置是否反转该轴
        this->isThrottleReverse = mapping->rotateAxis == 1;
    }else{
        QMessageBox::critical(this, "错误", "未检测到踏板踩下, 将重置该项!");
        this->throttleAxis = "";
        this->throttleAxisDeviceName = "";
        this->isThrottleReverse = false;
        unsave();
    }

    // 更新界面变化
    this->updateUI();
}

MappingRelation* ForceFeedbackSettingsWindow::getDevInputAxis(){
    // 初始化DirectInput
    if(!initDirectInput()){
        return nullptr;
    }
    // 连接设备
    if(!openDiDevice(MainWindow::getCurrentSelectedDeviceList())){
        return nullptr;
    }

    std::map<std::string, int> tempRecord;

    bool enableLogs = getEnablePovLog() || getEnableBtnLog() || getEnableAxisLog();

    // 监听设备按键状态
    for(int i=0; i<60; i++){
        // 获取设备状态数据
        auto res = getInputState(enableLogs);
        if(res.size() > 0){
            for(auto item : res){
                // 方向盘的轴
                if(item->dev_btn_type == (std::string)WHEEL_AXIS){
                    std::string btnStr = item->deviceName.toStdString() + "-" + item->dev_btn_name;
                    auto tmpAxis = tempRecord.find(btnStr);
                    // 第一次读到该轴的值
                    if(tmpAxis == tempRecord.end()){
                        tempRecord.insert_or_assign(btnStr, item->dev_btn_value);
                    }else{
                        // 不是第一次读到该轴的值, 与第一次的值比较, 大于一定量才能确定是该轴要新建映射
                        if(std::abs(item->dev_btn_value - tmpAxis->second) > AXIS_CHANGE_VALUE){
                            // 如果值由大变小, 说明这个轴是反的, 需要反转
                            if(item->dev_btn_value < tmpAxis->second){
                                item->rotateAxis = 1;
                            }

                            return item;
                        }
                    }
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


// 更新界面变化
void ForceFeedbackSettingsWindow::updateUI(bool isFirstUpdate){
    // 更新油门轴的label
    if(this->throttleAxis.isEmpty()){
        ui->label_3->setText("未设置");
        ui->label_3->setToolTip("");
        ui->label_3->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_3->setText(this->throttleAxis);
        ui->label_3->setToolTip(this->throttleAxisDeviceName);
        ui->label_3->setStyleSheet("QLabel{color:blue;}");
    }

    // 更新刹车轴的label
    if(this->brakeAxis.isEmpty()){
        ui->label_5->setText("未设置");
        ui->label_5->setToolTip("");
        ui->label_5->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_5->setText(this->brakeAxis);
        ui->label_5->setToolTip(this->brakeAxisDeviceName);
        ui->label_5->setStyleSheet("QLabel{color:blue;}");
    }

    // 更新盘面轴的label
    if(this->steeringWheelAxis.isEmpty()){
        ui->label_14->setText("未设置");
        ui->label_14->setToolTip("");
        ui->label_14->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_14->setText(this->steeringWheelAxis);
        ui->label_14->setToolTip(this->steeringWheelAxisDeviceName);
        ui->label_14->setStyleSheet("QLabel{color:blue;}");
    }

    if(this->isThrottleReverse){
        ui->checkBox->setChecked(true);
    }else{
        ui->checkBox->setChecked(false);
    }
    if(this->isBrakeReverse){
        ui->checkBox_2->setChecked(true);
    }else{
        ui->checkBox_2->setChecked(false);
    }

    // 百公里加速所需时间s
    ui->lineEdit->setText(removeUnnecessaryZero(QString::number(this->acceleration_100km_time_s)));
    // 百公里刹停所需距离
    ui->lineEdit_2->setText(std::to_string(this->stop_100km_dis_m).data());
    // 车辆最高时速
    ui->lineEdit_3->setText(std::to_string(this->maxSpeed_km_h).data());
    // 最大力回馈强度
    ui->lineEdit_4->setText(removeUnnecessaryZero(QString::number(this->maxForceFeedbackGain)));


    // 恒定力模式
    ui->checkBox_3->setChecked(this->isConstantForceMode);
    // 恒定回正力强度
    ui->lineEdit_5->setText(removeUnnecessaryZero(QString::number(this->constantCorrectiveForceGain)));
    // 恒定阻尼
    ui->lineEdit_6->setText(removeUnnecessaryZero(QString::number(this->constantDampingGain)));


    // 检查是否是恒定力模式
    checkConstantForceMode();

    // 初次更新为非手动更新, 需要将窗口标题恢复
    if(isFirstUpdate){
        save();
    }
}

void ForceFeedbackSettingsWindow::on_pushButton_2_clicked()
{
    if(MainWindow::getCurrentSelectedDeviceList().isEmpty()){
        QMessageBox::critical(this, "错误", "还未选择设备!");
        return;
    }

    ui->pushButton_2->setEnabled(false);
    ui->pushButton_2->setText("等待刹车踩下...");
    this->repaint();

    auto mapping = getDevInputAxis();

    ui->pushButton_2->setText("点击后踩下刹车");
    ui->pushButton_2->setEnabled(true);

    if(mapping != nullptr){
        if(!mapping->dev_btn_name.empty()){
            if(mapping->deviceName == this->throttleAxisDeviceName && mapping->dev_btn_name == this->throttleAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为油门!");
                return;
            }
            if(mapping->deviceName == this->steeringWheelAxisDeviceName && mapping->dev_btn_name == this->steeringWheelAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为转向轴!");
                return;
            }
            this->brakeAxis = mapping->dev_btn_name.data();
            this->brakeAxisDeviceName = mapping->deviceName;
            unsave();
        }

        // 设置是否反转该轴
        this->isBrakeReverse = mapping->rotateAxis == 1;
    }else{
        QMessageBox::critical(this, "错误", "未检测到踏板踩下, 将重置该项!");
        this->brakeAxis = "";
        this->brakeAxisDeviceName = "";
        this->isBrakeReverse = false;
        unsave();
    }

    // 更新界面变化
    this->updateUI();
}


void ForceFeedbackSettingsWindow::on_lineEdit_editingFinished()
{
    bool ok;
    double value = ui->lineEdit->text().toDouble(&ok);
    // 输入超出范围
    if (!ok || value < 0.1 || value > 100.00) {
        ui->lineEdit->setText(QString::number(default_acceleration_100km_time_s, 'f', 2));
        this->acceleration_100km_time_s = default_acceleration_100km_time_s;
    }else{
        this->acceleration_100km_time_s = value;
    }
}


void ForceFeedbackSettingsWindow::on_lineEdit_2_editingFinished()
{
    bool ok;
    int value = ui->lineEdit_2->text().toInt(&ok);
    // 输入超出范围
    if (!ok || value < 0 || value > 100) {
        ui->lineEdit_2->setText(std::to_string(default_stop_100km_dis_m).data());
        this->stop_100km_dis_m = default_stop_100km_dis_m;
    }else{
        this->stop_100km_dis_m = value;
    }

}


void ForceFeedbackSettingsWindow::on_lineEdit_3_editingFinished()
{
    bool ok;
    int value = ui->lineEdit_3->text().toInt(&ok);
    // 输入超出范围
    if (!ok || value < 0 || value > 1000) {
        ui->lineEdit_3->setText(std::to_string(default_maxSpeed_km_h).data());
        this->maxSpeed_km_h = default_maxSpeed_km_h;
    }else{
        this->maxSpeed_km_h = value;
    }

}


void ForceFeedbackSettingsWindow::on_checkBox_stateChanged(int arg1)
{
    this->isThrottleReverse = ui->checkBox->isChecked();
    unsave();
}


void ForceFeedbackSettingsWindow::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{
    this->isBrakeReverse = ui->checkBox_2->isChecked();
    unsave();
}


void ForceFeedbackSettingsWindow::on_pushButton_3_clicked()
{
    if(MainWindow::getCurrentSelectedDeviceList().isEmpty()){
        QMessageBox::critical(this, "错误", "还未选择设备!");
        return;
    }

    ui->pushButton_3->setEnabled(false);
    ui->pushButton_3->setText("等待方向盘转动...");
    this->repaint();

    auto mapping = getDevInputAxis();

    ui->pushButton_3->setText("点击后转动方向盘");
    ui->pushButton_3->setEnabled(true);

    if(mapping != nullptr){
        if(!mapping->dev_btn_name.empty()){
            if(mapping->deviceName == this->brakeAxisDeviceName && mapping->dev_btn_name == this->brakeAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为刹车!");
                return;
            }
            if(mapping->deviceName == this->throttleAxisDeviceName && mapping->dev_btn_name == this->throttleAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为油门!");
                return;
            }

            this->steeringWheelAxis = mapping->dev_btn_name.data();
            this->steeringWheelAxisDeviceName = mapping->deviceName;
            unsave();
        }
    }else{
        QMessageBox::critical(this, "错误", "未检测到方向盘转动, 将重置该项!");
        this->steeringWheelAxis = "";
        this->steeringWheelAxisDeviceName = "";
        unsave();
    }

    // 更新界面变化
    this->updateUI();
}


void ForceFeedbackSettingsWindow::on_lineEdit_4_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_4->text().toDouble(&ok);

    // 校验输入
    if (!ok) {
        ui->lineEdit_4->setText(QString::number(default_max_forcefeedback_gain, 'f', 2));
        this->maxForceFeedbackGain = default_max_forcefeedback_gain;
    }else if(value < 0.000005){
        ui->lineEdit_4->setText("0");
        this->maxForceFeedbackGain = 0.0;
    }else if(value > 1.0){
        ui->lineEdit_4->setText("1");
        this->maxForceFeedbackGain = 1.0;
    }else{
        this->maxForceFeedbackGain = value;
    }
}


void ForceFeedbackSettingsWindow::on_pushButton_4_clicked()
{
    save();
    emit settingsChangeSignal();
}

void ForceFeedbackSettingsWindow::unsave(){
    setWindowTitle("力反馈模拟设置 *设置未保存");
}
void ForceFeedbackSettingsWindow::save(){
    setWindowTitle("力反馈模拟设置");
}

void ForceFeedbackSettingsWindow::on_lineEdit_textChanged(const QString &arg1)
{
    unsave();
}


void ForceFeedbackSettingsWindow::on_lineEdit_2_textChanged(const QString &arg1)
{
    unsave();
}


void ForceFeedbackSettingsWindow::on_lineEdit_3_textChanged(const QString &arg1)
{
    unsave();
}


void ForceFeedbackSettingsWindow::on_lineEdit_4_textChanged(const QString &arg1)
{
    unsave();
}


void ForceFeedbackSettingsWindow::checkConstantForceMode(){
    if(ui->checkBox_3->isChecked()){
        ui->lineEdit->setDisabled(true);
        ui->lineEdit_2->setDisabled(true);
        ui->lineEdit_3->setDisabled(true);
        ui->lineEdit_4->setDisabled(true);

        ui->lineEdit_5->setDisabled(false);
        ui->lineEdit_6->setDisabled(false);
    }else{
        ui->lineEdit->setDisabled(false);
        ui->lineEdit_2->setDisabled(false);
        ui->lineEdit_3->setDisabled(false);
        ui->lineEdit_4->setDisabled(false);

        ui->lineEdit_5->setDisabled(true);
        ui->lineEdit_6->setDisabled(true);
    }

    QList<QLineEdit*> lineEditList = {ui->lineEdit, ui->lineEdit_2, ui->lineEdit_3, ui->lineEdit_4, ui->lineEdit_5, ui->lineEdit_6};
    for(auto lineEdit : lineEditList){
        // 输入框没被禁用
        if(lineEdit->isEnabled()){
            // 适配深色模式
            if (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark){
                lineEdit->setStyleSheet("QLineEdit{background-color: rgb(45, 45, 45); color: white;}");
            }else{
                lineEdit->setStyleSheet("QLineEdit{background-color: white; color: black;}");
            }
        }
        // 被禁用
        else{
            if (qApp->styleHints()->colorScheme() == Qt::ColorScheme::Dark){
                lineEdit->setStyleSheet("QLineEdit{background-color: rgb(45, 45, 45); color: rgb(85, 85, 85);}");
            }else{
                lineEdit->setStyleSheet("QLineEdit{background-color: rgb(220, 220, 220); color: rgb(150, 150, 150);}");
            }
        }
    }
}



void ForceFeedbackSettingsWindow::on_checkBox_3_clicked()
{
    this->isConstantForceMode = ui->checkBox_3->isChecked();

    unsave();
    updateUI();
}


void ForceFeedbackSettingsWindow::on_lineEdit_5_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_5->text().toDouble(&ok);

    // 校验输入
    if (!ok) {
        ui->lineEdit_5->setText(QString::number(default_constant_corrective_force_gain, 'f', 2));
        this->constantCorrectiveForceGain = default_constant_corrective_force_gain;
    }else if(value < 0.000005){
        ui->lineEdit_5->setText("0");
        this->constantCorrectiveForceGain = 0.0;
    }else if(value > 1.0){
        ui->lineEdit_5->setText("1");
        this->constantCorrectiveForceGain = 1.0;
    }else{
        this->constantCorrectiveForceGain = value;
    }
}


void ForceFeedbackSettingsWindow::on_lineEdit_6_editingFinished()
{
    bool ok;
    double value = ui->lineEdit_6->text().toDouble(&ok);

    // 校验输入
    if (!ok) {
        ui->lineEdit_6->setText(QString::number(default_constant_damping_gain, 'f', 2));
        this->constantDampingGain = default_constant_damping_gain;
    }else if(value < 0.000005){
        ui->lineEdit_6->setText("0");
        this->constantDampingGain = 0.0;
    }else if(value > 1.0){
        ui->lineEdit_6->setText("1");
        this->constantDampingGain = 1.0;
    }else{
        this->constantDampingGain = value;
    }
}

