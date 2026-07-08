#include "qcheckbox.h"
#include "qframe.h"
#include "qlineedit.h"
#include "qpushbutton.h"
#include <QComboBox>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QRadioButton>
#include <QScrollArea>
#include <QString>
#include <QSvgRenderer>
#include <qmenu.h>

#pragma once

// 按钮样式级别
enum class ButtonLevel{
    normal,
    primary,
    success,
    warning,
    critical
};

//
// ui主题, 负责处理控件的样式
//
class Theme{
public:
    // 布局边距
    inline static int contentMargin = 18;

    static QString rootBg() {return "#0D121D";}

    // QFrame 的背景色#1a1f2a
    static QString panelBg()  { return "#1a1f2a"; }

    // QWidget 的背景色#1a1f2a
    static QString widgetBg() { return "#1a1f2a"; }

    // 文本的颜色 #c0c7d4
    static QString textColor() {return "#E5E7EB";}
    // 次级文本的颜色
    static QString secondTextColor() {return "#6B7280";}

    // 输入控件的背景色, 下拉框, 输入框
    static QString inputComponentBg() {return "#1F2937";}

    // 设置QGroupBox容器的样式
    static void setQGroupBoxStyleSheet(QGroupBox* target){
        QString style = QString(R"(
            QGroupBox {
                background-color: %1;
                border: 1px solid #2B3442;
                border-radius: 10px;
                margin-top: 18px;
                padding: 12px;
            }

            QGroupBox::title {
                subcontrol-origin: margin;
                subcontrol-position: top left;

                padding: 0 8px;
                margin-left: 12px;

                color: #E5E7EB;
                font-size: 13px;
                font-weight: 600;
            }

            /* 可选：hover 高亮（很现代） */
            QGroupBox:hover {
                border: 1px solid #3B82F6;
            }
        )").arg(panelBg());

        target->setStyleSheet(style);
    }

    // 设置QFrame容器的样式
    static void setQFrameStyleSheet(QFrame* target){
        QString style = QString(R"(
            QFrame{
                background-color: %1;
                color: #c0c7d4;
                border-radius: 10px;
            }
        )").arg(panelBg());

        target->setStyleSheet(style);
    }

    // 设置QWidget容器的样式
    static void setQWidgetStyleSheet(QWidget* target){
        QString style = QString(R"(
            QWidget{
                background-color: %1;
                color: %2;
                border-radius: 10px;
            }
        )").arg(panelBg(), textColor());

        target->setStyleSheet(style);
    }

