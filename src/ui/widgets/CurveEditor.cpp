#include "CurveEditor.h"
#include "qpoint.h"

#include <QPainter>
#include <QPainterPath>


CurveEditor::CurveEditor(QString xTitle, QString yTitle, QWidget *parent)
    : QWidget{parent}
{
    setFixedSize(QSize(600, 400));

    this->xTitle = xTitle;
    this->yTitle = yTitle;

    // 开启鼠标跟踪，实现悬停感应
    setMouseTracking(true);
}

QVector<CurveEditor::BezierLogicalPoint> CurveEditor::getPoints()
{
    return points;
}

void CurveEditor::setPoints(QVector<BezierLogicalPoint> points)
{
    this->points = points;
    sortPoints();

    // 重绘
    update();
}

void CurveEditor::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);

    drawBackground(painter);
    drawAxis(painter);
    drawCurve(painter);
    drawPoints(painter);
    drawPreviewText(painter);
}

void CurveEditor::sortPoints()
{
    std::sort(points.begin(), points.end(),[](auto&a,auto&b){
        return a.main.xAxisValue < b.main.xAxisValue;
    });

    // 显示所有点的 in out
    for(int i = 0; i < points.size(); i++){
        points[i].showIN = true;
        points[i].showOut = true;
        // 强制点的位置在有效范围
        forceBezierLogicalPointValid(points[i], i);
    }

    // 如果只有一个点, 隐藏 in out
    if(points.size() == 1){
        points[0].showIN = false;
        points[0].showOut = false;
    }

    // 如果有两个及以上的点, 隐藏第一个点的 in 和 最后一个点的 out
    if(points.size() >= 2){
        points[0].showIN = false;
        points[points.size() - 1].showOut = false;
    }
}

QPointF CurveEditor::toPhysicalPoint(LogicalPoint pt)
{
    // 当前组件的宽高
    double w = width();
    double h = height();

    // 边距
    double left = paddingLeft;
    double right = paddingRight;
    double top = paddingTop;
    double bottom = paddingBottom;

    // 有效的区域大小
    double plotW = w - left - right;
    double plotH = h - top - bottom;

    // 将逻辑点的xy值转成在当前组件的像素点坐标
    // 100.0为逻辑x轴最大值(代表100%)
    double x = left + (pt.xAxisValue / xAxisValueMax) * plotW;
    // 由于 y轴像素坐标是向下的, 所以当前组件的 y轴顶点 是 画布y轴的起点
    // 所以 y轴像素坐标值是反的, 需要(1.0 - pt.yAxisValue / yAxisValueMax)
    // 100.0为逻辑y轴最大值(代表100%)
    double y = top + (1.0 - pt.yAxisValue / yAxisValueMax) * plotH;

    return QPointF(x,y);
}

CurveEditor::LogicalPoint CurveEditor::toLogicalPoint(QPointF qpt)
{
    // 当前组件的宽高
    double w = width();
    double h = height();
    // 边距
    double left = paddingLeft;
    double right = paddingRight;
    double top = paddingTop;
    double bottom = paddingBottom;
    // 有效的区域大小
    double plotW = w - left - right;
    double plotH = h - top - bottom;

    // 逻辑点的xy轴的值
    double xAxisValue = (qpt.x() - left) / plotW * xAxisValueMax;
    double yAxisValue = - (((qpt.y() - top) / plotH) - 1.0) * yAxisValueMax ;

    return LogicalPoint{xAxisValue, yAxisValue};
}

CurveEditor::BezierLogicalPoint CurveEditor::createBezierLogicalPoint(LogicalPoint p)
{
    BezierLogicalPoint blp;
    blp.main = p;
    blp.in = createIn(p);
    blp.out = createOut(p);

    return blp;
}

CurveEditor::LogicalPoint CurveEditor::createIn(LogicalPoint p)
{
    LogicalPoint inLogical = p;
    inLogical.xAxisValue -= dis;
    return inLogical;
}

