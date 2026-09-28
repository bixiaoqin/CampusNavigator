#include "view/MainWindow.h"

#include "view/MapScene.h"
#include "view/CharacterItem.h"
#include "view/BuildingItem.h"
#include "view/IntersectionItem.h"
#include "view/AdminDialog.h"
#include "view/WeatherOverlay.h"
#include "view/NpcItem.h"
#include "data/CampusData.h"
#include "data/DatabaseManager.h"
#include "model/Edge.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStatusBar>
#include <QGroupBox>
#include <QMessageBox>
#include <QShowEvent>
#include <QResizeEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QInputDialog>
#include <QPainter>
#include <QDateTime>
#include <QFile>
#include <QStandardPaths>
#include <QDir>
#include <QCoreApplication>
#include <QApplication>
#include <QWheelEvent>
#include <QRandomGenerator>
#include <QGraphicsRectItem>
#include <QGraphicsEllipseItem>
#include <QGraphicsPixmapItem>
#include <vector>
#include <algorithm>
#include <cmath>

// ============================================================
//  辅助函数：把建筑名称列表解析为当前图中真实存在的ID列表
//  用途：路线/模式按"名称"而非写死ID定位，避免用户用管理员模式
//        增删改建筑后ID错位（例如删了北门导致功能崩溃）。
//  找不到的建筑名称会写入 missing（若传入），便于给出友好提示。
// ============================================================
static QList<int> resolveRouteByName(const Graph& campus,
                                     const QStringList& names,
                                     QStringList* missing = nullptr) {
    QList<int> ids;
    const auto& all = campus.allBuildings();
    for (const QString& n : names) {
        const Building* b = campus.getBuildingByName(n);
        if (!b) {
            // 容错：精确匹配失败时，尝试"包含"匹配
            // 例如用户把建筑命名为"2号公寓(宿舍楼16-20)"时仍能命中"2号公寓"
            for (const auto& [id, bb] : all) {
                if (bb.name().contains(n)) { b = &bb; break; }
            }
        }
        if (b) {
            ids.append(b->id());
        } else if (missing) {
            missing->append(n);
        }
    }
    return ids;
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("CampusNavigator - 校园探索与智能导航模拟系统"));
    resize(1000, 700);
    setupUI();
    initMapData();
}

void MainWindow::setupUI() {
    mapView_ = new QGraphicsView(this);
    mapView_->setRenderHints(QPainter::Antialiasing | QPainter::TextAntialiasing);
    mapView_->setDragMode(QGraphicsView::ScrollHandDrag);
    mapView_->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    mapView_->setFocusPolicy(Qt::StrongFocus);
    // 禁用滚动条，用滚轮缩放而非滚动
    mapView_->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mapView_->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器，拦截滚轮事件做缩放
    mapView_->viewport()->installEventFilter(this);

    auto* panel = new QGroupBox(tr("导航控制"));
    auto* pl = new QVBoxLayout(panel);
    // 收紧行间距与边距，让整列控件更紧凑，给上方路网编辑子面板留出更多空间
    pl->setSpacing(4);
    pl->setContentsMargins(8, 6, 8, 6);

    // ============================================================
    //  路网编辑子工具条（默认隐藏，进入"路网编辑模式"才显示）
    // ============================================================
    netEditPanel_ = new QFrame();
    netEditPanel_->setFrameShape(QFrame::StyledPanel);
    // 注意：这里【不】加 border-radius。QFrame 的圆角样式背景在窗口切到全屏时
    // 会触发 Qt 的合成层重绘 bug，导致子控件文字被画成只剩中间横线。
    // 保留浅黄底 + 橙色描边做高亮（6px 圆角视觉差异可忽略）。
    netEditPanel_->setStyleSheet(
        "QFrame{background:#FFFDE7;border:2px solid #FFB300;}");
    auto* neLayout = new QVBoxLayout(netEditPanel_);
    auto* neTitle = new QLabel(tr("🛠 路网编辑模式"));
    neTitle->setStyleSheet("color:#E65100;font-weight:bold;font-size:10pt;");
    neLayout->addWidget(neTitle);
    auto* neSub = new QLabel(
        tr("①添加路口：点空白处  ②连接：点两节点  ③删除：右键  ④拖动：挪路口"));
    neSub->setWordWrap(true);
    neSub->setStyleSheet("color:#555;font-size:8pt;");
    neLayout->addWidget(neSub);
    auto* neBtnLayout = new QHBoxLayout();
    QString neBtnStyle =
        "QPushButton{padding:5px 8px;font-size:9pt;border-radius:4px;"
        "background:#FFF3E0;border:1px solid #FFCC80;}"
        "QPushButton:checked{background:#FFB74D;color:white;border-color:#FFA726;}"
        "QPushButton:hover{background:#FFE0B2;}";
    netEditAddBtn_     = new QPushButton(tr("➕ 路口"));
    netEditConnectBtn_ = new QPushButton(tr("🔗 连接"));
    netEditDeleteBtn_  = new QPushButton(tr("🗑 删除"));
    netEditFinishBtn_  = new QPushButton(tr("✅ 完成"));
    for (auto* b : { netEditAddBtn_, netEditConnectBtn_, netEditDeleteBtn_ }) {
        b->setCheckable(true);
        b->setStyleSheet(neBtnStyle);
    }
    netEditFinishBtn_->setCheckable(false);
    netEditFinishBtn_->setStyleSheet(
        "QPushButton{padding:5px 8px;font-size:9pt;border-radius:4px;"
        "background:#C8E6C9;border:1px solid #81C784;}"
        "QPushButton:hover{background:#A5D6A7;}");
    neBtnLayout->addWidget(netEditAddBtn_);
    neBtnLayout->addWidget(netEditConnectBtn_);
    neBtnLayout->addWidget(netEditDeleteBtn_);
    neBtnLayout->addWidget(netEditFinishBtn_);
    neLayout->addLayout(neBtnLayout);
    pl->addWidget(netEditPanel_);
    netEditPanel_->setVisible(false);   // 默认隐藏

    startLabel_ = new QLabel(tr("起点: 未选择"));
    startLabel_->setStyleSheet("font-weight:bold;color:#E65100;");
    endLabel_ = new QLabel(tr("终点: 未选择"));
    endLabel_->setStyleSheet("font-weight:bold;color:#BF360C;");
    // 起点/终点放在同一行（横向 QHBox），节省一个垂直行，
    // 让上方路网编辑子面板的内容更舒展，不再被挤
    auto* startEndRow = new QHBoxLayout;
    startEndRow->setContentsMargins(0, 0, 0, 0);
    startEndRow->setSpacing(12);
    startEndRow->addWidget(startLabel_);
    startEndRow->addWidget(endLabel_);
    startEndRow->addStretch();
    pl->addLayout(startEndRow);

    // --- 建筑搜索框 ---
    auto* searchLabel = new QLabel(tr("🔍 建筑搜索:"));
    searchLabel->setStyleSheet("font-weight:bold;color:#333;margin-top:6px;");
    pl->addWidget(searchLabel);

    searchEdit_ = new QLineEdit();
    searchEdit_->setPlaceholderText(tr("输入建筑名称..."));
    searchEdit_->setStyleSheet(
        "QLineEdit{padding:3px 8px;border:1px solid #FFCC80;border-radius:4px;"
        "font-size:11px;background:#FFF8E1;}"
        "QLineEdit:focus{border:2px solid #FFB74D;background:#fff;}");
    pl->addWidget(searchEdit_);

    auto* searchBtnLayout = new QHBoxLayout;
    searchStartBtn_ = new QPushButton(tr("设为起点"));
    searchEndBtn_   = new QPushButton(tr("设为终点"));
    QString searchBtnStyle =
        "QPushButton{padding:3px 8px;font-size:10px;border-radius:4px;}"
        "QPushButton:pressed{background:#FFCC80;}";
    searchStartBtn_->setStyleSheet(
        searchBtnStyle + "QPushButton{background:#FFF3E0;border:1px solid #FFCC80;}"
                         "QPushButton:hover{background:#FFE0B2;}");
    searchEndBtn_->setStyleSheet(
        searchBtnStyle + "QPushButton{background:#FFF3E0;border:1px solid #FFCC80;}"
                         "QPushButton:hover{background:#FFE0B2;}");
    searchBtnLayout->addWidget(searchStartBtn_);
    searchBtnLayout->addWidget(searchEndBtn_);
    pl->addLayout(searchBtnLayout);

    connect(searchStartBtn_, &QPushButton::clicked, this, &MainWindow::onSearchStart);
    connect(searchEndBtn_,   &QPushButton::clicked, this, &MainWindow::onSearchEnd);

    navigateBtn_ = new QPushButton(tr("开始导航"));
    infoBtn_     = new QPushButton(tr("查看建筑信息"));
    clearBtn_    = new QPushButton(tr("清除路径"));
    adminBtn_    = new QPushButton(tr("管理员模式"));
    dayNightBtn_ = new QPushButton(tr("切换昼夜"));
    weatherBtn_  = new QPushButton(tr("切换天气"));
    randomNpcBtn_ = new QPushButton(tr("🎲 随机生成NPC"));

    QString s = "QPushButton{padding:4px 14px;font-size:11px;border-radius:5px;"
               "background:#FFF3E0;border:1px solid #FFCC80;}"
              "QPushButton:hover{background:#FFE0B2;}";
    navigateBtn_->setStyleSheet(s);
    infoBtn_->setStyleSheet(s);
    clearBtn_->setStyleSheet(s);
    adminBtn_->setStyleSheet(s);
    dayNightBtn_->setStyleSheet(s);
    weatherBtn_->setStyleSheet(s);
    randomNpcBtn_->setStyleSheet(s);

    pl->addWidget(navigateBtn_);
    pl->addWidget(infoBtn_);
    pl->addWidget(clearBtn_);
    pl->addWidget(adminBtn_);
    pl->addWidget(dayNightBtn_);
    pl->addWidget(weatherBtn_);
    pl->addWidget(randomNpcBtn_);

    // --- 模式切换区域 ---
    auto* modeLabel = new QLabel(tr("━━━ 模式切换 ━━━"));
    modeLabel->setStyleSheet("color:#9E9E9E;font-size:11px;margin-top:4px;");
    modeLabel->setAlignment(Qt::AlignCenter);
    pl->addWidget(modeLabel);

    freshmanBtn_ = new QPushButton(tr("🎓 新生模式"));
    visitorBtn_  = new QPushButton(tr("👥 访客模式"));
    normalBtn_   = new QPushButton(tr("🏠 普通模式"));
    routeBtn_    = new QPushButton(tr("📋 专属路线"));
    tipsBtn_     = new QPushButton(tr("💡 小贴士/导游"));

    freshmanBtn_->setStyleSheet(
        "QPushButton{padding:4px 14px;font-size:11px;border-radius:5px;"
        "background:#FFF3E0;border:1px solid #FFCC80;font-weight:bold;}"
        "QPushButton:hover{background:#FFE0B2;}"
        "QPushButton:checked{background:#FFB74D;color:white;border-color:#FFA726;}");
    visitorBtn_->setStyleSheet(
        "QPushButton{padding:4px 14px;font-size:11px;border-radius:5px;"
        "background:#FFF3E0;border:1px solid #FFCC80;font-weight:bold;}"
        "QPushButton:hover{background:#FFE0B2;}"
        "QPushButton:checked{background:#FF8A65;color:white;border-color:#FF7043;}");
    normalBtn_->setStyleSheet(
        "QPushButton{padding:4px 14px;font-size:11px;border-radius:5px;"
        "background:#FFF3E0;border:1px solid #FFCC80;}"
        "QPushButton:hover{background:#FFE0B2;}");
    routeBtn_->setStyleSheet(
        "QPushButton{padding:4px 10px;font-size:10px;border-radius:4px;"
        "background:#FFF3E0;border:1px solid #FFCC80;}"
        "QPushButton:hover{background:#FFE0B2;}");
    tipsBtn_->setStyleSheet(
        "QPushButton{padding:4px 10px;font-size:10px;border-radius:4px;"
        "background:#FFF3E0;border:1px solid #FFCC80;}"
        "QPushButton:hover{background:#FFE0B2;}");

    freshmanBtn_->setCheckable(true);
    visitorBtn_->setCheckable(true);

    pl->addWidget(freshmanBtn_);
    pl->addWidget(visitorBtn_);
    pl->addWidget(normalBtn_);
    pl->addWidget(routeBtn_);
    pl->addWidget(tipsBtn_);
    pl->addStretch();

    auto* hint = new QLabel(
        tr("操作说明:\n"
           "  点击建筑选起终点\n"
           "  点\"开始导航\"自动行走\n"
           "  WASD 手动移动(可中断导航)\n"
           "  昼夜/天气按钮切换效果\n"
           "  管理员密码: admin"));
    hint->setStyleSheet("color:#757575;font-size:10px;");
    pl->addWidget(hint);

    auto* cw = new QWidget;
    auto* ml = new QHBoxLayout(cw);
    ml->addWidget(mapView_, 3);
    ml->addWidget(panel, 1);
    setCentralWidget(cw);

    statusLabel_ = new QLabel(tr("就绪 - 用 WASD 移动角色，点击建筑导航"));
    statusBar()->addWidget(statusLabel_, 1);

    connect(navigateBtn_, &QPushButton::clicked, this, &MainWindow::onNavigate);
    connect(clearBtn_,    &QPushButton::clicked, this, &MainWindow::onClearPath);
    connect(infoBtn_,     &QPushButton::clicked, this, &MainWindow::onShowInfo);
    connect(adminBtn_,    &QPushButton::clicked, this, &MainWindow::onAdminMode);
    connect(dayNightBtn_, &QPushButton::clicked, this, &MainWindow::onToggleDayNight);
    connect(weatherBtn_,  &QPushButton::clicked, this, &MainWindow::onToggleWeather);
    connect(randomNpcBtn_, &QPushButton::clicked, this, &MainWindow::onRandomNpc);

    // 路网编辑工具条
    connect(netEditAddBtn_,     &QPushButton::clicked, this, &MainWindow::onNetEditSubModeAdd);
    connect(netEditConnectBtn_, &QPushButton::clicked, this, &MainWindow::onNetEditSubModeConnect);
    connect(netEditDeleteBtn_,  &QPushButton::clicked, this, &MainWindow::onNetEditSubModeDelete);
    connect(netEditFinishBtn_,  &QPushButton::clicked, this, &MainWindow::onExitNetworkEditMode);

    // 模式切换
    connect(freshmanBtn_, &QPushButton::clicked, this, &MainWindow::onSwitchFreshman);
    connect(visitorBtn_,  &QPushButton::clicked, this, &MainWindow::onSwitchVisitor);
    connect(normalBtn_,   &QPushButton::clicked, this, &MainWindow::onSwitchNormal);
    connect(routeBtn_,    &QPushButton::clicked, this, &MainWindow::onFreshmanRoute);
    connect(tipsBtn_,     &QPushButton::clicked, this, &MainWindow::onShowTips);

    // --- 迷你地图（角落缩略图视图）---
    // 复用同一个场景，固定小尺寸，不可交互
    miniMapView_ = new QGraphicsView(this);
    miniMapView_->setRenderHint(QPainter::Antialiasing);
    miniMapView_->setFixedSize(180, 140);
    miniMapView_->setWindowTitle("迷你地图");
    // 边框样式
    miniMapView_->setStyleSheet(
        "QGraphicsView{border:2px solid #FFCC80;background:#FFF3E0;border-radius:4px;}");
    miniMapView_->setEnabled(false);   // 不接收鼠标事件
    miniMapView_->show();

    // --- 手动移动定时器（30 FPS）---
    moveTimer_ = new QTimer(this);
    moveTimer_->setInterval(33);
    connect(moveTimer_, &QTimer::timeout, this, &MainWindow::onCharacterTimer);
    moveTimer_->start();

    // --- 自动导航定时器（30 FPS）---
    navTimer_ = new QTimer(this);
    navTimer_->setInterval(33);
    connect(navTimer_, &QTimer::timeout, this, &MainWindow::onNavigateTimer);

    // --- 天气/迷你地图同步定时器（10 FPS，够用）---
    // 复用 moveTimer_ 也行，但单独一个更清晰
    auto* auxTimer = new QTimer(this);
    auxTimer->setInterval(100);
    connect(auxTimer, &QTimer::timeout, this, [this](){
        // 推进天气粒子动画
        if (weatherOverlay_) weatherOverlay_->advance(1);
        // 更新迷你地图（让缩略图中心跟随角色）
        updateMiniMap();
    });
    auxTimer->start();

    // --- NPC 移动定时器（30 FPS，独立于角色移动定时器）---
    // NPC 在路网上自主游走，不因角色导航而暂停
    auto* npcTimer = new QTimer(this);
    npcTimer->setInterval(33);
    connect(npcTimer, &QTimer::timeout, this, [this]() {
        for (NpcItem* npc : npcs_) {
            if (npc) npc->updateMovement();
        }
    });
    npcTimer->start();
}

