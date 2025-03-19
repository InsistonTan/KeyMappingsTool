#ifndef DEADAREASETTINGS_H
#define DEADAREASETTINGS_H

#include <QMainWindow>

namespace Ui {
class DeadAreaSettings;
}

class DeadAreaSettings : public QMainWindow
{
    Q_OBJECT

public:
    explicit DeadAreaSettings(QWidget *parent = nullptr);
    ~DeadAreaSettings();

private slots:
    void on_pushButton_clicked();

private:
    Ui::DeadAreaSettings *ui;
    QString appDataDirPath; // 软件本地数据存放的路径
};

#endif // DEADAREASETTINGS_H
