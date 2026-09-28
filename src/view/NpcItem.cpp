#include "view/NpcItem.h"

#include "model/Graph.h"
#include "model/Building.h"

#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>
#include <cmath>
#include <cstdlib>

// NPC 尺寸常量
static constexpr int NPC_HEAD_R = 8;
static constexpr int NPC_BODY_W = 14;
static constexpr int NPC_BODY_H = 14;

NpcItem::NpcItem(const QString& name, const QString& dialog,
                 Outfit outfit, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , name_(name)
    , dialog_(dialog)
    , outfit_(outfit)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, false);
    setZValue(9);   // 比建筑高（Z=0），比玩家角色低（Z=10）
    // 初始空闲时间随机化（20~80帧 ≈ 0.7~2.7秒），避免所有 NPC 同时出发
    idleDuration_ = 20 + std::rand() % 60;
}

QRectF NpcItem::boundingRect() const {
    // 覆盖范围：头顶标识(-22) 到 脚下阴影(+28)
    return QRectF(-14, -22, 28, 50);
}

// ============================================================
//  绘制：4方向人物 + 行走动画（腿部交替 + 手臂摆动）
// ============================================================
void NpcItem::paint(QPainter* painter,
                    const QStyleOptionGraphicsItem* /*option*/,
                    QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor bodyColor = outfitColor();
    QColor bodyDark  = bodyColor.darker(130);
    QColor skinColor(255, 224, 189);

    // 悬停时加亮（视觉反馈：这个 NPC 可以对话）
    if (hovered_) bodyColor = bodyColor.lighter(115);

    qreal cx = 0, cy = 0;
    bool walking = (moveState_ == MoveState::Walking);

    // --- 行走动画偏移量 ---
    // 4帧循环：0=左腿前 1=并拢 2=右腿前 3=并拢
    qreal legSwing = 0;
    qreal armSwing = 0;
    if (walking) {
        if (walkFrame_ == 0) { legSwing = 2;  armSwing = 1.5; }
        else if (walkFrame_ == 2) { legSwing = -2; armSwing = -1.5; }
        // 帧1和3：偏移=0，自然过渡
    }

    // --- 0. 阴影（脚下椭圆，增加立体感）---
    painter->setBrush(QColor(0, 0, 0, 50));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(cx, cy + NPC_BODY_H + 8), 8, 3);

    // --- 1. 腿部（行走时交替摆动）---
    qreal legTop = cy + NPC_HEAD_R + NPC_BODY_H - 4;
    qreal legH = 10;

    painter->setBrush(QColor(55, 71, 79));   // 深色裤子
    painter->setPen(QPen(QColor(33, 33, 33), 0.8));

    if (moveDirection_ == Direction::Down || moveDirection_ == Direction::Up) {
        // 正面/背面：左右两条腿
        painter->drawRoundedRect(QRectF(cx - 5 + legSwing, legTop, 4, legH), 1, 1);
        painter->drawRoundedRect(QRectF(cx + 1 - legSwing, legTop, 4, legH), 1, 1);
        // 鞋子
        painter->setBrush(QColor(33, 33, 33));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(QRectF(cx - 5 + legSwing, legTop + legH - 3, 4, 4), 1, 1);
        painter->drawRoundedRect(QRectF(cx + 1 - legSwing, legTop + legH - 3, 4, 4), 1, 1);
    } else {
        // 侧面（左/右）：前后两条腿
        painter->setBrush(QColor(55, 71, 79));
        painter->setPen(QPen(QColor(33, 33, 33), 0.8));
        painter->drawRoundedRect(QRectF(cx - 2 + legSwing, legTop, 4, legH), 1, 1);
        painter->drawRoundedRect(QRectF(cx - 2 - legSwing, legTop + 1, 4, legH - 1), 1, 1);
        // 鞋子
        painter->setBrush(QColor(33, 33, 33));
        painter->setPen(Qt::NoPen);
        painter->drawRoundedRect(QRectF(cx - 2 + legSwing, legTop + legH - 3, 4, 4), 1, 1);
        painter->drawRoundedRect(QRectF(cx - 2 - legSwing, legTop + legH - 3, 4, 4), 1, 1);
    }

    // --- 2. 身体 ---
    QRectF bodyRect(cx - NPC_BODY_W/2, cy + NPC_HEAD_R - 1, NPC_BODY_W, NPC_BODY_H);
    painter->setBrush(bodyColor);
    painter->setPen(QPen(bodyDark, 1.2));
    painter->drawRoundedRect(bodyRect, 4, 4);

    // --- 3. 手臂（行走时摆动）---
    painter->setBrush(bodyColor);
    painter->setPen(QPen(bodyDark, 1));

    if (moveDirection_ == Direction::Down || moveDirection_ == Direction::Up) {
        // 正面/背面：左右两只手臂
        painter->drawRoundedRect(
            QRectF(cx - NPC_BODY_W/2 - 3, cy + NPC_HEAD_R + armSwing, 3, 10), 1, 1);
        painter->drawRoundedRect(
            QRectF(cx + NPC_BODY_W/2, cy + NPC_HEAD_R - armSwing, 3, 10), 1, 1);
        // 手
        painter->setBrush(skinColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(
            QPointF(cx - NPC_BODY_W/2 - 1.5, cy + NPC_HEAD_R + 10 + armSwing), 2, 2);
        painter->drawEllipse(
            QPointF(cx + NPC_BODY_W/2 + 1.5, cy + NPC_HEAD_R + 10 - armSwing), 2, 2);
    } else if (moveDirection_ == Direction::Left) {
        // 左侧：一只手臂可见
        painter->drawRoundedRect(
            QRectF(cx - NPC_BODY_W/2 - 1, cy + NPC_HEAD_R + armSwing, 3, 10), 1, 1);
        painter->setBrush(skinColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(
            QPointF(cx - NPC_BODY_W/2 + 0.5, cy + NPC_HEAD_R + 10 + armSwing), 2, 2);
    } else {
        // 右侧：一只手臂可见
        painter->drawRoundedRect(
            QRectF(cx + NPC_BODY_W/2 - 2, cy + NPC_HEAD_R - armSwing, 3, 10), 1, 1);
        painter->setBrush(skinColor);
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(
            QPointF(cx + NPC_BODY_W/2 - 0.5, cy + NPC_HEAD_R + 10 - armSwing), 2, 2);
    }

    // --- 4. 头部 ---
    painter->setBrush(skinColor);
    painter->setPen(QPen(QColor(220, 180, 150), 1.2));
    painter->drawEllipse(QPointF(cx, cy), NPC_HEAD_R, NPC_HEAD_R);

    // --- 5. 头发 ---
    if (moveDirection_ == Direction::Up) {
        // 背面：头发覆盖整个头部
        painter->setBrush(QColor(60, 50, 45));
        painter->setPen(QPen(QColor(40, 30, 25), 1));
        painter->drawEllipse(QPointF(cx, cy), NPC_HEAD_R, NPC_HEAD_R);
    } else {
        // 正面/侧面：头发覆盖头顶上半部分
        QPainterPath hair;
        hair.moveTo(-NPC_HEAD_R, -1);
        hair.cubicTo(-NPC_HEAD_R, -NPC_HEAD_R - 1,
                     -NPC_HEAD_R/2, -NPC_HEAD_R - 2,
                      0, -NPC_HEAD_R);
        hair.cubicTo(NPC_HEAD_R/2, -NPC_HEAD_R - 2,
                     NPC_HEAD_R, -NPC_HEAD_R - 1,
                     NPC_HEAD_R, -1);
        hair.lineTo(NPC_HEAD_R - 1, -NPC_HEAD_R + 2);
        hair.lineTo(-NPC_HEAD_R + 1, -NPC_HEAD_R + 2);
        hair.closeSubpath();
        painter->setBrush(QColor(60, 50, 45));
        painter->setPen(QPen(QColor(40, 30, 25), 1));
        painter->drawPath(hair);
    }

    // --- 6. 眼睛（方向感知）---
    painter->setBrush(Qt::black);
    painter->setPen(Qt::NoPen);
    if (moveDirection_ == Direction::Down) {
        // 正面：两只眼睛
        painter->drawEllipse(QPointF(-3, 1), 1.2, 1.5);
        painter->drawEllipse(QPointF( 3, 1), 1.2, 1.5);
    } else if (moveDirection_ == Direction::Left) {
        // 左侧脸：左眼可见
        painter->drawEllipse(QPointF(-3, 1), 1.2, 1.5);
    } else if (moveDirection_ == Direction::Right) {
        // 右侧脸：右眼可见
        painter->drawEllipse(QPointF(3, 1), 1.2, 1.5);
    }
    // Up: 背面，看不到眼睛

    // --- 7. NPC 头顶标识（感叹号或名字）---
    QFont font = painter->font();
    if (hovered_) {
        // 悬停时显示名字
        font.setPointSize(8);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(QPen(QColor(33, 33, 33), 1));
        QRectF nameRect(-30, -NPC_HEAD_R - 22, 60, 14);
        painter->drawText(nameRect, Qt::AlignCenter, name_);
    } else {
        // 黄色感叹号气泡（提示"可对话"）
        painter->setBrush(QColor(255, 193, 7));
        painter->setPen(QPen(QColor(245, 127, 23), 1.2));
        painter->drawEllipse(QPointF(0, -NPC_HEAD_R - 6), 4, 4);
        font.setPointSize(7);
        font.setBold(true);
        painter->setFont(font);
        painter->setPen(Qt::black);
        painter->drawText(QRectF(-4, -NPC_HEAD_R - 10, 8, 8),
                          Qt::AlignCenter, "!");
    }
}

// ============================================================
//  NPC 移动逻辑：在路网上自主游走
// ============================================================
//  状态机：Idle（停留）→ Walking（走向目标）→ Idle → ...
//  每次到达目标后随机停留 1~3 秒，再从当前建筑的邻居中
//  随机选一个作为下一个目标。
// ============================================================

void NpcItem::updateMovement() {
    if (moveState_ == MoveState::Idle) {
        // 空闲计时
        idleCounter_++;
        if (idleCounter_ >= idleDuration_) {
            // 时间到，选择下一个目标
            pickNewTarget();
            if (targetBuildingId_ >= 0) {
                moveState_ = MoveState::Walking;
            }
            idleCounter_ = 0;
        }
    } else if (moveState_ == MoveState::Walking) {
        // 朝目标移动
        QPointF curPos = pos();
        double dx = targetPos_.x() - curPos.x();
        double dy = targetPos_.y() - curPos.y();
        double dist = std::sqrt(dx * dx + dy * dy);

        if (dist <= moveSpeed_) {
            // 到达目标
            setPos(targetPos_);
            moveState_ = MoveState::Idle;
            homeBuildingId_ = targetBuildingId_;
            targetBuildingId_ = -1;
            walkFrame_ = 0;
            // 随机停留 1~3 秒（30~90帧）
            idleDuration_ = 30 + std::rand() % 60;
        } else {
            // 朝目标方向移动一步
            double nx = dx / dist;
            double ny = dy / dist;

            // 更新朝向（优先水平/垂直中较大的方向）
            if (std::abs(nx) > std::abs(ny)) {
                moveDirection_ = (nx > 0) ? Direction::Right : Direction::Left;
            } else {
                moveDirection_ = (ny > 0) ? Direction::Down : Direction::Up;
            }

            moveBy(nx * moveSpeed_, ny * moveSpeed_);

            // 推进行走动画帧（每3帧推进1帧，节奏舒缓）
            if (++frameCounter_ >= 3) {
                frameCounter_ = 0;
                walkFrame_ = (walkFrame_ + 1) % 4;
            }
        }
    }

    update();   // 触发重绘
}

// 随机选择下一个目标建筑（从当前建筑的邻居中选一个）
void NpcItem::pickNewTarget() {
    if (!graph_ || homeBuildingId_ < 0) return;

    const auto& edges = graph_->neighbors(homeBuildingId_);
    if (edges.empty()) return;

    // 随机选一个邻居
    int idx = std::rand() % static_cast<int>(edges.size());
    int targetId = edges[static_cast<size_t>(idx)].to;

    const Building* b = graph_->getBuilding(targetId);
    if (!b) return;

    targetBuildingId_ = targetId;
    // 添加随机偏移（±15像素），避免 NPC 站在建筑正中心
    qreal offX = static_cast<qreal>(std::rand() % 30) - 15;
    qreal offY = static_cast<qreal>(std::rand() % 30) - 15;
    targetPos_ = QPointF(b->x() + offX, b->y() + offY);
}

// ============================================================
//  事件处理
// ============================================================
void NpcItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(name_, dialog_);
    }
    QGraphicsObject::mousePressEvent(event);
}

void NpcItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = true;
    setCursor(QCursor(Qt::PointingHandCursor));
    update();
}

void NpcItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = false;
    unsetCursor();
    update();
}

QColor NpcItem::outfitColor() const {
    switch (outfit_) {
        case Outfit::Red:    return QColor(244, 67, 54);    // 红
        case Outfit::Green:  return QColor(76, 175, 80);    // 绿
        case Outfit::Purple: return QColor(156, 39, 176);   // 紫
        case Outfit::Orange: return QColor(255, 152, 0);    // 橙
        case Outfit::Pink:   return QColor(233, 30, 99);    // 粉
    }
    return QColor(150, 150, 150);
}
