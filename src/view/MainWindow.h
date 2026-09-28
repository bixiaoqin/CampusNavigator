#ifndef CAMPUSNAVIGATOR_VIEW_MAINWINDOW_H
#define CAMPUSNAVIGATOR_VIEW_MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGraphicsView>
#include <QTimer>
#include <QVector>
#include <QPoint>
#include <QColor>
#include <QList>
#include <QFrame>
#include "model/Graph.h"
#include <vector>

class MapScene;
class CharacterItem;
class WeatherOverlay;
class NpcItem;
class QGraphicsRectItem;

// ============================================================
//  应用模式：普通 / 新生 / 访客
// ============================================================
enum class AppMode {
    Normal,     // 普通模式：全部功能可用
    Freshman,   // 新生模式：开学专属路线+宿舍介绍+入住贴士
    Visitor     // 访客模式：游览路线+屏蔽私密区+电子导游
};

// ============================================================
//  路网编辑子工具
// ============================================================
enum class NetEditMode {
    AddJunction,    // ➊ 往地图上点空白处加新路口
    Connect,        // ➋ 点两个节点建立一条道路
    Delete          // ➌ 右键节点/道路删除
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

protected:
    void showEvent(QShowEvent* event) override;
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
    void resizeEvent(QResizeEvent* event) override;
    void changeEvent(QEvent* event) override;
    bool eventFilter(QObject* obj, QEvent* event) override;

private slots:
    void onBuildingClicked(int buildingId);
    void onNavigate();
    void onClearPath();
    void onShowInfo();
    void onCharacterTimer();   // 角色手动移动（WASD）
    void onAdminMode();
    void onNavigateTimer();    // 角色自动导航定时器
    void onToggleDayNight();   // 切换昼夜
    void onToggleWeather();    // 切换天气
    void onRandomNpc();        // 随机生成NPC
    void onNpcClicked(const QString& name, const QString& dialog);
    void onSearchStart();
    void onSearchEnd();

    // --- 地图画路（管理员，替代旧坐标点击器）---
    void onStartDrawRoad();          // 进入画路模式
    void onExitAdminMode();          // 退出管理员模式（由对话框按钮触发）
    void onGenerateNetwork();        // 生成/重置道路网格（交通路网）
    void handleDrawRoadClick(const QPointF& p);  // 处理画路时的地图点击
    void finishDrawRoad(int endId);  // 完成一条带拐点道路
    void cancelDrawRoad();           // 取消画路
    int  buildingIdAt(const QPointF& p);          // 命中建筑检测
    void addDrawMarker(qreal x, qreal y, const QColor& color, qreal r);
    void clearDrawMarkers();         // 清除临时标记

    // --- 模式切换 ---
    void onSwitchFreshman();  // 切换到新生模式
    void onSwitchVisitor();   // 切换到访客模式
    void onSwitchNormal();    // 切换回普通模式
    void onFreshmanRoute();   // 新生开学专属路线导航
    void onVisitorGuide();    // 访客游览路线导航
    void onShowTips();        // 显示入住小贴士/导游信息

    // --- 路网编辑模式（位置可调工具）---
    void setNetEditSubMode(NetEditMode m);    // 切换子工具（互斥单选）
    void onEnterNetworkEditMode();      // 进入（由 AdminDialog 按钮触发）
    void onExitNetworkEditMode();       // 退出（按"完成"按钮）
    void onNetEditSubModeAdd();         // 子工具：添加路口
    void onNetEditSubModeConnect();     // 子工具：连接道路
    void onNetEditSubModeDelete();      // 子工具：删除
    void onIntersectionMoved(int id, QPointF pos);   // 拖动中（实时刷新连路）
    void onIntersectionDragFinished(int id, QPointF pos);  // 拖动结束（落库）
    void onIntersectionClickedEdit(int id);          // 点击（连接工具用）
    void onIntersectionRightClickedEdit(int id);     // 右键（删除工具用）
    void onRoadRightClickedEdit(int fromId, int toId); // 道路右键（删除工具用）
    void onEmptyScenePressedEdit(QPointF pos, Qt::MouseButton button); // 空白处点击
    // 注：连接预览的鼠标跟随在 eventFilter 里直接处理（不单独成槽）

private:
    void setupUI();
    void initMapData();
    void initNpcs();

    // 计算角色实际行走路径：只走路口路网（建筑仅作门口接入点），
    // 保证角色始终在灰色道路上、绝不会穿过建筑中心。
    // 返回路口节点 id 序列（door → door）。
    std::vector<int> walkPathFor(int startB, int endB) const;

    void startAutoNavigate(const std::vector<int>& path);
    void stopAutoNavigate();
    void reloadMap();        // 从数据库重新加载并重建整张地图
    void updateMiniMap();

