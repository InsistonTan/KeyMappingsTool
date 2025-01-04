#ifndef LOGWINDOW_H
#define LOGWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include <QThread>

namespace Ui {
class LogWindow;
}

class LogWindow : public QMainWindow
{
    Q_OBJECT 

public slots:
    void updateLogSlot(QString log);

public:
    explicit LogWindow(QWidget *parent = nullptr);
    ~LogWindow();

    // 重写窗口关闭事件
    void closeEvent(QCloseEvent *event) override;



private slots:
    void on_checkBox_clicked();

    void on_checkBox_2_clicked();

    void on_checkBox_3_clicked();

private:
    Ui::LogWindow *ui;
};

#endif // LOGWINDOW_H
