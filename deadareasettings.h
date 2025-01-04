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
};

#endif // DEADAREASETTINGS_H