void MainWindow::initMapData() {
    // --- 初始化数据库 ---
    // 便携模式优先：若可执行文件同目录已存在 campus.db，则直接使用它，
    //   这样把整个 build-release 文件夹打包发给别人时，对方双击 exe 即可看到完整校园，无需安装/配置。
    // 否则回退到用户数据目录 %APPDATA%/CampusNavigator/campus.db（本机开发/日常使用）。
    QString exeDbPath = QCoreApplication::applicationDirPath() + "/campus.db";
    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QString appDataDbPath = dataDir + "/campus.db";

    QString dbPath;
    if (QFile::exists(exeDbPath)) {
        dbPath = exeDbPath;       // 便携版：使用 exe 同目录数据库
    } else {
        QDir().mkpath(dataDir);   // 确保用户数据目录存在
        dbPath = appDataDbPath;   // 本机模式：使用 AppData 数据库
    }

    if (!DatabaseManager::instance().init(dbPath)) {
        QMessageBox::warning(this, tr("数据库错误"),
            tr("无法初始化数据库，将使用内存数据。\n错误: %1")
                .arg(DatabaseManager::instance().lastError()));
        // 回退：使用硬编码数据
        CampusData::populate(campus_);
    } else {
        // 批量导入 PlaceIntroduction 目录下的建筑介绍 txt 文件
        // 尝试多个路径：可执行文件目录、源码目录
        QString placeDir = QCoreApplication::applicationDirPath() + "/PlaceIntroduction";
        if (!QDir(placeDir).exists()) {
            // 尝试源码目录（开发时用）
            placeDir = QCoreApplication::applicationDirPath() + "/../../PlaceIntroduction";
        }
        if (QDir(placeDir).exists()) {
            int imported = DatabaseManager::instance().importPlaceIntroductions(placeDir);
            qDebug() << "从 PlaceIntroduction 导入了" << imported << "栋建筑介绍";
        }

        // 从数据库加载建筑数据
        auto buildings = DatabaseManager::instance().loadAllBuildings();
        for (const auto& b : buildings) {
            campus_.addBuilding(b);
        }
        // 从数据库加载道路连接（管理员可动态增删，不再用硬编码）
        auto roads = DatabaseManager::instance().loadAllRoads();
        for (const auto& r : roads) {
            // 道路可能带拐点（折线），从 points JSON 解析；无拐点则为直线
            QVector<QPointF> wps = DatabaseManager::instance().jsonToWaypoints(r.points);
            campus_.addRoad(r.fromId, r.toId, r.weight, wps);
        }
        qDebug() << "从数据库加载：" << buildings.size() << "栋建筑，"
                 << roads.size() << "条道路";

        // 首次运行（无任何路口节点）或检测到早期网格 bug（斜向边/越界路口）时，
        // 自动重新生成干净的路网，让角色能沿灰色道路行走。
        // 生成前先备份数据库，真实建筑（node_kind=1）一律不动。
        if (DatabaseManager::instance().roadNetworkNeedsRebuild()) {
            QString dbp = DatabaseManager::instance().databasePath();
            if (!dbp.isEmpty()) {
                QString backup = dbp + ".bak_network_auto";
                if (!QFile::exists(backup)) QFile::copy(dbp, backup);
            }
            if (DatabaseManager::instance().generateRoadNetwork()) {
                // 重新载入含路口与路网的图数据
                campus_ = Graph();
                auto b2 = DatabaseManager::instance().loadAllBuildings();
                for (const auto& b : b2) campus_.addBuilding(b);
                auto r2 = DatabaseManager::instance().loadAllRoads();
                for (const auto& r : r2) {
                    QVector<QPointF> wps = DatabaseManager::instance().jsonToWaypoints(r.points);
                    campus_.addRoad(r.fromId, r.toId, r.weight, wps);
                }
                qDebug() << "已自动生成道路网格";
            }
        }
    }

    mapScene_ = new MapScene(this);
    mapScene_->loadFromGraph(campus_);
    mapView_->setScene(mapScene_);
    connect(mapScene_, &MapScene::buildingClicked,
            this,     &MainWindow::onBuildingClicked);
    // 路网编辑信号
    connect(mapScene_, &MapScene::intersectionMoved,
            this,     &MainWindow::onIntersectionMoved);
    connect(mapScene_, &MapScene::intersectionDragFinished,
            this,     &MainWindow::onIntersectionDragFinished);
    connect(mapScene_, &MapScene::intersectionClicked,
            this,     &MainWindow::onIntersectionClickedEdit);
    connect(mapScene_, &MapScene::intersectionRightClicked,
            this,     &MainWindow::onIntersectionRightClickedEdit);
    connect(mapScene_, &MapScene::roadRightClicked,
            this,     &MainWindow::onRoadRightClickedEdit);
    connect(mapScene_, &MapScene::emptyScenePressed,
            this,     &MainWindow::onEmptyScenePressedEdit);

    // --- 创建角色，放在正门（id=0）附近：优先落在其"门口"路口，避免站在建筑上 ---
    const Building* gate = campus_.getBuilding(0);
    QPointF startPos = gate ? QPointF(gate->x(), gate->y()) : QPointF(100, 500);
    if (gate) {
        int gj = campus_.nearestJunction(0);
        if (gj >= 0) {
            const Building* gjB = campus_.getBuilding(gj);
            if (gjB) startPos = QPointF(gjB->x(), gjB->y());
        }
    }
    character_ = new CharacterItem();
    character_->setPos(startPos);
    mapScene_->addItem(character_);

    // --- 创建天气特效层（覆盖整个地图）---
    QRectF sr = mapScene_->sceneRect();
    weatherOverlay_ = new WeatherOverlay();
    weatherOverlay_->setPos(sr.topLeft());
    weatherOverlay_->setAreaSize(sr.width(), sr.height());
    mapScene_->addItem(weatherOverlay_);

    // --- 迷你地图绑定到同一场景 ---
    miniMapView_->setScene(mapScene_);
    miniMapView_->fitInView(sr, Qt::KeepAspectRatio);

    // --- 放置 NPC（扩展功能4：NPC 系统）---
    initNpcs();

    // 统计可见建筑数（排除路口节点）
    int visibleBuildings = 0;
    for (const auto& [id, b] : campus_.allBuildings())
        if (b.nodeKind() == 1) ++visibleBuildings;

    statusLabel_->setText(
        tr("校园地图已加载 - %1 栋建筑，%2 条道路，%3 个NPC — WASD 移动角色")
            .arg(visibleBuildings)
            .arg(campus_.roadCount())
            .arg(npcs_.size()));
}

