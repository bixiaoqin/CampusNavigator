#include "view/CharacterItem.h"

#include <QPainter>
#include <QColor>
#include <QBrush>
#include <QPen>

// ============================================================
//  CharacterItem 实现：写实风格人物
// ============================================================
//  人物比例（像素）：
//    总高 ~48px，头 ~14px，身体 ~20px，腿 ~14px
//    头身比 ≈ 1:3.4（介于Q版和写实之间，俯视图合适）
//
//  颜色方案：
//    肤色  #F5C6A0   头发  #3E2723   上衣  #1565C0
//    裤子  #37474F   鞋子  #212121   眼睛  #212121
// ============================================================

static const QColor SKIN(245, 198, 160);
static const QColor HAIR(62, 39, 35);
static const QColor SHIRT(21, 101, 192);
static const QColor SHIRT_DARK(13, 71, 161);
static const QColor PANTS(55, 71, 79);
static const QColor SHOES(33, 33, 33);
static const QColor EYES(33, 33, 33);

// 人物各部位尺寸
static constexpr qreal HEAD_R = 7;     // 头部半径
static constexpr qreal BODY_W = 14;    // 身体宽度
static constexpr qreal BODY_H = 18;    // 身体高度
static constexpr qreal LEG_W = 5;      // 单腿宽度
static constexpr qreal LEG_H = 12;     // 腿长
static constexpr qreal ARM_W = 4;      // 手臂宽度

CharacterItem::CharacterItem(QGraphicsItem* parent)
    : QGraphicsObject(parent)
{
    setZValue(10);  // 角色在建筑之上
}

QRectF CharacterItem::boundingRect() const {
    return QRectF(-14, -28, 28, 52);
}

void CharacterItem::paint(QPainter* painter,
                          const QStyleOptionGraphicsItem* /*option*/,
                          QWidget* /*widget*/)
{
    painter->setRenderHint(QPainter::Antialiasing, true);

    qreal cx = 0, cy = 0;

    // 阴影（脚下椭圆，增加立体感）
    painter->setBrush(QColor(0, 0, 0, 60));
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(cx, cy + 22), 8, 3);

    // 按层次绘制：腿 → 身体 → 手臂 → 头
    drawLegs(painter, direction_, cx, cy, walkFrame_);
    drawBody(painter, direction_, cx, cy);
    drawArms(painter, direction_, cx, cy, walkFrame_);
    drawHead(painter, direction_, cx, cy);

    // 方向指示箭头（白色小三角，浮在头顶上方）
    painter->setBrush(QColor(255, 255, 255, 200));
    painter->setPen(QPen(QColor(33, 150, 243), 1));
    QPolygonF arrow;
    switch (direction_) {
        case Direction::Down:
            arrow << QPointF(cx, cy - 24) << QPointF(cx - 4, cy - 30) << QPointF(cx + 4, cy - 30);
            break;
        case Direction::Up:
            arrow << QPointF(cx, cy - 24) << QPointF(cx - 4, cy - 30) << QPointF(cx + 4, cy - 30);
            break;
        case Direction::Left:
            arrow << QPointF(cx - 2, cy - 24) << QPointF(cx - 6, cy - 28) << QPointF(cx - 6, cy - 20);
            break;
        case Direction::Right:
            arrow << QPointF(cx + 2, cy - 24) << QPointF(cx + 6, cy - 28) << QPointF(cx + 6, cy - 20);
            break;
    }
    painter->drawPolygon(arrow);
}

