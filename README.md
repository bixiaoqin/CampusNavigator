# CampusNavigator · 校园探索与智能导航模拟系统

C++ 课程设计 · 题目5（Qt 路线，三星难度）· 个人独立完成

基于天津理工大学真实校园地图，使用 Qt 6 Graphics View Framework 实现的交互式校园导航系统。

## 功能概览

- 25 栋建筑 + 40 条道路的校园地图（基于真实底图）
- Dijkstra 最短路径规划
- WASD 键盘控制角色漫游（精细 4 方向行走动画）
- 自主移动 NPC 系统（状态机 + 邻接表寻路 + 对话交互）
- 随机生成 NPC（按钮一键生成3-6个随机NPC，位置/服装/名字/对话均随机）
- 自动导航（沿规划路径平滑移动，可随时 WASD 中断）
- SQLite 数据库（建筑 + 道路 CRUD 管理）
- 昼夜模式切换 + 天气粒子系统（雨/雪/高温/大风）
- 迷你地图实时显示位置
- 新生引导模式 + 访客导览模式
- 管理员后台（建筑/道路增删改查）

## 一键编译运行

### 前置要求

- Qt 6.5+（推荐 Qt 6.11 LTS）
- MinGW 64-bit 编译器
- CMake 3.16+
- Ninja 构建工具

### 方式一：Qt Creator（推荐）

1. 打开 Qt Creator
2. 菜单 `文件 → 打开文件或项目`（Ctrl+O）
3. 选择本目录下的 `CMakeLists.txt`
4. 勾选 MinGW 64-bit Kit，点 `Configure Project`
5. 等待 CMake 配置完成
6. 点左下角绿色 ▶ 运行（Ctrl+R）

### 方式二：命令行

```bash
# 配置
cmake -B build -G Ninja -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/MinGW_64_bit"

# 编译
cmake --build build

# 运行
./build/CampusNavigator.exe
```

## 操作说明

| 操作 | 按键/动作 | 效果 |
|------|-----------|------|
| 移动角色 | W/A/S/D 或方向键 | 角色向对应方向行走 |
| 规划路径 | 点击建筑设起终点 → 点"开始导航" | 红色高亮路径 + 自动导航 |
| 搜索建筑 | 搜索框输入名称 → 设为起点/终点 | 模糊匹配建筑 |
| 查看信息 | 选中建筑 → 点"查看建筑信息" | 弹出建筑详情 |
| 管理数据 | 点"管理员模式"（密码 admin） | 打开建筑/道路管理界面 + 地图点击拾取场景坐标(x,y) |
| 切换昼夜 | 点"切换白天/夜间" | 叠加/移除夜间遮罩 |
| 切换天气 | 点"天气"按钮 | 晴→雨→高温→大风循环 |
| NPC 对话 | 点击地图上的 NPC | 弹出对话气泡 |
| 随机生成NPC | 点"🎲 随机生成NPC"按钮 | 随机生成3-6个NPC（位置/服装/对话随机） |
| 新生模式 | 点"新生模式" | 高亮关键建筑 + 途经弹窗 |
| 访客模式 | 点"访客模式" | 隐藏私密建筑 + 景点导览 |
| 缩放地图 | 鼠标滚轮 | 0.4x ~ 3.0x 缩放 |
| 拖拽地图 | 鼠标左键拖拽 | 平移地图视图 |

## 目录结构

```
CampusNavigator/
├── CMakeLists.txt              构建配置（C++17, Qt6 Core+Widgets+Sql, AUTOMOC）
├── main.cpp                    程序入口
├── README.md                   本文件
├── resources.qrc              Qt 资源文件
├── assets/
│   └── tjut_map.jpg            校园底图（1280×976）
├── PlaceIntroduction/          建筑介绍文本（16个）
├── docs/
│   ├── report.md               课程设计报告
│   ├── report.docx             课程设计报告（Word）
│   └── video-script.md         演示视频脚本
└── src/
    ├── model/                   模型层（纯数据 + 算法）
    │   ├── Building.h/.cpp      建筑类
    │   ├── Edge.h               道路边结构
    │   └── Graph.h/.cpp         路网图 + Dijkstra 最短路径
    ├── data/                    数据层
    │   ├── CampusData.h/.cpp    种子数据（25建筑 + 40道路）
    │   └── DatabaseManager.h/.cpp  SQLite CRUD（单例）
    └── view/                    视图层
        ├── BuildingItem.h/.cpp      建筑图元（标签 + 悬停选中）
        ├── RoadItem.h/.cpp          道路图元（高亮/淡化）
        ├── MapScene.h/.cpp          地图场景
        ├── CharacterItem.h/.cpp     角色精灵（4方向动画）
        ├── NpcItem.h/.cpp           NPC 系统（自主移动 + 对话）
        ├── WeatherOverlay.h/.cpp    天气粒子系统
        ├── AdminDialog.h/.cpp       管理员对话框（建筑 + 道路 CRUD）
        └── MainWindow.h/.cpp        主窗口（UI + 4定时器 + 导航 + 模式）
```

## 技术栈

- C++17（结构化绑定、std::move、enum class、try_emplace）
- Qt 6.11（Graphics View Framework、信号与槽、QTimer、QSqlDatabase）
- CMake + Ninja 构建
- SQLite 3 数据库
- MinGW-w64 编译器
