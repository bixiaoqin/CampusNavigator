@echo off
REM ============================================================
REM  一键编译脚本（便携版）
REM  放在项目根目录双击即可。首次运行自动配置，之后只编译。
REM  不再写死路径——跟着脚本所在位置走，换电脑/换盘符都能用。
REM ============================================================

set PATH=D:\Qt\Tools\mingw1310_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\Ninja;D:\Qt\6.11.1\mingw_64\bin;%PATH%

REM %~dp0 = 本脚本所在目录（= 项目根目录），不再写死 C:\...
cd /d "%~dp0"

REM 首次运行：build\Release 里没有 CMakeCache.txt 就先配置一次
if not exist "build\Release\CMakeCache.txt" (
    echo [首次运行] 正在配置项目...
    cmake -B "build/Release" -G Ninja -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="D:/Qt/6.11.1/mingw_64"
    if errorlevel 1 (
        echo.
        echo 配置失败。请检查：1) Qt 装在 D:\Qt\6.11.1\mingw_64 吗？2) 上面的 PATH 路径对吗？
        pause
        exit /b 1
    )
)

REM 编译
echo 正在编译...
cmake --build "build/Release" 2>&1
echo.
echo BUILD_EXIT_CODE=%ERRORLEVEL%
pause
