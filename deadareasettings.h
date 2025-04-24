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

    void on_lineEdit_textChanged(const QString &arg1);

    void on_lineEdit_2_textChanged(const QString &arg1);

private:
    Ui::DeadAreaSettings *ui;
    QString appDataDirPath; // 软件本地数据存放的路径

    void unsave();
    void save();
};

#endif // DEADAREASETTINGS_H
