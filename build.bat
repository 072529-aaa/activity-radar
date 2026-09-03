@echo off
chcp 65001 >nul
echo ========================================
echo   活动雷达 ActivityRadar - 构建脚本
echo   C++ Engine + Python GUI
echo ========================================
echo.

echo [1/3] 编译 C++ 搜索引擎...
where g++ >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 g++ 编译器，请先安装 MinGW-w64
    echo 安装命令: winget install BrechtSanders.WinLibs.POSIX.UCRT
    pause
    exit /b 1
)

g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -o activity_engine.exe src\activity_engine.cpp
if %errorlevel% neq 0 (
    echo 错误: C++ 编译失败
    pause
    exit /b 1
)
echo 成功: activity_engine.exe 已生成
echo.

echo [2/3] 验证 C++ 引擎...
activity_engine.exe --city 武汉 --json
if %errorlevel% neq 0 (
    echo 警告: 引擎运行测试失败
) else (
    echo 成功: 引擎运行正常
)
echo.

echo [3/3] 启动 Python GUI...
where python >nul 2>&1
if %errorlevel% neq 0 (
    echo 错误: 未找到 Python，请先安装 Python 3.x
    pause
    exit /b 1
)

echo 启动图形界面...
python app.py

pause
