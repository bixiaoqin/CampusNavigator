#ifndef CAMPUSNAVIGATOR_VIEW_ROADITEM_H
#define CAMPUSNAVIGATOR_VIEW_ROADITEM_H

#include <QGraphicsItem>
#include <QPainterPath>

// ============================================================
//  RoadItem - 地图上的道路图元
// ============================================================
//  道路支持折线：两端建筑之间可加入若干"拐点"(waypoints)，
//  以更自然的弯曲路线连接（如绕湖、绕楼）。无拐点时为直线。
//  三种状态：
//    - 普通道路：浅灰细线
//    - 高亮路径：柔和绿色半透明线
//    - 淡化路径：灰色极淡线（历史路径不抢视觉）
//  Z值=-1，在建筑图元下层，不会遮挡建筑标签。
//  管理员"路网编辑模式"下接受右键点击，发送 rightClicked 信号用于删除道路。
// ============================================================
class RoadItem : public QGraphicsObject {
    Q_OBJECT
public:
    // 构造：起点坐标、终点坐标、道路距离、两端建筑ID
    RoadItem(double x1, double y1, double x2, double y2, double distance,
             int fromId = -1, int toId = -1,
             QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    void setHighlighted(bool on) { highlighted_ = on; update(); }
    void setFaded(bool on) { faded_ = on; update(); }

    bool connects(int a, int b) const {
        return (fromId_ == a && toId_ == b) || (fromId_ == b && toId_ == a);
    }

    int fromId() const { return fromId_; }
    int toId() const { return toId_; }

    // 端点坐标 getter（拖拽路口时读取另一端坐标）
    double x1() const { return x1_; }
    double y1() const { return y1_; }
    double x2() const { return x2_; }
    double y2() const { return y2_; }

    // 更新端点坐标（拖拽路口时实时刷新道路绘制）
    void updateEndpoints(double x1, double y1, double x2, double y2);

    // 添加一个"拐点"（折线中间点），道路改为绕行该点
    void addWaypoint(const QPointF& wp);
    const QVector<QPointF>& waypoints() const { return waypoints_; }

    // 是否处于"可被右键删除"状态（管理员路网编辑模式）
    void setDeletable(bool on);
    bool isDeletable() const { return deletable_; }

signals:
    // 右键点击：用于"删除道路"工具
    void rightClicked(int fromId, int toId);

protected:
    // 启用悬停/光标（编辑模式下）
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void mouseReleaseEvent(QGraphicsSceneMouseEvent* event) override;

private:
    double x1_, y1_, x2_, y2_;
    double distance_;
    QVector<QPointF> waypoints_;   // 折线拐点（场景坐标）
    bool highlighted_ = false;
    bool faded_ = false;       // 历史路径淡化
    bool deletable_ = false;   // 处于编辑模式
    bool hovered_ = false;
    int fromId_ = -1, toId_ = -1;
};

#endif
