#ifndef CAMPUSNAVIGATOR_DATA_CAMPUSDATA_H
#define CAMPUSNAVIGATOR_DATA_CAMPUSDATA_H

#include "model/Graph.h"

// ============================================================
//  CampusData - 校园原始数据
// ============================================================
//  这个模块负责把"虚拟校园"的建筑和道路数据填充到 Graph 里。
//
//  为什么单独抽出来？
//    1. 数据与逻辑分离：以后改数据（加建筑/改坐标）只动这里
//    2. 阶段6接入 SQLite 后，可以从数据库读取替代硬编码
//    3. 让 Graph 类保持纯粹的算法职责
//
//  本虚拟校园含 45 栋建筑（≥ 任务书要求的 15 栋），
//  道路连接构成连通图，足以演示最短路径规划。
// ============================================================
namespace CampusData {

// 把所有建筑和道路填充进传入的 graph 对象
void populate(Graph& graph);

} // namespace CampusData

#endif // CAMPUSNAVIGATOR_DATA_CAMPUSDATA_H
