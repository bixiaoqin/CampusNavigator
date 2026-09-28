#include "model/Building.h"

#include <QString>

// ============================================================
//  Building 类的实现
// ============================================================

// 构造函数：成员初始化列表语法
// 冒号后逐个 成员(参数) 初始化，比在 {} 里赋值更高效
// （直接构造，少一次拷贝/赋值）
Building::Building(int id, QString name, double x, double y,
                   BuildingType type, QString info,
                   QString openHours, int floors, int nodeKind)
    : id_(id)
    , name_(std::move(name))      // std::move：移动语义，避免拷贝 QString
    , x_(x)
    , y_(y)
    , type_(type)
    , info_(std::move(info))
    , openHours_(std::move(openHours))
    , floors_(floors)
    , nodeKind_(nodeKind) {}

// 把建筑类型枚举转成中文字符串
// 用 switch 比一串 if-else 更清晰，编译器还能检查是否覆盖所有枚举值
QString Building::typeToString(BuildingType t) {
    switch (t) {
        case BuildingType::Gate:      return QStringLiteral("校门");
        case BuildingType::Classroom: return QStringLiteral("教学楼");
        case BuildingType::Library:   return QStringLiteral("图书馆");
        case BuildingType::Lab:       return QStringLiteral("实验楼");
        case BuildingType::Admin:     return QStringLiteral("行政楼");
        case BuildingType::Dormitory: return QStringLiteral("宿舍");
        case BuildingType::Canteen:   return QStringLiteral("食堂");
        case BuildingType::Sports:    return QStringLiteral("体育设施");
        case BuildingType::Shop:      return QStringLiteral("商店");
        case BuildingType::Hospital:  return QStringLiteral("医院");
        case BuildingType::Other:     return QStringLiteral("其他");
    }
    return QStringLiteral("未知");   // 兜底，防止编译器警告
}

// 生成格式化的建筑描述文字
// QStringLiteral 是 Qt 推荐的字符串字面量写法，比 QString("...") 高效
// （编译期生成，无运行期分配）
QString Building::description() const {
    return QStringLiteral("[%1] %2（%3）\n")
           .arg(id_)
           .arg(name_)
           .arg(typeToString(type_))
         + QStringLiteral("    坐标: (%1, %2)\n").arg(x_).arg(y_)
         + QStringLiteral("    楼层: %1 层\n").arg(floors_)
         + QStringLiteral("    开放时间: %1\n").arg(openHours_)
         + QStringLiteral("    简介: %1").arg(info_);
}
