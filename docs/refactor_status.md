# 重构详细状态

这个文件记录当前原地重构的详细状态。

## 当前策略

重构继续保持渐进：

- 保留现有 `SimpleC`、`IL`、`DevTools` 项目结构和构建目标。
- `CoreLib/` 和 `CoreLibTests/` 已删除，不再继续维护旧基础库。
- 每批保持 `SimpleC Debug|Win32` 可构建。
- 每批优先跑 `scripts\check_smoke.ps1`；只改前端样例时可单独跑 `scripts\check_simplec_smoke.ps1`。
- 涉及 `DevTools/X86_InstrCodeGen` 时额外构建并运行该工具。
- 能用标准库表达清楚的局部容器、所有权和 I/O 实现，应继续使用标准库，不再引入项目自定义基础设施。

## 当前验证状态

本轮删除 `CoreLib/` 和 `CoreLibTests/` 后，已验证通过：

- `SimpleC Debug|Win32`，`0 Warning(s), 0 Error(s)`
- `scripts\check_mainchain_no_corelib.ps1`
- `Debug\SimpleC.exe SimpleC\in.txt`
- `DevTools/X86_InstrCodeGen Debug|Win32`，`0 Warning(s), 0 Error(s)`
- `DevTools/X86_InstrCodeGen` 生成器运行

## SimpleC 前端状态

已完成：

- error list 使用 `std::vector<CompileError>`。
- 多个 AST 列表字段已经从自定义容器换成 `std::vector`。
- AST 节点所有权已经从 CoreLib `RefPtr` 换成 `std::shared_ptr`。
- Parser AST 节点创建已经从 raw `new` 换成 `std::make_shared`。
- `SyntaxVisitor` 工厂已经从 raw pointer / `RefPtr` 使用方式换成 `std::unique_ptr`。
- `SyntaxNode::Accept` visitor 入口已经从 nullable raw pointer 改为引用接口。
- `SyntaxVisitor::VisitXxx` 前端 node 参数已经从 raw pointer 改为引用接口。
- `SyntaxNode` 和 `SyntaxVisitor` 基类已经有虚析构，允许通过标准智能指针安全销毁派生对象。
- `CodeGenerator::CompiledCode` 已经从 `RefPtr` 换成 `std::unique_ptr`。
- CodeGenerator 局部变量表已经使用 `std::unordered_map`。
- `IL/CompileError.h` 已从 CoreLib `String/List/Exception` 解耦为 `std::wstring` 和标准异常边界。
- `Lexer` 已从 CoreLib `String/StringBuilder` 改成 `std::wstring` 输入、token 文本和内部 builder。
- AST 文本字段和 `ExpressionType::ToString()` 已从 CoreLib `String` 改成 `std::wstring`。
- Parser、SemanticsVisitor、SyntaxPrinter 已适配标准库宽字符串。
- SyntaxPrinter 已去掉 C 风格 `printf` / `wprintf_s` 输出和 `NULL` 判断，改为标准宽字符输出流和 `nullptr`。
- CodeGenerator unsupported-codegen 错误已改用标准 `std::runtime_error`。
- SimpleC 非测试代码已无 CoreLib 直接引用。
- `SimpleC.vcxproj` 已移除 CoreLib include directory 和 CoreLib project reference。

当前残留：

- SimpleC 主目标已经完成 CoreLib 脱钩。后续只需要防止新代码重新引入等价的旧基础设施。

## IL 和分析工具状态

已完成：

