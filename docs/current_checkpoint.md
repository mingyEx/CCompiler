# 当前检查点

这个文件记录当前可以安全续接的状态。

## 时间

- 保存日期：2026-05-31

## 最新已验证状态

- 目标：`SimpleC Debug|Win32`
- 构建：通过，`0 Warning(s), 0 Error(s)`
- 最小回归：`Debug\CoreLibTests.exe --corelib-self-test` 通过
- 自测输出：`CoreLib regression tests passed.`
- 主编译 smoke：`Debug\SimpleC.exe SimpleC\in.txt` 通过
- 临时 double codegen smoke：已通过，覆盖 `I2D`、double `+`、`-`、`*`、`/`、本地 double 存储和 ST0 double return
- 临时 smoke 产物：已清理

## 最近完成的主线工作

- 将 SimpleC 前端 AST 所有权从 CoreLib `RefPtr` 替换为 `std::shared_ptr`。
- 将 SimpleC parser 的 AST 节点创建从 raw `new` 替换为 `std::make_shared`。
- 将 `SyntaxVisitor` 工厂返回值从 raw pointer / `RefPtr` 使用方式替换为 `std::unique_ptr`。
- 为 SimpleC 的 `SyntaxNode` 和 `SyntaxVisitor` 基类补了虚析构，保证通过标准智能指针按基类删除是安全的。
- 将 `CodeGenerator::CompiledCode` 从 CoreLib `RefPtr` 替换为 `std::unique_ptr`。
- 删除未参与构建、仍使用旧 `SmartPointer` 和 `List::Add` parser API 的 `SimpleC/ParserTest.cpp`。
- 当前 SimpleC 前端 AST 相关文件中已清掉 `RefPtr<...Syntax...>`、`SmartPointer<...Syntax...>` 和 `new ...SyntaxNode`。
- 将 `IL/CompileError.h` 从 CoreLib `String/List/Exception` 解耦，改为 `std::wstring` 和 `std::runtime_error` 边界。
- 将 SimpleC `Lexer` 输入、token 文本和内部 token builder 从 CoreLib `String/StringBuilder` 改为 `std::wstring`。
- 将 SimpleC AST 文本字段、`ExpressionType::ToString()`、Parser、SemanticsVisitor、SyntaxPrinter 改为标准库宽字符串。
- 当前 SimpleC 前端核心语法树与 lexer/parser/semantic/printer 已不再直接依赖 `Basic.h`；CoreLib 字符串主要收缩到 CodeGenerator 进入 IL 的转换边界和 compiler pipeline 的 IL dump/optimizer 边界。
- 为 `X86CodeGenerator` 基类补虚析构，并将 compiler pipeline 顶层 x86 generator / optimizer 局部所有权改为 `std::unique_ptr`。
- 为 IL `Program::Dump`、`ControlFlowGraph::Dump`、x86 `Function_x86::Dump` 增加 `std::filesystem::path` 重载，pipeline 输出路径不再绕 CoreLib `String`。
- 移除 x86 codegen 中无条件写 `d:\programdump.asm` 的旧调试副作用。
- 为 IL `Function::SetName`、`AddVariable`、`AddParameter` 增加 `std::wstring` 接入口，SimpleC CodeGenerator 不再需要在变量名/参数名处手写 CoreLib `String` 转换。
- 将 IL `Function::Name`、`Variable::Name` 和 x86 `Function_x86::Name` 字段迁移为 `std::wstring`，保留旧 `CoreLib::String` 入口做兼容转发。
- 修正 SSA/out-of-SSA、const-indirection optimizer、register allocation 和 IR dump 中对变量名的字符串拼接，避免重新引入 CoreLib `String` 作为名称载体。
- 将 IL optimizer pipeline 的子 pass 所有权从 CoreLib `RefPtr<IntraProcOptimizer>` 替换为 `std::unique_ptr`，同时保留 CFG 图对象的 `RefPtr` 接口边界不动。
- 将 IL optimizer 工厂返回值从 raw `IntraProcOptimizer*` 改为 `std::unique_ptr<IntraProcOptimizer>`，并清掉 compiler pipeline 中的 optimizer raw `new` 拼装。
- 将 x86 code generator 工厂返回值从 raw `X86CodeGenerator*` 改为 `std::unique_ptr<X86CodeGenerator>`，pipeline 不再手动包装 raw pointer。
- 将 SimpleC 前端 visitor 接口从 `Accept(SyntaxVisitor*)` 改为 `Accept(SyntaxVisitor&)`，清掉语义分析、语法打印、代码生成和 pipeline 调用链里的 nullable visitor pointer 入口。
- 将 SimpleC `SyntaxVisitor::VisitXxx` 前端分派接口从 raw node pointer 改为 node 引用，`Syntax.cpp` dispatch 现在调用 `VisitXxx(*this)`。
- 将 IL `Operand::ToString()`、`Instruction::ToString()` 和 x86 `Instruction::ToString()` 从 CoreLib `String/StringBuilder` 改为直接返回 `std::wstring`。
- 将 CFG dump 中的 IR 文本缓冲从 CoreLib `StringBuilder` 改为 `std::wstring`。
- 将 IL `Instruction::Operands` 从 CoreLib `List<Operand>` 改为 `std::vector<Operand>`，并同步更新 CFG、optimizer、out-of-SSA、x86 codegen 和最小自测里的访问 API。
- 将 `ScopeDictionary` 从 CoreLib hash helper 解耦，改为标准 `std::unordered_map` 默认 hash/equality，并收缩为唯一使用 optimizer 的私有 helper。
- 清掉 IL 非测试路径中的 CoreLib `StringBuilder` 残留，并删除无调用的 CoreLib `String` dump/变量名兼容入口。
- 将 interference analysis 的 `LiveRange` 结果从 CoreLib `RefPtr` 改为 `std::shared_ptr`。
- 将 `ControlFlowGraph::Variables` 从 CoreLib `List<RefPtr<Variable>>` 改为 `std::vector<RefPtr<Variable>>`，同步更新 SSA、out-of-SSA、optimizer、register allocation 和 interference analysis 访问点。
- 将 `ControlFlowGraph::Nodes` 从 CoreLib `List<RefPtr<ControlFlowNode>>` 改为 `std::vector<RefPtr<ControlFlowNode>>`，同步更新 CFG 构建、支配树、liveness、SSA、dead-code/control-flow cleanup 和 register allocation 访问点。
- 将 `ControlFlowNode` 的支配树和支配边界列表从 CoreLib `List<ControlFlowNode*>` 改为 `std::vector<ControlFlowNode*>`，覆盖 dominator 计算、SSA phi 放置、CFG dump 和相关自测。
- 将 `ControlFlowNode::Entries` CFG edge 列表从 CoreLib `List<ControlFlowNode*>` 改为 `std::vector<ControlFlowNode*>`，同步更新 CFG 构建、SSA phi 参数、control-flow cleanup 和 dead-code cleanup。
- 将 optimizer 管线中的 `ControlFlowGraph` 所有权边界从 CoreLib `RefPtr<ControlFlowGraph>` 改为 `std::shared_ptr<ControlFlowGraph>`，`ControlFlowGraph::FromCode` 现在直接返回 `std::shared_ptr`。
- 将 CFG 节点所有权从 CoreLib `RefPtr<ControlFlowNode>` 改为 `std::shared_ptr<ControlFlowNode>`，CFG edge 仍使用 raw pointer 作为非拥有引用。
- 将 out-of-SSA 内部 `PhiClasses` 从 CoreLib `RefPtr<EnumerableIntSet>` 改为 `std::shared_ptr<EnumerableIntSet>`。
- 将 IL `Variable` 所有权从 CoreLib `RefPtr<Variable>` 改为 `std::shared_ptr<Variable>`，Function、CFG、SSA、out-of-SSA、optimizer 和 register allocation 调用点已同步更新。
- 当前 `IL` 目录已经没有 `RefPtr<...>` 命中；`SimpleC` 剩余 `RefPtr` 只在 `corelib_regression_tests.cpp` 中用于测试 CoreLib 自身。
- 将 CFG liveness 和 SSA phi placement 的局部 `List<IntSet>` 工作集改为 `std::vector<IntSet>`。
- 清掉 SimpleC/IL 主链路中的 `using namespace CoreLib::Basic`，改为精确导入仍未替换的 `IntSet`、`BitIntSet`、`LinkedList`、`LinkedNode` 和旧异常类型。
- `SimpleC/compiler_pipeline.cpp` 不再直接 include `Basic.h`，只在命令行边界保留对旧 CoreLib 异常的兼容捕获。
- `SimpleC/CodeGenerator.cpp` 的 unsupported-codegen 错误已从 CoreLib `NotSupportedException` 改为标准 `std::runtime_error`。
- 当前 SimpleC 非测试代码对 CoreLib 的直接引用只剩 compiler pipeline 的旧异常兼容捕获；CoreLib 自测入口仍故意保留 CoreLib 类型使用。
- 将 x86 `Function_x86::Code` 从 CoreLib `LinkedList<Instruction>` 改为 `std::list<Instruction>`，并把 x86 peephole 优化改为标准迭代器擦除。
- 将 IL `Function::Instructions` 和 `ControlFlowNode::Code` 从 CoreLib `LinkedList<Instruction>` 改为本地 `InstructionList` 过渡层；该层内部使用 `std::list`，保留稳定 `InstructionNode*` 语义以兼容 SSA/out-of-SSA 和优化器。
- 当前 SimpleC/IL 主链路已经没有 CoreLib `LinkedList` / `LinkedNode` 类型命中；剩余命中只在 `corelib_regression_tests.cpp` 中测试 CoreLib 自身。
- 将 IL `IntSet` / `BitIntSet` 迁移到 IL 本地标准库 backed 实现，interference analysis 不再依赖 CoreLib `Math` / `LibMath`。
- 将 IL `InvalidProgramException` 改为标准异常边界，并移除 optimizer 中旧 CoreLib `Exception.h` include。
- `IL.vcxproj` 已移除 CoreLib include 路径和 CoreLib project reference；当前 `IL` 目录没有 `CoreLib` 命中。
- `SimpleC/compiler_pipeline.cpp` 已移除 CoreLib 异常兼容 catch；SimpleC 非测试代码没有 `CoreLib::Basic` 直接引用。
- 新增独立 `CoreLibTests` 目标承载 CoreLib/IL 最小回归，`--corelib-self-test` 已从 `SimpleC` 主可执行文件拆出。
- `SimpleC.vcxproj` 已移除 CoreLib include directory 和 CoreLib project reference；当前 `SimpleC` 目标不再构建或链接 CoreLib。
- out-of-SSA parallel-copy 临时变量已从裸 `new Variable` 改为 `std::unique_ptr` 托管，保留现有 raw pointer IR 引用语义。
- CFG dominator visitor 的旧 `FakedList` 辅助容器已移除，改用 `std::span<ControlFlowNode*>` 表示临时边视图。
- `SimpleC` 公共头中的 broad `using namespace` 已收缩为精确类型引入，避免 Lexer/Parser/visitor/codegen 头继续向包含方泄漏 IL/Compiler 命名空间。
- `InstructionList` 仍保留稳定 `InstructionNode*` 语义，但旧全局 helper 调用点已全部迁到 `InstructionList` / `InstructionNode` 成员接口，并删除了兼容 helper 定义。

