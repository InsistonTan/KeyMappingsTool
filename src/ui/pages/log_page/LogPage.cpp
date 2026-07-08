#include "LogPage.h"
#include "qboxlayout.h"
#include "qcheckbox.h"
#include "qcoreapplication.h"
#include "qpushbutton.h"
#include "qsizepolicy.h"
#include "qtextedit.h"
#include "common/Global.h"

#include <common/Theme.h>

#include <QScrollBar>

#include <workers/LogWorker.h>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent)
{
    // 页面根布局
    QVBoxLayout* rootLayout = new QVBoxLayout(this);
    rootLayout->setSpacing(Theme::contentMargin);
    rootLayout->setContentsMargins(Theme::contentMargin,Theme::contentMargin,Theme::contentMargin,Theme::contentMargin);

    // ==========================
    // 第1行 容器
    // ==========================
    QFrame* line1 = new QFrame;
    QHBoxLayout* line1Layout = new QHBoxLayout(line1);
    line1Layout->setContentsMargins(16,16,16,16);
    line1Layout->setSpacing(8);

    // 创建 三个单选框, "按键日志","十字键日志","轴日志"
    QCheckBox* enableBtnLog = new QCheckBox(StringConstants::btnLog);
    QCheckBox* enablePovLog = new QCheckBox(StringConstants::povLog);
    QCheckBox* enableAxisLog = new QCheckBox(StringConstants::axisLog);
    QPushButton* clearLogBtn = new QPushButton(StringConstants::clearLog);

    // 设置事件监听
    connect(enableBtnLog, &QCheckBox::clicked, this, [=](){
        Global::setEnableBtnLog(enableBtnLog->isChecked());
    });
    connect(enablePovLog, &QCheckBox::clicked, this, [=](){
        Global::setEnablePovLog(enablePovLog->isChecked());
    });
    connect(enableAxisLog, &QCheckBox::clicked, this, [=](){
        Global::setEnableAxisLog(enableAxisLog->isChecked());
    });
    connect(clearLogBtn, &QPushButton::clicked, this, [=](){
        this->logArea->clear();
    });

    // 设置样式
    Theme::setCheckBoxStyleSheet(enableBtnLog);
    Theme::setCheckBoxStyleSheet(enablePovLog);
    Theme::setCheckBoxStyleSheet(enableAxisLog);
    Theme::setButtonStyleSheet(clearLogBtn, ButtonLevel::normal);

    // 添加到第一行布局
    line1Layout->addWidget(enableBtnLog);
    line1Layout->addWidget(enablePovLog);
    line1Layout->addWidget(enableAxisLog);
    line1Layout->addStretch();
    line1Layout->addWidget(clearLogBtn);

    // ==========================
    // 第2行 容器
    // ==========================
    QFrame* line2 = new QFrame;
    QHBoxLayout* line2Layout = new QHBoxLayout(line2);
    line2Layout->setSpacing(8);
    // 日志输出的区域
    logArea = new QTextEdit;
    Theme::setScrollBarStyleSheet(logArea);
    line2Layout->addWidget(logArea);


    // 设置样式
    Theme::setQFrameStyleSheet(line1);
    Theme::setQFrameStyleSheet(line2);

    // 第二行容器填满剩余位置
    line2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    // 添加到布局
    rootLayout->addWidget(line1);
    rootLayout->addWidget(line2);

    // 日志线程开启
    LogWorker *task = new LogWorker();
    // 连接信号和槽
    connect(task, &LogWorker::updateLogSignal, this, &LogPage::updateLogSlot);
    task->start();
}

LogPage::~LogPage()
{
}

void LogPage::updateLogSlot(QString log)
{
    logArea->append(log);

    // 处理一下qt事件
    QCoreApplication::processEvents();

    // 滚动条到底部
    QScrollBar *sbar = logArea->verticalScrollBar();
    sbar->setValue(sbar->maximum());
}
