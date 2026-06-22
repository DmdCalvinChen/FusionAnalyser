# 📝 MeshSegNet 自动分牙接入 TODO LIST

**交接背景提示**：
> "我们当前的工程架构是：前端 Three.js + 中间件 Python Flask + 底层计算核心 C++。目前 C++ 和 Python 的通信已经通过解析临时文件 `metrics.json` 完全跑通。现在的唯一任务是：引入 MeshSegNet 处理裸 STL 文件，将其生成的牙宽尺寸作为真实数据注入到 `metrics.json` 中，替换掉我们之前的 Mock 假数据。"

---

### 第一阶段：环境与代码库准备 (Environment & Codebase)
- [ ] **创建独立 AI 模块**：在项目根目录下新建 `ai_segmentation` 文件夹，用于隔离深度学习代码，避免与现有 Flask / C++ 服务发生依赖冲突。
- [ ] **配置 Python 依赖环境**：编写 `requirements_ai.txt`，重点引入 `torch` (PyTorch)、`vedo` (或 `vtk` 用于 3D 渲染读取)、`trimesh`、`numpy`、`scipy` 等 MeshSegNet 核心依赖。
- [ ] **提取核心推理代码**：从 `Tai-Hsien/MeshSegNet` 官方仓库中提取其前向推理脚本（Inference script）及网络结构定义文件（Network architecture），摒弃冗余的训练代码。

### 第二阶段：权重获取与本地试跑 (Weights & Local Testing)
- [ ] **寻找预训练权重**：在开源社区、论文复现代码或 MICCAI 3DTeethSeg 相关讨论区，寻找并下载适用于 MeshSegNet 的社区版预训练权重文件（`.pth` 或 `.pt` 格式）。
- [ ] **执行灰度测试**：编写一个简单的 `test_inference.py`，加载现有的 `example/upper.stl`，测试能否成功利用权重在单机环境下跑通预测，输出带有标签的 3D 点云或面片数据。

### 第三阶段：数据解析与服务打通 (Data Extraction & Integration)
- [ ] **提取正畸计算关键参数**：编写几何解析脚本。针对 AI 模型吐出的各个牙齿分类标签（FDI 11-28，31-48），自动计算每颗牙齿的边界（近中点到远中点距离），换算为精准的牙宽（毫米）。
- [ ] **对接中间层服务器**：修改现有的 `server.py`。当从前端接收到 STL 裸模上传时，**先**调用 AI 模块获取真实的单颗牙宽和牙弓长，将这些计算好的数值组装成真实的 `metrics.json`，**再**喂给已有的 C++ 计算引擎。
