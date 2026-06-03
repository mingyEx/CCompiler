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
- `DevTools/`：附属工具，包括旧 CFG 查看器和 x86 指令辅助工具。

`CoreLib/` 和 `CoreLibTests/` 已删除。主编译链路不再构建、链接或包含历史 CoreLib 基础库。

## 已完成的现代化方向

当前主线没有推翻现有 AST/IR，而是先拆掉外围自定义基础设施：

- 编译入口从 `wmain` 中拆出到独立 pipeline。
- 源文件加载和 dump 输出改用 C++ 标准库。
- SimpleC 前端 AST 容器、字符串和所有权边界已切到 `std::vector`、`std::wstring`、`std::shared_ptr` / `std::unique_ptr`。
- IL 主要容器、CFG 所有权、变量所有权、指令列表过渡层和 x86 文本输出已脱离 CoreLib。
- `SimpleC`、`IL`、`DevTools/X86_InstrCodeGen` 均已移除 CoreLib include/project reference。
- `CoreLib/` 和历史 `CoreLibTests/` 已从仓库删除。

## 当前遗留边界

现在不再存在“继续完善 CoreLib”的阶段。后续值得关注的是仍然带有旧 Windows/x86 假设或生成器耦合的区域：

- `IL/Assembly_x86.*`
- `IL/CodeEmitter_x86.cpp`
- `IL/X86CodeGen.cpp`
- `DevTools/X86_InstrCodeGen`

短期重点应放在 `DevTools/X86_InstrCodeGen` 的生成器关系、调用方式和验证方式上；IL 结构收口暂按当前指令后置。
