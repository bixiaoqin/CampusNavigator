#ifndef CAMPUSNAVIGATOR_MODEL_BUILDING_H
#define CAMPUSNAVIGATOR_MODEL_BUILDING_H

#include <QString>

// ============================================================
//  BuildingType - 建筑物类型枚举
// ============================================================
//  使用 enum class（强类型枚举）而非传统 enum，
//  避免枚举值污染外层作用域，也更类型安全。
//  这是 C++11 引入的现代写法，任务书要求用现代 C++ 特性。
// ============================================================
enum class BuildingType {
    Gate,        // 校门
    Classroom,   // 教学楼
    Library,     // 图书馆
    Lab,         // 实验楼
    Admin,       // 行政楼
    Dormitory,   // 宿舍
    Canteen,     // 食堂
    Sports,      // 体育设施
    Shop,        // 商店
    Hospital,    // 医院
    Other        // 其他
};

// ============================================================
//  Building - 建筑物类
// ============================================================
//  校园地图上的一个节点。每栋建筑既是地图上的图元，
//  也是路网图中的一个节点（node）。
//
//  采用面向对象封装：所有成员变量私有，通过公共 getter
//  访问。构造时初始化，运行期不可改 id/坐标（建筑不会
//  移动），但信息字段（简介、开放时间）允许管理员修改。
// ============================================================
class Building {
public:
    // 构造函数：用成员初始化列表初始化（比在函数体赋值更高效）
    // nodeKind: 0=路口节点(无名称不显示), 1=建筑出入口(正常显示)
    Building(int id, QString name, double x, double y,
             BuildingType type, QString info,
             QString openHours, int floors, int nodeKind = 1);

    // --- 只读 getter（const 成员函数，不修改对象状态）---
    int id() const { return id_; }
    const QString& name() const { return name_; }
    double x() const { return x_; }
    double y() const { return y_; }
    BuildingType type() const { return type_; }
    const QString& info() const { return info_; }
    const QString& openHours() const { return openHours_; }
    int floors() const { return floors_; }
    int nodeKind() const { return nodeKind_; }

    // --- 可写 setter（供管理员模式增删改查使用）---
    void setInfo(const QString& info) { info_ = info; }
    void setOpenHours(const QString& hours) { openHours_ = hours; }
    void setPosition(double x, double y) { x_ = x; y_ = y; }

    // 把建筑类型枚举转成中文（界面显示用）
    static QString typeToString(BuildingType t);

    // 返回格式化的建筑描述（调试和信息面板共用）
    QString description() const;

private:
    int id_;                // 唯一编号（也是路网图中的节点 id）
    QString name_;          // 名称，如"主教学楼"
    double x_;              // 地图坐标 x（水平，单位：像素）
    double y_;              // 地图坐标 y（垂直，单位：像素）
    BuildingType type_;     // 建筑类型
    QString info_;          // 简介文字
    QString openHours_;     // 开放时间，如"07:00-22:00"
    int floors_;            // 楼层数
    int nodeKind_ = 1;      // 节点类型: 0=路口(不显示), 1=建筑出入口(显示)
};

#endif // CAMPUSNAVIGATOR_MODEL_BUILDING_H
