# DisassembleImage

一个面向 Windows 的本地图片拆分工具。
它会根据 `input.obj` / `output.obj` 里的几何与 UV 信息，把输入图片拆分成切片图，并提供桌面界面完成参数配置、执行处理、结果导出和结果预览。

## 当前交付

- 平台：`Windows 10/11 x64`
- 界面：`Qt6 Widgets`
- 图像处理：`OpenCV`
- GPU 路径：`OpenCV + OpenCL + UMat`
- 回退策略：GPU 不可用时会明确提示，并自动回退到 CPU

## 目录说明

- `app/windows/`：桌面界面、任务表单、运行控制、预览与 3D 组件
- `core/`：核心处理逻辑、OBJ/UV 解析、切片计划、GPU 诊断
- `cmake/`：vcpkg 工具链和 Windows 部署辅助脚本
- `tests/`：smoke 测试
- `HD/`：示例输入和序列测试图片
- `release-packages/`：仓库内可直接分发的打包结果

## 直接使用

当前仓库内可直接使用的 Windows 包：

- `release-packages/DisassembleImage-v1.0.1-win64.zip`

使用步骤：

1. 下载并解压 `DisassembleImage-v1.0.1-win64.zip`
2. 保持解压后的目录结构不变
3. 运行 `DisassembleImageDesktop.exe`

注意：

- 不要只单独拷贝 `DisassembleImageDesktop.exe`
- `Qt`、`OpenCV` 运行库、插件目录、`input.obj`、`output.obj` 都需要和程序放在同一套目录中
- 如果你是从源码构建，优先使用部署目录里的程序，而不是只拿构建目录下的 `exe`

## 从源码构建

环境要求：

- Windows 10/11 x64
- Visual Studio 2022
- CMake 3.26 或更高
- Git

项目通过工程内 `vcpkg` 恢复主要依赖：

- `opencv`
- `qtbase`

相关配置：

- `CMakePresets.json`
- `vcpkg.json`
- `cmake/VcpkgWorkspaceToolchain.cmake`

Debug 构建：

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset build-windows-debug --target disassemble_desktop
cmake --build --preset build-windows-debug --target windows_deploy_check
```

Release 构建：

```powershell
cmake --preset windows-msvc-release
cmake --build --preset build-windows-release --target disassemble_desktop
cmake --build --preset build-windows-release --target windows_deploy_check
```

常用产物位置：

- Debug 可执行文件：`build/windows-msvc-debug/app/windows/Debug/DisassembleImageDesktop.exe`
- Debug 部署目录：`build/windows-msvc-debug/deploy/disassemble_desktop/`
- Release 可执行文件：`build/windows-msvc-release/app/windows/Release/DisassembleImageDesktop.exe`
- Release 部署目录：`build/windows-msvc-release/deploy/disassemble_desktop/`

## 使用说明

基本流程：

1. 启动桌面程序
2. 选择输入模式：单张图片或图片目录
3. 选择输入图片或输入目录
4. 选择输出目录
5. 检查或手动指定 `input.obj` / `output.obj`
6. 设置输出尺寸、输出前缀、拼接方式、输出策略
7. 如有需要，展开高级设置，选择 `自动 / CPU / GPU` 和并行线程数
8. 点击“开始处理”
9. 处理完成后查看摘要日志、结果目录和预览

## 结果预览说明

为避免大批量结果完成后界面卡住，当前版本做了以下调整：

- 处理完成后不再自动加载第一张原图、切片图和 3D 预览
- 缩略图只保留一小批样本用于快速浏览
- 3D 预览改为手动点击“加载 3D 预览”
- 预览贴图会按预览尺寸降采样，避免一次性占用过多内存

如果任务结果很多，这是预期行为，不是程序卡死。

## GPU / OpenCL 说明

当前 GPU 路径基于 `OpenCV + OpenCL + UMat`。

这意味着：

- 有显卡不等于一定能启用 GPU
- 机器必须存在可用的 OpenCL 运行环境
- 如果 OpenCL 不可用，程序会提示原因并回退到 CPU

界面里会给出：

- OpenCL 支持状态
- 可用 GPU 设备状态
- 平台枚举结果
- 失败阶段
- 回退原因

如果目标机要做部署或验收，请同时参考：

- `TARGET_MACHINE_CHECKLIST.md`

## 测试

Smoke 测试：

```powershell
cmake --build --preset build-windows-debug --target phase1_smoke phase2_smoke phase3_smoke phase4_smoke phase5_smoke
ctest --output-on-failure -L smoke -C Debug
ctest --output-on-failure -C Debug
```

## 常见问题

### 1. 启动时提示缺少 DLL

请从完整部署目录启动，或直接使用 `release-packages/DisassembleImage-v1.0.1-win64.zip`。
不要只拷贝单个 `exe` 文件。

### 2. 提示“未能启用 GPU，已回退到 CPU”

这通常说明当前机器没有可用的 OpenCL 运行环境，或显卡驱动链不完整。
这不是程序崩溃，而是设计内的正常回退。

### 3. 结果很多时为什么不自动显示所有大图和 3D

这是为了避免界面在任务结束瞬间卡住，也避免超大量结果把内存打满。
当前策略是先展示样本缩略图，再按需加载大图和 3D。

## 版本

当前仓库包含一个可直接分发的包：

- `release-packages/DisassembleImage-v1.0.1-win64.zip`

要点：普通使用直接拿包，开发时再走 CMake + vcpkg 构建流程。
