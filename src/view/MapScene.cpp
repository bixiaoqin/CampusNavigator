#include "view/MapScene.h"

#include "view/BuildingItem.h"
#include "view/RoadItem.h"
#include "view/IntersectionItem.h"
#include "model/Building.h"
#include "model/Edge.h"
#include "model/Graph.h"

#include <QBrush>
#include <QGraphicsPixmapItem>
#include <QGraphicsLineItem>
#include <QGraphicsSceneMouseEvent>
#include <QPixmap>
#include <QSet>
#include <QHash>
#include <cmath>
#include <vector>

// ============================================================
//  MapScene 实现
// ============================================================

MapScene::MapScene(QObject* parent)
    : QGraphicsScene(parent)
{
    // 场景底色：淡绿色草地，与地图边缘自然过渡
    setBackgroundBrush(QColor(232, 245, 233));   // 浅绿 #E8F5E9
}

// 从 Graph 数据构建整个校园地图
void MapScene::loadFromGraph(const Graph& graph) {
    // 清空旧数据（如果重复调用）
    clear();
    buildingItems_.clear();
    roadItems_.clear();
    intersectionItems_.clear();
    roadsByNode_.clear();
    connectionPreviewLine_ = nullptr;
    bgItem_ = nullptr;
    startPointId_ = -1;
    endPointId_ = -1;
    previewFromId_ = -1;

    // 0. 添加校园地图背景底图（最底层，Z=-2）
    //    依靠地图自带的斜俯视透视效果达成 2.5D 视觉
    QPixmap bgPixmap(QStringLiteral(":/assets/tjut_map.jpg"));
    if (!bgPixmap.isNull()) {
        QGraphicsPixmapItem* bgItem = new QGraphicsPixmapItem(bgPixmap);
        bgItem->setZValue(-2);
        bgItem->setPos(0, 0);
        // 底图铺满全场景，必须关闭鼠标交互，否则会"吃掉"所有空白处点击，
        // 导致路网编辑模式在草地点击无法触发 emptyScenePressed。
        bgItem->setAcceptedMouseButtons(Qt::NoButton);
        bgItem->setAcceptHoverEvents(false);
        bgItem->setEnabled(false);
        addItem(bgItem);
        bgItem_ = bgItem;
    }

    // 1. 先创建所有道路图元（放在下层，Z=-1）
    for (const auto& [id, building] : graph.allBuildings()) {
        const auto& edges = graph.neighbors(id);
        const Building* b = &building;

        for (const Edge& e : edges) {
            // 避免双向边重复绘制：只画 id 较小的方向
            if (e.to <= id) continue;

            const Building* nb = graph.getBuilding(e.to);
            if (!nb) continue;

            // 传入两端建筑 ID，便于路径高亮时精确匹配
            RoadItem* road = new RoadItem(
                b->x(), b->y(), nb->x(), nb->y(), e.weight,
                id, e.to);
            road->setDeletable(networkEditMode_);   // 编辑模式下允许右键删
            connect(road, &RoadItem::rightClicked,
                    this, &MapScene::roadRightClicked);
            addItem(road);
            roadItems_.append(road);

            // 加入拐点（折线绘制）：若边带 waypoints，逐点加到 RoadItem
            for (const QPointF& wp : e.waypoints) {
                road->addWaypoint(wp);
            }

            // 维护"节点→道路"反查表，供路口拖动时实时更新
            roadsByNode_[id].insert(road);
            roadsByNode_[e.to].insert(road);
        }
    }

    // 2. 再创建所有建筑图元（放在上层，Z=0）
    //    nodeKind=0 的路口节点不显示名称/弹窗，跳过建筑图元创建
    for (const auto& [id, building] : graph.allBuildings()) {
        if (building.nodeKind() == 0) continue;   // 路口节点不创建图元

        BuildingItem* item = new BuildingItem(&building);
        connect(item, &BuildingItem::clicked,
                this, &MapScene::buildingClicked);

        addItem(item);
        buildingItems_[id] = item;
    }

    // 2.5 创建可交互的路口图元（替换原哑火 QGraphicsEllipseItem）
    //    默认 setEditable(false) — 普通模式下不响应鼠标
    for (const auto& [id, building] : graph.allBuildings()) {
        if (building.nodeKind() != 0) continue;   // 只有路口节点创建图元

        IntersectionItem* it = new IntersectionItem(id);
        it->setPos(building.x(), building.y());
        it->setEditable(networkEditMode_);         // 跟随当前模式
        addItem(it);
        intersectionItems_[id] = it;

        // 转发信号到 MapScene 的信号（供 MainWindow 接收）
        connect(it, &IntersectionItem::positionChanged,
                this, &MapScene::intersectionMoved);
        connect(it, &IntersectionItem::dragFinished,
                this, &MapScene::intersectionDragFinished);
        connect(it, &IntersectionItem::clicked,
                this, &MapScene::intersectionClicked);
        connect(it, &IntersectionItem::rightClicked,
                this, &MapScene::intersectionRightClicked);
    }

    // 3. 场景大小 = 地图图片大小（1280×976），确保完整显示底图
    setSceneRect(0, 0, 1280, 976);
}