CurveEditor::LogicalPoint CurveEditor::createOut(LogicalPoint p)
{
    LogicalPoint outLogical = p;
    outLogical.xAxisValue += dis;
    return outLogical;
}

bool CurveEditor::isLogicalPointValid(LogicalPoint lp)
{
    return !(lp.xAxisValue < 0 || lp.xAxisValue > xAxisValueMax || lp.yAxisValue < 0 || lp.yAxisValue > yAxisValueMax);
}

void CurveEditor::forceLogicalPointValid(LogicalPoint& lp, int pIndex, PressTarget target)
{
    if(lp.xAxisValue < 0)
        lp.xAxisValue = 0;
    if(lp.xAxisValue > xAxisValueMax)
        lp.xAxisValue = xAxisValueMax;

    if(lp.yAxisValue < 0)
        lp.yAxisValue = 0;
    if(lp.yAxisValue > yAxisValueMax)
        lp.yAxisValue = yAxisValueMax;

    bool hasPrePoint = (pIndex - 1) >= 0;
    bool hasNextPoint = (pIndex + 1) < points.size();

    if(target == PressTarget::Main){
        // 主点的x轴值在 前一个out点和后一个in点之间
        if(hasPrePoint){
            lp.xAxisValue = std::max(lp.xAxisValue, points[pIndex - 1].out.xAxisValue + 0.000001);
        }
        if(hasNextPoint){
            lp.xAxisValue = std::min(lp.xAxisValue, points[pIndex + 1].in.xAxisValue - 0.000001);
        }
    }else if(target == PressTarget::In){
        // in点, x轴值不能超过主点
        lp.xAxisValue = std::min(lp.xAxisValue, points[pIndex].main.xAxisValue);
        // 且不能小于前一个点
        if(hasPrePoint){
            lp.xAxisValue = std::max(lp.xAxisValue, points[pIndex - 1].main.xAxisValue);
        }
    }else if(target == PressTarget::Out){
        // out点, x轴值不能小于主点
        lp.xAxisValue = std::max(lp.xAxisValue, points[pIndex].main.xAxisValue);
        // 且不能大于后一个点
        if(hasNextPoint){
            lp.xAxisValue = std::min(lp.xAxisValue, points[pIndex + 1].main.xAxisValue);
        }
    }
}

void CurveEditor::forceBezierLogicalPointValid(BezierLogicalPoint &blp,  int pIndex)
{
    // 强制点的位置在有效区域
    forceLogicalPointValid(blp.main, pIndex, PressTarget::Main);
    forceLogicalPointValid(blp.in, pIndex, PressTarget::In);
    forceLogicalPointValid(blp.out, pIndex, PressTarget::Out);
}

void CurveEditor::drawBackground(QPainter &painter)
{
    // 白色背景
    painter.fillRect(rect(), backgroundColor);
    painter.setRenderHint(QPainter::Antialiasing, true);
}