    // 设置下拉框样式
    static void setComboBoxStyleSheet(QComboBox* box, int minWidth = 100){
        if(box == nullptr)
            return;

        // 设置样式
        box->setStyleSheet(QString(R"(
        QComboBox {
            background-color: %3;
            color: %1;
            border: 1px solid #374151;
            border-radius: 6px;
            padding: 4px 10px 4px 10px; /* 右侧留空间给箭头 */
            min-height: 16px;
            min-width: %2px;
        }

        QComboBox:hover {
            border: 1px solid #4B5563;
            background-color: #243041;
        }

        QComboBox:disabled {
            background-color: #111827;
            color: #6B7280;
            border: 1px solid #1F2937;
        }

        /* =========================
           下拉箭头区域
        ========================= */
        QComboBox::drop-down {
            /*subcontrol-origin: padding;
            subcontrol-position: top right;
            width: 22px;
            border-left: 1px solid #374151;*/
            width: 0px;
            border: none;
        }

        QComboBox::down-arrow {
            image: none; /* 你可以换成 svg */
            width: 0;
            height: 0;
        }

        /* =========================
           下拉弹出列表（核心）
        ========================= */
        QComboBox QAbstractItemView {
            background-color: #111827;
            color: #E5E7EB;

            border: 1px solid #374151;
            border-radius: 8px;

            padding: 4px;
            outline: 0;
            selection-background-color: #2563EB;
            selection-color: white;
        }

        /* 每一项 */
        QComboBox QAbstractItemView::item {
            height: 14px;
            padding: 5px;
        }

        /* hover 效果 */
        QComboBox QAbstractItemView::item:hover {
            background-color: #1F2937;
            color: #F9FAFB;
        }

        /* 选中状态 */
        QComboBox QAbstractItemView::item:selected {
            background-color: #2563EB;
            color: white;
        }

        /* disabled item */
        QComboBox QAbstractItemView::item:disabled {
            color: #6B7280;
        }

        /* ===== 滚动条整体 ===== */
        QComboBox QAbstractItemView QScrollBar:vertical {
            background-color: #111827;
            width: 10px;
            margin: 2px;
            border: none;
        }

        /* ===== 滑块（handle）===== */
        QComboBox QAbstractItemView QScrollBar::handle:vertical {
            background: rgba(156, 163, 175, 0.35);  /* 半透明灰 */
            border-radius: 5px;
            min-height: 30px;
        }

        /* hover效果 */
        QComboBox QAbstractItemView QScrollBar::handle:vertical:hover {
            background: rgba(156, 163, 175, 0.6);
        }

        /* pressed效果 */
        QComboBox QAbstractItemView QScrollBar::handle:vertical:pressed {
            background: rgba(59, 130, 246, 0.7);
        }

        /* ===== 上下箭头 ===== */
        QComboBox QAbstractItemView QScrollBar::add-line:vertical,
        QComboBox QAbstractItemView QScrollBar::sub-line:vertical {
            height: 0px;
        }

        /* ===== 背景轨道 ===== */
        QComboBox QAbstractItemView QScrollBar::add-page:vertical,
        QComboBox QAbstractItemView QScrollBar::sub-page:vertical {
            background-color: #111827;
            border: none;
        }

        )").arg(textColor(), QString::number(minWidth), inputComponentBg()));


    }

    // 设置按钮样式
    static void setButtonStyleSheet(QPushButton* btn, ButtonLevel level = ButtonLevel::normal, QString customStyle = ""){
        QString style;
        switch(level){
        case ButtonLevel::normal:{
            style = QString(R"(
QPushButton {
    background-color: transparent;
    color: #E5E7EB;
    border: 1px solid #374151;
    border-radius: 8px;
    padding: 6px 12px;
    %1
}

QPushButton:hover {
    background-color: rgba(255, 255, 255, 0.04);
    border: 1px solid #4B5563;
}

QPushButton:pressed {
    background-color: rgba(255, 255, 255, 0.08);
    border: 1px solid #6B7280;
}

QPushButton:disabled {
    color: #6B7280;
    border: 1px solid #1F2937;
    background-color: transparent;
}
            )");
            break;
        }
        case ButtonLevel::primary:{
            style = QString(R"(
QPushButton {
    background-color: transparent;
    color: #60A5FA;
    border: 1px solid #3B82F6;
    border-radius: 8px;
    padding: 6px 12px;
    font-weight: 500;
    %1
}

QPushButton:hover {
    background-color: rgba(59, 130, 246, 0.08);
    border: 1px solid #60A5FA;
    color: #93C5FD;
}

QPushButton:pressed {
    background-color: rgba(59, 130, 246, 0.15);
    border: 1px solid #2563EB;
    color: #BFDBFE;
}

QPushButton:disabled {
    color: #4B5563;
    border: 1px solid #1F2937;
    background-color: transparent;
}
            )");
            break;
        }
        case ButtonLevel::warning:{
            style = QString(R"(
QPushButton {
    background-color: transparent;
    color: #FBBF24;
    border: 1px solid #F59E0B;
    border-radius: 8px;
    padding: 6px 12px;
    %1
}

QPushButton:hover {
    background-color: rgba(245, 158, 11, 0.10);
    border: 1px solid #FBBF24;
    color: #FCD34D;
}

QPushButton:pressed {
    background-color: rgba(245, 158, 11, 0.18);
    border: 1px solid #D97706;
    color: #FDE68A;
}

QPushButton:disabled {
    color: #6B7280;
    border: 1px solid #1F2937;
    background-color: transparent;
}
            )");
            break;
        }
        case ButtonLevel::critical:{
            style = QString(R"(
QPushButton {
    background-color: transparent;
    color: #F87171;
    border: 1px solid #EF4444;
    border-radius: 8px;
    padding: 6px 12px;
    font-weight: 500;
    %1
}

QPushButton:hover {
    background-color: rgba(239, 68, 68, 0.10);
    border: 1px solid #F87171;
    color: #FCA5A5;
}

QPushButton:pressed {
    background-color: rgba(239, 68, 68, 0.18);
    border: 1px solid #B91C1C;
    color: #FECACA;
}

QPushButton:disabled {
    color: #6B7280;
    border: 1px solid #1F2937;
    background-color: transparent;
}
            )");
            break;
        }
        case ButtonLevel::success:{
            style = QString(R"(
QPushButton {
    background-color: transparent;
    color: #34D399;                /* 主文字绿 */
    border: 1px solid #10B981;    /* 边框绿 */
    border-radius: 8px;
    padding: 6px 12px;
    font-weight: 500;
    %1
}

QPushButton:hover {
    background-color: rgba(16, 185, 129, 0.10);
    border: 1px solid #34D399;
    color: #6EE7B7;
}

QPushButton:pressed {
    background-color: rgba(16, 185, 129, 0.18);
    border: 1px solid #059669;
    color: #A7F3D0;
}

QPushButton:disabled {
    color: #6B7280;
    border: 1px solid #1F2937;
    background-color: transparent;
}
            )");

            break;
        }
        default:
            break;
        }

        btn->setStyleSheet(customStyle.isEmpty()
                ? style.arg("")
                : style.arg(customStyle));
    }

    // 设置输入框的样式
    static void setLineEditStyleSheet(QLineEdit* line){
        QString style = QString(R"(
        QLineEdit {
            background-color: %2;
            color: %1;
            border: 1px solid #374151;
            border-radius: 8px;
            padding: 6px 10px;
            selection-background-color: #3B82F6;
            selection-color: #FFFFFF;
        }

        QLineEdit:hover {
            border: 1px solid #4B5563;
            background-color: #243041;
        }

        QLineEdit:focus {
            border: 1px solid #7B8593;
            background-color: #243041;
        }

        QLineEdit:disabled {
            background-color: #0B1220;
            color: #6B7280;
            border: 1px solid #1F2937;
        }
        )").arg(textColor(), inputComponentBg());
        line->setStyleSheet(style);
    }

    // 创建 设备名称 + 按键名称 的组
    static QWidget* createStyledDeviceNameBtnNameGroup(QLabel* devName, QLabel* btnName){
        QWidget *chip = new QWidget;
        chip->setObjectName("chip");
        chip->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

        chip->setStyleSheet(R"(
            QWidget#chip {
                background-color: rgba(148, 163, 184, 0.08);
                border: 1px solid rgba(148, 163, 184, 0.18);
                border-radius: 8px;
                padding:0;
            })");

        QHBoxLayout *layout = new QHBoxLayout(chip);
        layout->setContentsMargins(0, 2, 0, 2);
        layout->setSpacing(0);
        layout->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        devName->setStyleSheet(R"(
            QLabel {
                color: #CBD5E1;
                font-family: Consolas, "SF Mono", monospace;
                font-size: 11px;
                background: transparent;
                border: none;
                padding: 0;
                margin:0;
            }
                )");
        devName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        devName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        btnName->setStyleSheet(R"(
            QLabel {
                color: #93C5FD;
                font-family: Consolas, "SF Mono", monospace;
                font-size: 11px;
                background: transparent;
                border: none;
                padding: 0;
                margin:0;
            }
                )");
        btnName->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        btnName->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);

