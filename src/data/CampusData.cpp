#include "data/CampusData.h"

#include "model/Building.h"

// ============================================================
//  CampusData 实现：校园完整地图数据（种子数据）
// ============================================================
//  本文件由 2026-09-28 从实际运行数据库 campus.db 导出固化：
//    69 栋真实建筑（nodeKind=1）+ 39 个路口节点（nodeKind=0）
//    + 192 条道路（无向，全部为直线段）
//  作用：别人拿到源码首次编译运行时，程序会用这份种子数据
//  自动生成 campus.db，看到的就是作者调试后的完整路网效果。
//  之后管理员模式的增删改仍写入 campus.db，与本文件无关。
// ============================================================

namespace CampusData {

void populate(Graph& graph) {
    // ============================================================
    //  真实建筑（69 栋，nodeKind=1，正常显示名称与标签）
    // ============================================================
    graph.addBuilding({0,  QStringLiteral("南门"), 370, 733, BuildingType::Gate, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({2,  QStringLiteral("东门"), 1184, 340, BuildingType::Gate, QStringLiteral("东门是天津理工大学的次要出入口，位于校园东侧，连接周边生活区和城市道路。门外设有共享单车停放点和出租车候客区，方便师生通勤出行。东门附近布置有学生公寓和后勤服务设施，是学生日常出入的高频通道。\n楼层信息：无（室外门区）"), QStringLiteral("开放时间：全天"), 1});
    graph.addBuilding({3,  QStringLiteral("明理农场"), 80, 120, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({4,  QStringLiteral("海运学院"), 260, 100, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({5,  QStringLiteral("图书馆"), 390, 110, BuildingType::Library, QStringLiteral("天津理工大学图书馆成立于1978年，建筑面积4.6万平方米，采用藏借阅一体化管理模式。馆藏纸质图书215余万册，阅览座位4600余席，配备280台计算机的电子阅览室。实现无线网络覆盖，建有Elsevier ScienceDirect、Springer、CNKI等60余个中外文数据库。\n楼层信息：共5层，1-2层借阅区，3层电子阅览室，4-5层自习区"), QStringLiteral("开放时间：07:30-22:30"), 1});
    graph.addBuilding({6,  QStringLiteral("教学楼28"), 550, 120, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({7,  QStringLiteral("体育馆"), 752, 95, BuildingType::Sports, QStringLiteral("体育馆是天津理工大学室内体育教学和赛事活动的主要场馆，建筑面积约1.2万平方米。馆内设有篮球场、羽毛球场、乒乓球场和体操房，可容纳3000名观众。承接校内运动会、文艺演出和大型集会活动，也是师生日常健身锻炼的场所。\n楼层信息：共3层，1层乒乓球和体操房，2层篮球馆，3层羽毛球馆和看台"), QStringLiteral("开放时间：06:00-22:00"), 1});
    graph.addBuilding({8,  QStringLiteral("钟楼"), 815, 178, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({9,  QStringLiteral("教学楼1"), 742, 207, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({10,  QStringLiteral("教学楼2"), 744, 263, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({11,  QStringLiteral("教学楼3"), 747, 318, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({12,  QStringLiteral("教学楼4"), 750, 358, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({13,  QStringLiteral("教学楼5"), 758, 405, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({14,  QStringLiteral("教学楼6"), 760, 445, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({15,  QStringLiteral("教学楼13"), 586, 502, BuildingType::Classroom, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({16,  QStringLiteral("管理学院"), 261, 364, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({17,  QStringLiteral("集成电路与自动化学院"), 101, 396, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({18,  QStringLiteral("艺术学院"), 217, 486, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({19,  QStringLiteral("化学化工学院"), 242, 597, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({20,  QStringLiteral("生命研究所"), 236, 653, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({21,  QStringLiteral("计算机学院"), 605, 279, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({22,  QStringLiteral("理学院"), 591, 353, BuildingType::Lab, QStringLiteral("理学院是天津理工大学数学、物理、化学等基础学科的教学科研单位，拥有应用数学、物理学、化学等本科专业。学院下设多个教研室和基础实验室，承担全校理工科基础课程教学任务。师资力量雄厚，多项科研成果在国内外学术期刊发表。\n楼层信息：共5层，1层实验室，2-3层教室，4-5层教师办公室和科研室"), QStringLiteral("开放时间：08:00-17:30"), 1});
    graph.addBuilding({23,  QStringLiteral("工程训练中心"), 512, 491, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({24,  QStringLiteral("材料学院"), 491, 434, BuildingType::Lab, QStringLiteral("材料科学与工程学院是天津理工大学的优势学院之一，设有材料科学与工程、材料物理、材料化学等专业。学院拥有天津市重点实验室和材料表征中心，配备扫描电镜、X射线衍射仪等大型仪器设备。在新型功能材料、纳米材料等研究领域处于国内领先水平。\n楼层信息：共6层，1-2层实验室，3-4层教研室，5层仪器中心，6层行政办公"), QStringLiteral("开放时间：08:00-17:30"), 1});
    graph.addBuilding({25,  QStringLiteral("机械工程学院"), 514, 547, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({26,  QStringLiteral("海洋能源学院"), 520, 608, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({27,  QStringLiteral("环安报告厅"), 424, 658, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({28,  QStringLiteral("环境科学与安全学院"), 541, 679, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({29,  QStringLiteral("大学生活动中心"), 1000, 370, BuildingType::Admin, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({30,  QStringLiteral("朝阳广场"), 1050, 447, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({31,  QStringLiteral("一食堂"), 1131, 441, BuildingType::Canteen, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({32,  QStringLiteral("二食堂"), 932, 297, BuildingType::Canteen, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({33,  QStringLiteral("酒店"), 1097, 305, BuildingType::Admin, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({34,  QStringLiteral("南创业楼"), 1192, 638, BuildingType::Lab, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({35,  QStringLiteral("南苑一站式"), 1184, 504, BuildingType::Shop, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({36,  QStringLiteral("体育中心"), 806, 509, BuildingType::Sports, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({37,  QStringLiteral("26号楼"), 295, 259, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({38,  QStringLiteral("23号楼"), 196, 431, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({39,  QStringLiteral("20号楼"), 239, 563, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({40,  QStringLiteral("21号楼"), 214, 522, BuildingType::Other, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({41,  QStringLiteral("1号公寓"), 967, 496, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({42,  QStringLiteral("2号公寓"), 967, 529, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({43,  QStringLiteral("3号公寓"), 971, 562, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({44,  QStringLiteral("4号公寓"), 1051, 558, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({45,  QStringLiteral("5号公寓"), 1137, 545, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({46,  QStringLiteral("6号公寓"), 1113, 510, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({47,  QStringLiteral("7号公寓"), 1088, 482, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({48,  QStringLiteral("8号公寓"), 977, 613, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({49,  QStringLiteral("9号公寓"), 983, 644, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({50,  QStringLiteral("10号公寓"), 991, 683, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({51,  QStringLiteral("11号公寓"), 1050, 580, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({52,  QStringLiteral("12号公寓"), 1112, 643, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({53,  QStringLiteral("14号公寓"), 1052, 620, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({54,  QStringLiteral("15号公寓"), 962, 724, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({55,  QStringLiteral("16号公寓"), 1071, 763, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({56,  QStringLiteral("17号公寓"), 1173, 724, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({57,  QStringLiteral("18号公寓"), 804, 723, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({58,  QStringLiteral("19号公寓"), 811, 772, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({59,  QStringLiteral("20号公寓"), 868, 47, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({60,  QStringLiteral("21号公寓"), 970, 80, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({61,  QStringLiteral("22号公寓"), 1038, 122, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({62,  QStringLiteral("24号公寓"), 931, 153, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({63,  QStringLiteral("25号公寓"), 932, 184, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({64,  QStringLiteral("26号公寓"), 944, 222, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({65,  QStringLiteral("27号公寓"), 952, 261, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({66,  QStringLiteral("28号公寓"), 1084, 253, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({67,  QStringLiteral("29号公寓"), 1081, 219, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({68,  QStringLiteral("30号公寓"), 1042, 181, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});
    graph.addBuilding({69,  QStringLiteral("31号公寓"), 1056, 153, BuildingType::Dormitory, QStringLiteral(""), QStringLiteral("全天"), 1});

    // ============================================================
    //  路口节点（39 个，nodeKind=0，不显示，构成路网骨架）
    // ============================================================
    graph.addBuilding({201, QStringLiteral(""), 89.33, 145.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({209, QStringLiteral(""), 114, 358, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({210, QStringLiteral(""), 218.67, 204, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({211, QStringLiteral(""), 404.67, 216, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({212, QStringLiteral(""), 631.33, 219.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({213, QStringLiteral(""), 630, 106, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({214, QStringLiteral(""), 808, 217.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({215, QStringLiteral(""), 841.33, 101.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({216, QStringLiteral(""), 998, 141.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({217, QStringLiteral(""), 40, 320, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({218, QStringLiteral(""), 194, 329.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({219, QStringLiteral(""), 394.67, 334, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({220, QStringLiteral(""), 394, 332.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({221, QStringLiteral(""), 630, 336.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({222, QStringLiteral(""), 813.33, 320, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({223, QStringLiteral(""), 874.67, 338, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({224, QStringLiteral(""), 1035.33, 330.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({225, QStringLiteral(""), 194, 330.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({226, QStringLiteral(""), 152, 462.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({227, QStringLiteral(""), 389.33, 476, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({228, QStringLiteral(""), 484, 474, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({229, QStringLiteral(""), 632, 480.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({230, QStringLiteral(""), 824.67, 481.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({231, QStringLiteral(""), 877.33, 480.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({232, QStringLiteral(""), 1049.33, 430, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({234, QStringLiteral(""), 138, 580, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({235, QStringLiteral(""), 375.33, 589.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({236, QStringLiteral(""), 490, 588.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({237, QStringLiteral(""), 635.33, 590.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({238, QStringLiteral(""), 784, 534, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({239, QStringLiteral(""), 882, 600, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({240, QStringLiteral(""), 1071.33, 596, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({242, QStringLiteral(""), 123.33, 704.67, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({243, QStringLiteral(""), 357.33, 704, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({244, QStringLiteral(""), 492, 719.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({245, QStringLiteral(""), 632, 717.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({246, QStringLiteral(""), 790, 770, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({247, QStringLiteral(""), 885.33, 713.33, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});
    graph.addBuilding({248, QStringLiteral(""), 1054.67, 696, BuildingType::Other,
                       QStringLiteral(""), QStringLiteral(""), 0, 0});

    // ============================================================
    //  道路（192 条，无向边；权重为实际距离）
    // ============================================================
    graph.addRoad(0, 235, 116.9);
    graph.addRoad(0, 243, 47.6);
    graph.addRoad(2, 224, 96.1);
    graph.addRoad(2, 232, 160.4);
    graph.addRoad(3, 201, 107.7);
    graph.addRoad(3, 209, 64);
    graph.addRoad(5, 211, 78.1);
    graph.addRoad(6, 212, 78.1);
    graph.addRoad(6, 213, 103);
    graph.addRoad(7, 214, 84.1);
    graph.addRoad(8, 214, 26.2);
    graph.addRoad(8, 215, 125.3);
    graph.addRoad(9, 214, 60.6);
    graph.addRoad(10, 222, 73.2);
    graph.addRoad(11, 221, 107);
    graph.addRoad(11, 222, 43);
    graph.addRoad(12, 221, 116.4);
    graph.addRoad(12, 222, 55.2);
    graph.addRoad(13, 222, 90.8);
    graph.addRoad(13, 230, 72.4);
    graph.addRoad(14, 229, 122.6);
    graph.addRoad(14, 230, 39.1);
    graph.addRoad(15, 228, 101.2);
    graph.addRoad(15, 229, 62.8);
    graph.addRoad(16, 218, 83.5);
    graph.addRoad(16, 219, 90.4);
    graph.addRoad(17, 217, 97.5);
    graph.addRoad(17, 225, 95.9);
    graph.addRoad(18, 226, 31.4);
    graph.addRoad(18, 227, 124);
    graph.addRoad(19, 234, 56.9);
    graph.addRoad(19, 235, 100.7);
    graph.addRoad(20, 234, 56.6);
    graph.addRoad(20, 235, 109.1);
    graph.addRoad(21, 213, 114.5);
    graph.addRoad(21, 221, 53.9);
    graph.addRoad(22, 220, 106.3);
    graph.addRoad(22, 221, 59.1);
    graph.addRoad(23, 228, 30.4);
    graph.addRoad(23, 229, 129.7);
    graph.addRoad(24, 220, 114);
    graph.addRoad(24, 228, 36);
    graph.addRoad(25, 228, 80.7);
    graph.addRoad(25, 236, 76.8);
    graph.addRoad(26, 236, 32.3);
    graph.addRoad(26, 237, 120.6);
    graph.addRoad(27, 235, 92.2);
    graph.addRoad(27, 236, 76.2);
    graph.addRoad(28, 236, 78);
    graph.addRoad(28, 244, 104.3);
    graph.addRoad(29, 223, 78.1);
    graph.addRoad(29, 224, 103);
    graph.addRoad(30, 231, 112.4);
    graph.addRoad(30, 232, 46.1);
    graph.addRoad(31, 224, 127.8);
    graph.addRoad(31, 232, 50.2);
    graph.addRoad(32, 215, 127.3);
    graph.addRoad(32, 223, 24.4);
    graph.addRoad(33, 216, 135.2);
    graph.addRoad(33, 224, 16.6);
    graph.addRoad(34, 240, 103.6);
    graph.addRoad(34, 248, 166.8);
    graph.addRoad(35, 232, 100);
    graph.addRoad(35, 240, 149.3);
    graph.addRoad(36, 230, 42.2);
    graph.addRoad(36, 238, 112.1);
    graph.addRoad(37, 211, 99.7);
    graph.addRoad(37, 219, 75.8);
    graph.addRoad(38, 218, 111.2);
    graph.addRoad(38, 226, 39.5);
    graph.addRoad(39, 226, 105.1);
    graph.addRoad(39, 234, 75.2);
    graph.addRoad(40, 226, 57.3);
    graph.addRoad(40, 234, 100.9);
    graph.addRoad(41, 231, 37.5);
    graph.addRoad(41, 232, 125.7);
    graph.addRoad(42, 231, 64.9);
    graph.addRoad(42, 239, 94.9);
    graph.addRoad(43, 231, 97.1);
    graph.addRoad(43, 239, 65.8);
    graph.addRoad(44, 232, 96.3);
    graph.addRoad(44, 240, 73.2);
    graph.addRoad(45, 232, 88.5);
    graph.addRoad(45, 240, 88.5);
    graph.addRoad(46, 232, 46.1);
    graph.addRoad(46, 240, 112.4);
    graph.addRoad(47, 232, 12.2);
    graph.addRoad(47, 240, 138);
    graph.addRoad(48, 239, 37.7);
    graph.addRoad(48, 240, 113.2);
    graph.addRoad(49, 239, 49.2);
    graph.addRoad(49, 240, 109.7);
    graph.addRoad(50, 239, 81.1);
    graph.addRoad(50, 247, 100.8);
    graph.addRoad(51, 232, 117);
    graph.addRoad(51, 240, 56.6);
    graph.addRoad(52, 240, 31.8);
    graph.addRoad(52, 248, 128.9);
    graph.addRoad(53, 239, 112);
    graph.addRoad(53, 240, 38);
    graph.addRoad(54, 239, 106.3);
    graph.addRoad(54, 247, 51);
    graph.addRoad(55, 247, 131.2);
    graph.addRoad(55, 248, 20.2);
    graph.addRoad(56, 240, 133.1);
    graph.addRoad(56, 248, 94.9);
    graph.addRoad(57, 238, 103.9);
    graph.addRoad(57, 246, 49);
    graph.addRoad(58, 246, 21.1);
    graph.addRoad(58, 247, 129);
    graph.addRoad(60, 215, 94.9);
    graph.addRoad(61, 215, 109.1);
    graph.addRoad(61, 216, 70.8);
    graph.addRoad(62, 215, 19.2);
    graph.addRoad(63, 215, 16.1);
    graph.addRoad(63, 223, 136.2);
    graph.addRoad(64, 215, 52.2);
    graph.addRoad(64, 223, 98.1);
    graph.addRoad(65, 215, 91.8);
    graph.addRoad(65, 223, 60.2);
    graph.addRoad(66, 216, 83.2);
    graph.addRoad(66, 224, 67.3);
    graph.addRoad(67, 216, 49.8);
    graph.addRoad(67, 224, 101.4);
    graph.addRoad(68, 215, 102.6);
    graph.addRoad(68, 216, 49.2);
    graph.addRoad(69, 215, 117.2);
    graph.addRoad(69, 216, 38);
    graph.addRoad(201, 209, 150);
    graph.addRoad(209, 210, 150);
    graph.addRoad(209, 217, 150);
    graph.addRoad(210, 211, 150);
    graph.addRoad(210, 218, 150);
    graph.addRoad(211, 212, 150);
    graph.addRoad(211, 219, 150);
    graph.addRoad(212, 213, 150);
    graph.addRoad(212, 220, 150);
    graph.addRoad(213, 214, 150);
    graph.addRoad(213, 221, 150);
    graph.addRoad(214, 215, 150);
    graph.addRoad(214, 222, 150);
    graph.addRoad(215, 216, 150);
    graph.addRoad(215, 223, 150);
    graph.addRoad(216, 224, 150);
    graph.addRoad(217, 218, 150);
    graph.addRoad(217, 225, 150);
    graph.addRoad(218, 219, 150);
    graph.addRoad(218, 226, 150);
    graph.addRoad(219, 220, 150);
    graph.addRoad(219, 227, 150);
    graph.addRoad(220, 221, 150);
    graph.addRoad(220, 228, 150);
    graph.addRoad(221, 222, 150);
    graph.addRoad(221, 229, 150);
    graph.addRoad(222, 223, 150);
    graph.addRoad(222, 230, 150);
    graph.addRoad(223, 224, 150);
    graph.addRoad(223, 231, 150);
    graph.addRoad(224, 232, 150);
    graph.addRoad(225, 226, 150);
    graph.addRoad(226, 227, 150);
    graph.addRoad(226, 234, 150);
    graph.addRoad(227, 228, 150);
    graph.addRoad(227, 235, 150);
    graph.addRoad(228, 229, 150);
    graph.addRoad(228, 236, 150);
    graph.addRoad(229, 230, 150);
    graph.addRoad(229, 237, 150);
    graph.addRoad(230, 231, 150);
    graph.addRoad(230, 238, 150);
    graph.addRoad(231, 232, 150);
    graph.addRoad(231, 239, 150);
    graph.addRoad(232, 240, 150);
    graph.addRoad(234, 235, 150);
    graph.addRoad(234, 242, 150);
    graph.addRoad(235, 236, 150);
    graph.addRoad(235, 243, 150);
    graph.addRoad(236, 237, 150);
    graph.addRoad(236, 244, 150);
    graph.addRoad(237, 238, 150);
    graph.addRoad(237, 245, 150);
    graph.addRoad(238, 239, 150);
    graph.addRoad(238, 246, 150);
    graph.addRoad(239, 240, 150);
    graph.addRoad(239, 247, 150);
    graph.addRoad(240, 248, 150);
    graph.addRoad(242, 243, 150);
    graph.addRoad(243, 244, 150);
    graph.addRoad(244, 245, 150);
    graph.addRoad(245, 246, 150);
    graph.addRoad(246, 247, 150);
    graph.addRoad(247, 248, 150);
}

} // namespace CampusData
