# DisassembleImage

用于把单张图片按 `OBJ/UV` 几何关系拆分成多张切片图，并在桌面程序中完成配置、处理、结果查看和 3D 对照。

## 功能概览

- Windows 原生桌面程序
- 支持单张图片和图片目录两种输入方式
- 支持图形化参数配置、预设保存和启动前校验
- 支持执行进度、日志、结果导出和结果目录打开
- 支持切片缩略图、大图预览和 3D 对照
- 支持 `自动 / CPU / GPU` 三种后端模式
- GPU 不可用时会明确提示并回退到 CPU

## 目录说明

- [app/windows](E:/DisassembleImage/app/windows): 桌面界面和运行控制
- [core](E:/DisassembleImage/core): 核心处理逻辑、模型和 GPU 探测
- [tests](E:/DisassembleImage/tests): smoke 测试
- [HD](E:/DisassembleImage/HD): 示例输入图和序列测试图
- [input.obj](E:/DisassembleImage/input.obj): 输入模型
- [output.obj](E:/DisassembleImage/output.obj): 输出模型

## 环境要求

### 运行环境

- Windows 10/11 x64
- 建议安装最新显卡驱动
- 若要测试 GPU，当前主路径依赖 `OpenCL`

### 开发环境

- Visual Studio 2022
- CMake 3.26 或更高版本
- Git

### 第三方依赖

项目当前通过工程内 `vcpkg` 恢复主要依赖：

- `OpenCV`
- `Qt6`

相关配置见：

- [CMakePresets.json](E:/DisassembleImage/CMakePresets.json)
- [vcpkg.json](E:/DisassembleImage/vcpkg.json)
- [VcpkgWorkspaceToolchain.cmake](E:/DisassembleImage/cmake/VcpkgWorkspaceToolchain.cmake)

## 直接使用二进制

当前已经生成了一份可直接分发的 Windows 二进制包：

- 本地压缩包：[DisassembleImage-v1.0-win64.zip](E:/DisassembleImage/build/release-packages/DisassembleImage-v1.0-win64.zip)
- 解压目录：[disassemble_desktop](E:/DisassembleImage/build/windows-msvc-release/deploy/disassemble_desktop)

直接使用方式：

1. 解压 `DisassembleImage-v1.0-win64.zip`
2. 保持解压后的目录结构不变
3. 运行：
   - [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-release/deploy/disassemble_desktop/DisassembleImageDesktop.exe)

注意：

- 不要只单独拷贝 `DisassembleImageDesktop.exe`
- `Qt/OpenCV` 运行库、插件目录、`input.obj`、`output.obj` 需要和程序放在同一套目录里
- 当前包适合本地分发和手动测试

如果后续要对外发布，建议把同一个 zip 上传到 GitHub Release，而不是直接提交到仓库。

## 构建说明

### Debug 构建

在仓库根目录执行：

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

### 生成后的可执行文件

开发目录：

- [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-debug/app/windows/Debug/DisassembleImageDesktop.exe)

推荐手动测试和分发使用 Release 部署目录：

- [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-release/deploy/disassemble_desktop/DisassembleImageDesktop.exe)

Debug 部署目录主要用于开发调试：

- [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-debug/deploy/disassemble_desktop/DisassembleImageDesktop.exe)

不要只单独拷贝 `exe`，需要和同目录下的 `Qt/OpenCV` 运行库一起保留。

## 使用说明

### 基本处理流程

1. 启动部署目录下的桌面程序。
2. 选择输入方式：
   - 单张图片
   - 图片目录
3. 选择输入图片或输入目录。
4. 选择输出目录。
5. 默认会自动探测：
   - [input.obj](E:/DisassembleImage/input.obj)
   - [output.obj](E:/DisassembleImage/output.obj)
6. 按需要设置：
   - 输出尺寸
   - 输出前缀
   - 拼接方式
   - 输出策略
7. 如需高级设置，可展开：
   - 计算后端
   - 并行开关
   - 并行线程数
8. 点击“开始处理”。
9. 处理完成后查看：
   - 摘要日志
   - 技术日志
   - 结果目录
   - 切片缩略图
   - 3D 对照

### 后端说明

- `自动`: 优先尝试 GPU，不可用时回退 CPU
- `仅 CPU`: 强制走 CPU
- `优先 GPU`: 优先走 GPU，不可用时给出明确提示并回退

### OpenCL / GPU 说明

当前 GPU 路线基于 `OpenCV + OpenCL + UMat`。

这意味着：

- 有显卡不等于一定能启用 GPU
- 必须有可用的 OpenCL 运行环境
- 如果缺少 OpenCL 或驱动不完整，程序会提示并回退 CPU

目标机检查可参考：

- [TARGET_MACHINE_CHECKLIST.md](E:/DisassembleImage/TARGET_MACHINE_CHECKLIST.md)

## 测试说明

### Smoke 测试

```powershell
cmake --build --preset build-windows-debug --target phase1_smoke phase2_smoke phase3_smoke phase4_smoke phase5_smoke
ctest --output-on-failure -L smoke -C Debug
ctest --output-on-failure -C Debug
```

### 序列测试素材

[HD](E:/DisassembleImage/HD) 目录中包含序列测试图：

- `a0000.jpg` 到 `a0199.jpg`

这些图片用于测试：

- 目录批处理
- CPU 并行
- 序列处理速度
- 同尺寸输入下的上下文复用与映射复用

## 常见问题

### 1. 启动时报缺少 DLL

请从完整部署目录启动，优先使用 Release 包：

- [DisassembleImage-v1.0-win64.zip](E:/DisassembleImage/build/release-packages/DisassembleImage-v1.0-win64.zip)
- [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-release/deploy/disassemble_desktop/DisassembleImageDesktop.exe)

开发调试时也可以使用 Debug 部署目录：

- [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-debug/deploy/disassemble_desktop/DisassembleImageDesktop.exe)

不要只复制单个 `exe`。

### 2. 显示“未能启用 GPU，已回退到 CPU”

说明当前机器没有可用的 `OpenCL` 运行环境，或驱动链不完整。  
这不是程序崩溃，而是按设计回退。

### 3. 中文日志乱码

请使用当前仓库中的新版本构建产物，不要混用旧的历史部署目录。

### 4. 序列处理还是慢

当前版本已经做了：

- CPU 默认自适应多线程
- 同尺寸序列图的处理上下文复用
- 像素映射复用

如果仍然慢，下一层瓶颈通常是：

- 大图 JPEG 读取
- JPEG 写出
- 磁盘吞吐

## 版本状态

当前仓库已经完成 `v1.0` 里程碑归档，相关记录见：

- [MILESTONES.md](E:/DisassembleImage/.planning/MILESTONES.md)

要点：这是一个已经完成 `v1.0` 桌面化交付的本地图片拆分工具，推荐从部署目录直接启动测试。
