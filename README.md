# DisassembleImage

一个面向 Windows 桌面的 OBJ/UV 图片切片工具。  
它根据 `input.obj` / `output.obj` 里的几何和 UV 关系，把单张图片或图片目录拆成切片结果，并提供本地图形界面完成参数配置、执行处理、结果查看和导出。

## 项目状态

- 平台：`Windows 10/11 x64`
- GUI：`Qt6 Widgets`
- 图像处理：`OpenCV`
- GPU 路径：`OpenCV + OpenCL + UMat`
- 构建方式：`CMake + vcpkg`
- 验证方式：`CTest smoke tests`
- 许可证：`Apache-2.0`

## 当前能力

- 支持单张图片和图片目录两种输入模式
- 支持 CPU / GPU / 自动 三种执行模式
- GPU 不可用时会给出诊断信息并自动回退到 CPU
- 处理完成后提供摘要日志、采样缩略图、按需大图预览和手动 3D 预览
- 提供 Windows 构建预设和最小 CI

## 输入契约

当前模型解析与切片流程依赖这些前提：

- 需要同时提供 `input.obj` 和 `output.obj`
- OBJ 解析当前依赖 `v`、`vt` 和四边形 `f` 面
- 遇到非四边面会直接报错
- `input.obj` 和 `output.obj` 需要保持面数量与拓扑对应

如果这些前提不满足，程序当前不会自动修复输入，而是直接终止当前处理。

## 快速开始

Windows 可执行包请从 GitHub Releases 下载：

- <https://github.com/qloveyzdd/DisassembleImage/releases>

标签版本推送后，仓库会自动构建并上传对应的 Windows 发布包。

运行时请保持部署目录完整，不要只单独拷贝 `DisassembleImageDesktop.exe`。  
`Qt`、`OpenCV` 运行库、插件目录以及所需模型文件需要和程序放在同一套目录结构内。

## 从源码构建

环境要求：

- Windows 10/11 x64
- Visual Studio 2022
- CMake 3.26+
- Git

项目通过 `vcpkg.json` 清单恢复主要依赖：

- `opencv`
- `qtbase`

准备依赖时二选一即可：

- 把 `vcpkg` 放在 `tools/vcpkg`
- 或提前设置环境变量 `VCPKG_ROOT`

配置文件：

- `CMakePresets.json`
- `vcpkg.json`
- `cmake/VcpkgWorkspaceToolchain.cmake`

Debug：

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset build-windows-debug --target disassemble_desktop
```

Release：

```powershell
cmake --preset windows-msvc-release
cmake --build --preset build-windows-release --target disassemble_desktop
```

如果你需要完整部署目录，再额外执行：

```powershell
cmake --build --preset build-windows-release --target windows_deploy_check
```

## 测试

```powershell
cmake --build --preset build-windows-debug --target phase1_smoke phase2_smoke phase3_smoke phase4_smoke phase5_smoke
ctest --test-dir build/windows-msvc-debug --output-on-failure -L smoke -C Debug
```

## 结果预览策略

为了避免大量结果在完成瞬间把界面卡住或把内存打满，当前版本采用按需预览策略：

- 完成后不自动加载第一张原图、切片图和 3D
- 缩略图只保留一小批样本用于快速浏览
- 3D 预览改为手动加载
- 预览贴图按预览尺寸降采样

结果很多时，先看到样本缩略图是预期行为，不是程序卡死。

## 仓库结构

- `app/windows/`：桌面界面、任务表单、运行控制、预览与 3D 组件
- `core/`：核心处理逻辑、OBJ/UV 解析、切片计划、GPU 诊断
- `cmake/`：vcpkg 工具链和 Windows 部署辅助脚本
- `tests/`：smoke tests
- `HD/`：本地样例素材

## 协作

- 贡献说明见 `CONTRIBUTING.md`
- 行为准则见 `CODE_OF_CONDUCT.md`
- 安全问题处理见 `SECURITY.md`

## 版本

当前源码版本：`1.0.1`