// ============================================================
//  绘制头部（含头发、面部特征）
// ============================================================
void CharacterItem::drawHead(QPainter* p, Direction dir, qreal cx, qreal cy) {
    qreal headY = cy - 14;

    // 脖子
    p->setBrush(SKIN.darker(110));
    p->setPen(Qt::NoPen);
    p->drawRect(QRectF(cx - 3, headY + HEAD_R - 1, 6, 4));

    // 头部（椭圆）
    p->setBrush(SKIN);
    p->setPen(QPen(SKIN.darker(140), 0.5));
    p->drawEllipse(QPointF(cx, headY), HEAD_R, HEAD_R + 1);

    // 头发（根据方向不同形状）
    p->setBrush(HAIR);
    p->setPen(Qt::NoPen);
    switch (dir) {
        case Direction::Down:
            // 正面：头发覆盖头顶上半部分
            p->drawPie(QRectF(cx - HEAD_R, headY - HEAD_R - 1,
                              HEAD_R * 2, HEAD_R * 2 + 2), 0 * 16, 180 * 16);
            break;
        case Direction::Up:
            // 背面：头发覆盖整个头部
            p->drawEllipse(QPointF(cx, headY), HEAD_R, HEAD_R + 1);
            break;
        case Direction::Left:
            // 左侧脸：头发覆盖头顶+右侧后脑勺
            // 上半圆（头顶）
            p->drawPie(QRectF(cx - HEAD_R, headY - HEAD_R - 1,
                              HEAD_R * 2, HEAD_R * 2 + 2), 0 * 16, 180 * 16);
            // 右侧后脑勺（从3点到6点的1/4弧）
            p->drawPie(QRectF(cx - HEAD_R, headY - HEAD_R - 1,
                              HEAD_R * 2, HEAD_R * 2 + 2), 270 * 16, 90 * 16);
            break;
        case Direction::Right:
            // 右侧脸：头发覆盖头顶+左侧后脑勺
            // 上半圆（头顶）
            p->drawPie(QRectF(cx - HEAD_R, headY - HEAD_R - 1,
                              HEAD_R * 2, HEAD_R * 2 + 2), 0 * 16, 180 * 16);
            // 左侧后脑勺（从9点到6点的1/4弧）
            p->drawPie(QRectF(cx - HEAD_R, headY - HEAD_R - 1,
                              HEAD_R * 2, HEAD_R * 2 + 2), 90 * 16, 90 * 16);
            break;
    }

    // 面部特征（仅Down和侧面方向可见）
    if (dir == Direction::Down) {
        // 眼睛
        p->setBrush(EYES);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx - 2.5, headY), 0.8, 1.2);
        p->drawEllipse(QPointF(cx + 2.5, headY), 0.8, 1.2);

        // 眉毛
        p->setPen(QPen(HAIR, 1));
        p->drawLine(QPointF(cx - 3.5, headY - 2), QPointF(cx - 1.5, headY - 2.2));
        p->drawLine(QPointF(cx + 1.5, headY - 2.2), QPointF(cx + 3.5, headY - 2));

        // 嘴
        p->setPen(QPen(QColor(180, 100, 100), 1));
        p->drawLine(QPointF(cx - 1.5, headY + 3), QPointF(cx + 1.5, headY + 3));
    } else if (dir == Direction::Left) {
        // 左侧脸：只画一只眼睛
        p->setBrush(EYES);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx - 3, headY), 0.8, 1.2);
    } else if (dir == Direction::Right) {
        // 右侧脸：只画一只眼睛
        p->setBrush(EYES);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx + 3, headY), 0.8, 1.2);
    }
}

// ============================================================
//  绘制身体（上衣）
// ============================================================
void CharacterItem::drawBody(QPainter* p, Direction dir, qreal cx, qreal cy) {
    qreal bodyTop = cy - 10;
    QRectF bodyRect(cx - BODY_W / 2, bodyTop, BODY_W, BODY_H);

    // 上衣主体（渐变）
    QLinearGradient grad(bodyRect.topLeft(), bodyRect.bottomLeft());
    grad.setColorAt(0, SHIRT);
    grad.setColorAt(1, SHIRT_DARK);
    p->setBrush(grad);
    p->setPen(QPen(SHIRT_DARK.darker(130), 0.5));
    p->drawRoundedRect(bodyRect, 3, 3);

    // 领口
    p->setBrush(SKIN);
    p->setPen(Qt::NoPen);
    if (dir == Direction::Down) {
        // V字领
        QPolygonF collar;
        collar << QPointF(cx - 3, bodyTop) << QPointF(cx, bodyTop + 4)
               << QPointF(cx + 3, bodyTop);
        p->drawPolygon(collar);
    }

    // 纽扣（仅正面和侧面可见）
    if (dir == Direction::Down) {
        p->setBrush(QColor(200, 200, 200));
        p->setPen(Qt::NoPen);
        for (int i = 0; i < 3; ++i) {
            p->drawEllipse(QPointF(cx, bodyTop + 5 + i * 4), 0.8, 0.8);
        }
    }
}

