#ifndef CAMPUSNAVIGATOR_VIEW_ADMINDIALOG_H
#define CAMPUSNAVIGATOR_VIEW_ADMINDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QTabWidget>

// ============================================================
//  AdminDialog - 管理员对话框（建筑 + 道路增删改查）
// ============================================================
//  通过密码验证后进入，可对建筑信息和道路连接进行 CRUD 操作。
//  修改后立即写入 SQLite 数据库，关闭对话框后主窗口自动刷新地图。
//
//  分两个 Tab：
//    1. 建筑管理：列表 + 表单 + 新增/修改/删除按钮
//    2. 道路管理：两个建筑下拉框 + 距离 + 添加按钮 + 道路列表 + 删除按钮
// ============================================================
class AdminDialog : public QDialog {
    Q_OBJECT

public:
    explicit AdminDialog(QWidget* parent = nullptr);

signals:
    // 点击"地图画路"按钮时发出，由主窗口进入画路模式并关闭本对话框
    void drawRoadRequested();
    // 点击"退出管理员模式"按钮时发出，由主窗口退出管理员模式并关闭本对话框
    void exitAdminRequested();
    // 点击"进入路网编辑模式"按钮时发出，由主窗口开启可调位置的路网编辑工具
    void enterNetworkEditRequested();

private slots:
    // --- 建筑管理 ---
    void onItemSelected(int row);
    void onAdd();
    void onUpdate();
    void onDelete();
    void onReload();
    void onResetDefaults();  // 恢复默认数据

    // --- 道路管理 ---
    void onRoadFromChanged();       // 起点变化时自动算距离
    void onRoadToChanged();         // 终点变化时自动算距离
    void onAddRoad();               // 添加道路
    void onDeleteRoad();            // 删除选中道路
    void onReloadRoads();           // 重新加载道路列表

private:
    void setupUI();
    void setupBuildingTab();        // 建筑管理页
    void setupRoadTab();            // 道路管理页
    void loadBuildingList();        // 加载建筑列表
    void loadRoadList();            // 加载道路列表
    void refreshBuildingCombos();   // 刷新道路页的两个建筑下拉框
    void updateDistanceFromCoords(); // 根据两建筑坐标自动计算距离
    void clearForm();

    // 顶层
    QTabWidget* tabWidget_ = nullptr;

    // --- 建筑管理页 ---
    QWidget*       buildingTab_   = nullptr;
    QListWidget*   listWidget_    = nullptr;
    QLineEdit*     idEdit_        = nullptr;
    QLineEdit*     nameEdit_      = nullptr;
    QLineEdit*     xEdit_         = nullptr;
    QLineEdit*     yEdit_         = nullptr;
    QComboBox*     typeCombo_     = nullptr;
    QTextEdit*     infoEdit_      = nullptr;
    QLineEdit*     hoursEdit_     = nullptr;
    QSpinBox*      floorsSpin_    = nullptr;

    // --- 道路管理页 ---
    QWidget*         roadTab_         = nullptr;
    QComboBox*       roadFromCombo_   = nullptr;  // 起点
    QComboBox*       roadToCombo_     = nullptr;  // 终点
    QDoubleSpinBox*  roadWeightSpin_  = nullptr;  // 距离
    QListWidget*     roadListWidget_  = nullptr;  // 已有道路列表
};

#endif // CAMPUSNAVIGATOR_VIEW_ADMINDIALOG_H
