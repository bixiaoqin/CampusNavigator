#include "view/AdminDialog.h"

#include "data/DatabaseManager.h"
#include "model/Building.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QMessageBox>
#include <QLabel>
#include <QInputDialog>
#include <QTabWidget>
#include <QList>
#include <cmath>

AdminDialog::AdminDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(tr("管理员模式 - 建筑与道路管理"));
    resize(780, 560);
    setupUI();
    loadBuildingList();
    refreshBuildingCombos();
    loadRoadList();
}

void AdminDialog::setupUI() {
    tabWidget_ = new QTabWidget(this);

    buildingTab_ = new QWidget;
    roadTab_     = new QWidget;
    setupBuildingTab();
    setupRoadTab();

    tabWidget_->addTab(buildingTab_, tr("建筑管理"));
    tabWidget_->addTab(roadTab_,     tr("道路管理"));

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addWidget(tabWidget_);

    // 底部：退出管理员模式按钮（避免用户只能靠反复进出管理员来操作）
    auto* exitBtn = new QPushButton(tr("退出管理员模式"));
    exitBtn->setStyleSheet(
        "QPushButton{padding:8px 20px;border-radius:4px;"
        "background:#FFEBEE;border:1px solid #EF9A9A;font-weight:bold;}"
        "QPushButton:hover{background:#FFCDD2;}");
    connect(exitBtn, &QPushButton::clicked, this, [this]() {
        emit exitAdminRequested();
        accept();   // 关闭对话框
    });
    mainLayout->addWidget(exitBtn, 0, Qt::AlignRight);
}

