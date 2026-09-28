#ifndef CAMPUSNAVIGATOR_VIEW_MAPSCENE_H
#define CAMPUSNAVIGATOR_VIEW_MAPSCENE_H

#include <QGraphicsScene>
#include <QMap>

// 前向声明
class BuildingItem;
class RoadItem;
class IntersectionItem;
class Graph;
class QGraphicsPixmapItem;

// 提前 include，因为 MapScene 内部用到了 RoadItem* 作为 QSet 元素
#include <QSet>
#include <QHash>

// ============================================================
//  MapScene - 校园地图场景（管理所有地图图元）
// ============================================================
//  继承 QGraphicsScene，负责：
//    1. 从 Graph 数据创建所有建筑图元、道路图元、路口图元
//    2. 管理选中状态（起点/终点高亮）
//    3. 高亮显示最短路径
//    4. 管理员"路网编辑模式"下提供路口增删/道路增删的交互入口
//
//  QGraphicsScene 是 Qt 图形视图框架的"画布"——
//  所有图元（BuildingItem、RoadItem、IntersectionItem、角色等）都放在这里，
//  由 QGraphicsView（"相机窗口")负责渲染和交互。
// ============================================================
class MapScene : public QGraphicsScene {
    Q_OBJECT

public:
    explicit MapScene(QObject* parent = nullptr);

    // 从 Graph 数据构建整个校园地图（调用一次即可）
    void loadFromGraph(const Graph& graph);

    // 获取指定建筑的图元
    BuildingItem* getBuildingItem(int buildingId) const;

    // 获取指定路口的图元
    IntersectionItem* getIntersectionItem(int nodeId) const;

    // 获取所有已加载的建筑ID列表（用于批量显示/隐藏）
    QList<int> allBuildingIds() const;

    // 批量操作：隐藏全部建筑 / 显示指定建筑（可同时标红）
    void hideAllBuildings();
    void showOnlyBuildings(const QList<int>& ids, bool selected = false);

    // --- 路径相关操作 ---

    // 高亮显示一条路径（传入节点 id 序列）
    void highlightPath(const std::vector<int>& pathNodeIds);

    // 清除路径高亮
    void clearPathHighlight();

    // 标记起终点
    void setStartPoint(int buildingId);
    void setEndPoint(int buildingId);
    void clearPoints();

    // --- 路网编辑（管理员模式专用）---

    // 切换是否处于"路网编辑模式"；true 时路口可拖动、可点击
    void setNetworkEditMode(bool on);
    bool isNetworkEditMode() const { return networkEditMode_; }

    // 仅控制路口"是否可拖动"。仅"添加路口"子工具需要可拖动，
    // 连接/删除子工具下应置 false，避免点击被误判为拖动而吞掉 clicked 信号。
    void setIntersectionMovable(bool on);

    // 在场景坐标 (x, y) 处新增一个路口图元（落库由 MainWindow 完成；id 由调用方分配）
    bool addIntersectionItem(int nodeId, qreal x, qreal y);
    // 按 id 移除路口（同时删除所有相关道路图元；落库由 MainWindow 完成）
    bool removeIntersection(int nodeId);
    // 在两个节点间加一条道路图元（落库由 MainWindow 完成）
    // waypoints 为折线拐点（场景坐标）；为空则直线连接
    bool addRoadItem(int fromId, int toId, const QVector<QPointF>& waypoints = {});
    // 按 id 对删除道路图元
    bool removeRoadItem(int fromId, int toId);

    // 更新某节点位置时，同步刷新所有连接它的道路
    void updateRoadsForNode(int nodeId, const QPointF& newPos);

    // 临时显示一条"连接预览"线（连接工具选中第一点时跟随鼠标）
    void setConnectionPreview(int fromNodeId, const QPointF& toPos);
    void clearConnectionPreview();

signals:
    // 用户点击建筑时转发出去（由 MainWindow 接收处理）
    void buildingClicked(int buildingId);
    // 用户点击路口时转发出去
    void intersectionClicked(int nodeId);
    // 路口拖拽过程持续发送（实时更新道路显示）
    void intersectionMoved(int nodeId, QPointF newPos);
    // 路口拖拽结束（MainWindow 持久化到数据库）
    void intersectionDragFinished(int nodeId, QPointF finalPos);
    // 路口右键点击（MainWindow 在"删除"工具下删它）
    void intersectionRightClicked(int nodeId);
    // 道路右键点击（MainWindow 在"删除"工具下删它）
    void roadRightClicked(int fromId, int toId);
    // 在空白处按下鼠标（左/右键）— MainWindow 据此判断是添加还是删除
    void emptyScenePressed(QPointF scenePos, Qt::MouseButton button);

protected:
    // 重写以捕获"点到空白处"事件
    void mousePressEvent(QGraphicsSceneMouseEvent* event) override;

private:
    // 存储所有建筑图元：buildingId -> BuildingItem*
    QMap<int, BuildingItem*> buildingItems_;
    // 存储所有道路图元（用于高亮切换）
    QList<RoadItem*> roadItems_;
    // 存储所有路口图元：nodeId -> IntersectionItem*
    QMap<int, IntersectionItem*> intersectionItems_;
    // 缓存：nodeId -> 该节点连着的所有 RoadItem（拖动时快速更新用）
    QHash<int, QSet<RoadItem*>> roadsByNode_;
    // 连接预览线
    QGraphicsLineItem* connectionPreviewLine_ = nullptr;
    // 校园底图（铺满全场景，本身不应拦截鼠标，故单独记录以便空白判定时排除）
    QGraphicsPixmapItem* bgItem_ = nullptr;

    int startPointId_ = -1;   // 当前选中的起点建筑 id
    int endPointId_   = -1;   // 当前选中的终点建筑 id
    bool networkEditMode_ = false;   // 是否处于管理员路网编辑模式
    int  previewFromId_ = -1;       // 连接预览的源节点 id
};

#endif // CAMPUSNAVIGATOR_VIEW_MAPSCENE_H