// 按 id 查建筑图元
BuildingItem* MapScene::getBuildingItem(int buildingId) const {
    return buildingItems_.value(buildingId, nullptr);
}

// 按 id 查路口图元
IntersectionItem* MapScene::getIntersectionItem(int nodeId) const {
    return intersectionItems_.value(nodeId, nullptr);
}

// 获取所有已加载的建筑ID列表
QList<int> MapScene::allBuildingIds() const {
    return buildingItems_.keys();
}

// 隐藏全部建筑图元
void MapScene::hideAllBuildings() {
    for (auto it = buildingItems_.begin(); it != buildingItems_.end(); ++it) {
        it.value()->setVisible(false);
    }
}

// 只显示指定ID的建筑（可同时标红），其余保持隐藏
void MapScene::showOnlyBuildings(const QList<int>& ids, bool selected) {
    for (int id : ids) {
        BuildingItem* item = buildingItems_.value(id, nullptr);
        if (item) {
            item->setVisible(true);
            if (selected) item->setSelected(true);
        }
    }
}

// 高亮显示最短路径（传入节点 id 序列）
void MapScene::highlightPath(const std::vector<int>& pathNodeIds) {
    // 清除建筑选中状态
    for (auto it = buildingItems_.begin(); it != buildingItems_.end(); ++it) {
        it.value()->setSelected(false);
    }
    // 清除所有道路高亮/淡化
    for (RoadItem* road : roadItems_) {
        road->setHighlighted(false);
        road->setFaded(false);
    }

    if (pathNodeIds.size() < 2) return;

    // 高亮新路径上的道路
    for (size_t i = 0; i + 1 < pathNodeIds.size(); ++i) {
        int from = pathNodeIds[i];
        int to   = pathNodeIds[i + 1];

        for (RoadItem* road : roadItems_) {
            if (road->connects(from, to)) {
                road->setHighlighted(true);
                break;
            }
        }
    }
}

// 清除路径高亮（完全清除，包括淡化）
void MapScene::clearPathHighlight() {
    for (auto it = buildingItems_.begin(); it != buildingItems_.end(); ++it) {
        it.value()->setSelected(false);
    }
    for (RoadItem* road : roadItems_) {
        road->setHighlighted(false);
        road->setFaded(false);
    }
}

// 设置起点
void MapScene::setStartPoint(int buildingId) {
    if (startPointId_ >= 0) {
        BuildingItem* old = getBuildingItem(startPointId_);
        if (old) old->setSelected(false);
    }
    startPointId_ = buildingId;
    BuildingItem* item = getBuildingItem(buildingId);
    if (item) item->setSelected(true);
}

// 设置终点
void MapScene::setEndPoint(int buildingId) {
    endPointId_ = buildingId;
}

// 清除起终点
void MapScene::clearPoints() {
    startPointId_ = -1;
    endPointId_ = -1;
    clearPathHighlight();
}

// ============================================================
//  路网编辑模式（管理员用）
// ============================================================

