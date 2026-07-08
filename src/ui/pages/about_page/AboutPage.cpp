#include "AboutPage.h"
#include "common/StringConstants.h"

#include <QDesktopServices>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

#include <common/Theme.h>

AboutPage::AboutPage(QWidget *parent)
    : QWidget{parent}
{
    init();
}

void AboutPage::init()
{
    auto rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0,0,0,0);

    QScrollArea* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);
    Theme::setScrollBarStyleSheet(scroll);


    // 页面内容
    QWidget* content = new QWidget(this);
    auto mainLayout = new QVBoxLayout(content);
    mainLayout->setContentsMargins(40,40,40,40);
    mainLayout->setSpacing(24);

    // 标题
    QLabel* title = new QLabel(StringConstants::supportTitle);

    title->setStyleSheet(
        "font-size:28px;"
        "font-weight:600;"
        );

    QLabel* intro = new QLabel(StringConstants::supportIntro);

    intro->setWordWrap(true);

    intro->setStyleSheet(
        "color:#aab2c0;"
        "font-size:15px;"
        );

    mainLayout->addWidget(title);
    mainLayout->addWidget(intro);


    // GitHub star
    QWidget* starCard = createCard(StringConstants::supportFree, StringConstants::supportFreeIntro);

    QPushButton* starBtn = new QPushButton(StringConstants::githubProjectBtn);
    Theme::setButtonStyleSheet(starBtn, ButtonLevel::warning);

    starBtn->setFixedHeight(32);
    connect(starBtn, &QPushButton::clicked, [](){
            QDesktopServices::openUrl(QUrl("https://github.com/InsistonTan/KeyMappingsTool"));
    });

    auto starLayout = qobject_cast<QVBoxLayout*>(starCard->layout());
    starLayout->addWidget(starBtn);

    // 赞赏
    QWidget* donateCard = createCard(StringConstants::donate, StringConstants::donateIntro);

    // 赞赏码
    QLabel* qr = new QLabel;
    qr->setPixmap(
        QPixmap(":/images/donate_wechat.jpg")
            .scaled(
                400,
                400,
                Qt::KeepAspectRatio,
                Qt::SmoothTransformation
                ));

    qr->setAlignment(Qt::AlignCenter);

    donateCard->layout()->addWidget(qr);


    mainLayout->addWidget(starCard);
    mainLayout->addWidget(donateCard);
    mainLayout->addStretch();


    scroll->setWidget(content);
    rootLayout->addWidget(scroll);
}

QWidget *AboutPage::createCard(const QString &title, const QString &description)
{
    QWidget* card = new QWidget;

    card->setStyleSheet(
        R"(
        QWidget{
            background:#1e222b;
            border-radius:12px;
        }

        QLabel{
            border:none;
        }
        )"
        );

    auto layout = new QVBoxLayout(card);

    layout->setContentsMargins(24,20,24,20);

    QLabel* titleLabel = new QLabel(title);

    titleLabel->setStyleSheet(
        "font-size:20px;"
        "font-weight:600;"
        );

    QLabel* desc = new QLabel(description);

    desc->setWordWrap(true);

    desc->setStyleSheet(
        "color:#c0c7d4;"
        "font-size:14px;"
        );

    layout->addWidget(titleLabel);
    layout->addSpacing(10);
    layout->addWidget(desc);

    return card;
}
