# ============================================================
# CampusNavigator - 校园探索与智能导航模拟系统
# CMake 构建配置文件（阶段3：图形界面版）
# ============================================================

cmake_minimum_required(VERSION 3.16)
project(CampusNavigator VERSION 1.0.0 LANGUAGES CXX)

# --- C++ 标准设置 ---
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Qt 自动化处理开关 ---
# 阶段3 新增：MainWindow 和 BuildingItem 都用了 Q_OBJECT + 信号槽，
# AUTOMOC 会自动生成 moc_*.cpp 文件并编译进去
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# --- 查找 Qt6 模块 ---
# Core: 核心功能
# Widgets: 界面控件
# Sql: SQLite 数据库支持（阶段6 新增）
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Sql)

# --- 源文件列表（包含所有 .cpp）---
set(PROJECT_SOURCES
    main.cpp
    resources.qrc

    # 数据层
    src/model/Building.cpp
    src/model/Graph.cpp
    src/data/CampusData.cpp
    src/data/DatabaseManager.cpp

    # 视图层
    src/view/BuildingItem.cpp
    src/view/RoadItem.cpp
    src/view/IntersectionItem.cpp
    src/view/MapScene.cpp
    src/view/CharacterItem.cpp
    src/view/WeatherOverlay.cpp
    src/view/AdminDialog.cpp
    src/view/NpcItem.cpp
    src/view/MainWindow.cpp
)

# --- 生成可执行程序 ---
add_executable(CampusNavigator ${PROJECT_SOURCES})

# Windows 下隐藏控制台窗口（GUI 程序不需要黑框）
if(WIN32)
    set_target_properties(CampusNavigator PROPERTIES WIN32_EXECUTABLE TRUE)
endif()

# --- 链接 Qt 库 ---
target_link_libraries(CampusNavigator PRIVATE Qt6::Core Qt6::Widgets Qt6::Sql)

# --- 头文件包含路径 ---
target_include_directories(CampusNavigator PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

# --- 编译选项 ---
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    # MinGW 编码选项（源文件是 UTF-8，确保中文正确编译）
    target_compile_options(CampusNavigator PRIVATE
        -finput-charset=UTF-8
        -fexec-charset=UTF-8
    )
endif()

# --- 部署 PlaceIntroduction 目录到构建目录 ---
# 运行时从该目录批量导入建筑介绍 txt 文件
add_custom_command(TARGET CampusNavigator POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/PlaceIntroduction
        $<TARGET_FILE_DIR:CampusNavigator>/PlaceIntroduction
    COMMENT "Copying PlaceIntroduction directory to build output"
)
