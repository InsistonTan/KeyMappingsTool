#ifndef FORCEFEEDBACKSETTINGSWINDOW_H
#define FORCEFEEDBACKSETTINGSWINDOW_H
#include "mapping_relation.h"
#include <QMainWindow>

#define default_acceleration_100km_time_s 2.78
#define default_stop_100km_dis_m 30
#define default_maxSpeed_km_h 265

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
    QString brakeAxis = ""; // 刹车所在的轴
    QString steeringWheelAxis = "";// 方向盘盘面所在的轴
    bool isThrottleReverse = false;// 是否反转油门
    bool isBrakeReverse = false;// 是否反转刹车
    double acceleration_100km_time_s = default_acceleration_100km_time_s;// 百公里加速所需时间(秒)
    int stop_100km_dis_m = default_stop_100km_dis_m;// 百公里刹停所需距离(米)
    int maxSpeed_km_h = default_maxSpeed_km_h;// 车辆最高时速(m/s)

    // 更新界面变化
    void updateUI();

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

private:
    Ui::ForceFeedbackSettingsWindow *ui;

    // 识别被踩下的轴
    MappingRelation* getDevInputAxis();


};

#endif // FORCEFEEDBACKSETTINGSWINDOW_H