// ============================================================
//  NPC 系统：在地图上放置会自主移动的 NPC，玩家点击可对话
//  扩展功能：NPC 在路网上随机游走（从当前建筑走到相邻建筑，
//  到达后停留片刻再选下一个目标），行走时有4方向动画。
// ============================================================
void MainWindow::initNpcs() {
    // NPC 配置：初始站在某栋建筑附近，有自己的名字、对话内容、衣服颜色
    struct NpcConfig {
        int     nearBuildingId;  // 初始站在哪栋建筑附近
        int     offsetX, offsetY;// 相对建筑中心的偏移（避免和建筑重合）
        QString name;
        QString dialog;
        NpcItem::Outfit outfit;
    };

    static const NpcConfig configs[] = {
        {7,  40, 20,  "图书管理员",
         "欢迎来到图书馆！我们藏书215万册，开放时间是7:30-22:30。\n"
         "二楼的阅览区最安静，适合自习。需要帮忙找书随时问我～",
         NpcItem::Outfit::Purple},

        {9,  30, -25, "计算机学院学长",
         "想学编程？这栋楼的三楼机房全天开放。\n"
         "推荐先学 C++，Qt 框架做 GUI 项目很实用！",
         NpcItem::Outfit::Red},
    };

    for (const auto& cfg : configs) {
        const Building* b = campus_.getBuilding(cfg.nearBuildingId);
        if (!b) continue;

        auto* npc = new NpcItem(cfg.name, cfg.dialog, cfg.outfit);
        // NPC 初始站在建筑附近（偏移一点避免和建筑图元重合）
        npc->setPos(b->x() + cfg.offsetX, b->y() + cfg.offsetY);
        // 设置路网图指针和驻地建筑ID，使NPC能自主游走
        npc->setGraph(&campus_);
        npc->setHomeBuildingId(cfg.nearBuildingId);
        mapScene_->addItem(npc);
        npcs_.push_back(npc);

        // 连接点击信号 → 主窗口弹出对话
        connect(npc, &NpcItem::clicked,
                this, &MainWindow::onNpcClicked);
    }
}

void MainWindow::onNpcClicked(const QString& name, const QString& dialog) {
    // 弹出对话气泡：用 QMessageBox 的 information 模拟 RPG 对话框
    QMessageBox::information(this,
        "💬 " + name + " 说：",
        dialog);
}

// ============================================================
//  扩展功能：随机生成NPC
//  点击"随机生成NPC"按钮后，在地图上随机生成3-6个NPC。
//  每个NPC的所在建筑、衣服颜色、名字、对话内容均随机选取。
//  再次点击会先清除上次随机生成的NPC，再重新生成一批。
// ============================================================
void MainWindow::onRandomNpc() {
    // --- 清除上次随机生成的NPC ---
    for (auto* npc : randomNpcs_) {
        auto it = std::find(npcs_.begin(), npcs_.end(), npc);
        if (it != npcs_.end()) npcs_.erase(it);
        delete npc;  // QGraphicsObject 析构会自动从场景移除
    }
    randomNpcs_.clear();

    // 收集所有建筑ID（随机选位置用，排除路口节点）
    QList<int> buildingIds;
    for (const auto& [id, b] : campus_.allBuildings())
        if (b.nodeKind() == 1) buildingIds.append(id);
    if (buildingIds.isEmpty()) return;

    // 随机生成 3-6 个NPC
    int count = QRandomGenerator::global()->bounded(3, 7);  // [3, 6]

    // NPC 名字池
    static const QStringList names = {
        "学生", "老师", "游客", "保安",
        "清洁工", "快递员", "新生", "校友"
    };
    // NPC 对话池
    static const QStringList dialogs = {
        "今天天气不错，适合在校园里散步～",
        "这所大学的校园真大啊，我差点迷路了！",
        "同学你好！请问图书馆怎么走？",
        "我刚下课，准备去食堂吃饭。",
        "校园里的风景四季都不同，很美。",
        "我是来参观的，这所学校真漂亮！",
        "今天有很多活动，操场那边很热闹。",
        "你好！我是这所学校的学生，很高兴认识你～"
    };
    // 5种衣服颜色
    static const NpcItem::Outfit outfits[] = {
        NpcItem::Outfit::Red, NpcItem::Outfit::Green,
        NpcItem::Outfit::Purple, NpcItem::Outfit::Orange,
        NpcItem::Outfit::Pink
    };

    int nameCount   = static_cast<int>(names.size());
    int dialogCount = static_cast<int>(dialogs.size());

    for (int i = 0; i < count; ++i) {
        // 随机选一栋建筑作为NPC初始位置
        int bIdx = QRandomGenerator::global()->bounded(buildingIds.size());
        int buildingId = buildingIds[bIdx];
        const Building* b = campus_.getBuilding(buildingId);
        if (!b) continue;

        // 随机名字、对话、衣服颜色、偏移量
        int nameIdx   = QRandomGenerator::global()->bounded(nameCount);
        int dialogIdx = QRandomGenerator::global()->bounded(dialogCount);
        int outfitIdx = QRandomGenerator::global()->bounded(5);
        int offsetX   = QRandomGenerator::global()->bounded(-40, 41);  // [-40, 40]
        int offsetY   = QRandomGenerator::global()->bounded(-40, 41);

        auto* npc = new NpcItem(names[nameIdx], dialogs[dialogIdx],
                                outfits[outfitIdx]);
        npc->setPos(b->x() + offsetX, b->y() + offsetY);
        npc->setGraph(&campus_);
        npc->setHomeBuildingId(buildingId);
        mapScene_->addItem(npc);
        npcs_.push_back(npc);
        randomNpcs_.push_back(npc);

        connect(npc, &NpcItem::clicked, this, &MainWindow::onNpcClicked);
    }

    statusLabel_->setText(
        tr("🎲 随机生成了 %1 个NPC（当前共 %2 个NPC）")
            .arg(count)
            .arg(npcs_.size()));
}

// --- 角色移动定时器槽：每帧根据按键状态更新角色位置 ---
void MainWindow::onCharacterTimer() {
    if (!character_) return;

    // 模态对话框打开或窗口失焦时，清除所有移动标志
    // 防止按键释放事件被对话框截获导致角色持续移动
    if (QApplication::activeModalWidget() || !isActiveWindow()) {
        moveUp_ = moveDown_ = moveLeft_ = moveRight_ = false;
        character_->setWalking(false);
        return;
    }

    qreal dx = 0, dy = 0;
    if (moveLeft_)  dx -= character_->speed();
    if (moveRight_) dx += character_->speed();
    if (moveUp_)    dy -= character_->speed();
    if (moveDown_)  dy += character_->speed();

    // 对角线移动归一化（避免斜向更快）
    if (dx != 0 && dy != 0) {
        double factor = 1.0 / std::sqrt(2.0);
        dx *= factor; dy *= factor;
    }

    if (dx == 0 && dy == 0) {
        character_->setWalking(false);   // 没按键：停止行走动画
        return;
    }

    // 正在移动：设置行走状态并推进动画帧
    character_->setWalking(true);
    character_->advanceFrame();

    // 更新朝向（优先水平/垂直中较大的方向）
    if (std::abs(dx) > std::abs(dy)) {
        character_->setDirection(dx > 0 ? CharacterItem::Direction::Right
                                         : CharacterItem::Direction::Left);
    } else {
        character_->setDirection(dy > 0 ? CharacterItem::Direction::Down
                                         : CharacterItem::Direction::Up);
    }

    // 移动角色
    character_->moveBy(dx, dy);

    // 边界检测：不让角色跑出场景范围
    QPointF pos = character_->pos();
    QRectF sceneRect = mapScene_->sceneRect();
    if (pos.x() < sceneRect.left())  pos.setX(sceneRect.left());
    if (pos.x() > sceneRect.right()) pos.setX(sceneRect.right());
    if (pos.y() < sceneRect.top())   pos.setY(sceneRect.top());
    if (pos.y() > sceneRect.bottom())pos.setY(sceneRect.bottom());
    if (pos != character_->pos()) character_->setPos(pos);

    // --- 新生模式：途经宿舍楼自动弹窗推送入住提示 ---
    // --- 访客模式：途经景点自动弹窗推送导游信息 ---
    if (appMode_ != AppMode::Normal) {
        QPointF charPos = character_->pos();
        for (const auto& [id, building] : campus_.allBuildings()) {
            if (building.nodeKind() == 0) continue;   // 路口节点不触发提示
            double d = std::sqrt(
                std::pow(charPos.x() - building.x(), 2) +
                std::pow(charPos.y() - building.y(), 2));
            if (d < 40.0 && id != lastTipBuildingId_) {
                lastTipBuildingId_ = id;

                if (appMode_ == AppMode::Freshman) {
                    // 新生模式：途经宿舍/关键点位弹窗
                    if (id == 39) {
                        QMessageBox::information(this,
                            tr("🏠 宿舍入住提示 - %1").arg(building.name()),
                            tr("您已到达%1。\n\n"
                               "📋 入住流程：到一楼宿管处登记\n"
                               "🔑 领取：宿舍钥匙+门禁卡\n"
                               "⚡ 用电提醒：禁用大功率电器(>500W)\n"
                               "🔧 报修：宿管处登记 / 后勤ext.8002").arg(building.name()));
                    } else if (id == 27) {
                        QMessageBox::information(this,
                            tr("📝 报到办理 - %1").arg(building.name()),
                            tr("报到办理点已到达！\n\n"
                               "1. 出示录取通知书+身份证\n"
                               "2. 领取学生证和校园卡\n"
                               "3. 缴费确认（学费+住宿费）\n"
                               "4. 领取宿舍分配单"));
                    } else if (id == 32) {
                        QMessageBox::information(this,
                            tr("💊 医保办理 - %1").arg(building.name()),
                            tr("校医院已到达！\n\n"
                               "📋 医保登记：一楼挂号处\n"
                               "💉 体检安排：开学第一周\n"
                               "📞 急诊24小时：ext.8120"));
                    }
                } else if (appMode_ == AppMode::Visitor) {
                    // 访客模式：途经景点弹窗导游
                    if (id == 31 || id == 28 || id == 27 || id == 29 || id == 13 || id == 14 || id == 5) {
                        QString intro;
                        switch (id) {
                            case 31: intro = QStringLiteral("⛲ 朝阳广场\n校园中心广场，周边有商铺和咖啡厅。\n定期举办文化活动和展览。"); break;
                            case 28: intro = QStringLiteral("🍱 一食堂\n大众餐窗口，家常菜性价比高。\n三层建筑，6:30-21:00营业。"); break;
                            case 27: intro = QStringLiteral("🎭 大学生活动中心\n社团活动/学生事务/心理咨询/剧场。\n校园文化生活的核心场所。"); break;
                            case 29: intro = QStringLiteral("🍜 二食堂\n风味小吃+清真窗口+智慧餐厅。\n6:30-21:00营业。"); break;
                            case 13: intro = QStringLiteral("🕰️ 钟楼\n校园标志性建筑，高约30米。\n每整点报时，是天理的精神象征。"); break;
                            case 14: intro = QStringLiteral("🏟️ 体育馆\n室内篮球/羽毛球/健身房/游泳馆。\n6:00-22:00开放。"); break;
                            case 5:  intro = QStringLiteral("📚 图书馆\n建筑面积4.6万平米，藏书215万册。\n藏借阅一体化管理，4600余阅览座位。"); break;
                        }
                        QMessageBox::information(this,
                            tr("🏛️ 景点导游 - %1").arg(building.name()),
                            intro);
                    }
                }
                break;  // 一次只弹一个窗
            }
            // 远离后重置，允许下次再弹
            if (d > 80.0 && id == lastTipBuildingId_) {
                lastTipBuildingId_ = -1;
            }
        }
    }

    // 镜头跟随角色（平滑过渡）
    updateCamera();
}

// --- 键盘按下：记录方向键状态 ---
void MainWindow::keyPressEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    // 如果正在自动导航，按任意方向键中断导航，切回手动
    if (navigating_) {
        if (event->key() == Qt::Key_W || event->key() == Qt::Key_Up ||
            event->key() == Qt::Key_S || event->key() == Qt::Key_Down ||
            event->key() == Qt::Key_A || event->key() == Qt::Key_Left ||
            event->key() == Qt::Key_D || event->key() == Qt::Key_Right) {
            stopAutoNavigate();
            statusLabel_->setText(tr("已中断导航 — WASD 手动控制"));
        }
    }

    switch (event->key()) {
        case Qt::Key_W: case Qt::Key_Up:    moveUp_    = true; break;
        case Qt::Key_S: case Qt::Key_Down:  moveDown_  = true; break;
        case Qt::Key_A: case Qt::Key_Left:  moveLeft_  = true; break;
        case Qt::Key_D: case Qt::Key_Right: moveRight_ = true; break;
        default: QMainWindow::keyPressEvent(event); return;
    }
    event->accept();
}

