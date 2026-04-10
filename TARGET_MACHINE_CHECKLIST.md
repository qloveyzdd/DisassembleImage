# 目标机检查清单

## 部署前

- 安装最新显卡驱动，不要只装系统默认显示驱动。
- 确认目标机能正常识别 GPU 设备。
- 使用部署目录中的程序启动，不要只拷贝单个 `exe`。

## 启动检查

- 打开 [DisassembleImageDesktop.exe](E:/DisassembleImage/build/windows-msvc-debug/deploy/disassemble_desktop/DisassembleImageDesktop.exe)。
- 确认程序能正常启动，没有缺少 `Qt`、`OpenCV` 或运行库的报错。
- 查看主界面的 `OpenCL 诊断` 区域，记录：
  - `OpenCL 支持`
  - `可用 GPU 设备`
  - `设备名称`
  - `设备厂商`
  - `驱动版本`
  - `OpenCL C 版本`
  - `诊断结论`

## 运行验证

- 选择输入图 [a0000.jpg](E:/DisassembleImage/HD/a0000.jpg)。
- 确认模型文件 [input.obj](E:/DisassembleImage/input.obj) 和 [output.obj](E:/DisassembleImage/output.obj) 可用。
- 选择一个新的空输出目录。
- 分别测试 `自动`、`仅 CPU`、`优先 GPU` 三档后端。
- 每次运行都记录：
  - 是否成功启动处理
  - 上次实际后端
  - 是否发生回退
  - 回退原因
  - 耗时
  - 结果数量
  - 结果尺寸

## 预期结果

- `仅 CPU` 必须稳定跑通。
- `自动` 模式下，如果 GPU 不可用，必须明确提示并回退到 CPU。
- `优先 GPU` 模式下，如果 GPU 不可用，也必须明确提示原因。
- 结果图数量和尺寸要符合预期，肉眼看不能明显错乱。
- 程序能正常打开结果目录，日志信息可读。

## 验收记录

- Intel / AMD 机器的验收结果，请整理到你们当前使用的验收记录、issue 或 release 备注中。
