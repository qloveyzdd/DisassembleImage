# 贡献指南

感谢你愿意参与 DisassembleImage。

## 开始之前

- 提交前请先确认你有权公开相关代码、样例图片、模型文件和截图。
- 功能改动尽量保持小而清晰，不要顺手做无关重构。
- 如果改动会影响输入契约、导出格式或 UI 行为，请先开 issue 说明原因。

## 本地开发

推荐环境：

- Windows 10/11 x64
- Visual Studio 2022
- CMake 3.26+
- Git

配置与构建：

```powershell
cmake --preset windows-msvc-debug
cmake --build --preset build-windows-debug --target disassemble_desktop
```

运行 smoke tests：

```powershell
cmake --build --preset build-windows-debug --target phase1_smoke phase2_smoke phase3_smoke phase4_smoke phase5_smoke
ctest --test-dir build/windows-msvc-debug --output-on-failure -L smoke -C Debug
```

## 提交建议

- 一个提交只解决一件事。
- 提交信息尽量说明改动的用户价值或修复点。
- 如果改了构建、打包或依赖版本，请同步更新 README 和相关配置。

## Bug 报告

建议在 issue 里提供这些信息：

- 输入模式：单图 / 目录
- 使用的 `input.obj` / `output.obj`
- CPU / GPU / 自动 选择
- OpenCL 状态和错误提示
- 是否能按 [TARGET_MACHINE_CHECKLIST.md](/E:/DisassembleImage/TARGET_MACHINE_CHECKLIST.md) 复现
- 最小可复现样例或截图

## Pull Request

- 说明问题、方案和验证方式。
- 如果界面有变化，附一张截图或短录屏。
- 不要把 `build/`、发布 zip、IDE 私有配置提交进仓库。
