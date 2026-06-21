# FusionAnalyser (WebUI Edition) 🦷

> **⚠️ 架构变更说明 (2026 更新)**：
> 本项目最初是一个基于 C++ 和 Qt5 开发的桌面端正畸分析软件。由于跨平台兼容性问题（尤其是 macOS 的无边框窗口焦点缺陷），我们已经**全面重构并转型为纯前端（WebUI）轻量化架构**。
> 原有庞大的 Qt UI 代码已被移除，但核心的**C++正畸数学算法**仍保留在 `src/` 目录下，作为开源医学计算的参考资料。

## 🌟 项目简介

FusionAnalyser 现已成为一个开箱即用的**纯前端数字正畸模型分析系统**。它完全运行在浏览器中，无需安装任何后端服务，也不依赖复杂的运行环境。医生或研究人员可以直接导入口内扫描的 STL 模型，并快速计算关键的正畸临床指标。

## ✨ 核心功能

*   **🦷 3D 模型渲染引擎 (基于 Three.js)**
    *   原生支持导入并渲染高精度的上下颌 `.stl` 模型。
    *   支持旋转、缩放、平移等全方位查看。
    *   医疗级的高级暗色系光照和材质渲染。
*   **📊 Bolton 指数自动分析**
    *   **前牙比 (Anterior Ratio 3-3)**：自动计算并评判是否在 78.8% 标准范围内。
    *   **全牙比 (Overall Ratio 6-6)**：自动计算并评判是否在 91.5% 标准范围内。
    *   精确给出上下颌牙量偏大的具体毫米数。
*   **📏 拥挤度与间隙评估 (Crowding Analysis)**
    *   根据输入的牙宽和可用牙弓长度，自动计算模型拥挤度/间隙大小。
    *   自动分级：散在间隙、正常、I度(轻度)、II度(中度)、III度(重度)拥挤。
*   **📋 医疗级十字牙列数据表**
    *   标准的 FDI 牙位排布（18-28, 48-38），支持缺失牙快速标记。

## 🚀 快速启动

得益于纯前端零依赖的架构，你只需要一个简单的 HTTP 静态服务器即可运行本项目。

1. 克隆本项目：
   ```bash
   git clone https://github.com/your-username/FusionAnalyser.git
   cd FusionAnalyser/orthodontic-analyzer
   ```

2. 启动任意静态文件服务器（例如使用 macOS 自带的 Python）：
   ```bash
   python3 -m http.server 8765
   ```

3. 在浏览器中访问：
   **http://localhost:8765**

## 📂 目录结构

```text
FusionAnalyser/
├── orthodontic-analyzer/    # 🎯 全新的纯前端 WebUI 主程序
│   ├── index.html           # 应用主入口
│   ├── style.css            # 医疗级深色主题样式
│   ├── js/
│   │   ├── main.js          # 事件绑定与初始化
│   │   ├── viewer.js        # Three.js 3D 渲染引擎
│   │   ├── analyzer.js      # Bolton / 拥挤度计算逻辑
│   │   └── ui.js            # 十字牙列表与结果卡片交互
│   └── example/             # 存放用于测试的本地 STL 演示模型
│
└── src/                     # 📚 遗留的 C++ 核心算法库 (仅作参考)
    ├── common_ext/bolton/   # 原生 Bolton 算法 C++ 实现
    ├── common_ext/data/     # 原生拥挤度、Spee曲线、磨牙关系算法
    ├── common_ext/meshExt/  # 牙齿 OBB 包围盒及牙弓曲线拟合逻辑
    └── common_ext/machine/  # Delaunay、GJK碰撞检测、凸包等3D底层算法
```

## 📜 许可证协议
本项目基于原始开源协议发布（详见 `LICENSE` 文件）。所有新重构的 WebUI 代码遵循相同的开源精神。

## 🐛 已知问题 (2026 待解决)
*   **前后端对接问题**：
    *   目前 WebUI 前端已与 C++ `backend_engine` 通过 Python 跨系统调用打通了通讯链路。
    *   但 C++ 侧缺少有效的牙齿自动分割（Segmentation）逻辑，因此目前 `main.cpp` 在处理上传的 STL 模型时，只能向前端返回硬编码（Hardcoded）的测试牙宽数据。
    *   前端 `viewer.js` 的真实牙弓长度动态测算结果与后端返回的固定死数据产生冲突，会导致前端“拥挤度计算”经常出现负数（散在间隙）的不匹配现象。
    *   后续需要新开对话，引入或实现真实的 C++ 模型分割与计算逻辑来解决此问题。
