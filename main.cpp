// ============================================================
//  CampusNavigator - 阶段3 图形界面主程序
// ============================================================
//  从本阶段起，程序从控制台升级为图形界面：
//    - 使用 QMainWindow + QGraphicsView 显示校园地图
//    - 建筑以彩色矩形显示，可点击选择
//    - 支持选起终点 → Dijkstra 路径高亮
//
//  中文编码问题已彻底解决：Qt GUI 控件（QLabel、QMessageBox 等）
//  原生完美支持 UTF-8，无需任何额外编码设置。
// ============================================================

#include <QApplication>        // GUI 应用对象（替代 QCoreApplication）
#include "view/MainWindow.h"

int main(int argc, char* argv[]) {
    // QApplication：支持图形界面的 Qt 应用入口
    // 内部自动处理字体渲染、事件循环、DPI 缩放等
    QApplication app(argc, argv);

    // 设置应用程序元信息（显示在任务栏和窗口标题中）
    QApplication::setApplicationName("CampusNavigator");
    QApplication::setApplicationDisplayName(
        QStringLiteral("校园探索与智能导航模拟系统"));

    // 创建并显示主窗口
    MainWindow window;
    window.show();   // 窗口默认不可见，必须手动 show

    // 进入事件循环（处理鼠标点击、键盘输入、重绘等）
    // exec() 返回时程序退出
    return app.exec();
}
