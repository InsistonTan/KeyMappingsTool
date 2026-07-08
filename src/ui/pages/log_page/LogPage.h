#pragma once

#include <QTextEdit>
#include <QWidget>

class LogPage : public QWidget
{
    Q_OBJECT

private:
    QTextEdit* logArea;

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

public slots:
    void updateLogSlot(QString log);
};
