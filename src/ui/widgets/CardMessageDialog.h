#pragma once

#include "qnamespace.h"
#include "common/StringConstants.h"

#include <QDialog>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGraphicsDropShadowEffect>
#include <QPropertyAnimation>
#include <QGraphicsOpacityEffect>

#include <common/Theme.h>
#include <qpushbutton.h>

//
// 卡片式消息弹窗
//
class CardMessageDialog : public QDialog
{
    Q_OBJECT    
public:

    enum Type {
        Info,
        Warning,
        Error,
        Success
    };

    explicit CardMessageDialog(Type type,
                               const QString& title,
                               const QString& message,
                               QWidget* customContentWidget = nullptr,
                               QVector<QPushButton*> customButtons = {},
                               QWidget *parent = nullptr)
        : QDialog(parent)
    {
        setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);
        setFixedWidth(460);

        QString accent;
        QString iconColor;
        switch (type) {
        case Info:
            accent = "#3B82F6";
            iconColor = "#60A5FA";
            break;
        case Warning:
            accent = "#F59E0B";
            iconColor = "#FBBF24";
            break;
        case Error:
            accent = "#EF4444";
            iconColor = "#F87171";
            break;
        case Success:
            accent = "#10B981";
            iconColor = "#34D399";
            break;
        }

        QWidget *container = new QWidget(this);
        container->setObjectName("container");

        // 阴影
        auto *shadow = new QGraphicsDropShadowEffect(container);
        shadow->setBlurRadius(40);
        shadow->setOffset(0, 12);
        shadow->setColor(QColor(0, 0, 0, 100));

        container->setGraphicsEffect(shadow);

        container->setStyleSheet(QString(R"(
            QWidget#container {
                background-color: #0B1220;
                border-radius: 14px;
                border: 1px solid rgba(148,163,184,0.15);
            }

            QLabel {
                color: #E2E8F0;
            }

            QLabel#title {
                font-size: 15px;
                font-weight: 600;
                color: #F1F5F9;
            }

            QLabel#msg {
                font-size: 13px;
                color: #94A3B8;
            }

            QPushButton {
                background-color: #111827;
                color: #E2E8F0;
                border: 1px solid rgba(148,163,184,0.15);
                padding: 6px 14px;
                border-radius: 8px;
            }

            QPushButton:hover {
                background-color: #1F2937;
            }

            QPushButton:default {
                background-color: %1;
                border: 1px solid %1;
                color: white;
            }

            QPushButton:default:hover {
                opacity: 0.9;
            }
        )").arg(accent));

        // ===== 左侧 accent bar =====
        QWidget *accentBar = new QWidget();
        accentBar->setFixedWidth(4);
        accentBar->setStyleSheet(QString("background-color:%1;border-radius:2px;").arg(accent));

        // ===== icon badge =====
        QLabel *icon = new QLabel();
        icon->setFixedSize(28, 28);
        icon->setAlignment(Qt::AlignCenter);
        icon->setStyleSheet(QString(R"(
            background-color: rgba(%1, 0.15);
            color: %1;
            border-radius: 14px;
        )").arg(iconColor));

        icon->setText(type == Info ? "i" : type == Warning ? "!" : type == Success ? "✓" : "×");
        icon->setStyleSheet(QString(R"(
            QLabel {
                background-color: rgba(%1, 0.15);
                color: %1;
                border-radius: 14px;
                font-weight: 600;
                font-size: 16px;
            }
        )").arg(iconColor));

        // ===== title =====
        QLabel *titleLabel = new QLabel(title);
        titleLabel->setObjectName("title");

        // ===== message =====
        QLabel *msgLabel = new QLabel(message);
        msgLabel->setObjectName("msg");
        msgLabel->setWordWrap(true);

        // ===== layout =====
        QVBoxLayout *contentLayout = new QVBoxLayout();
        contentLayout->setContentsMargins(0, 0, 0, 0);
        contentLayout->setSpacing(10);

        QHBoxLayout *headerLayout = new QHBoxLayout();
        headerLayout->setSpacing(12);
        headerLayout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        headerLayout->addWidget(icon);
        headerLayout->addWidget(titleLabel);
        headerLayout->addStretch();

        contentLayout->addLayout(headerLayout);
        contentLayout->addWidget(msgLabel);

        // 自定义内容组件
        if(customContentWidget != nullptr){
            contentLayout->addWidget(customContentWidget);
        }

        QHBoxLayout *btnLayout = new QHBoxLayout();
        btnLayout->addStretch();
        // ===== buttons =====
        if(customButtons.isEmpty()){
            QPushButton *okBtn = new QPushButton(StringConstants::btnText_confirm);
            okBtn->setDefault(true);
            Theme::setButtonStyleSheet(okBtn, ButtonLevel::primary);
            connect(okBtn, &QPushButton::clicked, this, &QDialog::accept);

            QPushButton *cancelBtn = new QPushButton(StringConstants::btnText_cancel);
            Theme::setButtonStyleSheet(cancelBtn, ButtonLevel::normal);
            connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

            btnLayout->addWidget(cancelBtn);
            btnLayout->addWidget(okBtn);
        }else{
            for(int i = 0; i < customButtons.size(); i++){
                btnLayout->addWidget(customButtons[i]);
                connect(customButtons[i], &QPushButton::clicked, this, [=](){
                    clickedBtn = customButtons[i];
                    this->done(i);
                });
            }
        }

        contentLayout->addSpacing(6);
        contentLayout->addLayout(btnLayout);

        QHBoxLayout *rootLayout = new QHBoxLayout(container);
        rootLayout->setContentsMargins(16, 14, 16, 14);
        rootLayout->setSpacing(12);

        rootLayout->addWidget(accentBar);
        rootLayout->addLayout(contentLayout);

        QVBoxLayout *outer = new QVBoxLayout(this);
        outer->addWidget(container);
        outer->setContentsMargins(20, 20, 20, 20);
    }

    static void info(QWidget *parent, const QString& title, const QString& msg)
    {
        CardMessageDialog dlg(Info, title, msg, parent);
        dlg.exec();
    }

    static void warning(QWidget *parent, const QString& title, const QString& msg)
    {
        CardMessageDialog dlg(Warning, title, msg, parent);
        dlg.exec();
    }

    static void error(QWidget *parent, const QString& title, const QString& msg)
    {
        CardMessageDialog dlg(Error, title, msg, parent);
        dlg.exec();
    }

    static void success(QWidget *parent, const QString& title, const QString& msg)
    {
        CardMessageDialog dlg(Success, title, msg, parent);
        dlg.exec();
    }

    QPushButton* clickedButton(){
        return clickedBtn;
    }

private:
    QPushButton* clickedBtn = nullptr;

protected:

    void showEvent(QShowEvent *event) override
    {
        QDialog::showEvent(event);
        playShowAnimation();
    }

    void playShowAnimation()
    {
        auto *opacity = new QGraphicsOpacityEffect(this);
        setGraphicsEffect(opacity);
        opacity->setOpacity(0.0);

        auto *anim = new QPropertyAnimation(opacity, "opacity");
        anim->setDuration(180);
        anim->setStartValue(0.0);
        anim->setEndValue(1.0);
        anim->setEasingCurve(QEasingCurve::OutCubic);

        connect(anim, &QPropertyAnimation::finished, this, [=] {
            setGraphicsEffect(nullptr);
        });

        anim->start(QAbstractAnimation::DeleteWhenStopped);
    }
};