// --- 键盘释放：清除方向键状态 ---
void MainWindow::keyReleaseEvent(QKeyEvent* event) {
    if (event->isAutoRepeat()) return;

    switch (event->key()) {
        case Qt::Key_W: case Qt::Key_Up:    moveUp_    = false; break;
        case Qt::Key_S: case Qt::Key_Down:  moveDown_  = false; break;
        case Qt::Key_A: case Qt::Key_Left:  moveLeft_  = false; break;
        case Qt::Key_D: case Qt::Key_Right: moveRight_ = false; break;
        default: QMainWindow::keyReleaseEvent(event); return;
    }
    event->accept();
}

void MainWindow::onBuildingClicked(int id) {
    const Building* b = campus_.getBuilding(id); if (!b) return;

    // 路网编辑模式：建筑点击用于"连接道路"的第一/第二点
    if (networkEditMode_ && netEditMode_ == NetEditMode::Connect) {
        if (connectFirstId_ < 0) {
            connectFirstId_ = id;
            statusLabel_->setText(
                tr("🔗 已选第一点: %1 — 请点【第二点】（路口或建筑）").arg(b->name()));
        } else if (connectFirstId_ == id) {
            statusLabel_->setText(tr("⚠ 不能连接到自己，请选其他点"));
        } else {
            // 用欧式距离建一条直路
            if (DatabaseManager::instance().addRoadEuclidean(connectFirstId_, id)) {
                mapScene_->addRoadItem(connectFirstId_, id);
                statusLabel_->setText(
                    tr("✅ 已连接 %1 ↔ %2 — 可继续点下一对的起点，或换工具")
                       .arg(campus_.getBuilding(connectFirstId_)->name(), b->name()));
            } else {
                statusLabel_->setText(tr("⚠ 连接失败（可能已存在）"));
            }
            // 清掉"已选第一点"提示
            if (auto* ii = mapScene_->getIntersectionItem(connectFirstId_)) ii->setSelectedVisual(false);
            connectFirstId_ = -1;
            mapScene_->clearConnectionPreview();
        }
        return;
    }

    if (selectedStartId_ < 0) {
        selectedStartId_ = id; mapScene_->setStartPoint(id);
        startLabel_->setText(tr("起点: %1").arg(b->name()));
        statusLabel_->setText(tr("已选起点: %1 — 请点终点").arg(b->name()));
    } else if (selectedEndId_ < 0) {
        selectedEndId_ = id;
        endLabel_->setText(tr("终点: %1").arg(b->name()));
        statusLabel_->setText(tr("已选终点: %1 — 点导航").arg(b->name()));
    } else {
        mapScene_->clearPoints();
        selectedStartId_ = id; selectedEndId_ = -1;
        mapScene_->setStartPoint(id);
        startLabel_->setText(tr("起点: %1").arg(b->name()));
        endLabel_->setText(tr("终点: 未选择"));
        statusLabel_->setText(tr("重新选择起点: %1").arg(b->name()));
    }
}

void MainWindow::onNavigate() {
    if (selectedStartId_ < 0 || selectedEndId_ < 0) {
        QMessageBox::information(this,tr("提示"),tr("请先选择起点和终点！")); return;
    }

    // 关键：先走"建筑→最近路口→路网→最近路口→建筑"的路由，而不是直接
    // 在两栋建筑之间求最短路（手动路网下建筑是孤立节点，直连必为不可达）。
    std::vector<int> path = walkPathFor(selectedStartId_, selectedEndId_);
    if (path.empty()) {
        statusLabel_->setText(tr("不可达！两栋建筑附近没有连通的路口路网"
                                 "（请在路网编辑模式里把它们的路口用道路连起来）"));
        return;
    }

    // 角色实际行走的"路口路网"路径（即上面拼好的完整路径）
    std::vector<int> walkPath = path;

    // 计算沿实际路网行走的总距离（含建筑↔最近路口的接入段）
    double total = 0.0;
    for (size_t i = 0; i + 1 < walkPath.size(); ++i) {
        bool foundEdge = false;
        for (const Edge& e : campus_.neighbors(walkPath[i])) {
            if (e.to == walkPath[i + 1]) { total += e.weight; foundEdge = true; break; }
        }
        if (!foundEdge) {
            // 建筑 ↔ 最近路口 的接入段（数据库里没有这条边，用欧氏距离估算）
            const Building* a = campus_.getBuilding(walkPath[i]);
            const Building* b = campus_.getBuilding(walkPath[i + 1]);
            if (a && b) {
                double dx = a->x() - b->x(), dy = a->y() - b->y();
                total += std::sqrt(dx * dx + dy * dy);
            }
        }
    }

    // 显示起终点建筑名称
    const Building* sb = campus_.getBuilding(selectedStartId_);
    const Building* eb = campus_.getBuilding(selectedEndId_);
    QString ps = (sb ? sb->name() : "?") + tr(" -> ") + (eb ? eb->name() : "?");

    statusLabel_->setText(tr("最短路径(%1米): %2 — 角色沿道路自动导航中...").arg(total,0,'f',0).arg(ps));

    // --- 启动角色自动导航（内部会按 起点→终点 高亮实际行走的路口路网）---
    startAutoNavigate({selectedStartId_, selectedEndId_});
}

// ============================================================
//  自动导航实现
// ============================================================

// 计算角色实际行走的路口路网路径（建筑仅作门口接入，不穿过建筑中心）
//   完整路径 = 起点建筑 → 最近路口 → 路网 → 最近路口 → 终点建筑
//   这样无论用户手动画了多少路口，导航都从"建筑门口最近的那个路口"出发，
//   沿真实道路走到"终点建筑门口最近的路口"，最后接入终点建筑。
std::vector<int> MainWindow::walkPathFor(int startB, int endB) const {
    int js = campus_.nearestJunction(startB);
    int je = campus_.nearestJunction(endB);
    if (js >= 0 && je >= 0) {
        // 只走路口节点（node_kind==0）的最短路径，保证角色在道路上
        std::vector<int> w = campus_.dijkstraJunctions(js, je);
        if (!w.empty()) {
            // 拼成 建筑 → 最近路口 → ... → 最近路口 → 建筑
            std::vector<int> full;
            full.push_back(startB);
            for (int id : w) full.push_back(id);
            full.push_back(endB);
            return full;
        }
    }
    // 兜底：若两建筑之间本来就存在直连道路（极少数情况）
    return campus_.dijkstraWithWeather(startB, endB, weatherMode_);
}

// 启动自动导航：角色沿路径节点序列逐个移动
void MainWindow::startAutoNavigate(const std::vector<int>& path) {
    if (!character_ || path.empty()) return;

    // 如果正在导航，先停止
    stopAutoNavigate();

    // 保存路径（建筑 id 序列，供其他逻辑参考）
    navPath_ = path;
    navigating_ = true;

    // 角色只走"路口路网"。path 可能是 [起点, 终点]（普通导航）或
    // [站1, 站2, ..., 站N]（新生/访客多站漫游）。逐段计算"建筑→最近路口→
    // 路网→最近路口→建筑"的实际行走路径并拼接，保证角色依次经过每个站点。
    std::vector<int> walkPath;
    for (size_t i = 0; i + 1 < path.size(); ++i) {
        std::vector<int> leg = walkPathFor(path[i], path[i + 1]);
        if (leg.empty()) continue;            // 容错：跳过不可达段
        if (walkPath.empty()) walkPath = leg;
        else walkPath.insert(walkPath.end(), leg.begin() + 1, leg.end());
    }
    if (walkPath.empty()) walkPath = path;   // 兜底

    // 高亮角色实际要走的道路（路口路网），而非穿建筑的路径
    mapScene_->highlightPath(walkPath);

    // 构建完整折线点列：逐段取出道路有序点列
    // 角色将严格沿这些灰色道路行走（含网格拐点）
    navPoints_.clear();
    const Building* b0 = campus_.getBuilding(walkPath[0]);
    if (!b0) return;
    navPoints_.append(QPointF(b0->x(), b0->y()));

    for (size_t i = 0; i + 1 < walkPath.size(); ++i) {
        QVector<QPointF> seg;
        if (campus_.getRoadPolyline(walkPath[i], walkPath[i + 1], seg)) {
            // seg 含两端与拐点，跳过与上一个点重合的起点
            for (int k = 1; k < seg.size(); ++k) navPoints_.append(seg[k]);
        } else {
            // 找不到道路（理论上不会发生），退化为直连终点
            const Building* bn = campus_.getBuilding(walkPath[i + 1]);
            if (bn) navPoints_.append(QPointF(bn->x(), bn->y()));
        }
    }

    navPointIndex_ = 0;
    character_->setPos(navPoints_[0]);
    if (navPoints_.size() > 1) {
        navPointIndex_ = 1;   // 下一个目标点
    }

    // 启动导航定时器
    navTimer_->start();

    // 导航期间禁用手动移动定时器（避免冲突）
    moveTimer_->stop();
}

// 停止自动导航
void MainWindow::stopAutoNavigate() {
    if (navTimer_) navTimer_->stop();
    navigating_ = false;
    navPath_.clear();
    navTargetIndex_ = 0;

    // 恢复手动移动定时器
    if (moveTimer_ && !moveTimer_->isActive()) {
        moveTimer_->start();
    }
}

// 导航定时器槽：每帧让角色沿折线点列前进一步（含道路拐点）
void MainWindow::onNavigateTimer() {
    if (!character_ || !navigating_) return;

    // 已走完所有折线点 → 到达终点
    if (navPointIndex_ >= navPoints_.size()) {
        stopAutoNavigate();
        statusLabel_->setText(tr("已到达目的地！— WASD 可继续手动移动"));
        return;
    }

    // 当前位置与目标点
    QPointF curPos = character_->pos();
    QPointF target = navPoints_[navPointIndex_];

    // 计算方向向量
    double dx = target.x() - curPos.x();
    double dy = target.y() - curPos.y();
    double dist = std::sqrt(dx * dx + dy * dy);

    // 角色速度（每帧移动的像素数）
    double speed = character_->speed() * 1.5;   // 导航时稍快一点

    if (dist <= speed) {
        // 已到达当前折线点
        character_->setPos(target);

        // 前进到下一个折线点
        ++navPointIndex_;
        if (navPointIndex_ >= navPoints_.size()) {
            stopAutoNavigate();
            statusLabel_->setText(tr("已到达目的地！— WASD 可继续手动移动"));
            return;
        }
        return;
    }

    // 还没到目标 → 朝目标方向移动一步
    double nx = dx / dist;   // 单位方向向量
    double ny = dy / dist;

    // 更新角色朝向
    if (std::abs(nx) > std::abs(ny)) {
        character_->setDirection(nx > 0 ? CharacterItem::Direction::Right
                                         : CharacterItem::Direction::Left);
    } else {
        character_->setDirection(ny > 0 ? CharacterItem::Direction::Down
                                         : CharacterItem::Direction::Up);
    }

    // 移动
    character_->moveBy(nx * speed, ny * speed);

    // 镜头跟随角色（平滑过渡）
    updateCamera();
}

void MainWindow::onClearPath() {
    stopAutoNavigate();   // 清除时也要停止导航
    mapScene_->clearPoints();
    selectedStartId_=-1; selectedEndId_=-1;
    startLabel_->setText(tr("起点: 未选择")); endLabel_->setText(tr("终点: 未选择"));
    statusLabel_->setText(tr("已清除 — 请重新选择"));
}

void MainWindow::onShowInfo() {
    int t = (selectedEndId_>=0)?selectedEndId_:selectedStartId_;
    if(t<0){ QMessageBox::information(this,tr("提示"),tr("请先点击一栋建筑！")); return;}
    const Building* b = campus_.getBuilding(t); if(!b) return;
    QMessageBox::information(this,tr("建筑信息-%1").arg(b->name()), b->description());
}

