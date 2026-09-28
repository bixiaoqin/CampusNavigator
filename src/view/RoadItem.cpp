#include "view/RoadItem.h"

#include <QPainter>
#include <QPen>
#include <QPainterPath>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <cmath>

static constexpr int ROAD_PADDING = 20;

RoadItem::RoadItem(double x1, double y1, double x2, double y2,
                   double distance, int fromId, int toId,
                   QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , x1_(x1), y1_(y1), x2_(x2), y2_(y2)
    , distance_(distance)
    , fromId_(fromId), toId_(toId)
{
    // Z=-1，在建筑图元(Z=0)下层，不会遮挡建筑标签
    setZValue(-1);
    // 默认不接收鼠标事件（不挡路）；编辑模式由 setDeletable(true) 打开
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
}

QRectF RoadItem::boundingRect() const {
    qreal minX = qMin(x1_, x2_), maxX = qMax(x1_, x2_);
    qreal minY = qMin(y1_, y2_), maxY = qMax(y1_, y2_);
    for (const QPointF& wp : waypoints_) {
        minX = qMin(minX, wp.x());
        maxX = qMax(maxX, wp.x());
        minY = qMin(minY, wp.y());
        maxY = qMax(maxY, wp.y());
    }
    return QRectF(minX - ROAD_PADDING, minY - ROAD_PADDING,
                  maxX - minX + ROAD_PADDING * 2,
                  maxY - minY + ROAD_PADDING * 2);
}

void RoadItem::paint(QPainter* painter,
                      const QStyleOptionGraphicsItem* /*option*/,
                      QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 构建路径（起点 → 各拐点 → 终点，折线绘制）
    QPainterPath path;
    path.moveTo(QPointF(x1_, y1_));
    for (const QPointF& wp : waypoints_) {
        path.lineTo(wp);
    }
    path.lineTo(QPointF(x2_, y2_));

    if (highlighted_) {
        // ============================================================
        //  高亮导航路线：红色半透明，高对比度
        //  外层光晕(红色粗) + 中层(亮红) + 内层高光(白色细)
        // ============================================================
        QPen glowPen(QColor(244, 67, 54, 60), 8,
                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(glowPen);
        painter->drawPath(path);

        QPen mainPen(QColor(229, 57, 53, 190), 4,
                     Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(mainPen);
        painter->drawPath(path);

        QPen highlightPen(QColor(255, 255, 255, 160), 1,
                          Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin);
        painter->setPen(highlightPen);
        painter->drawPath(path);

    } else if (faded_) {
        // ============================================================
        //  历史路径：极淡灰色，几乎不可见，不抢视觉
        // ============================================================
        painter->setPen(QPen(QColor(200, 200, 200, 25), 1,
                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);

    } else {
        // ============================================================
        //  普通道路：浅灰半透明细线
        // ============================================================
        painter->setPen(QPen(QColor(200, 200, 200, 70), 2,
                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);

        painter->setPen(QPen(QColor(240, 240, 240, 90), 0.8,
                             Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(path);
    }
}

// 更新端点坐标（拖拽路口时实时刷新道路）
void RoadItem::updateEndpoints(double x1, double y1, double x2, double y2) {
    prepareGeometryChange();
    x1_ = x1; y1_ = y1;
    x2_ = x2; y2_ = y2;
    update();
}

// 添加一个折线拐点（场景坐标）
void RoadItem::addWaypoint(const QPointF& wp) {
    prepareGeometryChange();
    waypoints_.append(wp);
    update();
}

void RoadItem::setDeletable(bool on) {
    if (deletable_ == on) return;
    deletable_ = on;
    if (on) {
        // 接受右键 + 悬停（视觉反馈）
        setAcceptedMouseButtons(Qt::RightButton);
        setAcceptHoverEvents(true);
    } else {
        setAcceptedMouseButtons(Qt::NoButton);
        setAcceptHoverEvents(false);
        hovered_ = false;
    }
    update();
}

void RoadItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = true;
    update();
}

void RoadItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = false;
    update();
}

void RoadItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::RightButton && deletable_) {
        emit rightClicked(fromId_, toId_);
    }
    QGraphicsItem::mousePressEvent(event);
}

void RoadItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* /*event*/) {
    // 右键事件在 press 时已经发出；这里只做基类收尾
    QGraphicsItem::mouseReleaseEvent(nullptr);
}
