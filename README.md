# 活动雷达 ActivityRadar v2.0

> **C++ 极速引擎 + Python 优雅界面** — 你的城市活动脉搏，聚合 AI 科技盛会与马拉松赛事，实时捕捉志愿者招募机会。

---

## 项目简介

活动雷达是一款**双语言架构**的桌面应用：

- **C++ 核心引擎** (`activity_engine.exe`)：零依赖、毫秒级响应的活动搜索引擎，内置 30 场真实活动数据，支持城市筛选、类型过滤、关键词搜索、多维度排序，输出 JSON 供上层调用。
- **Python GUI 前端** (`app.py`)：基于 Tkinter 的 Claude 式浅色文气界面，调用 C++ 引擎获取数据，提供城市切换、IP 定位、活动详情弹窗、志愿者招募高亮等完整交互。

武汉活动**红色边框高亮**，志愿者招募信息**金色卡片突出**。

## 功能特性

- 城市切换：武汉 / 北京 / 上海 / 广州 / 深圳 / 杭州 / 成都 / 南京 / 长沙 / 重庆 / 合肥 / 厦门 / 全部
- 类型筛选：AI 活动 / 马拉松 / 全部
- 关键词搜索：标题、地点、主办方、描述、标签全文检索
- 排序方式：按时间 / 武汉优先 / 志愿者优先
- 志愿者招募专区：招募人数、截止日期、岗位、福利、报名方式
- IP 自动定位：一键定位当前城市
- 活动详情弹窗：完整信息展示

## 技术架构

```
┌─────────────────────────────────┐
│     Python Tkinter GUI          │  ← 用户交互层
│  (app.py, 零外部依赖)           │
└──────────────┬──────────────────┘
               │ subprocess + JSON
┌──────────────▼──────────────────┐
│     C++ Search Engine           │  ← 核心计算层
│  (activity_engine.exe, C++17)   │
│  零依赖 · 单文件 · 毫秒响应      │
└──────────────┬──────────────────┘
               │
┌──────────────▼──────────────────┐
│     内置活动数据 (30场)          │
│  C++ 硬编码 + JSON 双份备份      │
└─────────────────────────────────┘
```

## 快速开始

### 环境要求

- **C++ 编译器**：MinGW-w64 (GCC 11+) 或 MSVC
- **Python**：3.8+（Tkinter 为标准库，无需额外安装）

### 一键构建运行

```bash
# Windows
build.bat
```

### 手动编译

```bash
# 1. 编译 C++ 引擎（静态编译，无需DLL）
g++ -std=c++17 -O2 -static -static-libgcc -static-libstdc++ -o activity_engine.exe src/activity_engine.cpp

# 2. 运行 Python GUI
python app.py
```

### 仅使用 C++ 命令行

```bash
# 查看武汉所有活动
activity_engine.exe --city 武汉

# 搜索马拉松
activity_engine.exe --type marathon --city 全部

# JSON 输出（供程序调用）
activity_engine.exe --city 武汉 --json

# 仅看招募志愿者的活动
activity_engine.exe --volunteer --json

# 列出所有城市
activity_engine.exe --cities
```

## 命令行参数

| 参数 | 说明 | 默认值 |
|------|------|--------|
| `--city <城市>` | 筛选城市，用 `全部` 显示所有 | 武汉 |
| `--type <类型>` | `ai` / `marathon` / `all` | all |
| `--search <关键词>` | 搜索关键词 | - |
| `--sort <方式>` | `date` / `wuhan` / `volunteer` | date |
| `--volunteer` | 仅显示招募志愿者的活动 | - |
| `--json` | 输出 JSON 格式 | - |
| `--cities` | 列出所有可用城市 | - |
| `--help` | 显示帮助 | - |

## 项目结构

```
activity-radar/
├── src/
│   └── activity_engine.cpp   # C++ 搜索引擎源码
├── data/
│   └── activities.json       # 活动数据 (Python 回退用)
├── app.py                    # Python Tkinter GUI
├── build.bat                 # Windows 一键构建脚本
├── activity_engine.exe       # 编译后生成
└── README.md
```

## 数据说明

当前内置 30 场活动数据（2026 年），涵盖：
- **武汉 AI 活动**：智博会、聚合智能大会、AI 三展联动、ACCON 芯片大会、智能体创新大赛、黑客松等 15 场
- **武汉马拉松**：后官湖半马、光谷半马、江滩迷你马、木兰山越野等 4 场
- **其他城市 AI 活动**：北京、上海、杭州、深圳、合肥等 5 场
- **其他城市马拉松**：北马、上马、杭马、成马、广马等 6 场

数据仅供参考，请以官方发布为准。

## 设计亮点

- **C++ 零依赖**：不使用任何第三方库，纯标准库实现，单文件即可编译
- **Python 零依赖**：仅使用标准库 Tkinter，无需 pip install
- **优雅降级**：C++ 引擎不可用时，Python 自动回退到内置 JSON 数据
- **Claude 式 UI**：暖米白底色、衬线标题、陶土橙强调色、大量留白
- **武汉优先**：武汉活动红色边框高亮，排序可设为武汉优先
- **Windows 中文兼容**：使用宽字符 API 处理命令行参数，UTF-8 输出

## License

MIT
