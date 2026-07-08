#pragma once

#include <QIcon>
#include <QPixmap>
#include <QSvgRenderer>
#include <QHash>
#include <QByteArray>

//
// 图标工厂类
//
class IconFactory
{
public:
    // 图标名称枚举
    enum class IconEnum{
        keymapping,
        log,
        settings,
        about,
        support,
        steeringWheel,
        ffb_simulate,
        pedal,
        gamepad,
        keyboard,
        arrow_right,
        arrow_down
    };
    static QString iconEnumtoString(IconEnum iconEnum){
        switch(iconEnum){
        case IconEnum::keymapping:
            return "keymapping";
        case IconEnum::log:
            return "log";
        case IconEnum::settings:
            return "settings";
        case IconEnum::about:
            return "about";
        case IconEnum::support:
            return "support";
        case IconEnum::steeringWheel:
            return "steeringWheel";
        case IconEnum::ffb_simulate:
            return "ffb_simulate";
        case IconEnum::pedal:
            return "pedal";
        case IconEnum::gamepad:
            return "gamepad";
        case IconEnum::keyboard:
            return "keyboard";
        case IconEnum::arrow_right:
            return "arrow_right";
        case IconEnum::arrow_down:
            return "arrow_down";
        default:
            break;
        }
    }

    static QIcon icon(IconEnum iconEnum, int size = 24);
    static QIcon colorIcon(IconEnum iconEnum, QColor color, int size = 24);

private:
    static QPixmap renderSvg(const QString& path, int size);
    static QString iconPath(const QString& name);

    // 将svg图标强制改色
    static QPixmap coloredSvg(const QString &file, const QColor &color, int size);

private:
};
