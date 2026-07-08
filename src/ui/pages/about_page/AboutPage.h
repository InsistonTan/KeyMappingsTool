#pragma once

#include <QWidget>

class AboutPage : public QWidget
{
    Q_OBJECT
public:
    explicit AboutPage(QWidget *parent = nullptr);

private:
    void init();

    QWidget* createCard(
        const QString& title,
        const QString& description
        );
signals:
};