// ============================================================
//  建筑管理页（保持原有逻辑）
// ============================================================
void AdminDialog::setupBuildingTab() {
    // 左侧：建筑列表
    auto* listGroup = new QGroupBox(tr("建筑列表"));
    auto* listLayout = new QVBoxLayout(listGroup);
    listWidget_ = new QListWidget;
    listLayout->addWidget(listWidget_);

    auto* reloadBtn = new QPushButton(tr("刷新列表"));
    listLayout->addWidget(reloadBtn);
    connect(reloadBtn, &QPushButton::clicked, this, &AdminDialog::onReload);

    // 右侧：表单
    auto* formGroup = new QGroupBox(tr("建筑详情"));
    auto* formLayout = new QFormLayout(formGroup);

    idEdit_    = new QLineEdit;
    idEdit_->setReadOnly(true);
    nameEdit_  = new QLineEdit;
    xEdit_     = new QLineEdit;
    yEdit_     = new QLineEdit;
    typeCombo_ = new QComboBox;
    typeCombo_->addItem("校门",     static_cast<int>(BuildingType::Gate));
    typeCombo_->addItem("教学楼",   static_cast<int>(BuildingType::Classroom));
    typeCombo_->addItem("图书馆",   static_cast<int>(BuildingType::Library));
    typeCombo_->addItem("实验楼",   static_cast<int>(BuildingType::Lab));
    typeCombo_->addItem("行政楼",   static_cast<int>(BuildingType::Admin));
    typeCombo_->addItem("宿舍",     static_cast<int>(BuildingType::Dormitory));
    typeCombo_->addItem("食堂",     static_cast<int>(BuildingType::Canteen));
    typeCombo_->addItem("体育设施", static_cast<int>(BuildingType::Sports));
    typeCombo_->addItem("商店",     static_cast<int>(BuildingType::Shop));
    typeCombo_->addItem("医院",     static_cast<int>(BuildingType::Hospital));
    typeCombo_->addItem("其他",     static_cast<int>(BuildingType::Other));

    infoEdit_  = new QTextEdit;
    infoEdit_->setMaximumHeight(80);
    hoursEdit_ = new QLineEdit;
    floorsSpin_ = new QSpinBox;
    floorsSpin_->setRange(1, 50);

    formLayout->addRow(tr("ID:"),       idEdit_);
    formLayout->addRow(tr("名称:"),     nameEdit_);
    formLayout->addRow(tr("X 坐标:"),   xEdit_);
    formLayout->addRow(tr("Y 坐标:"),   yEdit_);
    formLayout->addRow(tr("类型:"),     typeCombo_);
    formLayout->addRow(tr("简介:"),     infoEdit_);
    formLayout->addRow(tr("开放时间:"), hoursEdit_);
    formLayout->addRow(tr("楼层数:"),   floorsSpin_);

    auto* btnLayout = new QHBoxLayout;
    auto* addBtn    = new QPushButton(tr("新增"));
    auto* updateBtn = new QPushButton(tr("修改"));
    auto* deleteBtn = new QPushButton(tr("删除"));
    auto* clearBtn  = new QPushButton(tr("清空表单"));
    auto* resetBtn  = new QPushButton(tr("恢复默认数据"));

    QString btnStyle =
        "QPushButton{padding:6px 12px;border-radius:4px;"
        "background:#E3F2FD;border:1px solid #90CAF9;}"
        "QPushButton:hover{background:#BBDEFB;}";
    addBtn->setStyleSheet(btnStyle);
    updateBtn->setStyleSheet(btnStyle);
    deleteBtn->setStyleSheet(btnStyle);
    clearBtn->setStyleSheet(btnStyle);

    // 重置按钮用更显眼的橙色样式，提示其危险性
    QString resetStyle =
        "QPushButton{padding:6px 12px;border-radius:4px;"
        "background:#FFEBEE;border:1px solid #EF9A9A;color:#C62828;}"
        "QPushButton:hover{background:#EF9A9A;color:#fff;}";
    resetBtn->setStyleSheet(resetStyle);
    resetBtn->setToolTip(tr("删除所有建筑和道路，恢复为16栋天理默认数据"));

    btnLayout->addWidget(addBtn);
    btnLayout->addWidget(updateBtn);
    btnLayout->addWidget(deleteBtn);
    btnLayout->addWidget(clearBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(resetBtn);
    formLayout->addRow(btnLayout);

    connect(addBtn,    &QPushButton::clicked, this, &AdminDialog::onAdd);
    connect(updateBtn, &QPushButton::clicked, this, &AdminDialog::onUpdate);
    connect(deleteBtn, &QPushButton::clicked, this, &AdminDialog::onDelete);
    connect(clearBtn,  &QPushButton::clicked, this, &AdminDialog::clearForm);
    connect(resetBtn,  &QPushButton::clicked, this, &AdminDialog::onResetDefaults);
    connect(listWidget_, &QListWidget::currentRowChanged,
            this, &AdminDialog::onItemSelected);

    auto* mainLayout = new QHBoxLayout(buildingTab_);
    mainLayout->addWidget(listGroup, 1);
    mainLayout->addWidget(formGroup, 2);
}

// ============================================================
//  道路管理页（新增）
// ============================================================
void AdminDialog::setupRoadTab() {
    // --- 顶部：添加道路表单 ---
    auto* addGroup = new QGroupBox(tr("添加道路（连接两栋建筑）"));
    auto* addForm  = new QFormLayout(addGroup);

    roadFromCombo_ = new QComboBox;
    roadToCombo_   = new QComboBox;
    roadWeightSpin_ = new QDoubleSpinBox;
    roadWeightSpin_->setRange(1.0, 99999.0);
    roadWeightSpin_->setSuffix(" 米");
    roadWeightSpin_->setDecimals(1);

    addForm->addRow(tr("起点建筑:"), roadFromCombo_);
    addForm->addRow(tr("终点建筑:"), roadToCombo_);
    addForm->addRow(tr("距离:"),     roadWeightSpin_);

    // 提示：选择两栋建筑后自动计算欧氏距离
    auto* hint = new QLabel(
        tr("提示：选择起止建筑后自动按坐标计算距离，可手动调整。"));
    hint->setStyleSheet("color:#666; font-size:12px;");
    addForm->addRow(hint);

    auto* addRoadBtn = new QPushButton(tr("添加道路"));
    addRoadBtn->setStyleSheet(
        "QPushButton{padding:6px 18px;border-radius:4px;"
        "background:#E8F5E9;border:1px solid #A5D6A7;}"
        "QPushButton:hover{background:#C8E6C9;}");
    addForm->addRow(addRoadBtn);

    // [已隐藏] 地图画路（带拐点）按钮：保留后端代码，仅在管理员面板中不显示
    //   后端 onStartDrawRoad/handleDrawRoadClick/finishDrawRoad 等仍可用，
    //   重新显示只需解除下面这段的注释并恢复 AdminDialog.h 的信号声明。
#if 0
    auto* drawRoadBtn = new QPushButton(tr("🖊 地图画路（带拐点）"));
    drawRoadBtn->setStyleSheet(
        "QPushButton{padding:6px 18px;border-radius:4px;"
        "background:#E3F2FD;border:1px solid #90CAF9;font-weight:bold;}"
        "QPushButton:hover{background:#BBDEFB;}");
    addForm->addRow(drawRoadBtn);
    connect(drawRoadBtn, &QPushButton::clicked, this, [this]() {
        emit drawRoadRequested();
        accept();
    });
    auto* drawHint = new QLabel(
        tr("推荐：点\"地图画路\"后，先点起点建筑，在地图上逐点加拐点，"
           "再点终点建筑结束，程序自动连成折线并建路；无需手抄坐标。"));
    drawHint->setWordWrap(true);
    drawHint->setStyleSheet("color:#1565C0; font-size:12px;");
    addForm->addRow(drawHint);
#endif

    connect(addRoadBtn,    &QPushButton::clicked, this, &AdminDialog::onAddRoad);

    // 进入路网编辑模式：关闭对话框，由主窗口打开可调位置的路网工具
    auto* editBtn = new QPushButton(tr("✏ 进入路网编辑模式"));
    editBtn->setStyleSheet(
        "QPushButton{padding:6px 18px;border-radius:4px;"
        "background:#FFF3E0;border:1px solid #FFCC80;font-weight:bold;}"
        "QPushButton:hover{background:#FFE0B2;}");
    addForm->addRow(editBtn);

    auto* editHint = new QLabel(
        tr("进入后可手动在街道交汇处放路口、连接两节点成道路、拖动调整位置、"
           "右键删除——完全按你的设计摆放，而不是算法自动生成。"));
    editHint->setWordWrap(true);
    editHint->setStyleSheet("color:#E65100; font-size:12px;");
    addForm->addRow(editHint);

    connect(editBtn, &QPushButton::clicked, this, [this]() {
        emit enterNetworkEditRequested();
        accept();   // 关闭对话框，交还主窗口处理
    });

    connect(roadFromCombo_, &QComboBox::currentIndexChanged,
            this, &AdminDialog::onRoadFromChanged);
    connect(roadToCombo_,   &QComboBox::currentIndexChanged,
            this, &AdminDialog::onRoadToChanged);


    // --- 底部：道路列表 + 删除 ---
    auto* listGroup = new QGroupBox(tr("现有道路列表（选中后可删除）"));
    auto* listLayout = new QVBoxLayout(listGroup);
    roadListWidget_ = new QListWidget;
    listLayout->addWidget(roadListWidget_);

    auto* delBtn = new QPushButton(tr("删除选中道路"));
    delBtn->setStyleSheet(
        "QPushButton{padding:6px 18px;border-radius:4px;"
        "background:#FFEBEE;border:1px solid #EF9A9A;}"
        "QPushButton:hover{background:#FFCDD2;}");
    listLayout->addWidget(delBtn);

    connect(delBtn, &QPushButton::clicked, this, &AdminDialog::onDeleteRoad);

    auto* mainLayout = new QVBoxLayout(roadTab_);
    mainLayout->addWidget(addGroup);
    mainLayout->addWidget(listGroup, 1);
}

// ============================================================
//  建筑管理：列表/表单 操作
// ============================================================
void AdminDialog::loadBuildingList() {
    listWidget_->clear();
    auto buildings = DatabaseManager::instance().loadAllBuildings();
    for (const auto& b : buildings) {
        QString itemText = QString("[%1] %2").arg(b.id()).arg(b.name());
        QListWidgetItem* item = new QListWidgetItem(itemText);
        item->setData(Qt::UserRole, b.id());
        listWidget_->addItem(item);
    }
}

void AdminDialog::onItemSelected(int row) {
    if (row < 0) return;
    int id = listWidget_->item(row)->data(Qt::UserRole).toInt();

    Building b(0, "", 0, 0, BuildingType::Other, "", "", 1);
    if (DatabaseManager::instance().getBuilding(id, b)) {
        idEdit_->setText(QString::number(b.id()));
        nameEdit_->setText(b.name());
        xEdit_->setText(QString::number(b.x()));
        yEdit_->setText(QString::number(b.y()));
        typeCombo_->setCurrentIndex(typeCombo_->findData(static_cast<int>(b.type())));
        infoEdit_->setPlainText(b.info());
        hoursEdit_->setText(b.openHours());
        floorsSpin_->setValue(b.floors());
    }
}

void AdminDialog::onAdd() {
    bool ok = false;
    int newId = QInputDialog::getInt(this, tr("新增建筑"),
                                      tr("请输入新建筑 ID:"), 100, 0, 9999, 1, &ok);
    if (!ok) return;

    Building b(newId, nameEdit_->text(),
               xEdit_->text().toDouble(), yEdit_->text().toDouble(),
               static_cast<BuildingType>(typeCombo_->currentData().toInt()),
               infoEdit_->toPlainText(), hoursEdit_->text(),
               floorsSpin_->value());

    if (DatabaseManager::instance().addBuilding(b)) {
        QMessageBox::information(this, tr("成功"), tr("建筑已新增"));
        loadBuildingList();
        refreshBuildingCombos();   // 道路页下拉框同步
    } else {
        QMessageBox::warning(this, tr("失败"), tr("新增失败，ID 可能已存在"));
    }
}

void AdminDialog::onUpdate() {
    if (idEdit_->text().isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先从列表选择一栋建筑"));
        return;
    }
    Building b(idEdit_->text().toInt(), nameEdit_->text(),
               xEdit_->text().toDouble(), yEdit_->text().toDouble(),
               static_cast<BuildingType>(typeCombo_->currentData().toInt()),
               infoEdit_->toPlainText(), hoursEdit_->text(),
               floorsSpin_->value());

    if (DatabaseManager::instance().updateBuilding(b)) {
        QMessageBox::information(this, tr("成功"), tr("建筑信息已更新"));
        loadBuildingList();
        refreshBuildingCombos();
    } else {
        QMessageBox::warning(this, tr("失败"), tr("更新失败"));
    }
}

void AdminDialog::onDelete() {
    if (idEdit_->text().isEmpty()) {
        QMessageBox::warning(this, tr("提示"),
            tr("请先在左侧列表中点击选中一栋建筑，再点删除"));
        return;
    }
    int id = idEdit_->text().toInt();

    auto ret = QMessageBox::question(this, tr("确认删除"),
        tr("确定删除建筑 [%1] 吗？\n与之相连的所有道路也会被删除！此操作不可撤销。")
            .arg(nameEdit_->text()));
    if (ret != QMessageBox::Yes) return;

    if (DatabaseManager::instance().deleteBuilding(id)) {
        QMessageBox::information(this, tr("成功"), tr("建筑已删除"));
        clearForm();
        loadBuildingList();
        refreshBuildingCombos();
        loadRoadList();   // 道路可能也被删了，刷新
    } else {
        QMessageBox::warning(this, tr("失败"), tr("删除失败"));
    }
}

void AdminDialog::onReload() {
    loadBuildingList();
    refreshBuildingCombos();
}

void AdminDialog::onResetDefaults() {
    auto ret = QMessageBox::warning(this, tr("警告"),
        tr("确定要恢复默认数据吗？\n所有自定义建筑和道路都会被删除，并恢复为16栋天理默认数据。"),
        QMessageBox::Yes | QMessageBox::No, QMessageBox::No);
    if (ret != QMessageBox::Yes) return;

    if (DatabaseManager::instance().resetToDefaultData()) {
        QMessageBox::information(this, tr("成功"), tr("已恢复默认数据，关闭对话框后地图会自动刷新"));
        clearForm();
        loadBuildingList();
        refreshBuildingCombos();
        loadRoadList();
    } else {
        QMessageBox::warning(this, tr("失败"), tr("恢复默认数据失败"));
    }
}

void AdminDialog::clearForm() {
    idEdit_->clear();
    nameEdit_->clear();
    xEdit_->clear();
    yEdit_->clear();
    typeCombo_->setCurrentIndex(0);
    infoEdit_->clear();
    hoursEdit_->clear();
    floorsSpin_->setValue(1);
}

// ============================================================
//  道路管理：列表/操作
// ============================================================
void AdminDialog::refreshBuildingCombos() {
    roadFromCombo_->blockSignals(true);
    roadToCombo_->blockSignals(true);
    roadFromCombo_->clear();
    roadToCombo_->clear();

    auto buildings = DatabaseManager::instance().loadAllBuildings();
    for (const auto& b : buildings) {
        QString text = QString("[%1] %2").arg(b.id()).arg(b.name());
        roadFromCombo_->addItem(text, b.id());
        roadToCombo_->addItem(text, b.id());
    }
    roadFromCombo_->blockSignals(false);
    roadToCombo_->blockSignals(false);

    // 触发一次距离自动计算
    onRoadFromChanged();
}

// 自动计算两栋建筑之间的欧氏距离，填入 weight 输入框
void AdminDialog::onRoadFromChanged() { updateDistanceFromCoords(); }
void AdminDialog::onRoadToChanged()   { updateDistanceFromCoords(); }

void AdminDialog::updateDistanceFromCoords() {
    if (roadFromCombo_->count() == 0 || roadToCombo_->count() == 0) return;

    int fromId = roadFromCombo_->currentData().toInt();
    int toId   = roadToCombo_->currentData().toInt();
    if (fromId == toId) {
        roadWeightSpin_->setValue(0);
        return;
    }

    Building a(0,"",0,0,BuildingType::Other,"","",1);
    Building b(0,"",0,0,BuildingType::Other,"","",1);
    if (DatabaseManager::instance().getBuilding(fromId, a) &&
        DatabaseManager::instance().getBuilding(toId, b)) {
        double dx = a.x() - b.x();
        double dy = a.y() - b.y();
        double dist = std::sqrt(dx*dx + dy*dy);
        roadWeightSpin_->setValue(dist);
    }
}

void AdminDialog::onAddRoad() {
    if (roadFromCombo_->count() == 0 || roadToCombo_->count() == 0) {
        QMessageBox::warning(this, tr("提示"), tr("请先添加建筑"));
        return;
    }

    int fromId = roadFromCombo_->currentData().toInt();
    int toId   = roadToCombo_->currentData().toInt();
    double w   = roadWeightSpin_->value();

    if (fromId == toId) {
        QMessageBox::warning(this, tr("提示"), tr("起点和终点不能是同一栋建筑"));
        return;
    }
    if (w <= 0) {
        QMessageBox::warning(this, tr("提示"), tr("距离必须大于 0"));
        return;
    }

    if (DatabaseManager::instance().addRoad(fromId, toId, w)) {
        QMessageBox::information(this, tr("成功"), tr("道路已添加"));
        loadRoadList();
    } else {
        QMessageBox::warning(this, tr("失败"), tr("添加失败，道路可能已存在"));
    }
}

void AdminDialog::onDeleteRoad() {
    int row = roadListWidget_->currentRow();
    if (row < 0) {
        QMessageBox::warning(this, tr("提示"), tr("请先选中一条道路"));
        return;
    }

    // 从 item 的 userData 取出 from/to
    QVariant data = roadListWidget_->item(row)->data(Qt::UserRole);
    if (!data.isValid()) return;
    QList<int> ids = data.value<QList<int>>();
    if (ids.size() != 2) return;
    int fromId = ids[0], toId = ids[1];

    auto ret = QMessageBox::question(this, tr("确认删除"),
        tr("确定删除这条道路吗？"));
    if (ret != QMessageBox::Yes) return;

    if (DatabaseManager::instance().deleteRoad(fromId, toId)) {
        QMessageBox::information(this, tr("成功"), tr("道路已删除"));
        loadRoadList();
    } else {
        QMessageBox::warning(this, tr("失败"), tr("删除失败"));
    }
}


void AdminDialog::onReloadRoads() {
    loadRoadList();
}

void AdminDialog::loadRoadList() {
    roadListWidget_->clear();
    auto roads = DatabaseManager::instance().loadAllRoads();
    for (const auto& r : roads) {
        Building a(0,"",0,0,BuildingType::Other,"","",1);
        Building b(0,"",0,0,BuildingType::Other,"","",1);
        QString fromName = QString::number(r.fromId);
        QString toName   = QString::number(r.toId);
        if (DatabaseManager::instance().getBuilding(r.fromId, a)) fromName = a.name();
        if (DatabaseManager::instance().getBuilding(r.toId, b))   toName   = b.name();

        QString text = QString("%1  →  %2    (%3 米)")
                           .arg(fromName).arg(toName).arg(r.weight, 0, 'f', 1);

        QList<int> ids; ids << r.fromId << r.toId;
        auto* item = new QListWidgetItem(text);
        item->setData(Qt::UserRole, QVariant::fromValue(ids));
        roadListWidget_->addItem(item);
    }
}