void MapScene::setNetworkEditMode(bool on) {
    if (networkEditMode_ == on) return;
    networkEditMode_ = on;
    for (auto it = intersectionItems_.begin(); it != intersectionItems_.end(); ++it) {
        it.value()->setEditable(on);
    }
    for (RoadItem* r : roadItems_) {
        r->setDeletable(on);
    }
    if (!on) {
        clearConnectionPreview();
    }
}

void MapScene::setIntersectionMovable(bool on) {
    for (auto it = intersectionItems_.begin(); it != intersectionItems_.end(); ++it) {
        it.value()->setMovable(on);
    }
}

bool MapScene::addIntersectionItem(int nodeId, qreal x, qreal y) {
    if (intersectionItems_.contains(nodeId)) return false;
    IntersectionItem* it = new IntersectionItem(nodeId);
    it->setPos(x, y);
    it->setEditable(networkEditMode_);
    addItem(it);
    intersectionItems_[nodeId] = it;
    // 转发信号
    connect(it, &IntersectionItem::positionChanged,
            this, &MapScene::intersectionMoved);
    connect(it, &IntersectionItem::dragFinished,
            this, &MapScene::intersectionDragFinished);
    connect(it, &IntersectionItem::clicked,
            this, &MapScene::intersectionClicked);
    return true;
}

bool MapScene::removeIntersection(int nodeId) {
    auto it = intersectionItems_.find(nodeId);
    if (it == intersectionItems_.end()) return false;
    // 先删所有连到这个节点的道路图元（图元删除会一并清理 roadsByNode_）
    auto roadIt = roadsByNode_.find(nodeId);
    if (roadIt != roadsByNode_.end()) {
        // 复制一份集合（removeRoadItem 会改 roadsByNode_）
        QSet<RoadItem*> toDelete = roadIt.value();
        for (RoadItem* r : toDelete) {
            int other = (r->fromId() == nodeId) ? r->toId() : r->fromId();
            removeRoadItem(nodeId, other);
        }
    }
    // 再删路口图元
    removeItem(it.value());
    delete it.value();
    intersectionItems_.erase(it);
    return true;
}

bool MapScene::addRoadItem(int fromId, int toId, const QVector<QPointF>& waypoints) {
    if (fromId == toId) return false;
    // 已存在就跳过
    for (RoadItem* r : roadItems_) {
        if (r->connects(fromId, toId)) return false;
    }
    // 找两端坐标：路口优先，否则用建筑图元
    QPointF a, b;
    auto lookup = [&](int id, QPointF& out) -> bool {
        if (auto* ii = intersectionItems_.value(id, nullptr)) {
            out = ii->pos(); return true;
        }
        if (auto* bi = buildingItems_.value(id, nullptr)) {
            out = bi->pos(); return true;
        }
        return false;
    };
    if (!lookup(fromId, a) || !lookup(toId, b)) return false;
    double w = std::hypot(a.x() - b.x(), a.y() - b.y());
    RoadItem* road = new RoadItem(a.x(), a.y(), b.x(), b.y(), w, fromId, toId);
    // 折线拐点
    for (const QPointF& wp : waypoints) {
        road->addWaypoint(wp);
    }
    addItem(road);
    roadItems_.append(road);
    roadsByNode_[fromId].insert(road);
    roadsByNode_[toId].insert(road);
    return true;
}

bool MapScene::removeRoadItem(int fromId, int toId) {
    for (int i = 0; i < roadItems_.size(); ++i) {
        RoadItem* r = roadItems_[i];
        if (r && r->connects(fromId, toId)) {
            roadsByNode_[fromId].remove(r);
            roadsByNode_[toId].remove(r);
            removeItem(r);
            delete r;
            roadItems_.removeAt(i);
            return true;
        }
    }
    return false;
}

void MapScene::updateRoadsForNode(int nodeId, const QPointF& newPos) {
    auto it = roadsByNode_.find(nodeId);
    if (it == roadsByNode_.end()) return;
    for (RoadItem* r : it.value()) {
        if (!r) continue;
        if (r->fromId() == nodeId) {
            r->updateEndpoints(newPos.x(), newPos.y(), r->x2(), r->y2());
        } else if (r->toId() == nodeId) {
            r->updateEndpoints(r->x1(), r->y1(), newPos.x(), newPos.y());
        }
    }
}

