#ifndef CAMPUSNAVIGATOR_MODEL_EDGE_H
#define CAMPUSNAVIGATOR_MODEL_EDGE_H

#include <QVector>
#include <QPoint>

// ============================================================
//  Edge - 图的边（道路）
// ============================================================
//  表示从某个节点出发的一条有向边。
//  校园道路是无向的，在 Graph 里用 addEdge 双向添加即可。
//
//  天气标签（用于天气智能导航）：
//    shelter - 有连廊/遮雨棚覆盖
//    tree    - 树荫等级 0(无)~3(茂密)
//    lake    - 临湖大风区
//
//  道路支持在两端建筑之间添加若干"拐点"(waypoints)，
//  以折线形式绘制出弯曲的道路（如绕过湖泊、建筑等）。
// ============================================================
struct Edge {
    int    to;       // 边的终点（目标建筑 id）
    double weight;   // 权重（道路距离，单位：米，用于最短路径计算）

    // 天气标签（默认值，不影响原有逻辑）
    bool   shelter = false;  // 有连廊遮雨棚
    int    tree = 0;         // 树荫等级 0-3
    bool   lake = false;     // 临湖大风区

    // 道路拐点（地图画路时按序点击加入，折线绘制用）
    QVector<QPointF> waypoints;

    // 原有构造函数（保持兼容）
    Edge(int to, double weight) : to(to), weight(weight) {}

    // 带拐点（折线）的构造函数
    Edge(int to, double weight, const QVector<QPointF>& wps)
        : to(to), weight(weight), waypoints(wps) {}

    // 带天气标签的构造函数
    Edge(int to, double weight, bool s, int t, bool l)
        : to(to), weight(weight), shelter(s), tree(t), lake(l) {}

    // 带天气标签 + 拐点的构造函数
    Edge(int to, double weight, bool s, int t, bool l, const QVector<QPointF>& wps)
        : to(to), weight(weight), shelter(s), tree(t), lake(l), waypoints(wps) {}
};

#endif // CAMPUSNAVIGATOR_MODEL_EDGE_H
