#ifndef CAMPUSNAVIGATOR_VIEW_TEAMMEMBERITEM_H
#define CAMPUSNAVIGATOR_VIEW_TEAMMEMBERITEM_H

#include <QGraphicsObject>

// ============================================================
//  TeamMemberItem - 小组组员图元（可拖动，头像标注）
// ============================================================
//  圆形头像 + 姓名标签，不同颜色区分组员。
//  支持鼠标拖动改变位置，模拟组员日常走动。
//  Z=8，在建筑之上、角色之下。
// ============================================================
class TeamMemberItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit TeamMemberItem(int id, const QString& name, const QColor& color,
                            QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    int memberId() const { return id_; }
    QString memberName() const { return name_; }
    QColor memberColor() const { return color_; }

    // 设置目标位置（用于顺滑移动动画）
    void setTargetPos(qreal x, qreal y) { targetX_ = x; targetY_ = y; }

    // 每帧推进：当前位置向目标位置靠近（顺滑移动）
    void advancePos();

signals:
    void positionChanged(int id, qreal x, qreal y);
    void clicked(int id);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseMoveEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    int id_;
    QString name_;
    QColor color_;
    qreal targetX_, targetY_;  // 目标位置（顺滑移动用）
    bool dragging_ = false;
    bool hovered_ = false;
};

#endif