void MapScene::setConnectionPreview(int fromNodeId, const QPointF& toPos) {
    previewFromId_ = fromNodeId;
    if (!connectionPreviewLine_) {
        connectionPreviewLine_ = new QGraphicsLineItem();
        connectionPreviewLine_->setZValue(2);
        QPen pen(QColor(255, 100, 100), 2, Qt::DashLine);
        connectionPreviewLine_->setPen(pen);
        addItem(connectionPreviewLine_);
    }
    QPointF fromPos;
    if (auto* ii = intersectionItems_.value(fromNodeId, nullptr)) {
        fromPos = ii->pos();
    } else if (auto* bi = buildingItems_.value(fromNodeId, nullptr)) {
        fromPos = bi->pos();
    } else {
        clearConnectionPreview();
        return;
    }
    connectionPreviewLine_->setLine(fromPos.x(), fromPos.y(), toPos.x(), toPos.y());
    connectionPreviewLine_->setVisible(true);
}

void MapScene::clearConnectionPreview() {
    if (connectionPreviewLine_) {
        connectionPreviewLine_->setVisible(false);
    }
    previewFromId_ = -1;
}

// 在场景空白处按下时触发（连接工具下作为"鼠标跟随"端点 / 添加工具下作为新路口位置）
void MapScene::mousePressEvent(QGraphicsSceneMouseEvent* event) {
    // —— 右键删除（路网编辑模式下）：由场景层直接接管，最可靠 ——
    // 关键：用 items()（该位置下"全部"图元，按 Z 从高到低）而非 itemAt()
    // （仅最顶层）。因为天气蒙版 WeatherOverlay(Z=100) / 夜间蒙版(Z=100) 始终
    // 盖在路口节点(Z=1)之上，itemAt() 会返回蒙版而漏掉下方的路口，导致
    // "右键节点无反应、只能删道路"。遍历全部图元即可绕过装饰层遮挡。
    if (event->button() == Qt::RightButton && networkEditMode_) {
        const QList<QGraphicsItem*> hits = items(event->scenePos());
        // 优先：命中路口节点 → 删节点（连带的道路由 removeIntersection 一并清理）
        for (QGraphicsItem* h : hits) {
            for (IntersectionItem* n : intersectionItems_) {
                if (static_cast<QGraphicsItem*>(n) == h) {
                    emit intersectionRightClicked(n->nodeId());
                    return;   // 不再派发给图元，由 MainWindow 弹窗确认后删除
                }
            }
        }
        // 没点中路口：交给基类（道路图元 RoadItem 自身处理右键删除）
        QGraphicsScene::mousePressEvent(event);
        return;
    }

    // 判断该点是否"点中可交互图元"。场景里有很多铺满全屏的装饰层
    // （底图 bgItem_、天气层 WeatherOverlay、夜间蒙版 nightOverlay_ 等），
    // 它们会被 itemAt() 当作"命中物"返回，从而把空白处误判为"点中图元"。
    // 因此不能简单用 itemAt()==nullptr 判断空白，而要看该点是否落在
    // 真正的可交互图元（建筑 / 道路 / 路口）上。没有则可视为"空白处"。
    QList<QGraphicsItem*> itemsHere = items(event->scenePos());
    bool hitInteractive = false;
    for (QGraphicsItem* it : itemsHere) {
        bool found = false;
        for (BuildingItem* b : buildingItems_)
            if (static_cast<QGraphicsItem*>(b) == it) { found = true; break; }
        if (!found) for (RoadItem* r : roadItems_)
            if (static_cast<QGraphicsItem*>(r) == it) { found = true; break; }
        if (!found) for (IntersectionItem* n : intersectionItems_)
            if (static_cast<QGraphicsItem*>(n) == it) { found = true; break; }
        if (found) { hitInteractive = true; break; }
    }
    if (!hitInteractive) {
        // 点到空白处（或只点到了装饰层）：转发给 MainWindow
        emit emptyScenePressed(event->scenePos(), event->button());
    }
    QGraphicsScene::mousePressEvent(event);
}
