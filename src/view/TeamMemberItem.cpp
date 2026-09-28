#include "view/TeamMemberItem.h"

#include <QPainter>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <cmath>

static constexpr qreal AVATAR_R = 10;   // 头像半径
static constexpr int LABEL_H = 14;      // 姓名标签高度

TeamMemberItem::TeamMemberItem(int id, const QString& name, const QColor& color,
                               QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , id_(id), name_(name), color_(color)
    , targetX_(0), targetY_(0)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsMovable, true);
    setFlag(ItemSendsGeometryChanges, true);
    setZValue(8);  // 建筑之上，角色之下
}

QRectF TeamMemberItem::boundingRect() const {
    return QRectF(-AVATAR_R - 4, -AVATAR_R - LABEL_H - 6,
                  AVATAR_R * 2 + 8, AVATAR_R * 2 + LABEL_H + 10);
}

void TeamMemberItem::paint(QPainter* painter,
                           const QStyleOptionGraphicsItem* /*option*/,
                           QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    // 阴影
    painter->setBrush(QColor(0, 0, 0, 50));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(1, 1), AVATAR_R, AVATAR_R);

    // 头像外圈（白色边框）
    painter->setBrush(Qt::white);
    painter->setPen(QPen(color_.darker(130), hovered_ ? 2.5 : 1.5));
    painter->drawEllipse(QPointF(0, 0), AVATAR_R + 1, AVATAR_R + 1);

    // 头像内圈（彩色填充）
    QColor fillColor = hovered_ ? color_.lighter(120) : color_;
    painter->setBrush(fillColor);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0, 0), AVATAR_R, AVATAR_R);

    // 头像内文字（姓名首字）
    QFont font = painter->font();
    font.setPointSize(8);
    font.setBold(true);
    painter->setFont(font);
    painter->setPen(Qt::white);
    if (!name_.isEmpty()) {
        painter->drawText(QRectF(-AVATAR_R, -AVATAR_R, AVATAR_R * 2, AVATAR_R * 2),
                          Qt::AlignCenter, name_.left(1));
    }

    // 姓名标签（头像下方）
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(name_);
    QRectF labelRect(-textW / 2 - 3, AVATAR_R + 2, textW + 6, LABEL_H);

    painter->setBrush(QColor(255, 255, 255, 220));
    painter->setPen(QPen(color_, 1));
    painter->drawRoundedRect(labelRect, 3, 3);

    painter->setPen(QColor(50, 50, 50));
    painter->drawText(labelRect, Qt::AlignCenter, name_);
}

void TeamMemberItem::advancePos() {
    if (dragging_) return;  // 拖动中不自动移动

    QPointF cur = pos();
    qreal dx = targetX_ - cur.x();
    qreal dy = targetY_ - cur.y();
    qreal dist = std::sqrt(dx * dx + dy * dy);

    if (dist < 0.5) return;  // 已到达目标

    // 每帧移动 10% 距离（顺滑移动）
    qreal step = std::min(dist * 0.1, 3.0);  // 最大每帧3像素
    qreal nx = dx / dist * step;
    qreal ny = dy / dist * step;
    moveBy(nx, ny);

    emit positionChanged(id_, pos().x(), pos().y());
}

void TeamMemberItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        dragging_ = true;
        emit clicked(id_);
    }
    QGraphicsObject::mousePressEvent(event);
}

void TeamMemberItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event) {
    QGraphicsObject::mouseMoveEvent(event);
    // 拖动中更新目标位置
    targetX_ = pos().x();
    targetY_ = pos().y();
    emit positionChanged(id_, pos().x(), pos().y());
}

void TeamMemberItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    dragging_ = false;
    targetX_ = pos().x();
    targetY_ = pos().y();
    emit positionChanged(id_, pos().x(), pos().y());
    QGraphicsObject::mouseReleaseEvent(event);
}

void TeamMemberItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = true;
    setCursor(QCursor(Qt::PointingHandCursor));
    setScale(1.15);
    update();
}

void TeamMemberItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = false;
    unsetCursor();
    setScale(1.0);
    update();
}
