#include "model/Graph.h"

#include <queue>        // priority_queue（Dijkstra 用，阶段4会用上）
#include <limits>       // numeric_limits（无穷大）
#include <vector>
#include <stdexcept>
#include <cmath>        // sqrt（updatePosition 计算边权重用）
#include <algorithm>    // std::remove_if（removeRoad 用）

// ============================================================
//  Graph 类的实现
// ============================================================

// 添加建筑：插入到 buildings_ 映射表
void Graph::addBuilding(const Building& b) {
    buildings_.insert({b.id(), b});
    // 同时为该节点初始化一个空的邻接边列表
    // （否则孤立建筑调用 neighbors() 时 at() 会抛异常）
    adjacency_.try_emplace(b.id());
}

// 按 id 查建筑
// 用 find 避免误插入（operator[] 在 key 不存在时会插入空值）
const Building* Graph::getBuilding(int id) const {
    auto it = buildings_.find(id);
    if (it == buildings_.end()) {
        return nullptr;   // 没找到
    }
    return &(it->second);
}

// 按名称查建筑（精确匹配）
// 遍历 buildings_ 比对 name()，用于路线/模式按名称定位，
// 避免用户增删改建筑后硬编码 ID 错位导致功能失效。
const Building* Graph::getBuildingByName(const QString& name) const {
    for (const auto& [id, b] : buildings_) {
        if (b.name() == name) {
            return &b;
        }
    }
    return nullptr;   // 没找到
}

// 添加无向道路：两个方向各加一条边
void Graph::addRoad(int fromId, int toId, double distance) {
    // 校验两端建筑都存在（防止硬编码数据出错）
    if (buildings_.find(fromId) == buildings_.end() ||
        buildings_.find(toId)   == buildings_.end()) {
        return;   // 静默跳过无效道路（阶段6可改为抛异常）
    }
    adjacency_[fromId].emplace_back(toId, distance);
    adjacency_[toId].emplace_back(fromId, distance);
    ++roadCount_;
}

// 添加带"拐点"的无向道路（折线绘制用）
void Graph::addRoad(int fromId, int toId, double distance,
                    const QVector<QPointF>& waypoints) {
    if (buildings_.find(fromId) == buildings_.end() ||
        buildings_.find(toId)   == buildings_.end()) {
        return;
    }
    adjacency_[fromId].emplace_back(toId, distance, waypoints);
    adjacency_[toId].emplace_back(fromId, distance, waypoints);
    ++roadCount_;
}

// 添加带天气标签的无向道路
void Graph::addRoad(int fromId, int toId, double distance,
                    bool shelter, int tree, bool lake) {
    if (buildings_.find(fromId) == buildings_.end() ||
        buildings_.find(toId)   == buildings_.end()) {
        return;
    }
    adjacency_[fromId].emplace_back(toId, distance, shelter, tree, lake);
    adjacency_[toId].emplace_back(fromId, distance, shelter, tree, lake);
    ++roadCount_;
}

// 获取从 a 到 b 的道路点列，按 a->b 方向有序。
// 若 a->b 的边带 waypoints 拐点，则连成折线（起点 + 拐点 + 终点）；
// 否则仅返回起、终两点直线。
bool Graph::getRoadPolyline(int a, int b, QVector<QPointF>& poly) const {
    poly.clear();
    const Building* ba = getBuilding(a);
    const Building* bb = getBuilding(b);
    if (!ba || !bb) return false;

    QPointF start(ba->x(), ba->y());
    QPointF end(bb->x(), bb->y());

    // 查找 a->b 的边，若带拐点则连成折线，否则仅两点直线
    const auto& edges = neighbors(a);
    for (const Edge& e : edges) {
        if (e.to == b) {
            if (!e.waypoints.isEmpty()) {
                poly.append(start);
                for (const QPointF& wp : e.waypoints) poly.append(wp);
                poly.append(end);
            } else {
                poly.append(start);
                poly.append(end);
            }
            return true;
        }
    }
    // 找不到直接相连的边，兜底返回两点直线
    poly.append(start);
    poly.append(end);
    return false;
}

// 取邻居列表
const std::vector<Edge>& Graph::neighbors(int id) const {
    // at() 比 operator[] 安全，key 不存在时抛异常而非插入
    return adjacency_.at(id);
}

// 返回离建筑最近的路口节点 id（欧几里得距离最小）
int Graph::nearestJunction(int buildingId) const {
    const Building* b = getBuilding(buildingId);
    if (!b) return -1;
    int best = -1;
    double bestDist = std::numeric_limits<double>::max();
    for (const auto& [id, node] : buildings_) {
        if (node.nodeKind() != 0) continue;   // 只找路口节点
        double dx = node.x() - b->x();
        double dy = node.y() - b->y();
        double d = dx * dx + dy * dy;
        if (d < bestDist) {
            bestDist = d;
            best = id;
        }
    }
    return best;
}