void CurveEditor::drawAxis(QPainter &painter)
{
    // 内边距
    double left = paddingLeft;
    double right = paddingRight;
    double top = paddingTop;
    double bottom = paddingBottom;

    int w = width();
    int h = height();

    QRect plotRect(left, top, w - left - right, h - top - bottom);

    // 画 x,y 轴
    // 灰色
    QPen axisPen(axisColor);
    axisPen.setWidth(1);
    painter.setPen(axisPen);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // x轴
    painter.drawLine(
        plotRect.bottomLeft(),
        plotRect.bottomRight()
        );

    // 刻度线之间的间隔(步长)
    int step = 20;
    // 刻度线的线长
    double lineLen = 2;
    // 刻度数字矩形的宽和高
    int numberWidth = 25, numberHeight = 10;
    int titleWidth = this->width(), titleHeight = 15;

    // xy轴的标题的矩形(矩形左边位置, 顶部位置, 矩形的宽度, 矩形的高度)
    QRect xTitleRect(0, this->height() - ((paddingBottom + titleHeight)/2), titleWidth - (paddingRight/2), titleHeight);
    QRect yTitleRect((paddingLeft/2), ((paddingTop - titleHeight)/2), titleWidth, titleHeight);
    // 画出标题
    painter.drawText(xTitleRect, Qt::AlignRight, xTitle);
    painter.drawText(yTitleRect, Qt::AlignLeft, yTitle);

    // x轴刻度, 步长20
    for(double x = 0; x < xAxisValueMax + 1; x += step){
        auto startP = toPhysicalPoint({x, 0});//刻度线逻辑起始点: x轴值为 x, y轴为0
        auto endP = toPhysicalPoint({x, lineLen});//刻度线逻辑终点: x轴值为 x, y轴为线长
        // 不画原点的刻度线
        if(x > 1){
            painter.drawLine(startP, endP);
        }

        // 刻度数字, 整数显示
        QString label = QString::number(x, 'f', 0) + "%";
        QRect textRect(startP.x() - (numberWidth/2), startP.y(), numberWidth, numberHeight);
        painter.drawText(textRect, Qt::AlignCenter, label);
    }

    // y轴
    painter.drawLine(
        plotRect.bottomLeft(),
        plotRect.topLeft()
        );
    // y轴刻度, 步长20
    for(double y = 0; y < yAxisValueMax + 1; y += step){
        auto startP = toPhysicalPoint({0, y});//刻度线逻辑起始点: y轴值为 y, x轴为0
        auto endP = toPhysicalPoint({lineLen, y});//刻度线逻辑终点: y轴值为 y, x轴为线长
        // 不画原点的刻度线和数值
        if(y > 1){
            painter.drawLine(startP, endP);
            // 刻度数字, 整数显示
            QString label = QString::number(y, 'f', 0) + "%";
            QRect textRect(startP.x() - numberWidth, startP.y() - (numberHeight/2), numberWidth, numberHeight);
            painter.drawText(textRect, Qt::AlignCenter, label);
        }
    }
}

void CurveEditor::drawCurve(QPainter &painter)
{
    // 没成线
    if(points.size() < 2)
        return;

    // 画线
    QPainterPath path;

    // 第一个点
    QPointF first = toPhysicalPoint(points[0].main);

    path.moveTo(first);

    // 从第二个点开始, 遍历
    for(int i = 1; i < points.size(); i++){
        // 前一个点
        const auto &prev = points[i-1];
        // 当前点
        const auto &curr = points[i];

        // 使用前一点的 out 和当前点的 in 作为贝塞尔控制点
        path.cubicTo(toPhysicalPoint(prev.out), toPhysicalPoint(curr.in), toPhysicalPoint(curr.main));
    }

    // 画笔设置
    QPen pen(pathColor);
    pen.setWidth(2);
    painter.setPen(pen);
    // 画线
    painter.drawPath(path);
}

void CurveEditor::drawPoints(QPainter &painter)
{
    if(points.size() < 1)
        return;


    // 不画边框
    QPen pointPen(Qt::NoPen);
    // 红色点
    QBrush pointBrush(mainPointColor);
    painter.setBrush(pointBrush);
    painter.setPen(pointPen);

    // 画主点
    for (const auto &p : points)
    {
        QPointF pos = toPhysicalPoint(p.main);
        painter.drawEllipse(pos, pointRadius, pointRadius);
    }


    QPen pointPen2(Qt::NoPen);
    // 红色点
    QBrush pointBrush2(ctrlPointColor);
    painter.setBrush(pointBrush2);
    painter.setPen(pointPen2);
    // 画控制点
    for (const auto &p : points)
    {
        if(p.showIN){
            QPointF in = toPhysicalPoint(p.in);
            painter.drawEllipse(in, pointRadius, pointRadius);
        }

        if(p.showOut){
            QPointF out = toPhysicalPoint(p.out);
            painter.drawEllipse(out, pointRadius, pointRadius);
        }
    }
}

void CurveEditor::drawPreviewText(QPainter &painter)
{
    if(previewText.isEmpty())
        return;

    QPen axisPen(previewColor);
    axisPen.setWidth(1);
    painter.setPen(axisPen);
    QFont font = painter.font();
    font.setPointSize(8);
    painter.setFont(font);

    // 画出预览信息
    painter.drawText(previewTextRec, Qt::AlignCenter, previewText);
}

