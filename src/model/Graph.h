#ifndef CAMPUSNAVIGATOR_MODEL_GRAPH_H
#define CAMPUSNAVIGATOR_MODEL_GRAPH_H

#include <QHash>          // Qt 版的哈希表（对 int 键优化）
#include <QVector>        // Qt 版的动态数组（对应 std::vector）
#include <unordered_map>  // STL 哈希表
#include <vector>

#include "model/Building.h"
#include "model/Edge.h"

// ============================================================
//  Graph - 校园路网图
// ============================================================
//  用邻接表（adjacency list）存储路网：
//    节点 = 建筑（Building）
//    边   = 道路（Edge），带权重（距离）
//
//  邻接表比邻接矩阵更适合稀疏图（校园道路数量远小于
//  节点数的平方），空间更省、遍历邻居更快。
//
//  本类同时管理两类数据：
//    1. buildings_: id -> Building 映射，存所有建筑信息
//    2. adjacency_: id -> 邻接边列表，存道路连接关系
//
//  Dijkstra 最短路径算法留到阶段4实现，这里先留接口。
// ============================================================
class Graph {
public:
    Graph() = default;

    // --- 建筑管理 ---
    // 添加一栋建筑到图中（成为路网的一个节点）
    void addBuilding(const Building& b);

    // 按 id 取建筑（找不到返回 nullptr）
    const Building* getBuilding(int id) const;

    // 按名称取建筑（名称精确匹配，找不到返回 nullptr）
    // 用途：路线/模式用建筑"名称"而非写死ID，避免用户增删改建筑后ID错位
    const Building* getBuildingByName(const QString& name) const;

    // 取所有建筑（供界面遍历显示用）
    const std::unordered_map<int, Building>& allBuildings() const { return buildings_; }

    // --- 道路管理 ---
    // 添加一条无向道路（自动双向加边，因为路可以双向走）
    void addRoad(int fromId, int toId, double distance);

    // 添加带"拐点"的无向道路（折线绘制用，waypoints 为中间点序列）
    void addRoad(int fromId, int toId, double distance,
                 const QVector<QPointF>& waypoints);

    // 添加带天气标签的道路（shelter/tree/lake）
    void addRoad(int fromId, int toId, double distance,
                 bool shelter, int tree, bool lake);

    // 获取从建筑 a 到建筑 b 的道路点列（按 a->b 方向有序），
    // 若边带 waypoints 拐点则连成折线，否则仅返回起、终两点直线。
    // 找不到返回 false。供角色沿路行走使用。
    bool getRoadPolyline(int a, int b, QVector<QPointF>& poly) const;

    // 取某节点的所有邻接边（邻居）
    const std::vector<Edge>& neighbors(int id) const;

    // 移除一条无向道路（双向都删），并更新道路计数
    void removeRoad(int fromId, int toId);

    // 移除一栋建筑（同时清理所有连到它的道路）
    void removeBuilding(int id);

    // 返回离指定建筑最近的"路口节点"（nodeKind=0）的 id，
    // 用于导航时让角色从建筑门口（最近路口）起步，而不是站在建筑中心。
    // 找不到路口返回 -1。
    int nearestJunction(int buildingId) const;

    // 更新节点坐标并重算相关边权重（拖拽路口时调用）
    void updatePosition(int id, double x, double y);

    // --- 统计信息 ---
    int buildingCount() const { return static_cast<int>(buildings_.size()); }
    int roadCount() const;   // 双向算一条，返回无向道路数

    // --- 最短路径（阶段4实现，先声明接口）---
    // 返回从 start 到 end 的最短路径上的节点 id 序列
    // 找不到路径返回空 vector
    std::vector<int> dijkstra(int startId, int endId) const;

    // 天气感知最短路径：根据天气模式调整权重后规划路线
    // weatherMode: 0=晴 1=雨 2=高温 3=大风
    std::vector<int> dijkstraWithWeather(int startId, int endId, int weatherMode) const;

    // 只走"路口节点"（node_kind==0）的最短路径，专供角色沿真实路网行走使用。
    // 关键：寻路时只经过灰色道路（路口网格），绝不会把建筑中心当成可穿过的路，
    // 因此角色永远走在道路上、不会出现在建筑上。start/end 必须是路口节点，
    // 否则返回空序列。
    std::vector<int> dijkstraJunctions(int startId, int endId) const;

private:
    // id -> 建筑
    std::unordered_map<int, Building> buildings_;

    // id -> 邻接边列表（邻接表）
    std::unordered_map<int, std::vector<Edge>> adjacency_;

    // 记录无向道路数（每次 addRoad +1）
    int roadCount_ = 0;
};

#endif // CAMPUSNAVIGATOR_MODEL_GRAPH_H
