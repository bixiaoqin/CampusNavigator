#ifndef CAMPUSNAVIGATOR_VIEW_NPCITEM_H
#define CAMPUSNAVIGATOR_VIEW_NPCITEM_H

#include <QGraphicsObject>
#include <QString>

// 前向声明
class Graph;

// ============================================================
//  NpcItem - 地图上的 NPC 角色（会自主移动，点击可对话）
// ============================================================
//  继承 QGraphicsObject，支持信号与槽。
//  NPC 会在路网上自主游走——随机选择相邻建筑作为目标，
//  沿道路行走，到达后停留片刻再选下一个目标。
//  行走时有4方向动画（腿部交替+手臂摆动）。
//  玩家点击 NPC 可弹出对话气泡。
//  扩展功能：NPC 系统（会移动的 NPC）。
// ============================================================
class NpcItem : public QGraphicsObject {
    Q_OBJECT

public:
    // NPC 配色方案（不同 NPC 穿不同颜色衣服，便于区分）
    enum class Outfit {
        Red,    // 红衣（学生）
        Green,  // 绿衣（保安）
        Purple, // 紫衣（老师）
        Orange, // 橙衣（食堂阿姨）
        Pink    // 粉衣（护士）
    };

    // 移动方向（4方向行走动画）
    enum class Direction { Up, Down, Left, Right };

    // 移动状态
    enum class MoveState {
        Idle,     // 站立停留
        Walking   // 正在走向目标建筑
    };

    // 构造：指定 NPC 名字、对话内容、衣服颜色
    NpcItem(const QString& name, const QString& dialog,
            Outfit outfit, QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter,
               const QStyleOptionGraphicsItem* option,
               QWidget* widget) override;

    // --- NPC 移动控制 ---
    // 设置路网图指针（NPC 根据邻接表选择行走目标）
    void setGraph(const Graph* graph) { graph_ = graph; }

    // 设置 NPC 的"驻地"建筑 ID（起始位置附近的建筑）
    void setHomeBuildingId(int id) { homeBuildingId_ = id; }

    // 推进 NPC 移动逻辑（由外部定时器每帧调用）
    void updateMovement();

    QString name()   const { return name_; }
    QString dialog() const { return dialog_; }

signals:
    void clicked(const QString& name, const QString& dialog);  // 玩家点击时发射

protected:
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;
    void hoverEnterEvent(QGraphicsSceneHoverEvent* event) override;
    void hoverLeaveEvent(QGraphicsSceneHoverEvent* event) override;

private:
    QString name_;
    QString dialog_;
    Outfit outfit_;
    bool hovered_ = false;

    // --- 移动状态 ---
    const Graph* graph_ = nullptr;       // 路网图指针
    int homeBuildingId_ = -1;            // 当前所在建筑 ID
    int targetBuildingId_ = -1;          // 目标建筑 ID
    QPointF targetPos_;                  // 目标坐标（含随机偏移）
    MoveState moveState_ = MoveState::Idle;
    Direction moveDirection_ = Direction::Down;

    int idleCounter_ = 0;                // 空闲计时（帧）
    int idleDuration_ = 30;              // 空闲持续帧数

    static constexpr double moveSpeed_ = 2.0;  // 移动速度（像素/帧）
    int walkFrame_ = 0;                  // 行走动画帧 0-3
    int frameCounter_ = 0;              // 帧率控制计数器

    // --- 私有方法 ---
    void pickNewTarget();               // 随机选择下一个目标建筑
    QColor outfitColor() const;          // 根据 outfit 枚举返回衣服颜色
};

#endif // CAMPUSNAVIGATOR_VIEW_NPCITEM_H