// ============================================================
//  绘制手臂（行走时摆动）
// ============================================================
void CharacterItem::drawArms(QPainter* p, Direction dir, qreal cx, qreal cy, int frame) {
    qreal bodyTop = cy - 10;
    qreal armY = bodyTop + 3;

    // 行走时手臂摆动偏移（缩减到±1，自然幅度）
    qreal swing = 0;
    if (walking_) {
        swing = (frame == 0 || frame == 2) ? 1.0 : 0;
    }

    p->setBrush(SHIRT);
    p->setPen(QPen(SHIRT_DARK.darker(130), 0.5));

    if (dir == Direction::Down || dir == Direction::Up) {
        // 正面/背面：左右两只手臂
        p->drawRoundedRect(QRectF(cx - BODY_W / 2 - ARM_W, armY + swing, ARM_W, 12), 2, 2);
        p->drawRoundedRect(QRectF(cx + BODY_W / 2, armY - swing, ARM_W, 12), 2, 2);
        // 手
        p->setBrush(SKIN);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx - BODY_W / 2 - ARM_W / 2, armY + 12 + swing), 2, 2);
        p->drawEllipse(QPointF(cx + BODY_W / 2 + ARM_W / 2, armY + 12 - swing), 2, 2);
    } else if (dir == Direction::Left) {
        // 左侧：一只手臂可见
        p->drawRoundedRect(QRectF(cx - BODY_W / 2 - ARM_W + 2, armY + swing, ARM_W, 12), 2, 2);
        p->setBrush(SKIN);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx - BODY_W / 2 - ARM_W / 2 + 2, armY + 12 + swing), 2, 2);
    } else if (dir == Direction::Right) {
        // 右侧：一只手臂可见
        p->drawRoundedRect(QRectF(cx + BODY_W / 2 - 2, armY - swing, ARM_W, 12), 2, 2);
        p->setBrush(SKIN);
        p->setPen(Qt::NoPen);
        p->drawEllipse(QPointF(cx + BODY_W / 2 + ARM_W / 2 - 2, armY + 12 - swing), 2, 2);
    }
}

// ============================================================
//  绘制腿部（行走时交替摆动）
// ============================================================
void CharacterItem::drawLegs(QPainter* p, Direction dir, qreal cx, qreal cy, int frame) {
    qreal legTop = cy + 8;

    // 行走动画：4帧，迈步幅度缩减到±1（自然步幅，不劈叉）
    // 帧0: 左腿微前(+1), 右腿微后(-1)
    // 帧1: 双腿并拢（过渡帧）
    // 帧2: 左腿微后(-1), 右腿微前(+1)
    // 帧3: 双腿并拢（过渡帧）
    qreal leftOffset = 0, rightOffset = 0;
    if (walking_) {
        if (frame == 0) { leftOffset = 1; rightOffset = -1; }
        else if (frame == 2) { leftOffset = -1; rightOffset = 1; }
        // frame 1和3：保持0，双腿并拢（过渡帧）
    }

    p->setBrush(PANTS);
    p->setPen(QPen(PANTS.darker(140), 0.5));

    if (dir == Direction::Down || dir == Direction::Up) {
        // 正面/背面：左右两条腿，间距缩减
        p->drawRoundedRect(QRectF(cx - LEG_W - 0.5 + leftOffset, legTop, LEG_W, LEG_H), 2, 2);
        p->drawRoundedRect(QRectF(cx + 0.5 + rightOffset, legTop, LEG_W, LEG_H), 2, 2);
        // 鞋子
        p->setBrush(SHOES);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(QRectF(cx - LEG_W - 0.5 + leftOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
        p->drawRoundedRect(QRectF(cx + 0.5 + rightOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
    } else if (dir == Direction::Left) {
        // 左侧：两条腿前后排列，间距缩减
        p->drawRoundedRect(QRectF(cx - 1 + leftOffset, legTop, LEG_W, LEG_H), 2, 2);
        p->drawRoundedRect(QRectF(cx - 4 + rightOffset, legTop, LEG_W, LEG_H), 2, 2);
        p->setBrush(SHOES);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(QRectF(cx - 1 + leftOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
        p->drawRoundedRect(QRectF(cx - 4 + rightOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
    } else if (dir == Direction::Right) {
        // 右侧：两条腿前后排列，间距缩减
        p->drawRoundedRect(QRectF(cx - 3 + rightOffset, legTop, LEG_W, LEG_H), 2, 2);
        p->drawRoundedRect(QRectF(cx + rightOffset, legTop, LEG_W, LEG_H), 2, 2);
        p->setBrush(SHOES);
        p->setPen(Qt::NoPen);
        p->drawRoundedRect(QRectF(cx - 3 + rightOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
        p->drawRoundedRect(QRectF(cx + rightOffset, legTop + LEG_H - 3, LEG_W, 4), 1, 1);
    }
}