void CurveEditor::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {

        // 鼠标位置（像素坐标）
        QPointF pos = event->pos();

        // 逻辑点
        auto logiPoint = toLogicalPoint(pos);

        // 逻辑点边界检查
        if(isLogicalPointValid(logiPoint)){
            // 生成 贝塞尔逻辑点 添加到列表
            points.append(createBezierLogicalPoint(logiPoint));
            sortPoints();

            // 重绘界面
            update();

            emit pointChanged();
        }
    }
}

void CurveEditor::mousePressEvent(QMouseEvent *event)
{
    // 鼠标左键按下
    if (event->button() == Qt::LeftButton){
        // 碰撞检测：遍历所有点，判断鼠标位置是否命中
        for (int i = 0; i < points.size(); i++) {
            auto& cur = points[i];
            if (isPointHit(event->pos(), toPhysicalPoint(cur.main))) {
                currPressPointIndex = i;
                currPressTarget = Main;
                return;
            }
            else if (isPointHit(event->pos(), toPhysicalPoint(cur.in))) {
                currPressPointIndex = i;
                currPressTarget = In;
                return;
            } else if (isPointHit(event->pos(), toPhysicalPoint(cur.out))) {
                currPressPointIndex = i;
                currPressTarget = Out;
                return;
            }
        }
    }
}

void CurveEditor::mouseMoveEvent(QMouseEvent *event)
{
    bool needUpdate = false;

    // 移动点
    if (currPressPointIndex >= 0 && currPressPointIndex < points.size()) {
        // 新的逻辑值
        auto newLogicalP = toLogicalPoint(event->pos());

        // 主点, 带着in,out跑
        if(currPressTarget == Main){
            double changedX = newLogicalP.xAxisValue - points[currPressPointIndex].main.xAxisValue;
            double changedY = newLogicalP.yAxisValue - points[currPressPointIndex].main.yAxisValue;
            points[currPressPointIndex].main.xAxisValue += changedX;
            points[currPressPointIndex].main.yAxisValue += changedY;

            //points[currPressPointIndex].in.xAxisValue += changedX;
            //points[currPressPointIndex].in.yAxisValue += changedY;

            //points[currPressPointIndex].out.xAxisValue += changedX;
            //points[currPressPointIndex].out.yAxisValue += changedY;

            // 点的位置强制在有效范围
            forceBezierLogicalPointValid(points[currPressPointIndex], currPressPointIndex);
        }
        else if(currPressTarget == In){
            // 点的位置强制在有效范围
            forceLogicalPointValid(newLogicalP, currPressPointIndex, PressTarget::In);
            points[currPressPointIndex].in = newLogicalP;
        }
        else if(currPressTarget == Out){
            // 点的位置强制在有效范围
            forceLogicalPointValid(newLogicalP, currPressPointIndex, PressTarget::Out);
            points[currPressPointIndex].out = newLogicalP;
        }

        needUpdate = true;
    }

    // 鼠标位置是否命中了主点
    bool isHitAnyMainPoint = false;

    // 鼠标移动到主点, 显示预览信息
    for(const auto& point : points){
        // 命中
        if(isPointHit(event->pos(), toPhysicalPoint(point.main))){
            isHitAnyMainPoint = true;

            QString newPreviewText = QString("(%1%,%2%)").arg(
                QString::number(point.main.xAxisValue, 'f', 0),
                QString::number(point.main.yAxisValue, 'f', 0)
            );

            // 只有发生变化时才更新
            if(newPreviewText != previewText){
                previewText = newPreviewText;
                previewTextRec = QRect(event->pos().x() - previewTextWidth,
                                       event->pos().y() - previewTextHeight,
                                       previewTextWidth,
                                       previewTextHeight);
                // 重绘
                needUpdate = true;
            }

            // 结束
            break;
        }
    }

    // 没有命中任任何一个主点, 并且预览信息不为空, 重置预览信息
    if(isHitAnyMainPoint == false && !previewText.isEmpty()){
        previewText = "";
        // 重绘
        needUpdate = true;
    }

    if(needUpdate)
        update();
}

