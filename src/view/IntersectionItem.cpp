#include "view/IntersectionItem.h"

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <cmath>

// ============================================================
//  IntersectionItem 实现
// ============================================================

IntersectionItem::IntersectionItem(int nodeId, QGraphicsItem* parent)
    : QGraphicsObject(parent), nodeId_(nodeId)
{
    // Z=1 在最上层，确保可被点击拖拽（建筑图元 Z=0，道路 Z=-1）
    setZValue(1);

    // ItemSendsGeometryChanges 用于触发 itemChange(ItemPositionHasChanged)
    setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
    // 默认不可编辑——普通模式下不响应鼠标，管理员编辑模式才打开
    setEditable(false);
}

QRectF IntersectionItem::boundingRect() const
{
    // 16x16 的矩形区域，中心在 (0,0)
    return QRectF(-8, -8, 16, 16);
}

QPainterPath IntersectionItem::shape() const
{
    // 命中区域放大到半径 10（显示仍是 5~7px 的小圆点），
    // 方便用左键点击/右键删除时更容点中。
    QPainterPath p;
    p.addEllipse(QRectF(-10, -10, 20, 20));
    return p;
}

void IntersectionItem::paint(QPainter* painter,
                             const QStyleOptionGraphicsItem* /*option*/,
                             QWidget* /*widget*/)
{
    // 路口节点：默认白色半透明圆点（浅灰蓝描边，保证在浅色底图上可见）；
    // 悬停时变大变亮（蓝色），视觉选中时金黄色（已选作"连接第一点"）。
    QColor color;
    if (selectedVisual_) {
        color = QColor(255, 200, 50);    // 金黄——已选作"连接第一点"
    } else if (hovered_) {
        color = QColor(80, 160, 255);
    } else {
        color = QColor(255, 255, 255, 175);   // 白色半透明
    }
    painter->setBrush(color);
    painter->setPen(QPen(QColor(120, 130, 150), 1));

    qreal r = (hovered_ || selectedVisual_) ? 7.0 : 5.0;
    painter->drawEllipse(boundingRect().center(), r, r);

    // 悬停或选中时画外圈提示
    if (hovered_ || selectedVisual_) {
        painter->setBrush(Qt::NoBrush);
        QColor ring = selectedVisual_ ? QColor(255, 200, 50, 140)
                                      : QColor(80, 160, 255, 120);
        painter->setPen(QPen(ring, 2));
        painter->drawEllipse(boundingRect().center(), r + 3, r + 3);
    }
}

void IntersectionItem::setEditable(bool editable) {
    editable_ = editable;
    // 拖动 + 选中（可拖动性随后由 setMovable 按当前子工具再细化）
    setFlag(QGraphicsItem::ItemIsMovable, editable);
    setFlag(QGraphicsItem::ItemIsSelectable, editable);
    if (editable) {
        setAcceptHoverEvents(true);
        setCursor(Qt::PointingHandCursor);
        // 必须接收右键：否则右键事件会穿透到路口下方的道路图元，
        // 导致"右键节点"实际右键成了道路（只能删路，删不了节点）。
        setAcceptedMouseButtons(Qt::LeftButton | Qt::RightButton);
    } else {
        setAcceptHoverEvents(false);
        setCursor(Qt::ArrowCursor);
        setSelectedVisual(false);   // 退出编辑模式时清掉视觉选中
        setAcceptedMouseButtons(Qt::LeftButton);  // 恢复默认，避免吞掉非编辑态右键
    }
    update();
}

void IntersectionItem::setMovable(bool movable) {
    setFlag(QGraphicsItem::ItemIsMovable, movable);
    // 可拖动时给"抓手"光标，仅可点击时给"手指"光标
    setCursor(movable ? Qt::SizeAllCursor : Qt::PointingHandCursor);
}

void IntersectionItem::setSelectedVisual(bool on) {
    if (selectedVisual_ == on) return;
    selectedVisual_ = on;
    update();
}

QVariant IntersectionItem::itemChange(GraphicsItemChange change,
                                       const QVariant& value)
{
    if (change == ItemPositionHasChanged) {
        // 拖拽过程中实时通知 MapScene 更新道路
        emit positionChanged(nodeId_, value.toPointF());
    }
    return QGraphicsObject::itemChange(change, value);
}

void IntersectionItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = true;
    update();
}

void IntersectionItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/)
{
    hovered_ = false;
    update();
}

void IntersectionItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // 同时记录左/右键按下位置（release 时用于区分点击 vs 拖动）
    if (event->button() == Qt::LeftButton || event->button() == Qt::RightButton) {
        pressScenePos_ = event->scenePos();
    }
    QGraphicsObject::mousePressEvent(event);
}

void IntersectionItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    // 先调用基类处理选中状态/拖动结束
    QGraphicsObject::mouseReleaseEvent(event);

    if (event->button() == Qt::LeftButton) {
        // 判断是"点击"还是"拖动"：鼠标位移 < 3 像素视为点击
        QPointF delta = event->scenePos() - pressScenePos_;
        if (std::hypot(delta.x(), delta.y()) < 3.0) {
            emit clicked(nodeId_);
        }
        // 拖拽结束，通知持久化（无论是否实际移动，都发；持久化层自己决定是否写库）
        emit dragFinished(nodeId_, pos());
    }
    // 右键删除统一由 MapScene::mousePressEvent 接管（更可靠），此处不再处理
}
