#include "view/BuildingItem.h"

#include <QPainter>
#include <QFont>
#include <QFontMetrics>
#include <QColor>
#include <QCursor>
#include <QGraphicsSceneMouseEvent>
#include <QGraphicsSceneHoverEvent>

#include "model/Building.h"

// ============================================================
//  BuildingItem 实现：圆角白底悬浮标签模式
// ============================================================

static constexpr int DOT_R = 6;            // 标记点半径
static constexpr int LABEL_PADDING_X = 8;  // 标签水平内边距
static constexpr int LABEL_PADDING_Y = 4;  // 标签垂直内边距
static constexpr int LABEL_GAP = 10;       // 标签与标记点的间距

BuildingItem::BuildingItem(const Building* building, QGraphicsItem* parent)
    : QGraphicsObject(parent)
    , building_(building)
{
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setFlag(ItemIsMovable, false);

    if (building_) {
        setPos(building_->x(), building_->y());
        // 标签避让：偶数ID在上方，奇数ID在下方
        labelAbove_ = (building_->id() % 2 == 0);
    }
}

QRectF BuildingItem::boundingRect() const {
    if (!building_) return QRectF(-20, -20, 40, 40);

    QFont font;
    font.setPointSizeF(11 * labelScale_);
    font.setBold(false);
    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(building_->name());
    int labelW = textW + LABEL_PADDING_X * 2;
    int labelH = fm.height() + LABEL_PADDING_Y * 2;

    // 标签居中于原点（覆盖圆点），包围框覆盖标签+选中发光区域
    int halfW = qMax(labelW / 2 + 4, DOT_R + 10);
    int halfH = qMax(labelH / 2 + 4, DOT_R + 10);
    return QRectF(-halfW, -halfH, halfW * 2, halfH * 2);
}

void BuildingItem::paint(QPainter* painter,
                         const QStyleOptionGraphicsItem* /*option*/,
                         QWidget* /*widget*/)
{
    if (!building_) return;

    painter->setRenderHint(QPainter::Antialiasing, true);

    QColor color = typeColor();

    // ============================================================
    //  1. 标记点（建筑位置的彩色小圆点，隐藏在标签下方）
    // ============================================================
    if (selected_) {
        // 选中时外圈发光（从标签边缘透出形成光晕）
        painter->setBrush(QColor(color.red(), color.green(), color.blue(), 80));
        painter->setPen(Qt::NoPen);
        painter->drawEllipse(QPointF(0, 0), DOT_R + 6, DOT_R + 6);
    }

    // 圆点仍然绘制，但被上层不透明标签背景覆盖，视觉上不可见
    painter->setBrush(Qt::white);
    painter->setPen(QPen(color.darker(130), 1.5));
    painter->drawEllipse(QPointF(0, 0), DOT_R + 2, DOT_R + 2);

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(QPointF(0, 0), DOT_R, DOT_R);

    // ============================================================
    //  2. 圆角白底名称标签（居中于原点，覆盖圆点）
    // ============================================================
    QFont font = painter->font();
    font.setPointSizeF(11 * labelScale_);
    font.setBold(hovered_ || selected_);
    painter->setFont(font);

    QFontMetrics fm(font);
    int textW = fm.horizontalAdvance(building_->name());
    int labelW = textW + LABEL_PADDING_X * 2;
    int labelH = fm.height() + LABEL_PADDING_Y * 2;

    // 标签居中于原点，覆盖下方的圆点
    QRectF labelRect(-labelW / 2, -labelH / 2, labelW, labelH);

    // 标签背景色
    QColor bgColor;
    QColor borderColor;
    QColor textColor;

    if (selected_) {
        // 选中：金色边框+浅黄背景
        bgColor = QColor(255, 248, 225, 255);
        borderColor = QColor(255, 152, 0);
        textColor = QColor(80, 40, 0);
    } else if (hovered_) {
        // 悬停：浅蓝背景+蓝色边框
        bgColor = QColor(227, 242, 253, 255);
        borderColor = QColor(33, 150, 243);
        textColor = QColor(20, 60, 120);
    } else {
        // 默认：白底+浅灰边框
        bgColor = QColor(255, 255, 255, 255);
        borderColor = QColor(200, 200, 200);
        textColor = QColor(50, 50, 50);
    }

    // 阴影效果（增加层次感）
    painter->setBrush(QColor(0, 0, 0, 30));
    painter->setPen(Qt::NoPen);
    painter->drawRoundedRect(labelRect.translated(1, 1), 5, 5);

    // 标签背景（不透明，覆盖圆点）
    painter->setBrush(bgColor);
    painter->setPen(QPen(borderColor, (hovered_ || selected_) ? 2 : 1));
    painter->drawRoundedRect(labelRect, 5, 5);

    // 文字
    painter->setPen(textColor);
    painter->drawText(labelRect, Qt::AlignCenter, building_->name());

    // 标签居中覆盖圆点，无需连接线
}

void BuildingItem::setSelected(bool selected) {
    selected_ = selected;
    update();
}

void BuildingItem::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        emit clicked(buildingId());
    }
    QGraphicsObject::mousePressEvent(event);
}

void BuildingItem::hoverEnterEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = true;
    setCursor(QCursor(Qt::PointingHandCursor));
    update();
}

void BuildingItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* /*event*/) {
    hovered_ = false;
    unsetCursor();
    update();
}

QColor BuildingItem::typeColor() const {
    // 统一红色实心圆点
    return QColor(244, 67, 54);  // Material Red 500
}
