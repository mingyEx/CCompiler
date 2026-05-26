# CCompiler 架构说明

## 编译流水线概览

`SimpleC` 目前实现的是一个完整的教学型编译器流水线：

1. `Lexer` 将源文本切分成 token。
2. `Parser` 构建 `Syntax` 抽象语法树。
3. `SemanticsVisitor` 执行基础语义检查。
4. `CodeGenerator` 将语法树降低到项目自定义中间代码。
5. `IL/CFG` 为每个函数重建控制流图。
6. `IL/Optimization` 运行基于 SSA 的优化 pass。
7. `IL/X86CodeGen` 将优化后的 IR 降低成 x86 风格指令。

命令行入口现在分成两层：

- [Main.cpp](</C:/Users/mingy/Documents/New project/CCompiler/SimpleC/Main.cpp:1>) 只负责进程启动和参数入口。
- [compiler_pipeline.cpp](</C:/Users/mingy/Documents/New project/CCompiler/SimpleC/compiler_pipeline.cpp:1>) 负责端到端编译流程。

## 当前模块地图

- `SimpleC/`：前端、语义检查、编译流程编排。
- `IL/`：中间表示、CFG 构建、SSA 转换、优化、x86 lowering。
- `CoreLib/`：历史兼容基础库，包含自定义字符串、容器、智能指针和 I/O 包装。
- `DevTools/`：附属工具，包括旧 CFG 查看器和 x86 指令辅助工具。

## 已完成的第一轮现代化方向

第一轮没有推翻现有 AST/IR，而是先降低外围层的修改难度：

- 编译入口从 `wmain` 中拆出到独立 pipeline。
- 源文件加载开始使用 C++ 标准库。
- `CoreLib::IO::File` 和 `CoreLib::IO::Path` 部分委托给 `std::filesystem` 与标准文件流。
- `CoreLib::IO::FileStream` 内部逐步改成标准库文件流，同时保留旧 `Stream` 接口。
- `CoreLib::IO::TextIO` 主构建路径减少了对 Win32 文本转换 API 的依赖。
- 默认编译路径不再执行生成的机器码。
- 多数 IL dump/export 路径已改用 C++ 标准库写文件。

## 当前遗留边界

仍然明显依赖历史基础设施或 Windows/x86 假设的区域：

- `CoreLib/LibString.h`
- `CoreLib/SmartPointer.h`
- `CoreLib/List.h`
- `CoreLib/Dictionary.h`
- `CoreLib/TextIO.*`
- `CoreLib/Stream.*`
- `IL/Assembly_x86.*`
- `IL/CodeEmitter_x86.cpp`
- `IL/X86CodeGen.cpp`

从“先替换标准库可替换部分”的目标看，下一阶段更应该继续缩小 `SimpleC` 前端和 `CoreLib` 对 `RefPtr`、`List`、`Dictionary`、手写 I/O 包装的依赖，而不是继续扩展后端功能。
