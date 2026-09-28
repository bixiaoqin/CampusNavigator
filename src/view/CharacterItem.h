#ifndef CAMPUSNAVIGATOR_VIEW_CHARACTERITEM_H
#define CAMPUSNAVIGATOR_VIEW_CHARACTERITEM_H

#include <QGraphicsObject>

// ============================================================
//  CharacterItem - 玩家角色图元（写实风格）
// ============================================================
//  地图上的可控制角色，用 WASD 键移动。
//  写实风格人物绘制：头部(发/眉/眼/鼻/嘴)、上身(衣领/纽扣)、
//  手臂(摆动)、腿部(交替行走动画)、鞋子。
//  4个方向各有独立绘制，行走时腿部交替摆动。
// ============================================================
class CharacterItem : public QGraphicsObject {
    Q_OBJECT

public:
    explicit CharacterItem(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    enum class Direction { Up, Down, Left, Right };
    void setDirection(Direction d) { direction_ = d; update(); }

    // 行走状态（WASD按下时设true，松开设false）
    void setWalking(bool w) { walking_ = w; if (!w) walkFrame_ = 0; update(); }
    bool isWalking() const { return walking_; }

    // 推进动画帧（由外部定时器调用，内部做帧率控制）
    void advanceFrame() {
        if (!walking_) return;
        // 帧率控制：每3次调用推进1帧，脚步节奏舒缓
        if (++frameCounter_ >= 3) {
            frameCounter_ = 0;
            walkFrame_ = (walkFrame_ + 1) % 4;
        }
        update();
    }

    void setSpeed(double s) { speed_ = s; }
    double speed() const { return speed_; }

signals:
    void positionChanged(qreal x, qreal y);

private:
    Direction direction_ = Direction::Down;
    double speed_ = 5.0;
    bool walking_ = false;       // 是否在行走
    int walkFrame_ = 0;          // 行走动画帧 0-3
    int frameCounter_ = 0;       // 帧率控制计数器

    // 各部位绘制方法
    void drawHead(QPainter* p, Direction dir, qreal cx, qreal cy);
    void drawBody(QPainter* p, Direction dir, qreal cx, qreal cy);
    void drawLegs(QPainter* p, Direction dir, qreal cx, qreal cy, int frame);
    void drawArms(QPainter* p, Direction dir, qreal cx, qreal cy, int frame);
};

#endif
