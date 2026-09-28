#include "data/DatabaseManager.h"
#include "data/CampusData.h"
#include "model/Graph.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QDateTime>
#include <QTextStream>
#include <QJsonDocument>
#include <QJsonArray>
#include <cmath>
#include <algorithm>
#include <unordered_map>

// ============================================================
//  DatabaseManager 实现
// ============================================================

DatabaseManager& DatabaseManager::instance() {
    static DatabaseManager inst;
    return inst;
}

DatabaseManager::~DatabaseManager() {
    close();
}

bool DatabaseManager::init(const QString& dbPath) {
    dbPath_ = dbPath;   // 记录路径，供生成路网前备份使用
    // 添加 SQLite 数据库连接
    db_ = QSqlDatabase::addDatabase("QSQLITE");
    db_.setDatabaseName(dbPath);

    if (!db_.open()) {
        qWarning() << "无法打开数据库:" << db_.lastError().text();
        return false;
    }

    // 建表
    if (!createTable()) return false;

    // 兼容旧数据库：如果 buildings 表缺少 floor_plan_path 列，自动添加
    QSqlQuery alterQuery(db_);
    alterQuery.exec("PRAGMA table_info(buildings)");
    bool hasFloorPlan = false;
    while (alterQuery.next()) {
        if (alterQuery.value(1).toString() == "floor_plan_path") {
            hasFloorPlan = true;
            break;
        }
    }
    if (!hasFloorPlan) {
        QSqlQuery alterStmt(db_);
        alterStmt.exec("ALTER TABLE buildings ADD COLUMN floor_plan_path TEXT");
        qDebug() << "已为 buildings 表添加 floor_plan_path 列";
    }

    // 兼容旧数据库：如果 buildings 表缺少 node_kind 列，自动添加
    {
        QSqlQuery nkCheck(db_);
        nkCheck.exec("PRAGMA table_info(buildings)");
        bool hasNodeKind = false;
        while (nkCheck.next()) {
            if (nkCheck.value(1).toString() == "node_kind") {
                hasNodeKind = true;
                break;
            }
        }
        if (!hasNodeKind) {
            QSqlQuery nkAlter(db_);
            nkAlter.exec("ALTER TABLE buildings ADD COLUMN node_kind INTEGER DEFAULT 1");
            qDebug() << "已为 buildings 表添加 node_kind 列";
        }
    }

    // 兼容旧数据库：如果 roads 表缺少 points 列（拐点），自动添加。
    // 非破坏迁移：仅新增列，原有建筑与道路数据全部保留。
    {
        QSqlQuery pc(db_);
        pc.exec("PRAGMA table_info(roads)");
        bool hasPoints = false;
        while (pc.next()) {
            if (pc.value(1).toString() == "points") {
                hasPoints = true;
                break;
            }
        }
        if (!hasPoints) {
            QSqlQuery pa(db_);
            pa.exec("ALTER TABLE roads ADD COLUMN points TEXT");
            qDebug() << "已为 roads 表添加 points 列（拐点），原有数据保留";
        }
    }

    // 注意：早期版本曾在此处做"版本迁移"——检测到路口节点(id>=100)或旧名"正门"
    // 就清空 buildings/roads 并重填默认数据。但路口节点是交通路网的合法数据，
    // 该判定会误删用户自建的建筑，现已移除。建筑数据永不在初始化时被清空。

    // 如果 buildings 表为空，填入默认建筑+道路数据
    QSqlQuery checkQuery(db_);
    checkQuery.exec("SELECT COUNT(*) FROM buildings");
    if (checkQuery.next() && checkQuery.value(0).toInt() == 0) {
        if (!seedDefaultData()) return false;
        qDebug() << "数据库已填入天津理工大学默认数据（建筑+道路）";
    }

    // 兼容旧版本数据库：如果 roads 表为空（之前没有该表），
    // 单独填入默认道路数据，避免老用户升级后道路消失
    QSqlQuery roadCheck(db_);
    roadCheck.exec("SELECT COUNT(*) FROM roads");
    if (roadCheck.next() && roadCheck.value(0).toInt() == 0) {
        struct DefaultRoad { int from; int to; double w; };
        static const DefaultRoad defaultRoads[] = {
            {0, 2, 349},  {0, 4, 140},  {0, 22, 500}, {22, 2, 160}, {2, 3, 160},
            {3, 4, 160},  {4, 5, 220},  {2, 6, 150},  {3, 7, 150},  {4, 8, 150},
            {5, 9, 150},  {23, 6, 179}, {6, 7, 160},  {7, 8, 160},  {8, 9, 220},
            {9, 21, 220}, {21, 17, 149},{6, 10, 264}, {7, 10, 210}, {8, 11, 210},
            {9, 20, 210}, {10, 11, 160},{11, 20, 220},{20, 16, 220},{16, 17, 295},
            {10, 18, 120},{11, 19, 120},{20, 13, 160},{16, 15, 160},{18, 19, 160},
            {12, 18, 160},{12, 14, 150},{19, 13, 224},{13, 15, 220},{15, 16, 160},
            {14, 24, 335},{15, 24, 488},{13, 24, 304},{17, 1, 130}, {16, 1, 191}
        };
        QSqlQuery roadInsert(db_);
        roadInsert.prepare(
            "INSERT OR REPLACE INTO roads (from_id, to_id, weight) VALUES (?, ?, ?)");
        for (const auto& r : defaultRoads) {
            int a = std::min(r.from, r.to);
            int b = std::max(r.from, r.to);
            roadInsert.addBindValue(a);
            roadInsert.addBindValue(b);
            roadInsert.addBindValue(r.w);
            roadInsert.exec();
        }
        qDebug() << "已为旧数据库补填默认道路数据";
    }

    initialized_ = true;
    qDebug() << "数据库初始化成功:" << dbPath;
    return true;
}

