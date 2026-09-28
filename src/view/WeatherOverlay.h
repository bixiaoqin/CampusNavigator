#ifndef CAMPUSNAVIGATOR_VIEW_WEATHEROVERLAY_H
#define CAMPUSNAVIGATOR_VIEW_WEATHEROVERLAY_H

#include <QGraphicsItem>
#include <QVector>
#include <QPointF>

// ============================================================
//  WeatherOverlay - 天气特效图元（雨/雪粒子）
// ============================================================
//  覆盖在整个地图上的半透明粒子层，用于模拟下雨/下雪效果。
//  每帧由 MainWindow 的定时器调用 advance() 更新粒子位置。
//
//  实现思路：
//    - 维护一组粒子（位置 + 速度）
//    - 每帧 y += vy（下落），x += vx（风偏移）
//    - 超出边界则重置到顶部
//    - 用 QPainter 画短线（雨）或圆点（雪）
// ============================================================
class WeatherOverlay : public QGraphicsItem {
public:
    enum class WeatherType { None, Rain, Snow };

    explicit WeatherOverlay(QGraphicsItem* parent = nullptr);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option,
               QWidget* widget = nullptr) override;

    // 设置天气类型（None=晴，Rain=雨，Snow=雪）
    void setWeather(WeatherType type);
    WeatherType weather() const { return weather_; }

    // 设置覆盖区域大小（通常等于场景大小）
    void setAreaSize(double w, double h);

    // 每帧更新粒子位置（由外部定时器调用）
    void advance(int phase) override;

private:
    struct Particle {
        QPointF pos;      // 位置
        QPointF vel;      // 速度
        double length;    // 粒子长度（雨用）
    };

    void initParticles();   // 根据天气类型初始化粒子

    WeatherType weather_ = WeatherType::None;
    double areaW_ = 800;   // 覆盖区域宽
    double areaH_ = 700;   // 覆盖区域高
    QVector<Particle> particles_;   // 粒子数组
};

#endif // CAMPUSNAVIGATOR_VIEW_WEATHEROVERLAY_H