    // --- 镜头跟随 ---
    void updateCamera();        // 平滑跟随角色，带边界限制
    void clampCenter(QPointF& center) const;  // 限制中心不超出地图边界

    // --- 夜间蒙版 ---
    void updateNightOverlay();  // 更新蒙版位置和大小（跟随地图）
    void updateLabelScale();    // 更新建筑标签字号（随缩放自适应）

    QGraphicsView*    mapView_  = nullptr;
    QGraphicsView*    miniMapView_ = nullptr;
    MapScene*         mapScene_ = nullptr;
    CharacterItem*    character_ = nullptr;
    WeatherOverlay*   weatherOverlay_ = nullptr;
    QGraphicsRectItem* nightOverlay_ = nullptr;  // 夜间蒙版（半透明深蓝）
    QLabel*           statusLabel_ = nullptr;
    QPushButton*      navigateBtn_ = nullptr;
    QPushButton*      clearBtn_    = nullptr;
    QPushButton*      infoBtn_     = nullptr;
    QPushButton*      adminBtn_    = nullptr;
    QPushButton*      dayNightBtn_ = nullptr;
    QPushButton*      weatherBtn_  = nullptr;
    QPushButton*      randomNpcBtn_ = nullptr;  // 随机生成NPC按钮
    QLabel*           startLabel_  = nullptr;
    QLabel*           endLabel_    = nullptr;
    QLineEdit*        searchEdit_       = nullptr;
    QPushButton*      searchStartBtn_   = nullptr;
    QPushButton*      searchEndBtn_     = nullptr;

    // --- 模式切换按钮 ---
    QPushButton*      freshmanBtn_   = nullptr;  // 新生模式
    QPushButton*      visitorBtn_    = nullptr;  // 访客模式
    QPushButton*      normalBtn_     = nullptr;  // 普通模式
    QPushButton*      routeBtn_      = nullptr;  // 专属路线按钮
    QPushButton*      tipsBtn_       = nullptr;  // 小贴士/导游按钮
    AppMode           appMode_ = AppMode::Normal;

    // --- 新生模式：途经宿舍时弹窗 ---
    int               lastTipBuildingId_ = -1;

    std::vector<NpcItem*> npcs_;          // 所有NPC（固定+随机）
    std::vector<NpcItem*> randomNpcs_;   // 随机生成的NPC（便于单独清理）

    Graph campus_;
    int selectedStartId_ = -1;
    int selectedEndId_   = -1;

    // --- 手动移动状态 ---
    QTimer* moveTimer_ = nullptr;
    bool moveUp_ = false, moveDown_ = false, moveLeft_ = false, moveRight_ = false;

    // --- 自动导航状态 ---
    QTimer* navTimer_ = nullptr;
    std::vector<int> navPath_;
    size_t navTargetIndex_ = 0;
    QPointF navTargetPos_;
    bool navigating_ = false;
    QVector<QPointF> navPoints_;   // 完整折线点列（含道路拐点），角色沿此行走
    int navPointIndex_ = 0;        // 当前目标点在 navPoints_ 中的下标

    // --- 缩放状态 ---
    qreal zoomLevel_ = 1.0;          // 当前缩放倍率
    static constexpr qreal ZOOM_MIN = 0.4;   // 最小缩小倍率
    static constexpr qreal ZOOM_MAX = 3.0;   // 最大放大倍率

    // --- 扩展功能状态 ---
    bool isNight_ = false;
    int  weatherMode_ = 0;
    bool isAdminMode_ = false;   // 管理员模式开关

    // --- 地图画路（管理员，替代旧坐标点击器）---
    bool drawRoadMode_ = false;        // 是否处于画路模式
    int  drawStartId_ = -1;            // 已选起点建筑 id
    QVector<QPointF> drawWaypoints_;   // 已点选的拐点（折线中间点）
    QList<QGraphicsItem*> drawMarkers_; // 临时标记（起点/拐点），便于清除

    // --- 路网编辑模式（位置可调工具）---
    bool networkEditMode_ = false;        // 是否处于路网编辑模式
    NetEditMode netEditMode_ = NetEditMode::AddJunction;
    int  connectFirstId_ = -1;           // 连接工具已选的"第一点"
    QFrame* netEditPanel_ = nullptr;     // 子工具条容器（隐藏在侧栏中）
    QPushButton* netEditAddBtn_     = nullptr;   // 添加路口
    QPushButton* netEditConnectBtn_ = nullptr;   // 连接道路
    QPushButton* netEditDeleteBtn_  = nullptr;   // 删除
    QPushButton* netEditFinishBtn_  = nullptr;   // 完成
};

#endif
