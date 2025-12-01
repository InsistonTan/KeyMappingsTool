#ifndef FORCEFEEDBACKSETTINGSWINDOW_H
#define FORCEFEEDBACKSETTINGSWINDOW_H
#include "mapping_relation.h"
#include <QMainWindow>

#define default_acceleration_100km_time_s 8 //默认百公里加速时间单位s
#define default_stop_100km_dis_m 30 // 默认百公里刹停距离单位m
#define default_maxSpeed_km_h 200 // 默认最高时速单位km/h
#define default_max_forcefeedback_gain 0.5 // 默认最大力回馈强度
#define default_constant_corrective_force_gain 0.5 // 默认恒定回正力强度
#define default_constant_damping_gain 0.5 // 默认恒定转向阻尼强度

namespace Ui {
class ForceFeedbackSettingsWindow;
}

class ForceFeedbackSettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ForceFeedbackSettingsWindow(QWidget *parent = nullptr);
    ~ForceFeedbackSettingsWindow();

    QString throttleAxis = ""; // 油门所在的轴
    QString throttleAxisDeviceName = ""; // 油门轴的设备名称

    QString brakeAxis = ""; // 刹车所在的轴
    QString brakeAxisDeviceName = ""; // 刹车轴的设备名称

    QString steeringWheelAxis = "";// 方向盘盘面所在的轴
    QString steeringWheelAxisDeviceName = "";// 方向盘盘面所在的轴的设备名称

    bool isThrottleReverse = false;// 是否反转油门
    bool isBrakeReverse = false;// 是否反转刹车

    double acceleration_100km_time_s = default_acceleration_100km_time_s;// 百公里加速所需时间(秒)
    int stop_100km_dis_m = default_stop_100km_dis_m;// 百公里刹停所需距离(米)
    int maxSpeed_km_h = default_maxSpeed_km_h;// 车辆最高时速(km/h)
    double maxForceFeedbackGain = default_max_forcefeedback_gain; // 最大力回馈强度

    bool isConstantForceMode = false;// 是否为恒定力反馈模式
    double constantCorrectiveForceGain = default_constant_corrective_force_gain;// 恒定回正力强度
    double constantDampingGain = default_constant_damping_gain;// 恒定转向阻尼强度

    // 更新界面变化
    void updateUI(bool isFirstUpdate = false);

signals:
    void settingsChangeSignal();

private slots:
    void on_pushButton_clicked();

    void on_pushButton_2_clicked();

    void on_lineEdit_editingFinished();

    void on_lineEdit_2_editingFinished();

    void on_lineEdit_3_editingFinished();

    void on_checkBox_stateChanged(int arg1);

    void on_checkBox_2_checkStateChanged(const Qt::CheckState &arg1);

    void on_pushButton_3_clicked();

    void on_lineEdit_4_editingFinished();

    void on_pushButton_4_clicked();

    void unsave();
    void save();

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

    void on_lineEdit_3_textChanged(const QString &arg1);

    void on_lineEdit_4_textChanged(const QString &arg1);

    void on_checkBox_3_clicked();

    void on_lineEdit_5_editingFinished();

    void on_lineEdit_6_editingFinished();

private:
    Ui::ForceFeedbackSettingsWindow *ui;

    // 识别被踩下的轴
    MappingRelation* getDevInputAxis();

    // 检查是否是恒定力反馈模式
    void checkConstantForceMode();

};

#endif // FORCEFEEDBACKSETTINGSWINDOW_H