        QLabel* conn = new QLabel(":");
        conn->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        conn->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
        conn->setStyleSheet(R"(
            QLabel {
                color: #CBD5E1;
                font-family: Consolas, "SF Mono", monospace;
                font-size: 11px;
                background: transparent;
                border: none;
                padding: 0;
                margin:0;
            })");

        layout->addWidget(devName);
        layout->addWidget(conn);
        layout->addWidget(btnName);


        return chip;
    }

    // 设置单选框的样式
    static void setCheckBoxStyleSheet(QCheckBox* target){
        QString style = QString(R"(
            QCheckBox {
                background-color: transparent;
                color: %1; /* label 文字 */
                spacing: 8px;
            }

            /* checkbox 外框 */
            QCheckBox::indicator {
                width: 16px;
                height: 16px;
                border-radius: 4px;
                border: 1px solid #94A3B8;
                background-color: #111827;
            }

            /* hover 状态 */
            QCheckBox::indicator:hover {
                border: 1px solid #CBD5E1;
            }

            /* checked 状态 */
            QCheckBox::indicator:checked {
                background-color: #3B82F6;
                border: 1px solid #3B82F6;
                image: url(:/icons/icon_checkbox_checked.svg);
            }

            /* disabled */
            QCheckBox::indicator:disabled {
                border: 1px solid #475569;
                background-color: #0B1220;
            }

            /* checked + disabled */
            QCheckBox::indicator:checked:disabled {
                background-color: #475569;
                border: 1px solid #475569;
            }
        )").arg(textColor());
        target->setStyleSheet(style);
    }

    // 设置 滚动条 的样式, 传入的 QWidget需要是有 QSCrollBar的组件
    static void setScrollBarStyleSheet(QWidget* target){
        QString style = QString(R"(
        QScrollArea{
            border:none;
            out-line:none;
        }
        /* =========================
           滚动条整体
        ========================= */
        QScrollBar:vertical, QScrollBar:horizontal {
            background: transparent;
            width: 10px;
            height: 10px;
            margin: 2px;
        }

        /* =========================
           轨道（关键！）
        ========================= */
        QScrollBar::groove:vertical,
        QScrollBar::groove:horizontal {
            background: transparent;
            border: none;
            border-radius: 5px;
        }

        /* =========================
           滑块（核心视觉）
        ========================= */
        QScrollBar::handle:vertical,
        QScrollBar::handle:horizontal {
            background: rgba(156, 163, 175, 0.28);  /* 低存在感灰 */
            border-radius: 5px;
            min-height: 30px;
            min-width: 30px;
        }

        /* hover：轻微增强 */
        QScrollBar::handle:vertical:hover,
        QScrollBar::handle:horizontal:hover {
            background: rgba(156, 163, 175, 0.45);
        }

        /* pressed：强调主色 */
        QScrollBar::handle:vertical:pressed,
        QScrollBar::handle:horizontal:pressed {
            background: rgba(59, 130, 246, 0.75);   /* 蓝色反馈 */
        }

        /* =========================
           去掉上下/左右按钮
        ========================= */
        QScrollBar::add-line:vertical,
        QScrollBar::sub-line:vertical,
        QScrollBar::add-line:horizontal,
        QScrollBar::sub-line:horizontal {
            height: 0px;
            width: 0px;
        }

        /* =========================
           页面空白区域
        ========================= */
        QScrollBar::add-page:vertical,
        QScrollBar::sub-page:vertical,
        QScrollBar::add-page:horizontal,
        QScrollBar::sub-page:horizontal {
            background: transparent;
        }

        /* =========================
           可选：hover整条增强体验
        ========================= */
        QScrollBar:hover {
            background: rgba(255, 255, 255, 0.02);
        }
        )");

        target->setStyleSheet(style);
    }

    // 设置单选框的样式
    static void setRadioButtonStyleSheet(QRadioButton* target){
        QString style = QString(R"(
            QRadioButton {
                color: %1;
                font-size: 13px;
                spacing: 8px;
            }

            QRadioButton::indicator {
                width: 16px;
                height: 16px;
                border-radius: 8px;
                border: 2px solid #4B5563;
                background: transparent;
            }

            QRadioButton::indicator:hover {
                border: 2px solid #6B7280;
            }

            QRadioButton::indicator:checked {
                border: 2px solid #3B82F6;
                background: qradialgradient(
                    cx: 0.5, cy: 0.5, radius: 0.5,
                    fx: 0.5, fy: 0.5,
                    stop: 0.3 #60A5FA,
                    stop: 1 transparent
                );
            }
        )").arg(textColor());
        target->setStyleSheet(style);
    }

    static void setTrayMemuStyleSheet(QMenu* menu){
        static QString trayMenuStyle = R"(
            QMenu
            {
                background-color: #202020;
                color: #E5E7EB;
                border: 1px solid #333333;
                padding: 6px;
                border-radius: 8px;
                font-size: 12px;
                font-family: "Segoe UI";
            }
            QMenu::item
            {
                height: 24px;
                padding-left: 8px;
                padding-right: 12px;
                border-radius: 6px;
                margin: 2px 4px;
            }
            QMenu::item:selected
            {
                background-color: #2D5BFF;
                color: white;
            }
            QMenu::item:disabled
            {
                color: #6B7280;
            }
            QMenu::separator
            {
                height: 1px;
                background-color: #3A3A3A;
                margin-left: 10px;
                margin-right: 10px;
                margin-top: 5px;
                margin-bottom: 5px;
            }
            QMenu::icon
            {
                padding-left: 6px;
            }
            )";

        menu->setStyleSheet(trayMenuStyle);
    }
};
