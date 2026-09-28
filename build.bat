@echo off
set PATH=D:\Qt\Tools\mingw1310_64\bin;D:\Qt\Tools\CMake_64\bin;D:\Qt\Tools\Ninja;D:\Qt\6.11.1\mingw_64\bin;%PATH%
cd /d "C:\Users\lenovo\WorkBuddy\2026-06-17-20-24-23\CampusNavigator\build\Desktop_Qt_6_11_1_MinGW_64_bit_Debug"
cmake --build . 2>&1
echo BUILD_EXIT_CODE=%ERRORLEVEL%