// --- 搜索建筑设为起点 ---
void MainWindow::onSearchStart() {
    QString keyword = searchEdit_->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请输入建筑名称！"));
        return;
    }

    // 在 Graph 中查找名称匹配的建筑（支持模糊匹配，排除路口节点）
    int foundId = -1;
    for (const auto& [id, building] : campus_.allBuildings()) {
        if (building.nodeKind() == 0) continue;   // 路口节点不参与搜索
        if (building.name().contains(keyword, Qt::CaseInsensitive)) {
            foundId = id;
            break;
        }
    }

    if (foundId < 0) {
        QMessageBox::information(this, tr("未找到"),
            tr("未找到包含\"%1\"的建筑。").arg(keyword));
        return;
    }

    selectedStartId_ = foundId;
    mapScene_->setStartPoint(foundId);
    const Building* b = campus_.getBuilding(foundId);
    startLabel_->setText(tr("起点: %1").arg(b->name()));
    statusLabel_->setText(tr("已通过搜索设起点: %1").arg(b->name()));
}

// --- 搜索建筑设为终点 ---
void MainWindow::onSearchEnd() {
    QString keyword = searchEdit_->text().trimmed();
    if (keyword.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("请输入建筑名称！"));
        return;
    }

    int foundId = -1;
    for (const auto& [id, building] : campus_.allBuildings()) {
        if (building.nodeKind() == 0) continue;   // 路口节点不参与搜索
        if (building.name().contains(keyword, Qt::CaseInsensitive)) {
            foundId = id;
            break;
        }
    }

    if (foundId < 0) {
        QMessageBox::information(this, tr("未找到"),
            tr("未找到包含\"%1\"的建筑。").arg(keyword));
        return;
    }

    selectedEndId_ = foundId;
    endLabel_->setText(tr("终点: %1").arg(campus_.getBuilding(foundId)->name()));
    statusLabel_->setText(tr("已通过搜索设终点: %1").arg(campus_.getBuilding(foundId)->name()));
}

void MainWindow::showEvent(QShowEvent* event) {
    QMainWindow::showEvent(event);
    if (mapScene_) {
        // 启动即放大显示（1.5x），便于看清道路网格与路口节点；
        // 用户可左键拖动平移、滚轮缩放查看完整地图
        zoomLevel_ = 1.5;
        mapView_->resetTransform();
        mapView_->scale(zoomLevel_, zoomLevel_);
        mapView_->centerOn(mapScene_->sceneRect().center());
        updateLabelScale();
    }
}

// --- 管理员模式：密码验证后打开管理对话框 ---
void MainWindow::onAdminMode() {
    // 若正处于画路模式，点此按钮先退出画路（回到管理面板）
    if (drawRoadMode_) {
        cancelDrawRoad();
    }

    // 首次进入需要密码；已处于管理员模式则直接打开管理面板（无需退出重进）
    if (!isAdminMode_) {
        bool ok = false;
        QString password = QInputDialog::getText(
            this, tr("管理员验证"),
            tr("请输入管理员密码:"),
            QLineEdit::Password, "", &ok);
        if (!ok) return;   // 用户取消

        if (password != "admin") {
            QMessageBox::warning(this, tr("验证失败"), tr("密码错误！"));
            return;
        }

        // 开启管理员模式
        isAdminMode_ = true;
        adminBtn_->setText(tr("管理面板"));
        QMessageBox::information(this, tr("管理员模式已开启"),
            tr("已进入管理员模式:\n\n"
               "  • 建筑/道路增删改查：在弹出的对话框中操作\n"
               "  • 退出管理员模式：点对话框底部\"退出管理员模式\""));
    }

    // 打开/重新打开管理员对话框（无论首次进入，还是已在管理面板时再点都打开）
    AdminDialog dialog(this);
    connect(&dialog, &AdminDialog::drawRoadRequested,
            this,    &MainWindow::onStartDrawRoad);
    connect(&dialog, &AdminDialog::exitAdminRequested,
            this,    &MainWindow::onExitAdminMode);
    connect(&dialog, &AdminDialog::enterNetworkEditRequested,
            this,    &MainWindow::onEnterNetworkEditMode);
    dialog.exec();

    // 关闭对话框后刷新地图
    reloadMap();
    reloadMap();
    if (!isAdminMode_) {
        statusLabel_->setText(tr("已退出管理员模式"));
    } else if (drawRoadMode_) {
        statusLabel_->setText(
            tr("🖊 画路模式：先点【起点建筑】→ 点地图加拐点 → 点【终点建筑】结束。右键取消。"));
    } else {
        statusLabel_->setText(
            tr("管理面板已打开 - 可增删建筑/道路，或点\"地图画路\"连拐点建路（可连画多条）"));
    }
}

// 退出管理员模式（由对话框底部按钮触发）
void MainWindow::onExitAdminMode() {
    isAdminMode_   = false;
    drawRoadMode_  = false;
    drawStartId_   = -1;
    drawWaypoints_.clear();
    clearDrawMarkers();
    adminBtn_->setText(tr("管理员模式"));
}

// 生成/重置道路网格（交通路网模式）
void MainWindow::onGenerateNetwork() {
    QMessageBox::StandardButton reply = QMessageBox::question(
        this, tr("生成道路网格"),
        tr("将重新生成路口网格并重置所有道路，使角色沿路网行走。\n"
           "你的 70 个建筑不会被删除或修改，仅道路表会被重排。\n\n"
           "是否继续？（程序会自动备份当前数据库）"),
        QMessageBox::Yes | QMessageBox::No);
    if (reply != QMessageBox::Yes) return;

    // 自动备份当前数据库，便于回退
    QString dbPath = DatabaseManager::instance().databasePath();
    if (!dbPath.isEmpty()) {
        QString backup = dbPath + ".bak_network_" +
                         QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
        QFile::copy(dbPath, backup);
    }

    // 重新生成路网
    bool ok = DatabaseManager::instance().generateRoadNetwork();
    if (!ok) {
        QMessageBox::warning(this, tr("失败"), tr("生成道路网格失败，请查看日志。"));
        return;
    }

    // 刷新地图
    reloadMap();
    int jc = DatabaseManager::instance().junctionCount();
    statusLabel_->setText(
        tr("🛣 已生成道路网格：%1 个路口，角色现在沿路网行走。").arg(jc));
}

// 从数据库重新加载并重建整张地图（建筑/道路含拐点）
void MainWindow::reloadMap() {
    if (!mapScene_) return;

    // 清理旧图元、NPC 与画路临时标记
    // 注意：此处【不要】重置 drawRoadMode_——从"地图画路"入口进入时，
    // onStartDrawRoad() 已置 true，紧接着的 reloadMap() 若重置会把它关掉，
    // 导致回到地图后点击又变回弹坐标。drawRoadMode_ 只由 finishDrawRoad/cancelDrawRoad 置 false。
    for (auto* npc : npcs_) delete npc;
    npcs_.clear();
    randomNpcs_.clear();
    clearDrawMarkers();
    drawStartId_ = -1;
    mapScene_->clear();

    // 重新加载数据
    campus_ = Graph();
    auto buildings = DatabaseManager::instance().loadAllBuildings();
    for (const auto& b : buildings) campus_.addBuilding(b);
    auto roads = DatabaseManager::instance().loadAllRoads();
    for (const auto& r : roads) {
        QVector<QPointF> wps = DatabaseManager::instance().jsonToWaypoints(r.points);
        campus_.addRoad(r.fromId, r.toId, r.weight, wps);
    }

    mapScene_->loadFromGraph(campus_);

    // 重新放置角色
    const Building* gate = campus_.getBuilding(0);
    QPointF startPos = gate ? QPointF(gate->x(), gate->y()) : QPointF(400, 500);
    character_ = new CharacterItem();
    character_->setPos(startPos);
    mapScene_->addItem(character_);

    // 重新放置天气层
    QRectF sr = mapScene_->sceneRect();
    weatherOverlay_ = new WeatherOverlay();
    weatherOverlay_->setPos(sr.topLeft());
    weatherOverlay_->setAreaSize(sr.width(), sr.height());
    mapScene_->addItem(weatherOverlay_);

    // 重新放置NPC
    initNpcs();

    // 统计可见建筑数（排除路口节点）
    int visCnt = 0;
    for (const auto& [id, b] : campus_.allBuildings())
        if (b.nodeKind() == 1) ++visCnt;

    statusLabel_->setText(
        tr("地图已刷新 - %1 栋建筑，%2 条道路，%3 个NPC")
            .arg(visCnt)
            .arg(campus_.roadCount())
            .arg(npcs_.size()));
}

// ============================================================
//  扩展功能1：昼夜系统（顶层半透明蒙版方案）
// ============================================================
void MainWindow::onToggleDayNight() {
    isNight_ = !isNight_;

    if (isNight_) {
        // 夜间：创建/显示半透明深蓝灰蒙版覆盖整张地图
        if (!nightOverlay_) {
            nightOverlay_ = new QGraphicsRectItem();
            nightOverlay_->setZValue(100);  // 高层，覆盖建筑/道路/角色
            nightOverlay_->setBrush(QColor(15, 25, 60, 130));  // 深蓝灰半透明
            nightOverlay_->setPen(Qt::NoPen);
            // 纯蒙版：关闭鼠标交互，避免吞掉路网编辑模式的点击
            nightOverlay_->setAcceptedMouseButtons(Qt::NoButton);
            mapScene_->addItem(nightOverlay_);
        }
        nightOverlay_->setVisible(true);
        updateNightOverlay();   // 更新蒙版大小和位置

        // 侧边面板暗色化（仅 mapView_ 背景微调，不影响侧边栏）
        dayNightBtn_->setText(tr("切换白天"));
        statusLabel_->setText(tr("已切换至夜间模式 — 蒙版覆盖整张地图"));
    } else {
        // 白天：隐藏蒙版，恢复明亮
        if (nightOverlay_) {
            nightOverlay_->setVisible(false);
        }
        dayNightBtn_->setText(tr("切换昼夜"));
        statusLabel_->setText(tr("已切换至白天模式"));
    }

    // 小地图同步昼夜效果
    if (miniMapView_) {
        if (isNight_) {
            miniMapView_->setStyleSheet(
                "QGraphicsView{border:2px solid #FFD54F;background:#3E2723;border-radius:4px;}");
        } else {
            miniMapView_->setStyleSheet(
                "QGraphicsView{border:2px solid #FFCC80;background:#FFF3E0;border-radius:4px;}");
        }
    }
}

// 更新夜间蒙版的位置和大小（跟随地图场景范围）
void MainWindow::updateNightOverlay() {
    if (!nightOverlay_ || !nightOverlay_->isVisible() || !mapScene_) return;

    // 蒙版覆盖整个场景矩形
    QRectF sr = mapScene_->sceneRect();
    nightOverlay_->setRect(sr);
}

// ============================================================
//  镜头跟随：平滑跟随角色，带边界限制
// ============================================================
void MainWindow::updateCamera() {
    if (!character_ || !mapView_ || !mapScene_) return;

    // 目标中心 = 角色位置
    QPointF target = character_->pos();

    // 边界限制：不让镜头中心超出地图边界
    clampCenter(target);

    // 平滑过渡：当前中心向目标中心靠近 15%（避免突兀抖动）
    QPointF current = mapView_->mapToScene(
        mapView_->viewport()->rect().center());
    QPointF smoothed(
        current.x() + (target.x() - current.x()) * 0.15,
        current.y() + (target.y() - current.y()) * 0.15);

    // 再次限制平滑后的中心
    clampCenter(smoothed);

    mapView_->centerOn(smoothed);

    // 更新蒙版和迷你地图
    updateNightOverlay();
    updateMiniMap();
}

