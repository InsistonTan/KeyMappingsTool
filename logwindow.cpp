#include "logwindow.h"
#include "ui_logwindow.h"
#include <QTimer>
#include <QScrollBar>
#include "global.h"
#include "logworker.h"

//重写窗口关闭事件
void LogWindow::closeEvent(QCloseEvent *event){
    event->ignore();  // 阻止关闭窗口
    this->hide();     // 隐藏窗口
}

void LogWindow::updateLogSlot(QString log){
    //QString srcText = this->ui->textEdit->toHtml();
    //this->ui->textEdit->setHtml(srcText.append(log));
    this->ui->textEdit->append(log);

    QScrollBar *sbar = this->ui->textEdit->verticalScrollBar();
    sbar->setValue(sbar->maximum());
}

LogWindow::LogWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::LogWindow)
{
    ui->setupUi(this);

    setWindowTitle("日志");

    // 线程开启
    LogWorker *task = new LogWorker();
    // 连接信号和槽
    connect(task, &LogWorker::updateLogSignal, this, &LogWindow::updateLogSlot);
    task->start();
}

LogWindow::~LogWindow()
{
    delete ui;
}

void LogWindow::on_checkBox_clicked()
{
    if(ui->checkBox->isChecked()){
        setEnableBtnLog(true);
    }else{
        setEnableBtnLog(false);
    }

}


void LogWindow::on_checkBox_2_clicked()
{
    if(ui->checkBox_2->isChecked()){
        setEnableAxisLog(true);
    }else{
        setEnableAxisLog(false);
    }
}


void LogWindow::on_checkBox_3_clicked()
{
    if(ui->checkBox_3->isChecked()){
        setEnablePovLog(true);
    }else{
        setEnablePovLog(false);
    }
}


void LogWindow::on_pushButton_clicked()
{
    ui->textEdit->clear();
}