void DatabaseManager::close() {
    if (db_.isOpen()) {
        db_.close();
    }
}

QString DatabaseManager::lastError() const {
    return db_.lastError().text();
}

// ============================================================
//  建表
// ============================================================
bool DatabaseManager::createTable() {
    QSqlQuery query(db_);
    // buildings 表：建筑信息
    QString sql =
        "CREATE TABLE IF NOT EXISTS buildings ("
        "  id INTEGER PRIMARY KEY,"
        "  name TEXT NOT NULL,"
        "  x REAL NOT NULL,"
        "  y REAL NOT NULL,"
        "  type INTEGER NOT NULL,"
        "  info TEXT,"
        "  open_hours TEXT,"
        "  floors INTEGER DEFAULT 1,"
        "  floor_plan_path TEXT,"
        "  node_kind INTEGER DEFAULT 1"
        ")";
    if (!query.exec(sql)) {
        qWarning() << "建表 buildings 失败:" << query.lastError().text();
        return false;
    }

    // roads 表：道路连接（无向边，按 from_id<to_id 规范存储）
    sql =
        "CREATE TABLE IF NOT EXISTS roads ("
        "  from_id INTEGER NOT NULL,"
        "  to_id   INTEGER NOT NULL,"
        "  weight  REAL    NOT NULL,"
        "  points  TEXT,"
        "  PRIMARY KEY (from_id, to_id)"
        ")";
    if (!query.exec(sql)) {
        qWarning() << "建表 roads 失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// ============================================================
//  填入默认数据（使用 CampusData 中的完整校园数据）
// ============================================================
bool DatabaseManager::seedDefaultData() {
    // 通过 CampusData::populate 获取完整路网（69建筑+39路口节点+192条道路）
    Graph graph;
    CampusData::populate(graph);

    // 插入所有节点（含路口节点 nodeKind=0）
    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO buildings (id, name, x, y, type, info, open_hours, floors, node_kind) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");

    for (const auto& [id, building] : graph.allBuildings()) {
        query.addBindValue(building.id());
        query.addBindValue(building.name());
        query.addBindValue(building.x());
        query.addBindValue(building.y());
        query.addBindValue(static_cast<int>(building.type()));
        query.addBindValue(building.info());
        query.addBindValue(building.openHours());
        query.addBindValue(building.floors());
        query.addBindValue(building.nodeKind());

        if (!query.exec()) {
            qWarning() << "插入建筑失败:" << query.lastError().text()
                       << " id=" << id;
            return false;
        }
    }

    // 插入所有道路（遍历邻接表，from<to 去重）
    QSqlQuery roadQuery(db_);
    roadQuery.prepare(
        "INSERT OR REPLACE INTO roads (from_id, to_id, weight, points) VALUES (?, ?, ?, ?)");

    for (const auto& [id, building] : graph.allBuildings()) {
        for (const Edge& e : graph.neighbors(id)) {
            if (id >= e.to) continue;   // 避免双向重复

            roadQuery.addBindValue(id);
            roadQuery.addBindValue(e.to);
            roadQuery.addBindValue(e.weight);
            // 拐点序列化成 JSON 存进 points 列（直线则为 "[]"，兼容旧读取逻辑）
            roadQuery.addBindValue(waypointsToJson(e.waypoints));
            if (!roadQuery.exec()) {
                qWarning() << "插入道路失败:" << roadQuery.lastError().text()
                           << " " << id << "->" << e.to;
                return false;
            }
        }
    }

    qDebug() << "已填入默认数据：建筑" << graph.buildingCount() << "栋";
    return true;
}

// ============================================================
//  查询接口
// ============================================================
std::vector<Building> DatabaseManager::loadAllBuildings() {
    std::vector<Building> result;
    if (!initialized_) return result;

    QSqlQuery query(db_);
    query.exec("SELECT id, name, x, y, type, info, open_hours, floors, node_kind FROM buildings ORDER BY id");

    while (query.next()) {
        Building b(
            query.value(0).toInt(),          // id
            query.value(1).toString(),       // name
            query.value(2).toDouble(),       // x
            query.value(3).toDouble(),       // y
            static_cast<BuildingType>(query.value(4).toInt()),  // type
            query.value(5).toString(),       // info
            query.value(6).toString(),       // openHours
            query.value(7).toInt(),          // floors
            query.value(8).toInt()           // nodeKind
        );
        result.push_back(std::move(b));
    }
    return result;
}

bool DatabaseManager::getBuilding(int id, Building& out) {
    QSqlQuery query(db_);
    query.prepare("SELECT id, name, x, y, type, info, open_hours, floors, node_kind FROM buildings WHERE id = ?");
    query.addBindValue(id);

    if (query.exec() && query.next()) {
        out = Building(
            query.value(0).toInt(),
            query.value(1).toString(),
            query.value(2).toDouble(),
            query.value(3).toDouble(),
            static_cast<BuildingType>(query.value(4).toInt()),
            query.value(5).toString(),
            query.value(6).toString(),
            query.value(7).toInt(),
            query.value(8).toInt()           // nodeKind
        );
        return true;
    }
    return false;
}

// ============================================================
//  修改接口（管理员增删改查）
// ============================================================
bool DatabaseManager::addBuilding(const Building& b) {
    QSqlQuery query(db_);
    query.prepare(
        "INSERT INTO buildings (id, name, x, y, type, info, open_hours, floors, node_kind) "
        "VALUES (?, ?, ?, ?, ?, ?, ?, ?, ?)");
    query.addBindValue(b.id());
    query.addBindValue(b.name());
    query.addBindValue(b.x());
    query.addBindValue(b.y());
    query.addBindValue(static_cast<int>(b.type()));
    query.addBindValue(b.info());
    query.addBindValue(b.openHours());
    query.addBindValue(b.floors());
    query.addBindValue(b.nodeKind());

    if (!query.exec()) {
        qWarning() << "新增建筑失败:" << query.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::updateBuilding(const Building& b) {
    QSqlQuery query(db_);
    query.prepare(
        "UPDATE buildings SET name=?, x=?, y=?, type=?, info=?, open_hours=?, floors=?, node_kind=? "
        "WHERE id=?");
    query.addBindValue(b.name());
    query.addBindValue(b.x());
    query.addBindValue(b.y());
    query.addBindValue(static_cast<int>(b.type()));
    query.addBindValue(b.info());
    query.addBindValue(b.openHours());
    query.addBindValue(b.floors());
    query.addBindValue(b.nodeKind());
    query.addBindValue(b.id());

    if (!query.exec()) {
        qWarning() << "更新建筑失败:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

bool DatabaseManager::deleteBuilding(int id) {
    QSqlQuery query(db_);
    query.prepare("DELETE FROM buildings WHERE id=?");
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "删除建筑失败:" << query.lastError().text();
        return false;
    }

    // 同步删除该建筑相关的所有道路（避免孤立道路）
    QSqlQuery roadClean(db_);
    roadClean.prepare("DELETE FROM roads WHERE from_id=? OR to_id=?");
    roadClean.addBindValue(id);
    roadClean.addBindValue(id);
    roadClean.exec();

    return query.numRowsAffected() > 0;
}

// 更新建筑/路口节点坐标（拖拽路口后持久化）
bool DatabaseManager::updateBuildingPosition(int id, double x, double y) {
    QSqlQuery query(db_);
    query.prepare("UPDATE buildings SET x=?, y=? WHERE id=?");
    query.addBindValue(x);
    query.addBindValue(y);
    query.addBindValue(id);

    if (!query.exec()) {
        qWarning() << "更新节点坐标失败:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// ============================================================
//  手动路网编辑（管理员模式的"位置可调"功能）
//  所有方法只动 buildings 表的 node_kind=0 行 和 roads 表
//  真实建筑（node_kind=1）一律不动
// ============================================================

// 获取下一个可用的路口 id（max(id)+1）
int DatabaseManager::nextJunctionId() const {
    QSqlQuery q(db_);
    if (!q.exec("SELECT COALESCE(MAX(id), -1) FROM buildings")) return -1;
    if (q.next()) return q.value(0).toInt() + 1;
    return -1;
}

// 在 (x,y) 处新增一个路口节点（node_kind=0），返回新 id
int DatabaseManager::addJunction(double x, double y) {
    int id = nextJunctionId();
    if (id < 0) return -1;
    QSqlQuery q(db_);
    q.prepare("INSERT INTO buildings (id, name, x, y, type, info, open_hours, floors, node_kind) "
              "VALUES (?, '', ?, ?, 0, '', '', 0, 0)");
    q.addBindValue(id);
    q.addBindValue(x);
    q.addBindValue(y);
    if (!q.exec()) {
        qWarning() << "新增路口失败:" << q.lastError().text();
        return -1;
    }
    return id;
}

// 删除一个路口节点，同时清理所有包含它的道路
bool DatabaseManager::removeJunctionWithRoads(int id) {
    if (!db_.transaction()) {
        qWarning() << "removeJunctionWithRoads: 无法开启事务";
        return false;
    }
    QSqlQuery q1(db_);
    q1.prepare("DELETE FROM roads WHERE from_id=? OR to_id=?");
    q1.addBindValue(id);
    q1.addBindValue(id);
    if (!q1.exec()) {
        db_.rollback();
        qWarning() << "删除路口相关道路失败:" << q1.lastError().text();
        return false;
    }
    QSqlQuery q2(db_);
    q2.prepare("DELETE FROM buildings WHERE id=? AND node_kind=0");
    q2.addBindValue(id);
    if (!q2.exec() || q2.numRowsAffected() == 0) {
        db_.rollback();
        qWarning() << "删除路口节点失败:" << q2.lastError().text();
        return false;
    }
    return db_.commit();
}

// 用欧式距离作为权重在两个节点间建一条路
bool DatabaseManager::addRoadEuclidean(int aId, int bId) {
    if (aId == bId) return false;
    if (!db_.transaction()) {
        qWarning() << "addRoadEuclidean: 无法开启事务";
        return false;
    }
    // 取两端坐标
    auto getXY = [&](int id, double& x, double& y) -> bool {
        QSqlQuery q(db_);
        q.prepare("SELECT x, y FROM buildings WHERE id=?");
        q.addBindValue(id);
        if (!q.exec() || !q.next()) return false;
        x = q.value(0).toDouble();
        y = q.value(1).toDouble();
        return true;
    };
    double x1, y1, x2, y2;
    if (!getXY(aId, x1, y1) || !getXY(bId, x2, y2)) {
        db_.rollback();
        return false;
    }
    double w = std::hypot(x1 - x2, y1 - y2);
    int a = std::min(aId, bId);
    int b = std::max(aId, bId);
    QSqlQuery ins(db_);
    ins.prepare("INSERT OR IGNORE INTO roads (from_id, to_id, weight, points) VALUES (?, ?, ?, NULL)");
    ins.addBindValue(a);
    ins.addBindValue(b);
    ins.addBindValue(w);
    if (!ins.exec()) {
        db_.rollback();
        qWarning() << "新增道路失败:" << ins.lastError().text();
        return false;
    }
    return db_.commit();
}


// ============================================================
//  道路管理接口实现（新增）
// ============================================================
std::vector<Road> DatabaseManager::loadAllRoads() {
    std::vector<Road> result;
    if (!initialized_) return result;

    QSqlQuery query(db_);
    query.exec("SELECT from_id, to_id, weight, points FROM roads ORDER BY from_id, to_id");

    while (query.next()) {
        Road r;
        r.fromId = query.value(0).toInt();
        r.toId   = query.value(1).toInt();
        r.weight = query.value(2).toDouble();
        r.points = query.value(3).toString();
        result.push_back(r);
    }
    return result;
}

bool DatabaseManager::addRoad(int fromId, int toId, double weight) {
    if (fromId == toId) return false;   // 自环不允许

    QSqlQuery query(db_);
    query.prepare(
        "INSERT OR REPLACE INTO roads (from_id, to_id, weight, points) VALUES (?, ?, ?, ?)");
    // 规范化：保证 from_id < to_id，避免重复存储
    int a = std::min(fromId, toId);
    int b = std::max(fromId, toId);
    query.addBindValue(a);
    query.addBindValue(b);
    query.addBindValue(weight);
    query.addBindValue(QString());   // points 列保留但不再使用，恒为空

    if (!query.exec()) {
        qWarning() << "新增道路失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// 新增一条带"拐点"的道路（points 为拐点 JSON；空字符串表示直线）
bool DatabaseManager::addRoad(int fromId, int toId, double weight, const QString& points) {
    if (fromId == toId) return false;   // 自环不允许

    QSqlQuery query(db_);
    query.prepare(
        "INSERT OR REPLACE INTO roads (from_id, to_id, weight, points) VALUES (?, ?, ?, ?)");
    // 规范化：保证 from_id < to_id，避免重复存储
    int a = std::min(fromId, toId);
    int b = std::max(fromId, toId);
    query.addBindValue(a);
    query.addBindValue(b);
    query.addBindValue(weight);
    query.addBindValue(points);

    if (!query.exec()) {
        qWarning() << "新增带拐点道路失败:" << query.lastError().text();
        return false;
    }
    return true;
}

// 拐点 -> JSON（格式：[[x,y],[x,y],...]，紧凑）
QString DatabaseManager::waypointsToJson(const QVector<QPointF>& wps) {
    QJsonArray arr;
    for (const QPointF& p : wps) {
        QJsonArray pt;
        pt.append(p.x());
        pt.append(p.y());
        arr.append(pt);
    }
    QJsonDocument doc(arr);
    return QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
}

// JSON -> 拐点（兼容 [[x,y],...] 格式；解析失败返回空）
QVector<QPointF> DatabaseManager::jsonToWaypoints(const QString& json) {
    QVector<QPointF> result;
    if (json.isEmpty()) return result;
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(json.toUtf8(), &err);
    if (doc.isNull() || !doc.isArray()) return result;
    for (const QJsonValue& v : doc.array()) {
        if (!v.isArray()) continue;
        QJsonArray pt = v.toArray();
        if (pt.size() >= 2) {
            result.append(QPointF(pt.at(0).toDouble(), pt.at(1).toDouble()));
        }
    }
    return result;
}

bool DatabaseManager::deleteRoad(int fromId, int toId) {
    QSqlQuery query(db_);
    // 双向都尝试删（不知道用户输入顺序）
    query.prepare("DELETE FROM roads WHERE (from_id=? AND to_id=?) OR (from_id=? AND to_id=?)");
    int a = std::min(fromId, toId);
    int b = std::max(fromId, toId);
    query.addBindValue(a);
    query.addBindValue(b);
    query.addBindValue(b);
    query.addBindValue(a);

    if (!query.exec()) {
        qWarning() << "删除道路失败:" << query.lastError().text();
        return false;
    }
    return query.numRowsAffected() > 0;
}

// 统计路口节点数量
int DatabaseManager::junctionCount() const {
    if (!initialized_) return 0;
    QSqlQuery query(db_);
    query.exec("SELECT COUNT(*) FROM buildings WHERE node_kind=0");
    if (query.next()) return query.value(0).toInt();
    return 0;
}

// ============================================================
//  生成道路网格网络（交通路网模式）
//  思路：在建筑分布范围铺一张"正交网格"，网格交叉点即"路口节点"，
//  路口之间用正交道路相连（构成路网），每栋建筑连到最近的路口
//  （相当于建筑的"门口"接入路网）。导航时角色只走路口路网，
//  不会出现在建筑上。本函数只重排 roads 表、新增路口节点，
//  绝不删除或修改任何真实建筑（node_kind=1）。
// ============================================================
bool DatabaseManager::generateRoadNetwork() {
    if (!initialized_ || !db_.isOpen()) {
        qWarning() << "数据库未初始化，无法生成路网";
        return false;
    }

    // 1. 读取所有建筑（含坐标），计算分布范围
    auto buildings = loadAllBuildings();
    if (buildings.empty()) return false;

    double minX = 1e9, maxX = -1e9, minY = 1e9, maxY = -1e9;
    for (const auto& b : buildings) {
        if (b.nodeKind() != 1) continue;   // 只统计真实建筑
        minX = std::min(minX, b.x());
        maxX = std::max(maxX, b.x());
        minY = std::min(minY, b.y());
        maxY = std::max(maxY, b.y());
    }
    if (minX > maxX || minY > maxY) return false;   // 没有真实建筑

    // 2. 清空现有道路 + 旧路口节点（node_kind=0），准备以路网方式重建。
    //    真实建筑（node_kind=1）一律保留，绝不动。
    {
        QSqlQuery wipe(db_);
        if (!wipe.exec("DELETE FROM roads")) {
            qWarning() << "清空道路失败:" << wipe.lastError().text();
            return false;
        }
        QSqlQuery wipeJ(db_);
        wipeJ.exec("DELETE FROM buildings WHERE node_kind=0");
    }

    // 3. 生成网格路口节点
    const double margin = 40;     // 在建筑范围外扩一点，让边缘建筑也能接路
    const double step   = 150;    // 路口间距（地图较小，避免路太短）
    // 网格范围限制在场景(1280x976)内，避免路口跑到地图外
    double gx0 = std::max(20.0, minX - margin);
    double gy0 = std::max(20.0, minY - margin);
    double gx1 = std::min(1260.0, maxX + margin);
    double gy1 = std::min(956.0,  maxY + margin);

    // 路口坐标结构
    struct P { double x, y; };

    // 网格按"行优先"填充：grid 下标 = r*cols + c（与下方 idx 一致）
    std::vector<std::pair<int, P>> grid;   // (junctionId, 坐标)
    std::unordered_map<int, P> jcoord;     // 路口ID -> 坐标，便于建筑接入时查坐标

    int cols = static_cast<int>((gx1 - gx0) / step) + 1;
    int rows = static_cast<int>((gy1 - gy0) / step) + 1;
    if (cols < 1) cols = 1;
    if (rows < 1) rows = 1;

    // 路口节点 id 从"当前最大建筑 id + 1"起，避免与真实建筑/admin新增建筑冲突
    int maxId = 0;
    for (const auto& b : buildings) maxId = std::max(maxId, b.id());
    int jid = maxId + 1;

    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            double x = gx0 + c * step;
            double y = gy0 + r * step;
            // 保险夹取，确保始终在场景内
            x = std::max(10.0, std::min(1270.0, x));
            y = std::max(10.0, std::min(966.0,  y));
            Building j(jid, "", x, y, BuildingType::Other,
                       "", "", 0, 0);   // nodeKind=0 = 路口
            addBuilding(j);
            grid.push_back({jid, {x, y}});
            jcoord[jid] = {x, y};
            ++jid;
        }
    }

    // 4. 连接正交相邻路口（横向 + 纵向），构成路网骨架
    //    注意：grid 为行优先填充，故 idx(r,c)=r*cols+c 才正确（早期版本索引写反，
    //    产生了斜向边）。这里严格按正交相邻连接，保证道路都是横平竖直。
    auto idx = [&](int r, int c) { return r * cols + c; };
    for (int r = 0; r < rows; ++r) {
        for (int c = 0; c < cols; ++c) {
            int id = grid[idx(r, c)].first;
            P p = grid[idx(r, c)].second;
            // 右邻（同列 c+1，必然正交）
            if (c + 1 < cols) {
                int id2 = grid[idx(r, c + 1)].first;
                P p2 = grid[idx(r, c + 1)].second;
                double w = std::hypot(p2.x - p.x, p2.y - p.y);
                addRoad(id, id2, w);
            }
            // 下邻（同行 r+1，必然正交）
            if (r + 1 < rows) {
                int id2 = grid[idx(r + 1, c)].first;
                P p2 = grid[idx(r + 1, c)].second;
                double w = std::hypot(p2.x - p.x, p2.y - p.y);
                addRoad(id, id2, w);
            }
        }
    }

    // 5. 每栋建筑连到最近的 2 个路口（门口接入路网）
    for (const auto& b : buildings) {
        if (b.nodeKind() != 1) continue;   // 只处理真实建筑
        // 找最近的路口并按距离排序
        std::vector<std::pair<double, int>> dists;
        for (const auto& kv : jcoord) {
            double d = std::hypot(kv.second.x - b.x(), kv.second.y - b.y());
            dists.emplace_back(d, kv.first);
        }
        std::sort(dists.begin(), dists.end());
        // 连最近的 2 个（足够形成路口接入，又不至于太密）
        int linked = 0;
        for (const auto& pr : dists) {
            const P& jp = jcoord[pr.second];
            double w = std::hypot(jp.x - b.x(), jp.y - b.y());
            addRoad(b.id(), pr.second, w);
            if (++linked >= 2) break;
        }
    }

    qDebug() << "已生成道路网格：路口" << grid.size() << "个，建筑接入完成";
    return true;
}

// ============================================================
//  判断路网是否需要重建（修复早期网格索引 bug 产生的斜向边/越界路口）
// ============================================================
bool DatabaseManager::roadNetworkNeedsRebuild() const {
    // 手动路网编辑模式下，绝不自动重建路网。
    // 用户自己摆放的路口，以及沿街道画的斜向边，都是合法的，
    // 不应被当成"网格损坏"而覆盖。否则每次启动都会 DELETE 所有
    // node_kind=0 路口并重生成固定正交网格，导致用户拖动/添加的
    // 节点位置全部丢失（即"重启后节点又变回不准确的位置"）。
    // 路网生成只由管理员在"生成道路网格"操作中主动触发（generateRoadNetwork）。
    return false;
}

// ============================================================
//  一键恢复默认数据：清空现有建筑、道路，重新写入天理默认数据
// ============================================================
bool DatabaseManager::resetToDefaultData() {
    if (!initialized_ || !db_.isOpen()) {
        qWarning() << "数据库未初始化，无法重置";
        return false;
    }

    QSqlQuery wipe(db_);
    if (!wipe.exec("DELETE FROM buildings") || !wipe.exec("DELETE FROM roads")) {
        qWarning() << "清空数据失败:" << wipe.lastError().text();
        return false;
    }

    if (!seedDefaultData()) {
        qWarning() << "重新写入默认数据失败";
        return false;
    }

    qDebug() << "已恢复默认天理校园数据";
    return true;
}

// ============================================================
//  从 PlaceIntroduction 目录批量导入建筑介绍 txt 文件
//  文件名 = 建筑名.txt，内容格式：
//    第1行：简介
//    第2行：开放时间
//    第3行：楼层信息
//  返回成功导入的建筑数量
// ============================================================
int DatabaseManager::importPlaceIntroductions(const QString& dirPath) {
    if (!initialized_ || !db_.isOpen()) {
        qWarning() << "数据库未初始化，无法导入";
        return 0;
    }

    QDir dir(dirPath);
    if (!dir.exists()) {
        qWarning() << "PlaceIntroduction 目录不存在:" << dirPath;
        return 0;
    }

    QStringList filters;
    filters << "*.txt";
    dir.setNameFilters(filters);

    int count = 0;
    QSqlQuery query(db_);
    // 仅当建筑当前没有介绍时才写入，绝不覆盖用户/历史已有的内容
    query.prepare("UPDATE buildings SET info=?, open_hours=?, floor_plan_path=? "
                  "WHERE name=? AND (info IS NULL OR info='')");

    const QFileInfoList files = dir.entryInfoList(QDir::Files);
    for (const QFileInfo& fi : files) {
        QString buildingName = fi.baseName();   // 文件名去掉扩展名
        QString filePath = fi.absoluteFilePath();

        QFile file(filePath);
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            qWarning() << "无法读取文件:" << filePath;
            continue;
        }

        QTextStream in(&file);
        // Qt 6 默认使用 UTF-8，无需 setCodec
        QString info = in.readLine();       // 第1行：简介
        QString openHours = in.readLine();  // 第2行：开放时间
        // 第3行（楼层信息）已存在 floors 字段中，这里附加到 info 末尾
        QString floorInfo = in.readLine();  // 第3行：楼层信息
        file.close();

        if (info.isEmpty()) continue;

        // 如果有楼层信息，追加到简介后面
        if (!floorInfo.isEmpty()) {
            info += "\n" + floorInfo;
        }

        query.addBindValue(info);
        query.addBindValue(openHours);
        query.addBindValue(filePath);     // 楼层平面图存储路径
        query.addBindValue(buildingName);

        if (query.exec() && query.numRowsAffected() > 0) {
            ++count;
            qDebug() << "已导入:" << buildingName;
        }
    }

    qDebug() << "批量导入完成，共" << count << "栋建筑";
    return count;
}
