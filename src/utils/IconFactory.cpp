#include "IconFactory.h"
#include "qscreen.h"
#include <QFile>
#include <QGuiApplication>
#include <QPainter>





QIcon IconFactory::icon(IconEnum iconEnum, int size)
{
    // 枚举转字符串
    QString iconName = iconEnumtoString(iconEnum);

    // 实际的图标的resource路径
    QString path = iconPath(iconName);
    // 读取图标
    //QPixmap pixmap = renderSvg(path, size);

    QIcon icon(path);
    return icon;
}

QIcon IconFactory::colorIcon(IconEnum iconEnum, QColor color, int size)
{
    // 枚举转字符串
    QString iconName = iconEnumtoString(iconEnum);

    // 实际的图标的resource路径
    QString path = iconPath(iconName);
    // 读取图标
    QPixmap pixmap = coloredSvg(path, color, size);

    QIcon icon(pixmap);
    return icon;
}


QString IconFactory::iconPath(const QString& name)
{
    return ":/icons/icon_" + name + ".svg";
}

QPixmap IconFactory::coloredSvg(const QString &file, const QColor &color, int size)
{
    QSvgRenderer renderer(file);

    qreal dpr = QGuiApplication::primaryScreen()->devicePixelRatio();

    QPixmap pixmap(size * dpr, size * dpr);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // 1. 先画 SVG（当做 alpha mask）
    renderer.render(&painter);

    // 2. 关键：只给已有像素“染色”
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(pixmap.rect(), color);

    painter.end();

    pixmap.setDevicePixelRatio(dpr);
    return pixmap;
}

QPixmap IconFactory::renderSvg(const QString& path, int size)
{
    QSvgRenderer renderer(path);
    QPixmap pixmap(size, size);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    renderer.render(&painter);
    return pixmap;
}



