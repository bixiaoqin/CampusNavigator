#ifndef CAMPUSNAVIGATOR_VIEW_BUILDINGITEM_H
#define CAMPUSNAVIGATOR_VIEW_BUILDINGITEM_H

#include <QGraphicsObject>
#include <model/Building.h>

// ============================================================
//  BuildingItem - 地图上的建筑标签图元（自定义 QGraphicsItem）
// ============================================================
//  【圆角白底标签模式】
//    在地图底图上，每栋建筑位置显示一个彩色小圆点标记，
//    上方/下方悬浮圆角白底名称标签。标签方向交替避让，
//    防止密集区域文字重叠。缩放时字号自适应调整。
//
//  交互：
//    - 悬停：标签高亮（加粗边框+背景变色）
//    - 点击：发射 clicked 信号，主窗口弹出详情面板
//    - 选中：金色边框+发光
// ============================================================
class BuildingItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit BuildingItem(const Building* building, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    const Building* building() const { return building_; }
    int buildingId() const { return building_ ? building_->id() : -1; }

    void setSelected(bool selected);
    bool isSelected() const { return selected_; }

    // 设置标签偏移方向（true=上方, false=下方），用于避让
    void setLabelAbove(bool above) { labelAbove_ = above; update(); }

    // 设置标签缩放因子（随地图缩放自适应）
    void setLabelScale(qreal scale) { labelScale_ = scale; update(); }

signals:
    void clicked(int buildingId);

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    const Building* building_;
    bool selected_ = false;
    bool hovered_ = false;
    bool labelAbove_ = true;     // 标签显示在上方还是下方（避让用）
    qreal labelScale_ = 1.0;     // 标签缩放因子（随地图缩放自适应）

    QColor typeColor() const;
};

#endif // CAMPUSNAVIGATOR_VIEW_BUILDINGITEM_H
