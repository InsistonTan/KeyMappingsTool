#include "ForceFeedbackSettingsWindow.h"
#include "ui_ForceFeedbackSettingsWindow.h"
#include "mainwindow.h"
#include "global.h"
#include<QMessageBox>

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
    if(MainWindow::getCurrentSelectedDeviceIndex() < 0){
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
            if(mapping->dev_btn_name == this->brakeAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为刹车!");
                return;
            }
            if(mapping->dev_btn_name == this->steeringWheelAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为转向轴!");
                return;
            }

            this->throttleAxis = mapping->dev_btn_name.data();
            emit settingsChangeSignal();
        }

        // 设置是否反转该轴
        this->isThrottleReverse = mapping->rotateAxis == 1;

        // 更新界面变化
        this->updateUI();
    }else{
        QMessageBox::critical(this, "错误", "未检测到踏板踩下!");
    }
}

MappingRelation* ForceFeedbackSettingsWindow::getDevInputAxis(){
    // 初始化DirectInput
    if(!initDirectInput()){
        return nullptr;
    }
    // 连接设备
    if(!openDiDevice(MainWindow::getCurrentSelectedDeviceIndex())){
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
                    auto tmpAxis = tempRecord.find(item->dev_btn_name);
                    // 第一次读到该轴的值
                    if(tmpAxis == tempRecord.end()){
                        tempRecord.insert_or_assign(item->dev_btn_name, item->dev_btn_value);
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
void ForceFeedbackSettingsWindow::updateUI(){
    // 更新油门轴的label
    if(this->throttleAxis.isEmpty()){
        ui->label_3->setText("未设置");
        ui->label_3->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_3->setText(this->throttleAxis);
        ui->label_3->setStyleSheet("QLabel{color:blue;}");
    }

    // 更新刹车轴的label
    if(this->brakeAxis.isEmpty()){
        ui->label_5->setText("未设置");
        ui->label_5->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_5->setText(this->brakeAxis);
        ui->label_5->setStyleSheet("QLabel{color:blue;}");
    }

    // 更新盘面轴的label
    if(this->steeringWheelAxis.isEmpty()){
        ui->label_14->setText("未设置");
        ui->label_14->setStyleSheet("QLabel{color:red;}");
    }else{
        ui->label_14->setText(this->steeringWheelAxis);
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
    ui->lineEdit->setText(QString::number(this->acceleration_100km_time_s, 'f', 2));
    // 百公里刹停所需距离
    ui->lineEdit_2->setText(std::to_string(this->stop_100km_dis_m).data());
    // 车辆最高时速
    ui->lineEdit_3->setText(std::to_string(this->maxSpeed_km_h).data());
    // 最大力回馈强度
    ui->lineEdit_4->setText(QString::number(this->maxForceFeedbackGain, 'f', 2));
}

void ForceFeedbackSettingsWindow::on_pushButton_2_clicked()
{
    if(MainWindow::getCurrentSelectedDeviceIndex() < 0){
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
            if(mapping->dev_btn_name == this->throttleAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为油门!");
                return;
            }
            if(mapping->dev_btn_name == this->steeringWheelAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为转向轴!");
                return;
            }
            this->brakeAxis = mapping->dev_btn_name.data();
            emit settingsChangeSignal();
        }

        // 设置是否反转该轴
        this->isBrakeReverse = mapping->rotateAxis == 1;

        // 更新界面变化
        this->updateUI();
    }else{
        QMessageBox::critical(this, "错误", "未检测到踏板踩下!");
    }
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

    emit settingsChangeSignal();
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

    emit settingsChangeSignal();
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

    emit settingsChangeSignal();
}


void ForceFeedbackSettingsWindow::on_checkBox_stateChanged(int arg1)
{
    this->isThrottleReverse = ui->checkBox->isChecked();
    emit settingsChangeSignal();
}


void ForceFeedbackSettingsWindow::on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1)
{
    this->isBrakeReverse = ui->checkBox_2->isChecked();
    emit settingsChangeSignal();
}


void ForceFeedbackSettingsWindow::on_pushButton_3_clicked()
{
    if(MainWindow::getCurrentSelectedDeviceIndex() < 0){
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
            if(mapping->dev_btn_name == this->brakeAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为刹车!");
                return;
            }
            if(mapping->dev_btn_name == this->throttleAxis.toStdString()){
                QMessageBox::critical(this, "错误", "该轴已被设置为油门!");
                return;
            }

            this->steeringWheelAxis = mapping->dev_btn_name.data();
            emit settingsChangeSignal();
        }

        // 更新界面变化
        this->updateUI();
    }else{
        QMessageBox::critical(this, "错误", "未检测到方向盘转动!");
    }
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

    emit settingsChangeSignal();
}