// 限制镜头中心不超出地图边界（防止空白露出）
void MainWindow::clampCenter(QPointF& center) const {
    if (!mapScene_ || !mapView_) return;

    QRectF sceneRect = mapScene_->sceneRect();
    QPointF viewHalf = mapView_->mapToScene(
        QPoint(mapView_->viewport()->width() / 2,
               mapView_->viewport()->height() / 2)) -
        mapView_->mapToScene(QPoint(0, 0));

    qreal halfW = qAbs(viewHalf.x());
    qreal halfH = qAbs(viewHalf.y());

    // 如果视图比场景还大，就不限制（让fitInView处理）
    if (halfW * 2 >= sceneRect.width()) return;
    if (halfH * 2 >= sceneRect.height()) return;

    center.setX(qBound(sceneRect.left() + halfW,
                       center.x(),
                       sceneRect.right() - halfW));
    center.setY(qBound(sceneRect.top() + halfH,
                       center.y(),
                       sceneRect.bottom() - halfH));
}

// ============================================================
//  滚轮缩放：以鼠标位置为中心，限制上下限
// ============================================================
// ============================================================
//  地图画路（管理员，替代旧坐标点击器）
//  流程：点起点建筑 → 点地图加拐点 → 点终点建筑，自动建路
// ============================================================

// 进入画路模式（由管理员对话框"地图画路"按钮触发）
void MainWindow::onStartDrawRoad() {
    if (!isAdminMode_) {
        statusLabel_->setText(tr("请先进入管理员模式再画路"));
        return;
    }
    drawRoadMode_ = true;
    drawStartId_   = -1;
    drawWaypoints_.clear();
    clearDrawMarkers();
    statusLabel_->setText(
        tr("🖊 画路模式：先点【起点建筑】→ 点地图加拐点 → 点【终点建筑】结束。右键取消。"));
}

// 命中建筑检测：返回该点所在的建筑 id（没有则返回 -1）
int MainWindow::buildingIdAt(const QPointF& p) {
    if (!mapScene_) return -1;
    for (int id : mapScene_->allBuildingIds()) {
        BuildingItem* item = mapScene_->getBuildingItem(id);
        if (item && item->sceneBoundingRect().contains(p)) return id;
    }
    return -1;
}

// 在地图上放置一个临时标记（起点/拐点）
void MainWindow::addDrawMarker(qreal x, qreal y, const QColor& color, qreal r) {
    if (!mapScene_) return;
    auto* m = new QGraphicsEllipseItem(x - r, y - r, r * 2, r * 2);
    m->setBrush(QBrush(color));
    m->setPen(QPen(color.darker(120), 1));
    m->setZValue(20);   // 在建筑之上，便于看清
    mapScene_->addItem(m);
    drawMarkers_.append(m);
}

// 清除所有临时标记
void MainWindow::clearDrawMarkers() {
    for (auto* m : drawMarkers_) {
        if (m) delete m;
    }
    drawMarkers_.clear();
}

// 取消画路
void MainWindow::cancelDrawRoad() {
    clearDrawMarkers();
    drawStartId_ = -1;
    drawWaypoints_.clear();
    drawRoadMode_ = false;
    statusLabel_->setText(tr("已取消画路。再次点对话框\"地图画路\"可重新开始。"));
}

// 处理画路时的地图点击
void MainWindow::handleDrawRoadClick(const QPointF& p) {
    int bid = buildingIdAt(p);
    if (bid >= 0) {
        // 命中的是建筑
        if (drawStartId_ < 0) {
            drawStartId_ = bid;
            const Building* b = campus_.getBuilding(bid);
            addDrawMarker(b->x(), b->y(), Qt::green, 7);
            statusLabel_->setText(
                tr("起点: %1。请点【终点建筑】完成建路。").arg(b->name()));
        } else if (bid == drawStartId_) {
            statusLabel_->setText(tr("起点和终点不能是同一栋建筑，请点另一栋建筑。"));
        } else {
            finishDrawRoad(bid);
        }
    } else {
        // 命中的是空白地图：起点已选时，作为"拐点"加入折线；否则提示先选起点
        if (drawStartId_ < 0) {
            statusLabel_->setText(tr("请先点【起点建筑】。"));
        } else {
            drawWaypoints_.append(p);
            addDrawMarker(p.x(), p.y(), Qt::yellow, 5);
            statusLabel_->setText(
                tr("已加拐点 (%1, %2)。可继续加拐点，或点【终点建筑】结束。")
                    .arg(p.x(), 0, 'f', 0).arg(p.y(), 0, 'f', 0));
        }
    }
}

// 完成一条道路并写入数据库（折线：起点 + 拐点 + 终点）
void MainWindow::finishDrawRoad(int endId) {
    const Building* a = campus_.getBuilding(drawStartId_);
    const Building* b = campus_.getBuilding(endId);
    if (!a || !b) { cancelDrawRoad(); return; }

    // 以折线（起点→各拐点→终点）各段欧式距离之和作为权重
    double w = 0.0;
    QPointF prev(a->x(), a->y());
    for (const QPointF& wp : drawWaypoints_) {
        w += QLineF(prev, wp).length();
        prev = wp;
    }
    w += QLineF(prev, QPointF(b->x(), b->y())).length();

    int wpCount = drawWaypoints_.size();
    QString pts = DatabaseManager::instance().waypointsToJson(drawWaypoints_);

    bool ok = DatabaseManager::instance().addRoad(drawStartId_, endId, w, pts);

    // 注意：保留 drawRoadMode_ = true，方便连续画多条路；
    // 仅重置"当前这条"的起点与拐点（下一条仍是起点→拐点→终点）
    drawStartId_   = -1;
    drawWaypoints_.clear();
    clearDrawMarkers();

    reloadMap();

    if (ok) {
        statusLabel_->setText(
            tr("✅ 道路已添加：%1 → %2（%3 米，%4 个拐点）。可继续点【起点建筑】画下一条；右键退出画路。")
                .arg(a->name()).arg(b->name())
                .arg(w, 0, 'f', 0).arg(wpCount));
    } else {
        statusLabel_->setText(
            tr("道路添加失败（可能已存在相同道路）。右键退出画路，或检查两端建筑。"));
        drawRoadMode_ = false;   // 失败时退出画路
    }
}

bool MainWindow::eventFilter(QObject* obj, QEvent* event) {
    // --- 管理员模式：地图交互（画路优先，否则弹出坐标）---
    if (obj == mapView_->viewport() && event->type() == QEvent::MouseButtonPress && isAdminMode_) {
        auto* me = static_cast<QMouseEvent*>(event);
        // 路网编辑模式：空白处的点击由 MapScene::mousePressEvent 自己处理；
        // 这里只在画路模式或普通管理员模式下拦截
        if (networkEditMode_) {
            return false;  // 放行，让 scene 处理
        }
        if (me->button() == Qt::RightButton) {
            // 右键取消正在进行的画路
            if (drawRoadMode_) cancelDrawRoad();
            return true;
        }
        if (me->button() == Qt::LeftButton) {
            QPointF scenePos = mapView_->mapToScene(me->pos());
            if (drawRoadMode_) {
                // 画路模式：把点击转给画路逻辑处理
                handleDrawRoadClick(scenePos);
            } else {
                // 普通管理员点击：弹出坐标（手动录入用）
                int x = static_cast<int>(std::round(scenePos.x()));
                int y = static_cast<int>(std::round(scenePos.y()));
                QMessageBox::information(this, tr("地图坐标"),
                    tr("场景坐标:\n  x = %1\n  y = %2\n\n"
                       "可复制使用:\n  (%1, %2)").arg(x).arg(y));
            }
            return true;  // 拦截，不走原有点击逻辑
        }
    }

    // --- 滚轮缩放 ---
    if (obj == mapView_->viewport() && event->type() == QEvent::Wheel) {
        // 管理员模式：直接屏蔽滚轮缩放，锁定原始比例
        if (isAdminMode_) {
            return true;  // 拦截滚轮事件
        }
        auto* wheelEvent = static_cast<QWheelEvent*>(event);

        qreal factor = (wheelEvent->angleDelta().y() > 0) ? 1.15 : 1.0 / 1.15;
        qreal newZoom = zoomLevel_ * factor;

        if (newZoom < ZOOM_MIN || newZoom > ZOOM_MAX) {
            return true;
        }

        zoomLevel_ = newZoom;
        mapView_->scale(factor, factor);
        updateLabelScale();
        updateNightOverlay();
        return true;
    }

    // --- 鼠标移动：连接预览跟随 ---
    if (obj == mapView_->viewport() && event->type() == QEvent::MouseMove) {
        if (networkEditMode_ && netEditMode_ == NetEditMode::Connect && connectFirstId_ >= 0) {
            auto* me = static_cast<QMouseEvent*>(event);
            QPointF scenePos = mapView_->mapToScene(me->pos());
            mapScene_->setConnectionPreview(connectFirstId_, scenePos);
            return false;  // 不拦截
        }
    }

    return QMainWindow::eventFilter(obj, event);
}

// 更新建筑标签字号（随缩放自适应）
void MainWindow::updateLabelScale() {
    if (!mapScene_) return;

    // 缩放越大，标签字号越小（保持视觉大小不变）
    // 缩放越小，标签字号越大（保持可读性）
    // 下限提高到 0.8，避免放大后名字过小看不清
    qreal scale = 1.0 / zoomLevel_;
    scale = qBound(0.8, scale, 1.8);  // 限制范围

    // 遍历所有已加载的建筑图元（含用户新增的 id>=45），更新标签缩放
    for (int id : mapScene_->allBuildingIds()) {
        BuildingItem* item = mapScene_->getBuildingItem(id);
        if (item) {
            item->setLabelScale(scale);
        }
    }
}

// ============================================================
//  扩展功能2：天气模拟（晴→雨→雪 循环切换）
// ============================================================
void MainWindow::onToggleWeather() {
    // 4种天气模式：0=晴 1=雨 2=高温 3=大风
    weatherMode_ = (weatherMode_ + 1) % 4;

    if (!weatherOverlay_) return;

    using WT = WeatherOverlay::WeatherType;
    switch (weatherMode_) {
        case 0:
            weatherOverlay_->setWeather(WT::None);
            weatherBtn_->setText(tr("天气:晴"));
            statusLabel_->setText(tr("天气: 晴朗 — 路线无天气调整"));
            break;
        case 1:
            weatherOverlay_->setWeather(WT::Rain);
            weatherBtn_->setText(tr("天气:雨"));
            statusLabel_->setText(tr("天气: 雨天 — 优先走连廊遮雨棚路段"));
            QMessageBox::information(this, tr("雨天出行提示"),
                tr("当前天气：雨天\n\n"
                   "请携带雨伞\n"
                   "注意路面积水，避免走临湖路段\n"
                   "导航已自动优先选择有连廊遮雨棚的路线\n"
                   "雨天路滑，请放慢步行速度"));
            break;
        case 2:
            weatherOverlay_->setWeather(WT::None);
            weatherBtn_->setText(tr("天气:高温"));
            statusLabel_->setText(tr("天气: 高温 — 优先走树荫密集步道"));
            QMessageBox::information(this, tr("高温出行提示"),
                tr("当前天气：高温\n\n"
                   "请做好防晒措施（遮阳帽/防晒霜）\n"
                   "多补充水分，避免中暑\n"
                   "导航已自动优先选择树荫密集的路线\n"
                   "尽量避免12:00-14:00户外长时间行走"));
            break;
        case 3:
            weatherOverlay_->setWeather(WT::None);
            weatherBtn_->setText(tr("天气:大风"));
            statusLabel_->setText(tr("天气: 大风 — 自动绕行明理湖临湖路段"));
            QMessageBox::information(this, tr("大风出行提示"),
                tr("当前天气：大风\n\n"
                   "远离明理湖等临湖露天区域\n"
                   "注意衣物防风，避免穿宽松外套\n"
                   "导航已自动绕行临湖大风路段\n"
                   "尽量走有建筑遮挡的路线"));
            break;
    }
}

// ============================================================
//  扩展功能3：迷你地图（角落缩略图，角色位置同步）
// ============================================================
void MainWindow::updateMiniMap() {
    if (!miniMapView_ || !mapScene_) return;

    // 迷你地图始终显示完整场景
    // 由于复用同一场景，角色的移动会自动反映在迷你地图上
    // 这里只需要确保缩略图保持 fitInView 状态
    static QRectF lastSceneRect;
    QRectF curRect = mapScene_->sceneRect();
    if (curRect != lastSceneRect) {
        miniMapView_->fitInView(curRect, Qt::KeepAspectRatio);
        lastSceneRect = curRect;
    }

    // 让迷你地图中心跟随角色（可选：让角色始终在迷你图中心）
    if (character_) {
        miniMapView_->centerOn(character_);
    }
}

