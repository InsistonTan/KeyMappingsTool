#pragma once

#include "common/Theme.h"
#include "qcontainerfwd.h"
#include <QPaintEvent>
#include <QWidget>
#include <QMouseEvent>
#include <qjsonobject.h>


//
// 曲线编辑器
//
class CurveEditor : public QWidget
{
    Q_OBJECT
public:
    inline const static QString xAxisValueKey = "xAxisValue";
    inline const static QString yAxisValueKey = "yAxisValue";
    inline const static QString mainKey = "main";
    inline const static QString inKey = "in";
    inline const static QString outKey = "out";

    // 逻辑点(x轴,y轴的逻辑值, 并非像素点绝对坐标值)
    struct LogicalPoint{
        double xAxisValue;
        double yAxisValue;

        QJsonObject toJson(){
            QJsonObject obj;
            obj[xAxisValueKey] = xAxisValue;
            obj[yAxisValueKey] = yAxisValue;
            return obj;
        }

        LogicalPoint static fromJson(QJsonObject obj){
            LogicalPoint p;
            if(obj.contains(xAxisValueKey) && obj[xAxisValueKey].isDouble())
                p.xAxisValue = obj[xAxisValueKey].toDouble();
            if(obj.contains(yAxisValueKey) && obj[yAxisValueKey].isDouble())
                p.yAxisValue = obj[yAxisValueKey].toDouble();

            return p;
        }
    };
    // 贝塞尔逻辑点
    struct BezierLogicalPoint{
        LogicalPoint main;// 主点
        LogicalPoint in;// 控制点1, 进入
        LogicalPoint out;// 控制点2, 出去
        bool showIN = true;// 显示 控制点1, 进入
        bool showOut = true;//显示 控制点2, 出去

        QJsonObject toJson(){
            QJsonObject obj;
            obj[mainKey] = main.toJson();
            obj[inKey] = in.toJson();
            obj[outKey] = out.toJson();
            return obj;
        }
        BezierLogicalPoint static fromJson(QJsonObject obj){
            BezierLogicalPoint p;
            if(obj.contains(mainKey) && obj[mainKey].isObject()){
                p.main = LogicalPoint::fromJson(obj[mainKey].toObject());
            }
            if(obj.contains(inKey) && obj[inKey].isObject()){
                p.in = LogicalPoint::fromJson(obj[inKey].toObject());
            }
            if(obj.contains(outKey) && obj[outKey].isObject()){
                p.out = LogicalPoint::fromJson(obj[outKey].toObject());
            }
            return p;
        }
    };

    // 控制点与主点之间的距离
    inline const static int dis = 5;

    explicit CurveEditor(QString xTitle, QString yTitle, QWidget *parent = nullptr);

    // 获取已添加的所有点
    QVector<BezierLogicalPoint> getPoints();

    // 设置点
    void setPoints(QVector<BezierLogicalPoint> points);

    // 根据系数 t(0-1), 获取由 曲线段的 起始点的主点,终点的主点,起始点的out,终点的in 加权平均后的点
    static LogicalPoint bezierPoint(const LogicalPoint &beginMain, const LogicalPoint &endMain, const LogicalPoint &beginOut, const LogicalPoint &endIn, double t);

    // 根据 x轴的逻辑值, 获取曲线上 y轴的逻辑值
    static double getYAxisLogicalValue(QVector<BezierLogicalPoint> points, double logicalX);

protected:
    // 绘制事件
    void paintEvent(QPaintEvent *event) override;

    // 鼠标双击事件
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    // 鼠标按下事件
    void mousePressEvent(QMouseEvent* event) override;
    // 鼠标移动事件
    void mouseMoveEvent(QMouseEvent *event) override;
    // 鼠标释放事件
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    // 贝塞尔逻辑点列表
    QVector<BezierLogicalPoint> points = {};

    // 逻辑点的最大值
    inline static const double xAxisValueMax = 100.0;
    inline static const double yAxisValueMax = 100.0;

    // 曲线区域的margin
    inline static const double paddingLeft = 60;
    inline static const double paddingRight = 40;
    inline static const double paddingTop = 40;
    inline static const double paddingBottom = 60;

    // 点的半径
    inline static const double pointRadius = 4;

    // 颜色
    // inline static const QColor axisColor = QColor("#9CA3AF");// xy坐标轴
    // inline static const QColor backgroundColor = QColor("#ffffff");// 背景
    // inline static const QColor pathColor = QColor("#3B82F6");// 曲线
    // inline static const QColor mainPointColor = QColor("#EF4444");// 主点
    // inline static const QColor ctrlPointColor = QColor("#000000");// 控制点(in, out)
    // inline static const QColor previewColor = QColor("#000000");// 预览信息

    inline static const QColor axisColor      = QColor("#A0A0A0");   // 坐标轴
    inline static const QColor backgroundColor = QColor(Theme::widgetBg());  // 背景
    inline static const QColor pathColor      = QColor("#3B82F6");   // 曲线
    inline static const QColor mainPointColor = QColor("#EF4444");   // 主点
    inline static const QColor ctrlPointColor = QColor("#D0D0D0");   // 控制点
    inline static const QColor previewColor   = QColor("#FFFFFF");   // 预览信息

    // xy轴的标题
    QString xTitle;
    QString yTitle;

    // 鼠标移动到主点时显示的预览信息
    QString previewText;
    // 预览信息的矩形
    QRect previewTextRec;
    // 预览信息宽度和高度
    int previewTextWidth = 65, previewTextHeight = 15;


    // 当前鼠标press的点的索引
    int currPressPointIndex = -1;
    // 鼠标按下的点的类型
    enum PressTarget{None, Main, In, Out};
    PressTarget currPressTarget = None;


    // 画背景
    void drawBackground(QPainter& painter);
    // 画xy坐标轴
    void drawAxis(QPainter& painter);
    // 画曲线
    void drawCurve(QPainter& painter);
    // 画点
    void drawPoints(QPainter& painter);
    // 画预览信息
    void drawPreviewText(QPainter& painter);


    // 把点按照x坐标排序
    void sortPoints();

    // 将逻辑点的x轴,y轴的值, 转成实际像素点的x,y坐标
    QPointF toPhysicalPoint(LogicalPoint pt);

    // 将实际像素点位置, 转成逻辑点
    LogicalPoint toLogicalPoint(QPointF qpt);

    // 根据逻辑点, 创建贝塞尔逻辑点
    BezierLogicalPoint createBezierLogicalPoint(LogicalPoint p);
    LogicalPoint createIn(LogicalPoint p);
    LogicalPoint createOut(LogicalPoint p);

    // 当前鼠标位置是否在 点p 的可操作范围
    bool isPointHit(const QPointF &p, const QPointF &mousePos) {
        return QVector2D(p - mousePos).length() < pointRadius + 2;
    }

    // 逻辑点是否有效
    bool isLogicalPointValid(LogicalPoint lp);
    // 强制逻辑点有效, 控制值在有效范围内
    void forceLogicalPointValid(LogicalPoint& lp, int pIndex, PressTarget target);
    // 强制贝塞尔逻辑的位置有效
    void forceBezierLogicalPointValid(BezierLogicalPoint& blp, int pIndex);



signals:
    void pointChanged();
};
