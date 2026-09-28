#include "view/WeatherOverlay.h"

#include <QPainter>
#include <QRandomGenerator>

// ============================================================
//  WeatherOverlay 实现
// ============================================================

WeatherOverlay::WeatherOverlay(QGraphicsItem* parent)
    : QGraphicsItem(parent)
{
    // 放在最上层（高于角色），让粒子覆盖整个画面
    setZValue(100);
    // 纯装饰层：关闭鼠标交互，否则会盖住全屏、吞掉所有点击
    // （导致路网编辑模式点空白/点建筑都没反应）
    setAcceptedMouseButtons(Qt::NoButton);
    setAcceptHoverEvents(false);
}

void WeatherOverlay::setAreaSize(double w, double h) {
    areaW_ = w;
    areaH_ = h;
    if (weather_ != WeatherType::None) {
        initParticles();
    }
}

void WeatherOverlay::setWeather(WeatherType type) {
    weather_ = type;
    initParticles();
    update();
}

void WeatherOverlay::initParticles() {
    particles_.clear();
    if (weather_ == WeatherType::None) return;

    // 粒子数量：雨多一点，雪少一点
    int count = (weather_ == WeatherType::Rain) ? 150 : 80;

    auto* rng = QRandomGenerator::global();
    for (int i = 0; i < count; ++i) {
        Particle p;
        // 用 generateDouble() 生成 [0,1) 随机数，手动映射到目标范围
        // 避免 Qt 6.1.1 MinGW 下 bounded(double) 歧义编译错误
        p.pos = QPointF(rng->generateDouble() * areaW_,
                        rng->generateDouble() * areaH_);

        if (weather_ == WeatherType::Rain) {
            // 雨：快速下落 + 轻微向右倾斜
            p.vel = QPointF(2.0, 12.0 + rng->generateDouble() * 4.0);
            p.length = 8.0 + rng->generateDouble() * 6.0;
        } else {
            // 雪：缓慢飘落 + 左右摇摆
            p.vel = QPointF((rng->generateDouble() * 2.0 - 1.0),
                            2.0 + rng->generateDouble() * 2.0);
            p.length = 3.0 + rng->generateDouble() * 3.0;
        }
        particles_.push_back(p);
    }
}

QRectF WeatherOverlay::boundingRect() const {
    return QRectF(0, 0, areaW_, areaH_);
}

void WeatherOverlay::paint(QPainter* painter,
                            const QStyleOptionGraphicsItem* /*option*/,
                            QWidget* /*widget*/)
{
    if (weather_ == WeatherType::None) return;

    if (weather_ == WeatherType::Rain) {
        // 雨：蓝色半透明短线
        painter->setPen(QPen(QColor(100, 150, 255, 180), 1.5));
        for (const auto& p : particles_) {
            // 画一条从 pos 沿速度方向的长线
            double len = p.length;
            QPointF end(p.pos.x() + p.vel.x() * len / 10,
                        p.pos.y() + p.vel.y() * len / 10);
            painter->drawLine(p.pos, end);
        }
    } else {
        // 雪：白色圆点
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor(255, 255, 255, 220));
        for (const auto& p : particles_) {
            painter->drawEllipse(p.pos, p.length / 2, p.length / 2);
        }
    }
}

// 每帧更新粒子位置
void WeatherOverlay::advance(int phase) {
    if (phase == 0 || weather_ == WeatherType::None) return;

    for (auto& p : particles_) {
        p.pos += p.vel;

        // 超出底部 → 重置到顶部
        if (p.pos.y() > areaH_) {
            p.pos.setY(-10);
            p.pos.setX(QRandomGenerator::global()->generateDouble() * areaW_);
        }
        // 超出左右 → 回绕
        if (p.pos.x() < 0)  p.pos.setX(areaW_);
        if (p.pos.x() > areaW_) p.pos.setX(0);
    }

    update();   // 触发重绘
}