void MainWindow::resizeEvent(QResizeEvent* event) {
    QMainWindow::resizeEvent(event);

    // 迷你地图固定在主视图右上角
    if (miniMapView_ && mapView_) {
        const int margin = 10;
        int x = mapView_->width() - miniMapView_->width() - margin;
        int y = margin;
        // 相对于父窗口的坐标
        QPoint mapTopLeft = mapView_->mapToParent(QPoint(0, 0));
        miniMapView_->move(mapTopLeft.x() + x, mapTopLeft.y() + y);
        miniMapView_->raise();   // 置于地图之上
    }
}

// 窗口状态变化（窗口↔全屏/最大化）。
// 关键修复：路网编辑子面板(QFrame + 样式背景)若在"窗口模式下先显示过、再切全屏"，
// 其文字渲染缓存不会随 DPI/尺寸变化失效，文字会变成只剩中间横线。
// （反过来——全屏状态下才打开面板——是全新渲染，则正常。）
// 切换后用 hide→show 强制 Qt 按新的 DPI/尺寸重新渲染该面板及其子控件。
void MainWindow::changeEvent(QEvent* event) {
    QMainWindow::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange && netEditPanel_) {
        // 延迟到下一轮事件循环，确保全屏切换已真正应用（拿到新尺寸/DPI 后再重绘）
        QTimer::singleShot(0, this, [this]() {
            if (netEditPanel_ && netEditPanel_->isVisible()) {
                netEditPanel_->setVisible(false);
                netEditPanel_->setVisible(true);
            }
        });
    }
}

// ============================================================
//  新生模式 / 访客模式 实现
// ============================================================

// --- 切换到新生模式 ---
void MainWindow::onSwitchFreshman() {
    appMode_ = AppMode::Freshman;
    lastTipBuildingId_ = -1;
    freshmanBtn_->setChecked(true);
    visitorBtn_->setChecked(false);
    routeBtn_->setText(tr("📋 开学专属路线"));
    tipsBtn_->setText(tr("💡 宿舍入住贴士"));
    statusLabel_->setText(tr("已切换到新生模式 — 点击\"开学专属路线\"开始导航"));

    // 新生路线目标建筑（按名称查找，避免管理员增删改后ID错位）
    // 东门 → 2号公寓 → 南苑一站式 → 一食堂 → 体育馆
    QStringList freshmanNames = {
        QStringLiteral("东门"), QStringLiteral("2号公寓"),
        QStringLiteral("南苑一站式"), QStringLiteral("一食堂"), QStringLiteral("体育馆")
    };
    QList<int> ids = resolveRouteByName(campus_, freshmanNames);

    // 先隐藏全部建筑（包括管理员添加的），再只显示新生路线目标建筑
    mapScene_->hideAllBuildings();
    mapScene_->showOnlyBuildings(ids, true);  // 显示并标红
}

// --- 切换到访客模式 ---
void MainWindow::onSwitchVisitor() {
    appMode_ = AppMode::Visitor;
    lastTipBuildingId_ = -1;
    visitorBtn_->setChecked(true);
    freshmanBtn_->setChecked(false);
    routeBtn_->setText(tr("📋 游览路线"));
    tipsBtn_->setText(tr("💡 电子导游"));
    statusLabel_->setText(tr("已切换到访客模式 — 私密区域已屏蔽，点击\"游览路线\"开始"));

    // 访客路线目标建筑（按名称查找，避免管理员增删改后ID错位）
    // 朝阳广场 → 一食堂 → 大学生活动中心 → 二食堂 → 钟楼 → 体育馆 → 图书馆
    QStringList visitorNames = {
        QStringLiteral("朝阳广场"), QStringLiteral("一食堂"),
        QStringLiteral("大学生活动中心"), QStringLiteral("二食堂"),
        QStringLiteral("钟楼"), QStringLiteral("体育馆"), QStringLiteral("图书馆")
    };
    QList<int> ids = resolveRouteByName(campus_, visitorNames);

    // 先隐藏全部建筑（包括管理员添加的），再只显示访客路线目标建筑
    mapScene_->hideAllBuildings();
    mapScene_->showOnlyBuildings(ids, true);  // 显示并标红
}

// --- 切换回普通模式 ---
void MainWindow::onSwitchNormal() {
    appMode_ = AppMode::Normal;
    lastTipBuildingId_ = -1;
    freshmanBtn_->setChecked(false);
    visitorBtn_->setChecked(false);
    routeBtn_->setText(tr("📋 专属路线"));
    tipsBtn_->setText(tr("💡 小贴士/导游"));
    statusLabel_->setText(tr("已切换到普通模式 — 全部功能可用"));

    // 恢复所有建筑可见+取消选中（包括管理员添加的）
    QList<int> allIds = mapScene_->allBuildingIds();
    for (int id : allIds) {
        BuildingItem* item = mapScene_->getBuildingItem(id);
        if (item) {
            item->setVisible(true);
            item->setSelected(false);
        }
    }
}

// --- 新生开学专属路线 ---
void MainWindow::onFreshmanRoute() {
    if (appMode_ == AppMode::Freshman) {
        // 新生路线：东门→2号公寓→南苑一站式→一食堂→体育馆
        // 按名称查找，避免管理员增删改建筑后硬编码ID错位（例如删北门）
        QStringList freshmanNames = {
            QStringLiteral("东门"), QStringLiteral("2号公寓"),
            QStringLiteral("南苑一站式"), QStringLiteral("一食堂"), QStringLiteral("体育馆")
        };
        QStringList missing;
        QList<int> freshmanRoute = resolveRouteByName(campus_, freshmanNames, &missing);
        if (freshmanRoute.size() < 2) {
            QMessageBox::warning(this, tr("路线规划失败"),
                tr("以下建筑不存在，请检查名称或点击\"恢复默认数据\"：\n%1")
                  .arg(missing.join(QStringLiteral("、"))));
            return;
        }

        // 1) 先分段做 Dijkstra，拼出完整最短路径 fullPath（用于显示路径 + 距离）
        std::vector<int> fullPath;
        double total = 0;
        bool ok = true;
        for (size_t i = 0; i + 1 < freshmanRoute.size(); ++i) {
            auto segment = campus_.dijkstraWithWeather(freshmanRoute[i], freshmanRoute[i + 1], weatherMode_);
            if (segment.empty()) { ok = false; break; }
            for (size_t j = 0; j + 1 < segment.size(); ++j)
                for (const Edge& e : campus_.neighbors(segment[j]))
                    if (e.to == segment[j+1]) { total += e.weight; break; }
            if (i == 0) fullPath.insert(fullPath.end(), segment.begin(), segment.end());
            else fullPath.insert(fullPath.end(), segment.begin() + 1, segment.end());
        }
        if (!ok) {
            QMessageBox::warning(this, tr("路线规划失败"),
                tr("无法规划开学专属路线"));
            return;
        }

        // 2) 实际行走的道路由 startAutoNavigate 内部统一高亮（只走路口路网）
        //    （下面仅把经停站点建筑标红，方便辨认）

        // 3) 只标红5个站点建筑，中间经过的建筑一律不标红
        for (int id : freshmanRoute) {
            BuildingItem* item = mapScene_->getBuildingItem(id);
            if (item) item->setSelected(true);
        }

        selectedStartId_ = freshmanRoute[0];
        selectedEndId_ = freshmanRoute[4];
        const Building* startB = campus_.getBuilding(freshmanRoute[0]);
        const Building* endB = campus_.getBuilding(freshmanRoute[4]);
        startLabel_->setText(tr("起点: %1").arg(startB ? startB->name() : ""));
        endLabel_->setText(tr("终点: %1").arg(endB ? endB->name() : ""));
        statusLabel_->setText(tr("新生开学路线(共%1米, 经停5站) — 角色自动导航中...").arg(total, 0, 'f', 0));
        // 按站点序列导航：东门 → 2号公寓 → 南苑一站式 → 一食堂 → 体育馆，
        // 角色会依次经过每一站（途经站点时自动弹出入住/贴士提示）。
        startAutoNavigate(std::vector<int>(freshmanRoute.begin(), freshmanRoute.end()));

    } else if (appMode_ == AppMode::Visitor) {
        // 访客游览路线：朝阳广场→一食堂→大学生活动中心→二食堂→钟楼→体育馆→图书馆
        // 按名称查找，避免管理员增删改建筑后硬编码ID错位
        QStringList visitorNames = {
            QStringLiteral("朝阳广场"), QStringLiteral("一食堂"),
            QStringLiteral("大学生活动中心"), QStringLiteral("二食堂"),
            QStringLiteral("钟楼"), QStringLiteral("体育馆"), QStringLiteral("图书馆")
        };
        QStringList missingV;
        QList<int> visitorRoute = resolveRouteByName(campus_, visitorNames, &missingV);
        if (visitorRoute.size() < 2) {
            QMessageBox::warning(this, tr("路线规划失败"),
                tr("以下建筑不存在，请检查名称或点击\"恢复默认数据\"：\n%1")
                  .arg(missingV.join(QStringLiteral("、"))));
            return;
        }

        // 1) 分段 Dijkstra，拼出完整路径（用于距离估算 + 角色漫游）
        std::vector<int> fullPath;
        double total = 0;
        bool ok = true;
        for (size_t i = 0; i + 1 < visitorRoute.size(); ++i) {
            auto segment = campus_.dijkstraWithWeather(visitorRoute[i], visitorRoute[i + 1], weatherMode_);
            if (segment.empty()) { ok = false; break; }
            for (size_t j = 0; j + 1 < segment.size(); ++j)
                for (const Edge& e : campus_.neighbors(segment[j]))
                    if (e.to == segment[j+1]) { total += e.weight; break; }
            if (i == 0) fullPath.insert(fullPath.end(), segment.begin(), segment.end());
            else fullPath.insert(fullPath.end(), segment.begin() + 1, segment.end());
        }
        if (!ok) {
            QMessageBox::warning(this, tr("路线规划失败"),
                tr("无法规划游览路线"));
            return;
        }

        // 2) 实际行走的道路由 startAutoNavigate 内部统一高亮（只走路口路网）
        //    （下面仅把经停站点建筑标红，方便辨认）
        // 3) 只标红7个站点建筑，中间经过的建筑一律不标红
        for (int id : visitorRoute) {
            BuildingItem* item = mapScene_->getBuildingItem(id);
            if (item) item->setSelected(true);
        }

        selectedStartId_ = visitorRoute[0];
        selectedEndId_ = visitorRoute[6];
        const Building* startB = campus_.getBuilding(visitorRoute[0]);
        const Building* endB = campus_.getBuilding(visitorRoute[6]);
        startLabel_->setText(tr("起点: %1").arg(startB ? startB->name() : ""));
        endLabel_->setText(tr("终点: %1").arg(endB ? endB->name() : ""));

        int minutes = static_cast<int>(total / 60);
        statusLabel_->setText(tr("游览路线(共%1米, 约%2分钟, 经停7站) — 角色自动导航中...").arg(total, 0, 'f', 0).arg(minutes));
        // 按站点序列导航：角色依次经过朝阳广场→一食堂→大学生活动中心→二食堂→钟楼→体育馆→图书馆
        startAutoNavigate(std::vector<int>(visitorRoute.begin(), visitorRoute.end()));

    } else {
        QMessageBox::information(this, tr("提示"),
            tr("请先切换到新生模式或访客模式\n再点击专属路线按钮"));
    }
}

