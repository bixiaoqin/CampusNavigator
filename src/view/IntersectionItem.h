#ifndef CAMPUSNAVIGATOR_VIEW_INTERSECTIONITEM_H
#define CAMPUSNAVIGATOR_VIEW_INTERSECTIONITEM_H

#include <QGraphicsObject>

// ============================================================
//  IntersectionItem - 可拖拽路口节点图元
// ============================================================
//  代表路网中的十字路口/弯道节点（nodeKind=0）。
//  界面上显示为蓝灰色小圆点，支持鼠标拖拽调整位置。
//  拖拽时实时通知 MapScene 更新连接的道路；
//  释放时通知 MainWindow 持久化到数据库。
//  Z=1，在最上层，确保可被点击拖拽。
//  默认不可编辑（普通模式下不响应鼠标），
//  管理员模式的"路网编辑"中通过 setEditable(true) 打开。
// ============================================================
class IntersectionItem : public QGraphicsObject {
    Q_OBJECT
public:
    IntersectionItem(int nodeId, QGraphicsItem* parent = nullptr);

    int nodeId() const { return nodeId_; }

    QRectF boundingRect() const override;
    QPainterPath shape() const override;   // 命中区域（放大，便于点击/右键）
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    // 是否处于可编辑状态（管理员路网编辑模式）
    bool isEditable() const { return editable_; }
    void setEditable(bool editable);

    // 单独控制"是否可拖动"。仅"添加路口"模式需要可拖动；
    // 连接/删除模式下应置 false，避免误判点击为拖动而吞掉 clicked 信号。
    void setMovable(bool movable);

    // 视觉选中态（连接道路工具中作为"第一个点"高亮）
    void setSelectedVisual(bool on);
    bool isSelectedVisual() const { return selectedVisual_; }

signals:
    // 拖拽过程中持续发送（实时更新道路）
    void positionChanged(int nodeId, QPointF newPos);
    // 释放鼠标时发送（持久化到数据库）；如果没动也会发，等同点击结束
    void dragFinished(int nodeId, QPointF finalPos);
    // 普通点击（无拖动）时发送，用于"连接道路"工具
    void clicked(int nodeId);
    // 右键点击（无拖动）时发送，用于"删除路口"工具
    void rightClicked(int nodeId);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    int nodeId_;
    bool hovered_ = false;
    bool editable_ = false;        // 编辑模式开关
    bool selectedVisual_ = false;  // 视觉选中（连接工具的第一点）
    QPointF pressScenePos_;        // 鼠标按下时场景坐标，用于判点击/拖动
};

#endif // CAMPUSNAVIGATOR_VIEW_INTERSECTIONITEM_H