// 更新节点坐标并重算相关边权重（拖拽路口时调用）
void Graph::updatePosition(int id, double x, double y) {
    auto it = buildings_.find(id);
    if (it == buildings_.end()) return;
    it->second.setPosition(x, y);

    // 重算所有与该节点相关的边权重（双向）
    // 1. 正向边：从 id 出发的边
    auto& outEdges = adjacency_[id];
    for (auto& e : outEdges) {
        const Building* other = getBuilding(e.to);
        if (!other) continue;
        double dx = x - other->x();
        double dy = y - other->y();
        e.weight = std::sqrt(dx * dx + dy * dy);
    }
    // 2. 反向边：其他节点指向 id 的边
    for (auto& [otherId, edgeList] : adjacency_) {
        if (otherId == id) continue;
        for (auto& e : edgeList) {
            if (e.to == id) {
                const Building* other = getBuilding(otherId);
                if (!other) continue;
                double dx = x - other->x();
                double dy = y - other->y();
                e.weight = std::sqrt(dx * dx + dy * dy);
            }
        }
    }
}

// 移除一条无向道路（双向都删），并更新道路计数
void Graph::removeRoad(int fromId, int toId) {
    auto delFromList = [&](int node, int other) {
        auto it = adjacency_.find(node);
        if (it == adjacency_.end()) return;
        auto& v = it->second;
        auto sz = v.size();
        v.erase(std::remove_if(v.begin(), v.end(),
                               [&](const Edge& e){ return e.to == other; }),
                v.end());
        if (v.size() != sz) --roadCount_;
    };
    delFromList(fromId, toId);
    delFromList(toId,   fromId);
}

// 移除一栋建筑（同时清理所有连到它的道路）
void Graph::removeBuilding(int id) {
    // 1. 收集所有连到该节点的道路
    std::vector<int> neighbors;
    auto itAdj = adjacency_.find(id);
    if (itAdj != adjacency_.end()) {
        for (const Edge& e : itAdj->second) neighbors.push_back(e.to);
    }
    // 2. 删该节点出发的所有边（双向）
    for (int n : neighbors) removeRoad(id, n);
    // 3. 删邻接表条目
    adjacency_.erase(id);
    // 4. 删建筑
    buildings_.erase(id);
}

// 返回无向道路条数
int Graph::roadCount() const {
    return roadCount_;
}

// ============================================================
//  Dijkstra 最短路径算法
// ============================================================
//  这里提前实现了——有了图数据顺便把路径算出来，阶段2的
//  验证程序就能展示"从正门到图书馆最短怎么走"，效果直观。
//
//  算法思路（经典 Dijkstra）：
//    1. 维护 dist[]：起点到每个节点的当前最短距离，初始无穷大
//    2. 维护 prev[]：每个节点在最短路径上的前驱节点
//    3. 用最小堆（优先队列）每次取出当前距离最小的未确定节点
//    4. 用它松弛（relax）所有邻居：若经过它到邻居更短，则更新
//    5. 重复直到处理完终点
//
//  时间复杂度：O((V+E) log V)，V=节点数，E=边数
// ============================================================
std::vector<int> Graph::dijkstra(int startId, int endId) const {
    // 起终点合法性检查
    if (buildings_.find(startId) == buildings_.end() ||
        buildings_.find(endId)   == buildings_.end()) {
        return {};
    }
    if (startId == endId) {
        return {startId};
    }

    // dist[v] = 起点到 v 的当前最短距离
    std::unordered_map<int, double> dist;
    // prev[v] = v 在最短路径上的前驱节点
    std::unordered_map<int, int> prev;
    // visited[v] = v 是否已确定最短路径
    std::unordered_map<int, bool> visited;

    // 初始化：所有距离设为无穷大
    const double INF = std::numeric_limits<double>::infinity();
    for (const auto& kv : buildings_) {
        dist[kv.first] = INF;
    }
    dist[startId] = 0.0;

    // 最小堆：pair<距离, 节点id>，按距离从小到大出队
    // greater 使 priority_queue 变成最小堆（默认是最大堆）
    using PD = std::pair<double, int>;
    std::priority_queue<PD, std::vector<PD>, std::greater<PD>> pq;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();   // C++17 结构化绑定，拆开 pair
        pq.pop();

        if (visited[u]) continue;   // 旧记录跳过（堆里可能有重复）
        visited[u] = true;

        if (u == endId) break;      // 已确定终点最短路径，提前结束

        // 松弛所有邻居
        const auto& edges = neighbors(u);
        for (const Edge& e : edges) {
            double nd = d + e.weight;   // 经过 u 到 e.to 的距离
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }

    // 回溯路径：从终点沿 prev 反向走回起点
    std::vector<int> path;
    if (dist[endId] == INF) {
        return path;   // 不可达
    }
    for (int cur = endId; cur != startId; cur = prev[cur]) {
        path.push_back(cur);
    }
    path.push_back(startId);
    std::reverse(path.begin(), path.end());   // 反转成正向
    return path;
}

