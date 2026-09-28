#ifndef CAMPUSNAVIGATOR_DATA_DATABASEMANAGER_H
#define CAMPUSNAVIGATOR_DATA_DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>
#include <vector>
#include <QVector>
#include <QPoint>
#include "model/Building.h"

// ============================================================
//  Road - 道路记录（建筑之间的连接关系）
//  weight = 距离（像素/米），无向边
//  points = 道路中间拐点 JSON 字符串（折线走向）；为空表示直线连接。
// ============================================================
struct Road {
    int    fromId;   // 起点建筑 ID
    int    toId;     // 终点建筑 ID
    double weight;   // 距离权值
    QString points;  // 中间拐点 JSON 字符串（折线走向）
};

// ============================================================
//  DatabaseManager - SQLite 数据库管理类
// ============================================================
//  负责建筑信息的持久化存储：
//    1. 创建/打开 SQLite 数据库文件
//    2. 建表、初始化数据
//    3. 提供查询、增删改查接口
//
//  采用单例模式：整个程序共享一个数据库连接
// ============================================================
class DatabaseManager {
public:
    // 获取单例实例
    static DatabaseManager& instance();

    // 初始化数据库（建表 + 若空则填入默认数据）
    // dbPath: 数据库文件路径，如 "campus.db"
    bool init(const QString& dbPath);

    // 关闭数据库
    void close();

    // --- 查询接口 ---

    // 读取所有建筑（用于加载到 Graph）
    std::vector<Building> loadAllBuildings();

    // 按 id 查询建筑
    bool getBuilding(int id, Building& out);

    // --- 修改接口（管理员模式）---

    // 新增建筑
    bool addBuilding(const Building& b);

    // 更新建筑信息（名称、简介、开放时间等）
    bool updateBuilding(const Building& b);

    // 删除建筑
    bool deleteBuilding(int id);

    // 更新建筑/路口节点坐标（拖拽路口后持久化）
    bool updateBuildingPosition(int id, double x, double y);

    // --- 手动路网编辑（管理员模式"位置可调"功能专用）---
    // 获取下一个可用的路口 id（max(id)+1），避免与已有建筑/路口撞车
    int nextJunctionId() const;
    // 在 (x,y) 处新增一个路口节点（node_kind=0），返回新 id；失败返回 -1
    int addJunction(double x, double y);
    // 删除一个路口节点，同时清理所有包含它的道路
    bool removeJunctionWithRoads(int id);
    // 用欧式距离作为权重在两个节点间建一条路（无向，已存在则忽略）
    bool addRoadEuclidean(int aId, int bId);

    // --- 道路管理接口（新增）---
    // 读取所有道路连接（用于加载到 Graph）
    std::vector<Road> loadAllRoads();

    // 统计路口节点（node_kind=0）数量，用于判断是否已生成路网
    int junctionCount() const;

    // 判断当前路网是否需要（重新）生成/修复：
    //   - 没有任何路口节点；或
    //   - 存在"路口↔路口"的斜向（非正交）边（说明早期网格生成有索引 bug）；或
    //   - 有路口节点跑到场景(1280x976)之外。
    // 满足任一条件返回 true，initMapData 会自动备份并重新生成干净路网。
    bool roadNetworkNeedsRebuild() const;

    // 生成道路网格网络（交通路网模式）：
    //   清空现有道路 → 在建筑分布范围生成网格路口节点 → 连接正交相邻路口
    //   → 每栋建筑连到最近路口。仅重组 roads 表与新增路口节点，
    //   不删除/不修改任何建筑（node_kind=1）。返回是否成功。
    bool generateRoadNetwork();

    // 新增一条道路（无向，自动去重）；道路以直线连接两端
    bool addRoad(int fromId, int toId, double weight);

    // 新增一条带"拐点"的道路（points 为拐点 JSON；空字符串表示直线）
    bool addRoad(int fromId, int toId, double weight, const QString& points);

    // 拐点 <-> JSON 互转（道路折线存储用）
    static QString waypointsToJson(const QVector<QPointF>& wps);
    static QVector<QPointF> jsonToWaypoints(const QString& json);

    // 删除一条道路（双向都删）
    bool deleteRoad(int fromId, int toId);

    // 清空并恢复默认数据（管理员一键重置）
    bool resetToDefaultData();

    // 从 PlaceIntroduction 目录批量导入建筑介绍 txt 文件
    // dirPath: PlaceIntroduction 目录路径
    // 文件名 = 建筑名.txt，内容3行：简介/开放时间/楼层信息
    int importPlaceIntroductions(const QString& dirPath);

    // 返回最后一次错误信息
    QString lastError() const;

    // 返回当前数据库文件路径（用于生成路网前自动备份）
    QString databasePath() const { return dbPath_; }

private:
    DatabaseManager() = default;
    ~DatabaseManager();
    DatabaseManager(const DatabaseManager&) = delete;
    DatabaseManager& operator=(const DatabaseManager&) = delete;

    // 创建表结构
    bool createTable();
    // 填入默认校园数据（首次启动时调用）
    bool seedDefaultData();

    QSqlDatabase db_;
    bool initialized_ = false;
    QString dbPath_;   // 当前数据库文件路径（init 时记录）
};

#endif // CAMPUSNAVIGATOR_DATA_DATABASEMANAGER_H