## 最近完成的 correctness 修复

这些修复有效，但有一部分已经偏向 IL/x86 后端，不应继续扩展成主线：

- 修复 SimpleC double lowering，使 double 参数、局部变量、声明和 64 位临时量在 IL 中携带 `DataType::Double`。
- 修复 SSA 变量重命名，使新版本变量保留原始 `DataType`。
- 防止 x86 寄存器分配器把非 4 字节值分配到通用寄存器。
- 增加一小段安全 x87 codegen：double `I2D`、`FAdd`、`FSub`、`FMul`、`FDiv`、浮点 move、浮点 return。
- `D2I` 仍故意保持 unsupported，避免 x87 默认 rounding 与 C 风格 truncation 语义不一致。

## 与原始目标的偏离

当前最高优先级已经调整为：尽快把 SimpleC/IL 编译链路里能替换的 CoreLib 依赖改成 STL/标准库，最终甩掉 CoreLib 作为编译器主链路依赖。

偏离点：

- 最近曾经从标准库替换主线偏到 IL/x86 double codegen correctness。
- 这批 x87 修复本身有价值，但不应继续扩展后端功能。
- 后续应暂停 double compare、`D2I`、更多 x86 codegen 功能，除非它们直接阻塞当前固定验证。

纠偏状态：

