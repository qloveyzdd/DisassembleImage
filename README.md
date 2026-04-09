# DisassembleImage

一个在 Windows 上运行的本地图像拆分工具。

它会根据 `input.obj` / `output.obj` 里的几何与 UV 信息，把输入图片拆分成多张切片图，并提供桌面界面完成参数配置、执行处理、结果预览、3D 对照和导出。

## 当前状态

- 已完成 `v1.0`
- 目标平台：`Windows 10/11 x64`
- 图形界面：`Qt6 Widgets`
- 图像处理：`OpenCV`
- GPU 路线：`OpenCV + OpenCL + UMat`
- 当 GPU 不可用时，会明确提示并自动回退到 CPU

## 目录说明

- `app/windows/`：桌面界面、任务表单、运行控制、预览与 3D 组件
- `core/`：核心处理逻辑、OBJ/UV 解析、切片计划、GPU 诊断
- `cmake/`：vcpkg 工具链和 Windows 部署辅助脚本
- `tests/`：Phase 1 到 Phase 5 的 smoke 测试
- `HD/`：示例输入和序列测试图片
- `input.obj`：输入模型
- `output.obj`：输出模型
- `TARGET_MACHINE_CHECKLIST.md`：目标机部署与验收检查清单

## 直接使用二进制

优先使用已经打好的 Release 包：

- GitHub Release：[v1.0](https://github.com/qloveyzdd/DisassembleImage/releases/tag/v1.0)
- 直接下载：[DisassembleImage-v1.0-win64.zip](https://github.com/qloveyzdd/DisassembleImage/releases/download/v1.0/DisassembleImage-v1.0-win64.zip)

使用步骤：

1. 下载并解压 `DisassembleImage-v1.0-win64.zip`
2. 保持解压后的目录结构不变
3. 运行 `DisassembleImageDesktop.exe`

注意：

- 不要只单独拷贝 `DisassembleImageDesktop.exe`
- `Qt`、`OpenCV` 运行库、插件目录、`input.obj`、`output.obj` 需要和程序放在同一套目录中
- 如果你是从源码构建，请优先使用部署目录中的可执行文件，而不是只拿构建目录下的 `exe`

## 从源码构建

### 环境要求

- Windows 10/11 x64
- Visual Studio 2022
- CMake 3.26 或更高版本
- Git

项目通过工程内 `vcpkg` 恢复主要依赖：

- `opencv`
- `qtbase`

相关配置：

- `CMakePresets.json`
- `vcpkg.json`
- `cmake/VcpkgWorkspaceToolchain.cmake`

### Debug 构建

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset build-windows-debug --target disassemble_desktop
cmake --build --preset build-windows-debug --target windows_deploy_check
```

### Release 构建

```powershell
cmake --preset windows-msvc-release
cmake --build --preset build-windows-release --target disassemble_desktop
cmake --build --preset build-windows-release --target windows_deploy_check
```

### 可执行文件位置

Debug 开发构建：

- `build/windows-msvc-debug/app/windows/Debug/DisassembleImageDesktop.exe`

Debug 部署目录：

- `build/windows-msvc-debug/deploy/disassemble_desktop/DisassembleImageDesktop.exe`

Release 部署目录：

- `build/windows-msvc-release/deploy/disassemble_desktop/DisassembleImageDesktop.exe`

本地压缩包：

- `build/release-packages/DisassembleImage-v1.0-win64.zip`

## 使用说明

### 基本流程

1. 启动桌面程序
2. 选择输入模式：
   - 单张图片
   - 图片目录
3. 选择输入图片或输入目录
4. 选择输出目录
5. 检查或手动指定：
   - `input.obj`
   - `output.obj`
6. 设置常用参数：
   - 输出尺寸
   - 输出前缀
   - 拼接方式
   - 输出策略
7. 如有需要，展开高级设置：
   - 后端模式：`自动 / CPU / GPU`
   - 并行开关
   - 并行线程数
8. 点击开始处理
9. 处理完成后查看：
   - 摘要日志
   - 技术日志
   - 本次结果区
   - 缩略图、大图和 3D 对照

### 输出策略

- `禁止覆盖`
- `覆盖同名`
- `自动重命名`

### 逐面输出命名

当选择逐面输出时，程序会根据面的二维位置自动命名，例如：

- `top_left`
- `center_left`
- `bottom_right`

## GPU / OpenCL 说明

当前 GPU 路线基于 `OpenCV + OpenCL + UMat`。

这意味着：

- 有显卡不等于一定能启用 GPU
- 机器必须有可用的 OpenCL 运行环境
- 如果 OpenCL 不可用，程序会提示原因并回退到 CPU

界面中已经提供：

- OpenCL 支持状态
- 可用 GPU 设备状态
- 平台枚举结果
- 失败阶段
- 回退原因

如果目标机要做部署或验收，请同时参考：

- `TARGET_MACHINE_CHECKLIST.md`

## 测试

### Smoke 测试

```powershell
cmake --build --preset build-windows-debug --target phase1_smoke phase2_smoke phase3_smoke phase4_smoke phase5_smoke
ctest --output-on-failure -L smoke -C Debug
ctest --output-on-failure -C Debug
```

### 序列测试素材

`HD/` 目录内包含 `a0000.jpg` 到 `a0199.jpg` 共 200 张序列图，可用于：

- 目录批处理验证
- CPU 并行验证
- 序列性能验证
- 同尺寸输入下的上下文复用和映射复用验证

## 常见问题

### 1. 启动时提示缺少 DLL

请从完整部署目录启动，或直接使用 Release 包。

不要只拷贝单个 `exe` 文件。

### 2. 程序提示“未能启用 GPU，已回退到 CPU”

这通常说明当前机器没有可用的 OpenCL 运行环境，或显卡驱动链不完整。

这不是程序崩溃，而是设计内的正常回退。

### 3. 中文日志乱码

请使用当前仓库构建出的新版本部署目录，不要混用历史遗留部署产物。

### 4. 序列处理还是慢

当前版本已经做了这些优化：

- CPU 默认自适应多线程
- 同尺寸序列图处理上下文复用
- 像素映射复用

如果仍然慢，下一层瓶颈通常是：

- 大图 JPEG 读取
- JPEG 写出
- 磁盘吞吐

## 版本

当前仓库已完成 `v1.0` 归档，相关里程碑记录位于：

- `.planning/MILESTONES.md`

要点：推荐直接使用 Release 包，开发时再走 CMake + vcpkg 构建流程。
