# FusionAnalyser

<div align="center">

<img src="src/FusionAnalyser/res/dark/images/titlebar/FusionAnalyser.png" alt="FusionAnalyser Logo" width="200"/>

**专业、开源的数字化牙科模型测量分析软件**

[官网](https://www.ai-align.cn/fusionanalyser/) • [快速开始](#构建) • [文档](#使用说明)

[English](./README_EN.md) | 简体中文

</div>

FusionAnalyser 是一款专业、开源的数字化牙科模型测量分析软件，面向正畸临床场景提供高效、准确的模型分析能力。本软件基于 Qt 和 OpenGL 开发，为正畸医生提供完整的数字化模型分析解决方案。

**源代码仓库**：https://github.com/aialign2025/FusionAnalyser

## 软件亮点

- **更简单**：步骤引导的"傻瓜式"操作流程
- **更精准**：严格遵循教科书定义的测量项目，精准细致的定点测量
- **更直观**：临床参考值展示、异常值提醒，分析报告清晰完整
- **更开放**：自由开放的数据接口，兼容各品牌口扫、台扫数字模型

## 构建

项目基于 Qt 5 和 OpenGL 开发，支持 Windows 平台。

### 系统要求

- **编译器**: MSVC 2019/2022 (Visual Studio)
- **Qt**: Qt 5.12+（需包含 QtOpenGL、QtXml、QtXmlPatterns、QtScript、QtNetwork、QtSvg、QtPrintSupport 模块）
- **vcglib**: Visualization and Computer Graphics Library（必须克隆在项目根目录，与 `src` 同级）
- **第三方库**（已包含在 `src/external/`）：GLEW、FreeType、jhead、SpdLog、QuaZip、Google Breakpad

### 项目结构

```
fusionalign/
├── build.bat              # Windows 快速构建脚本
├── vcglib/                # VCG 库（需手动克隆，与 src 同级）
├── src/
│   ├── external/          # 第三方库
│   ├── common_base/       # 基础工具库
│   ├── common/            # 核心框架
│   ├── common_ext/        # 牙科扩展功能
│   ├── UI_Common/         # 共享 UI 组件
│   ├── FusionAnalyser/    # 主应用程序
│   ├── sampleplugins/     # 牙科分析插件
│   └── distrib/           # 构建输出目录
└── LICENSE.txt
```

### 构建步骤

1. 确保已安装 Visual Studio 2019/2022 和 Qt 5.15.2
2. 克隆 vcglib 到项目根目录：
   ```cmd
   git clone https://github.com/cnr-isti-vclab/vcglib.git
   ```
3. 修改 `build.bat` 中的 Qt 路径（如需要）：
   ```bat
   set QT_DIR=D:\Qt\5.15.2\msvc2019_64
   ```
4. 运行构建脚本：
   ```cmd
   build.bat
   ```
5. 清理重建（可选）：
   ```cmd
   build.bat clean
   ```

**输出位置：**
- Release 版本：`src\distrib\FusionAnalyser.exe`
- Debug 版本：`src\distribD\FusionAnalyser.exe`
- 插件目录：`src\distrib\plugins\`


## 技术架构

FusionAnalyser 采用插件式架构设计，包含以下主要模块：

- **主应用程序**：FusionAnalyser（3D 渲染视口和用户界面）
- **核心插件**：测量分析、模型标记、渲染装饰、文件导入/导出
- **公共库**：基础工具、3D 处理库、牙科扩展、UI 组件

## 许可证

本项目使用 **GNU General Public License v3.0**（GPL v3）

- 详见 `LICENSE.txt` 文件
- 第三方库许可证详见 `THIRD_PARTY_LICENSES.md`

## 联系方式

- **官方网站**：https://www.ai-align.cn/fusionanalyser/
- **源代码仓库**：https://github.com/aialign2025/FusionAnalyser
- **问题反馈**：[GitHub Issues](https://github.com/aialign2025/FusionAnalyser/issues)

---

**注意**：本软件仅供研究和教学使用，临床诊断请咨询专业医生。