- 最新一批已经转向更高价值的 IL CoreLib 依赖拆除，完成 IR/x86 文本输出的 `std::wstring` 化、`Instruction::Operands` 的 `std::vector` 化、`ControlFlowGraph::Variables` / `Nodes` 的 `std::vector` 化、CFG 支配树列表和 CFG edge `Entries` 的 `std::vector` 化，并把 optimizer 的 CFG 所有权边界、CFG 节点所有权、IL 变量所有权和 out-of-SSA `PhiClasses` 改为 `std::shared_ptr`；本轮继续清掉 CFG 局部 `List<IntSet>`、SimpleC codegen 的 CoreLib unsupported 异常、SimpleC/IL 主链路中的 broad CoreLib namespace import、IL/x86 指令链表对 CoreLib `LinkedList` / `LinkedNode` 的依赖、IL 项目对 CoreLib 的直接引用，以及 `InstructionList` 旧全局 helper API。
- 后续目标应继续拆 SimpleC/IL 主链路中的 `RefPtr`、`List`、`LinkedList`、`String` 入口，而不是做低价值风格化清理或新增后端功能。

## 续接步骤

1. 打开 `C:\Users\mingy\Documents\New project\CCompiler`。
2. 构建 `SimpleC Debug|Win32`。
3. 运行 `Debug\CoreLibTests.exe --corelib-self-test`。
4. 对前端、IL、optimizer 或 pipeline 改动，额外运行 `Debug\SimpleC.exe SimpleC\in.txt`。
5. 每批通过后清理 `SimpleC/in.*` smoke 产物。

## 下一小目标

- 最高优先级：继续拆 SimpleC/IL 主链路 CoreLib 依赖；`--corelib-self-test` 已拆到独立 `CoreLibTests` 目标，下一步继续扫描 SimpleC 非测试源码中的残留接口边界。
- 避免机械 cast、visitor 小修和纯风格化改动，除非它们直接服务于 CoreLib 依赖移除。
- CoreLib 内部 `String`、`Stream`、`TextIO`、`LibIO` correctness 仍保留，但低于“主链路甩掉 CoreLib”。
- 暂停扩展 x86/codegen 新功能。