// ============================================================
//  天气感知 Dijkstra：根据天气模式调整边权重后规划路线
//  weatherMode: 0=晴 1=雨 2=高温 3=大风
//    雨天：shelter=true的路权重×0.7（优先走连廊遮雨棚）
//    高温：tree等级高的路权重×(1-0.1*tree)（优先走树荫密集步道）
//    大风：lake=true的路权重×1.5（绕行临湖大风路段）
//    晴天：不调整（等同原dijkstra）
// ============================================================
std::vector<int> Graph::dijkstraWithWeather(int startId, int endId, int weatherMode) const {
    if (buildings_.find(startId) == buildings_.end() ||
        buildings_.find(endId)   == buildings_.end()) {
        return {};
    }
    if (startId == endId) return {startId};

    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    std::unordered_map<int, bool> visited;

    const double INF = std::numeric_limits<double>::infinity();
    for (const auto& kv : buildings_) dist[kv.first] = INF;
    dist[startId] = 0.0;

    using PD = std::pair<double, int>;
    std::priority_queue<PD, std::vector<PD>, std::greater<PD>> pq;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (visited[u]) continue;
        visited[u] = true;
        if (u == endId) break;

        for (const Edge& e : neighbors(u)) {
            // 根据天气模式调整权重
            double w = e.weight;
            if (weatherMode == 1 && e.shelter) {
                w *= 0.7;   // 雨天：有遮雨棚的路更优
            } else if (weatherMode == 2) {
                w *= (1.0 - 0.1 * e.tree);  // 高温：树荫多的路更优
            } else if (weatherMode == 3 && e.lake) {
                w *= 1.5;   // 大风：临湖路段权重增大（绕行）
            }

            double nd = d + w;
            if (nd < dist[e.to]) {
                dist[e.to] = nd;
                prev[e.to] = u;
                pq.push({nd, e.to});
            }
        }
    }

    std::vector<int> path;
    if (dist[endId] == INF) return path;
    for (int cur = endId; cur != startId; cur = prev[cur]) path.push_back(cur);
    path.push_back(startId);
    std::reverse(path.begin(), path.end());
    return path;
}

// ============================================================
//  只走路口节点（node_kind==0）的 Dijkstra
//  用途：角色沿真实路网行走。路网里"建筑"也连着路口，若不限制，
//  普通 Dijkstra 会把建筑中心当成可穿过的"中转站"（例如 1009→18→1015），
//  导致角色走到建筑上。本函数只允许在路口节点间移动，保证角色始终在道路上。
// ============================================================
std::vector<int> Graph::dijkstraJunctions(int startId, int endId) const {
    const Building* s = getBuilding(startId);
    const Building* e = getBuilding(endId);
    if (!s || !e) return {};
    // 只接受路口节点作为起终点
    if (s->nodeKind() != 0 || e->nodeKind() != 0) return {};
    if (startId == endId) return {startId};

    std::unordered_map<int, double> dist;
    std::unordered_map<int, int> prev;
    const double INF = std::numeric_limits<double>::infinity();

    // 只初始化路口节点
    for (const auto& kv : buildings_) {
        if (kv.second.nodeKind() == 0) dist[kv.first] = INF;
    }
    dist[startId] = 0.0;

    using PD = std::pair<double, int>;
    std::priority_queue<PD, std::vector<PD>, std::greater<PD>> pq;
    pq.push({0.0, startId});

    while (!pq.empty()) {
        auto [d, u] = pq.top();
        pq.pop();
        if (d > dist[u]) continue;
        if (u == endId) break;

        for (const Edge& ed : neighbors(u)) {
            const Building* nb = getBuilding(ed.to);
            // 只允许走到另一个路口节点，跳过建筑（建筑只作门口接入，不可穿过）
            if (!nb || nb->nodeKind() != 0) continue;

            double nd = d + ed.weight;
            if (nd < dist[ed.to]) {
                dist[ed.to] = nd;
                prev[ed.to] = u;
                pq.push({nd, ed.to});
            }
        }
    }

    if (dist[endId] == INF) return {};

    std::vector<int> path;
    for (int cur = endId; cur != startId; cur = prev[cur]) path.push_back(cur);
    path.push_back(startId);
    std::reverse(path.begin(), path.end());
    return path;
}