// --- 小贴士/导游信息 ---
void MainWindow::onShowTips() {
    if (appMode_ == AppMode::Freshman) {
        // 宿舍入住小贴士
        QString tips = QStringLiteral(
            "🏠 宿舍入住小贴士\n"
            "━━━━━━━━━━━━━━━━━━\n\n"
            "📦 打包行李建议:\n"
            "  • 必备：身份证、录取通知书、银行卡、证件照\n"
            "  • 床上用品：学校统一发放(被子/床单/枕头)\n"
            "  • 生活用品：水杯、衣架、拖鞋、洗衣液\n"
            "  • 学习用品：笔记本、文具、充电器\n\n"
            "📝 入住报备流程:\n"
            "  1. 到宿舍楼一楼宿管处登记\n"
            "  2. 出示录取通知书和身份证\n"
            "  3. 领取宿舍钥匙和门禁卡\n"
            "  4. 签署住宿协议和安全承诺书\n\n"
            "⚡ 宿舍用电禁令:\n"
            "  • 禁用大功率电器(>500W)：热得快、电饭锅、电热毯\n"
            "  • 禁止私拉电线、改装插座\n"
            "  • 离开宿舍务必断电拔插头\n"
            "  • 违规用电将没收电器并通报\n\n"
            "🏘️ 周边生活配套:\n"
            "  • 超市：日用品、文具、零食(07:00-22:30)\n"
            "  • 食堂：学一(大众餐)/学二(风味)(06:30-21:00)\n"
            "  • 校医院：常见病诊疗/急诊(08:00-18:00)\n"
            "  • 快递点：北门外100米菜鸟驿站\n\n"
            "🔧 报修渠道:\n"
            "  • 水电维修：宿舍一楼宿管处登记\n"
            "  • 网络报修：校园网服务中心 ext.8001\n"
            "  • 家具损坏：后勤处 ext.8002\n"
            "  • 紧急维修：24小时值班 138xxxx0000");
        QMessageBox::information(this, tr("宿舍入住小贴士"), tips);

    } else if (appMode_ == AppMode::Visitor) {
        // 电子导游信息
        QString guide = QStringLiteral(
            "🏛️ 天津理工大学校园导游\n"
            "━━━━━━━━━━━━━━━━━━━━\n\n"
            "📍 游览景点介绍:\n\n"
            "🌊 明理湖:\n"
            "  天理标志性景观，湖面面积约5000平方米。\n"
            "  湖畔有休闲长椅和观景台，四季景色宜人。\n"
            "  建议游览：15分钟\n\n"
            "🕰️ 钟楼:\n"
            "  校园标志性建筑，高约30米。\n"
            "  每整点报时，是天理的精神象征。\n"
            "  建议游览：5分钟\n\n"
            "📚 图书馆:\n"
            "  建筑面积4.6万平米，藏书215万册。\n"
            "  藏借阅一体化管理，4600余阅览座位。\n"
            "  访客可参观一楼大厅和展览区。\n"
            "  建议游览：20分钟\n\n"
            "⛲ 朝阳广场:\n"
            "  校园中心广场，周边有商铺和咖啡厅。\n"
            "  广场定期举办文化活动和展览。\n"
            "  建议游览：10分钟\n\n"
            "🏟️ 体育场:\n"
            "  标准400米跑道，可容纳5000人。\n"
            "  校运会和大型活动举办地。\n"
            "  建议游览：10分钟\n\n"
            "⏱️ 游览总时长预估: 约60-90分钟\n"
            "🚻 卫生间: 图书馆一楼、体育场旁、学二食堂\n"
            "🛒 便利店: 朝阳广场周边、学二食堂一楼\n"
            "🚪 出口: 北门(公交)、东门(地铁)、南门(小吃街)");
        QMessageBox::information(this, tr("电子导游"), guide);

    } else {
        QMessageBox::information(this, tr("提示"),
            tr("请先切换到新生模式(入住贴士)或访客模式(电子导游)"));
    }
}

// --- 访客游览路线（委托给 onFreshmanRoute，内部按模式分发）---
void MainWindow::onVisitorGuide() {
    onFreshmanRoute();
}

// ============================================================
//  路网编辑模式（位置可调工具）——所有数据库写入只动 node_kind=0 和 roads
// ============================================================

// 把状态栏改成对应子工具的提示
static QString netEditHint(NetEditMode m) {
    switch (m) {
        case NetEditMode::AddJunction:
            return QObject::tr("➕ 路口工具：左键点地图空白处即放一个新路口；"
                               "已有路口可拖动调整位置");
        case NetEditMode::Connect:
            return QObject::tr("🔗 连接工具：依次点【起点】【终点】两个节点建一条路。"
                               "两者都可是路口或建筑");
        case NetEditMode::Delete:
            return QObject::tr("🗑 删除工具：右键点路口或道路即可删。"
                               "（删路口会同时清理所有连它的路）");
    }
    return QString();
}

void MainWindow::setNetEditSubMode(NetEditMode m) {
    netEditMode_ = m;
    // 仅"添加路口"模式允许拖动路口；连接/删除模式下路口只可点击不可拖动，
    // 否则点击会被误判为拖动而吞掉 clicked 信号，导致连不上路。
    mapScene_->setIntersectionMovable(m == NetEditMode::AddJunction);
    // 单选：清掉别的按钮选中态
    netEditAddBtn_->setChecked(m == NetEditMode::AddJunction);
    netEditConnectBtn_->setChecked(m == NetEditMode::Connect);
    netEditDeleteBtn_->setChecked(m == NetEditMode::Delete);
    // 清掉"第一点"高亮 + 预览
    if (connectFirstId_ >= 0) {
        if (auto* ii = mapScene_->getIntersectionItem(connectFirstId_)) {
            ii->setSelectedVisual(false);
        }
        connectFirstId_ = -1;
    }
    mapScene_->clearConnectionPreview();
    statusLabel_->setText(netEditHint(m));
}

void MainWindow::onEnterNetworkEditMode() {
    if (!isAdminMode_) isAdminMode_ = true;   // 隐式进入管理员
    networkEditMode_ = true;
    mapScene_->setNetworkEditMode(true);
    netEditPanel_->setVisible(true);
    setNetEditSubMode(NetEditMode::AddJunction);   // 默认进入"添加路口"工具
    statusLabel_->setText(netEditHint(NetEditMode::AddJunction));
}

void MainWindow::onExitNetworkEditMode() {
    networkEditMode_ = false;
    mapScene_->setNetworkEditMode(false);
    netEditPanel_->setVisible(false);
    // 清掉"第一点"高亮
    if (connectFirstId_ >= 0) {
        if (auto* ii = mapScene_->getIntersectionItem(connectFirstId_)) {
            ii->setSelectedVisual(false);
        }
        connectFirstId_ = -1;
    }
    mapScene_->clearConnectionPreview();
    statusLabel_->setText(tr("✅ 已退出路网编辑模式"));
}

void MainWindow::onNetEditSubModeAdd()     { setNetEditSubMode(NetEditMode::AddJunction); }
void MainWindow::onNetEditSubModeConnect() { setNetEditSubMode(NetEditMode::Connect); }
void MainWindow::onNetEditSubModeDelete()  { setNetEditSubMode(NetEditMode::Delete); }

// 拖动中：实时刷新连路
void MainWindow::onIntersectionMoved(int id, QPointF pos) {
    mapScene_->updateRoadsForNode(id, pos);
}

// 拖动结束：写库
void MainWindow::onIntersectionDragFinished(int id, QPointF pos) {
    if (DatabaseManager::instance().updateBuildingPosition(id, pos.x(), pos.y())) {
        // 内存 campus_ 同步更新坐标 + 重算相关边权重（否则后续 dijkstra 距离会过期）
        campus_.updatePosition(id, pos.x(), pos.y());
    } else {
        statusLabel_->setText(tr("⚠ 保存路口位置失败"));
    }
}

// 路口被左键点击（连接工具用）
void MainWindow::onIntersectionClickedEdit(int id) {
    if (!networkEditMode_) return;
    if (netEditMode_ != NetEditMode::Connect) return;

    if (connectFirstId_ < 0) {
        connectFirstId_ = id;
        if (auto* ii = mapScene_->getIntersectionItem(id)) ii->setSelectedVisual(true);
        statusLabel_->setText(tr("🔗 已选第一点 (路口#%1) — 请点【第二点】").arg(id));
    } else if (connectFirstId_ == id) {
        statusLabel_->setText(tr("⚠ 不能连接到自己，请选其他点"));
    } else {
        if (DatabaseManager::instance().addRoadEuclidean(connectFirstId_, id)) {
            mapScene_->addRoadItem(connectFirstId_, id);
            statusLabel_->setText(
                tr("✅ 已连接 路口#%1 ↔ 路口#%2 — 可继续连下一条")
                   .arg(connectFirstId_).arg(id));
        } else {
            statusLabel_->setText(tr("⚠ 连接失败（可能已存在）"));
        }
        if (auto* ii = mapScene_->getIntersectionItem(connectFirstId_)) ii->setSelectedVisual(false);
        connectFirstId_ = -1;
        mapScene_->clearConnectionPreview();
    }
}

// 路口被右键（编辑模式下任意子模式均可删除，带确认弹窗）
void MainWindow::onIntersectionRightClickedEdit(int id) {
    if (!networkEditMode_) return;
    if (QMessageBox::question(this, tr("删除路口"),
            tr("确定删除路口 #%1 吗？\n（所有连到它的道路会一并删除）").arg(id),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    if (!DatabaseManager::instance().removeJunctionWithRoads(id)) {
        statusLabel_->setText(tr("⚠ 删除路口失败"));
        return;
    }
    mapScene_->removeIntersection(id);
    // 内存 campus_ 也同步删
    campus_.removeBuilding(id);
    statusLabel_->setText(tr("🗑 已删除路口 #%1").arg(id));
}

// 道路被右键（编辑模式下任意子模式均可删除，带确认弹窗）
void MainWindow::onRoadRightClickedEdit(int fromId, int toId) {
    if (!networkEditMode_) return;
    if (QMessageBox::question(this, tr("删除道路"),
            tr("确定删除道路 #%1 ↔ #%2 吗？").arg(fromId).arg(toId),
            QMessageBox::Yes | QMessageBox::No) != QMessageBox::Yes) return;
    if (!DatabaseManager::instance().deleteRoad(fromId, toId)) {
        statusLabel_->setText(tr("⚠ 删除道路失败"));
        return;
    }
    mapScene_->removeRoadItem(fromId, toId);
    // 内存 campus_ 同步删
    campus_.removeRoad(fromId, toId);
    statusLabel_->setText(tr("🗑 已删除道路 #%1 ↔ #%2").arg(fromId).arg(toId));
}

// 空白处被点：添加模式 → 放新路口；连接模式 → 跟鼠标移动预览；删除模式 → 啥也不做
void MainWindow::onEmptyScenePressedEdit(QPointF pos, Qt::MouseButton button) {
    if (!networkEditMode_) return;
    if (button == Qt::LeftButton) {
        if (netEditMode_ == NetEditMode::AddJunction) {
            int newId = DatabaseManager::instance().addJunction(pos.x(), pos.y());
            if (newId < 0) {
                statusLabel_->setText(tr("⚠ 新增路口失败"));
                return;
            }
            // 在内存 campus_ 加一个路口节点（node_kind=0）+ 放入图元
            Building b(newId, QString(), pos.x(), pos.y(),
                       BuildingType::Other, QString(), QString(), 0, 0);
            campus_.addBuilding(b);
            mapScene_->addIntersectionItem(newId, pos.x(), pos.y());
            statusLabel_->setText(tr("➕ 已在 (%.0f, %.0f) 放置路口 #%1 — 可继续拖动调整")
                                  .arg(pos.x()).arg(pos.y()).arg(newId));
        } else if (netEditMode_ == NetEditMode::Connect) {
            // 在 connect 模式点空白没意义（不是节点）
            statusLabel_->setText(tr("⚠ 连接模式下请点节点（路口或建筑），不是空白处"));
        }
    } else if (button == Qt::RightButton) {
        // 右键空白：删除模式不做事；其他模式视为取消预览
        if (connectFirstId_ >= 0) {
            if (auto* ii = mapScene_->getIntersectionItem(connectFirstId_)) {
                ii->setSelectedVisual(false);
            }
            connectFirstId_ = -1;
            mapScene_->clearConnectionPreview();
            statusLabel_->setText(tr("已取消选中第一点"));
        }
    }
}

