#include "CollapsibleGroupBox.h"
#include "qboxlayout.h"
#include "qnamespace.h"
#include "utils/IconFactory.h"
#include <QFrame>
#include <common/Theme.h>

CollapsibleGroupBox::CollapsibleGroupBox(const QString& title, QWidget *parent, QLayout *contentLayout)
    : QWidget(parent)
{
    this->setStyleSheet(R"(
        CollapsibleGroupBox {
            background-color: transparent;
        }
    )");

    // ===== 主布局 =====
    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // =========================
    // Header（标题栏）
    // =========================
    headerButton = new QToolButton(this);
    headerButton->setText(title);
    headerButton->setCursor(Qt::PointingHandCursor);
    headerButton->setCheckable(true);
    headerButton->setChecked(false);
    headerButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    headerButton->setFixedHeight(42);
    headerButton->setStyleSheet(QString(R"(
        QToolButton {
            color: %1;
            font-size: 13px;
            font-weight: 500;

            padding: 8px 12px;
            border-radius: 10px;

            background-color: %2;
            border: 1px solid #1F2937;

            text-align: left;
        }

        QToolButton:hover {
            background-color: %2;
            border: 1px solid #4F5967;
        }

        QToolButton:checked {
            background-color: #334155;
            border: 1px solid #7F8997;
        }
    )").arg(Theme::textColor(), Theme::widgetBg()));

    // 自定义的 arrow 图标
    arrowRight = IconFactory::colorIcon(IconFactory::IconEnum::arrow_right,
                                            QColor(200, 200, 200),
                                            24);
    arrowDown = IconFactory::colorIcon(IconFactory::IconEnum::arrow_down,
                                       QColor(200, 200, 200),
                                       24);
    // 禁用自带的
    headerButton->setArrowType(Qt::NoArrow);
    // 设置默认图标和图标大小
    headerButton->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    headerButton->setIcon(arrowRight);
    headerButton->setIconSize(QSize(14,14));


    // =========================
    // 内容区域
    // =========================
    contentWidget = new QWidget(this);
    contentWidget->setObjectName("contentWidget");
    contentWidget->setStyleSheet(R"(
        QWidget#contentWidget {
            background-color: #0B1220;
            border: 1px solid #1F2937;
            border-top: none;
            border-radius: 10px;
        }
    )");
    contentWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);


    if(contentLayout == nullptr){
        contentLayout = new QVBoxLayout;
    }

    contentLayout->setContentsMargins(12, 10, 12, 10);
    contentLayout->setSpacing(8);
    contentWidget->setLayout(contentLayout);


    // =========================
    // 组装
    // =========================
    mainLayout->addWidget(headerButton);
    mainLayout->addWidget(contentWidget);

    // 默认折叠内容
    contentWidget->setVisible(false);

    // =========================
    // 折叠逻辑
    // =========================
    connect(headerButton, &QToolButton::toggled, this, [=](bool checked)
    {
        headerButton->setIcon(checked ? arrowDown : arrowRight);
        contentWidget->setVisible(checked);

    });
}

void CollapsibleGroupBox::setContentLayout(QLayout *layout)
{
    delete contentWidget->layout();

    layout->setContentsMargins(12, 10, 12, 10);
    layout->setSpacing(8);
    contentWidget->setLayout(layout);
}

void CollapsibleGroupBox::setContentVisible(bool visible)
{
    headerButton->setChecked(true);
    headerButton->setIcon(visible ? arrowDown : arrowRight);
    contentWidget->setVisible(visible);
}