void CurveEditor::mouseReleaseEvent(QMouseEvent *event)
{
    // 鼠标左键释放
    if(event->button() == Qt::LeftButton){
        currPressPointIndex = -1;
        currPressTarget = None;
        emit pointChanged();
    }
    // 鼠标右键释放, 删除点
    else if(event->button() == Qt::RightButton){
        // 碰撞检测：遍历所有主点，判断鼠标位置是否命中
        // 命中就删除点
        int deleteTargetIndex = -1;
        for (int i = 0; i < points.size(); i++) {
            auto& cur = points[i];
            if (isPointHit(event->pos(), toPhysicalPoint(cur.main))) {
                deleteTargetIndex = i;
            }
        }

        if(deleteTargetIndex > -1){
            points.remove(deleteTargetIndex);
            update();
            emit pointChanged();
        }
    }

}

CurveEditor::LogicalPoint CurveEditor::bezierPoint(const CurveEditor::LogicalPoint &beginMain,
                                                   const CurveEditor::LogicalPoint &endMain,
                                                   const CurveEditor::LogicalPoint &beginOut,
                                                   const CurveEditor::LogicalPoint &endIn,
                                                   double t)
{
    double u = 1 - t;
    double x = u*u*u * beginMain.xAxisValue + 3*u*u*t * endMain.xAxisValue + 3*u*t*t * beginOut.xAxisValue + t*t*t * endIn.xAxisValue;
    double y = u*u*u * beginMain.yAxisValue + 3*u*u*t * endMain.yAxisValue + 3*u*t*t * beginOut.yAxisValue + t*t*t * endIn.yAxisValue;
    return CurveEditor::LogicalPoint{x, y};
}

double CurveEditor::CurveEditor::getYAxisLogicalValue(QVector<BezierLogicalPoint> points, double logicalX)
{
    if (points.isEmpty())
        return 0.0;

    // 如果只有一个点, 判断 logicalX 是否足够靠近这个点, 如果足够靠近, 返回这个点的y值
    if(points.size() == 1 && std::abs(logicalX - points[0].main.xAxisValue) < 0.000001){
        return points[0].main.yAxisValue;
    }

    // 边界处理
    const auto &first = points.first();
    const auto &last  = points.last();
    if (logicalX <= first.main.xAxisValue)
        return first.main.yAxisValue;
    if (logicalX >= last.main.xAxisValue)
        return last.main.yAxisValue;

    // 遍历段
    for (int i = 0; i < points.size() - 1; ++i) {
        const auto &prev = points[i];
        const auto &curr = points[i+1];

        // 检查 targetX 是否在当前段的 x 范围内（假设单调递增）
        if (logicalX < prev.main.xAxisValue || logicalX > curr.main.xAxisValue) continue;

        // 段控制点
        auto p0 = prev.main;
        auto p1 = prev.out;   // 前一点的出控制点
        auto p2 = curr.in;   // 当前点的入控制点
        auto p3 = curr.main;

        // 二分法求解 t，使得 x(t) ≈ logicalX
        double tLow = 0.0, tHigh = 1.0;
        double tolerance = 1e-6;
        int maxIter = 100;

        for (int iter = 0; iter < maxIter; ++iter) {
            double tMid = (tLow + tHigh) * 0.5;
            double xMid = bezierPoint(p0, p1, p2, p3, tMid).xAxisValue;
            if (qAbs(xMid - logicalX) < tolerance) {
                // 找到足够精确的 t
                return bezierPoint(p0, p1, p2, p3, tMid).yAxisValue;
            }
            if (xMid < logicalX)
                tLow = tMid;
            else
                tHigh = tMid;
        }

        // 若迭代结束未收敛，返回中点对应的 y
        double tFinal = (tLow + tHigh) * 0.5;
        return bezierPoint(p0, p1, p2, p3, tFinal).yAxisValue;
    }

    return last.main.yAxisValue;
}