- 若干局部 `Dictionary` / `HashSet` 使用已换成 `std::unordered_map` / `std::unordered_set`。
- optimizer pipeline 子 pass 存储已换成 `std::vector`。
- optimizer pipeline 子 pass 所有权已从 CoreLib `RefPtr<IntraProcOptimizer>` 改为 `std::unique_ptr`。
- optimizer 工厂返回值已从 raw `IntraProcOptimizer*` 改为 `std::unique_ptr<IntraProcOptimizer>`。
- x86 binary emitter 字节缓冲已从 CoreLib `List<unsigned char>` 换成 `std::vector<unsigned char>`。
- x86 emitter 重定位表、函数元数据表、浮点常量表等多处局部存储已换成 `std::vector`。
- `Program_x86` 生成函数列表已换成 `std::vector<Function_x86>`。
- x86 `Function_x86::Name` 已迁移为 `std::wstring`，和 IL 函数名保持标准库字符串边界。
- x86 code generator 工厂返回值已从 raw `X86CodeGenerator*` 改为 `std::unique_ptr<X86CodeGenerator>`。
- CFG post-order traversal 结果容器已换成 `std::vector<ControlFlowNode*>`。
- interference analysis live-range 结果存储已换成 `std::vector<std::shared_ptr<LiveRange>>`。
- `Intermediate::Program` 函数列表已换成 `std::vector<Function>`。
- `Intermediate::Function` 变量和参数存储已换成 `std::vector<std::shared_ptr<Variable>>`。
- `ControlFlowGraph::VarDefs` 已换成 `std::vector<InstructionNode*>`。
- `Operand::ToString()`、`Instruction::ToString()` 和 x86 `Instruction::ToString()` 已从 CoreLib `String/StringBuilder` 改为 `std::wstring`。
- CFG dump 的 IR 文本缓冲已从 CoreLib `StringBuilder` 改为 `std::wstring`。
- `Intermediate::Instruction::Operands` 已从 CoreLib `List<Operand>` 改为 `std::vector<Operand>`。
- `ControlFlowGraph::Variables`、`Nodes`、支配树、支配边界、CFG edge 列表已迁移到标准库容器。
- optimizer 管线中的 CFG 所有权边界已从 CoreLib `RefPtr<ControlFlowGraph>` 改为 `std::shared_ptr<ControlFlowGraph>`。
- CFG 节点所有权已从 CoreLib `RefPtr<ControlFlowNode>` 改为 `std::shared_ptr<ControlFlowNode>`。
- out-of-SSA 内部 `PhiClasses` 已从 CoreLib `RefPtr<EnumerableIntSet>` 改为 `std::shared_ptr<EnumerableIntSet>`。
- IL `Variable` 所有权已从 CoreLib `RefPtr<Variable>` 改为 `std::shared_ptr<Variable>`。
- 当前 `IL` 目录已经没有 `RefPtr<...>` 命中。
- IL `Function::Instructions` 和 `ControlFlowNode::Code` 已从 CoreLib `LinkedList<Instruction>` 改为本地 `InstructionList` 过渡层。
- IL `IntSet` / `BitIntSet` 已迁移为 IL 本地标准库 backed 实现。
- `IL.vcxproj` 已移除 CoreLib include 路径和 CoreLib project reference；当前 `IL` 目录没有 CoreLib 命中。

注意：

- x87 double codegen 修复属于真实 correctness 修复，但已经接近后端扩展。后续不应继续沿这条线扩展，除非阻塞固定验证。
- 用户当前明确要求先不要继续 IL 结构收口。

## CoreLib 删除状态

已完成：

- `CoreLib/` 已删除。
- `CoreLibTests/` 已删除。
- 旧 `--corelib-self-test` 入口随 CoreLib 删除退役。
- 主链路验证改为 `scripts\check_smoke.ps1` 统一入口。

这意味着后续不再有“继续完善 String/Stream/TextIO/LibIO”的工作项。相关历史修复只作为已完成背景存在，不再作为当前计划。

## 最小回归覆盖

`scripts\check_mainchain_no_corelib.ps1` 当前覆盖：

- 扫描 `CCompiler.sln`、`SimpleC`、`IL`、`DevTools/X86_InstrCodeGen` 主链路，阻断 `CoreLib::`、`CoreLib\`、`Basic.h`、`RefPtr`、`SmartPointer`、`LinkedList`、`LinkedNode` 重新引入。

不足：

- 这还不是完整编译器测试体系。
- 缺少系统性的前端 parser/semantic regression。
- 缺少稳定的后端行为测试。
- `scripts\check_x86_generator_drift.ps1` 只检查生成器可运行、不会输出旧 `code.Add` API、声明仍匹配，并报告生成产物和 `IL/Instruction_x86.cpp` 的已知手工漂移；它不是 x86 行为测试。
- `scripts\check_simplec_smoke.ps1` 当前覆盖基础函数调用、局部变量和 if/else 样例；它仍不是系统性 parser/semantic 测试。

## 与原始目标的偏离总结

原始目标的优先级：

1. 先稳定并现代化内部实现。
2. 优先缩减自定义容器和手写资源管理。
3. 增加最小回归测试。
4. 压缩兼容层暴露面。
5. 最后再考虑大接口替换或架构升级。

当前结论：

- 主链路甩掉 CoreLib 的目标已经完成。
- 删除 CoreLib 后，继续在 CoreLib 内部做 correctness 或现代化已经没有意义。
- 后续应转向 DevTools 生成器关系、验证体系和必要的 IL/x86 边界整理。

## 当前下一步建议

短期：

- 增加更系统的 SimpleC 前端 parser/semantic regression。
- 继续检查 `DevTools/X86_InstrCodeGen` 与 `IL/Instruction_x86.cpp` 的手工差异，避免误覆盖生成产物。
- 每批继续保持 `scripts\check_smoke.ps1` 为绿。

暂不做：

- 更多 x86 codegen 功能。
- IL 深层结构收口。
- LLVM-style IR 重做。
- 大规模 public API 破坏。
- 大范围机械 cast 清理。
