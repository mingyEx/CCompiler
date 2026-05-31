# 重构进度看板

这个文件是短版进度表。详细状态见 [refactor_status.md](</C:/Users/mingy/Documents/New project/CCompiler/docs/refactor_status.md>)。

## 当前检查点

- 日期：2026-05-31
- 构建：`SimpleC Debug|Win32` 通过，`0 Warning(s), 0 Error(s)`
- 最小回归：`Debug\CoreLibTests.exe --corelib-self-test` 通过
- 主 smoke：`Debug\SimpleC.exe SimpleC\in.txt` 通过
- 当前阶段：阶段 1 后段，阶段 2 已开始
- 当前纠偏：最高优先级改为拆掉 SimpleC/IL 主链路中能替换的 CoreLib 依赖

## 阶段进度

| 阶段 | 状态 | 粗略进度 | 说明 |
| --- | --- | ---: | --- |
| 1. 稳定 CoreLib 内部 | 进行中 | 75% | `String`、`Stream`、`TextIO`、`LibIO`、容器和 `RefPtr` 已修过一批真实边界问题，并有自测覆盖。 |
| 2. 减少自定义容器和资源管理依赖 | 已开始 | 93% | `SimpleC` 前端 AST/lexer/parser/semantic/printer 已基本使用标准库字符串和智能指针，visitor 分派接口已改为引用；IL/x86 名称字段、IR/x86 文本输出已开始改为 `std::wstring`；`Instruction::Operands`、`ControlFlowGraph::Variables`、`ControlFlowGraph::Nodes`、CFG 支配树列表和 CFG edge `Entries` 已从 CoreLib `List` 改为 `std::vector`；CFG liveness / SSA phi placement 局部 `List<IntSet>` 已改为 `std::vector<IntSet>`；IL/x86 指令链表已脱离 CoreLib `LinkedList` / `LinkedNode`；`InstructionList` 旧全局 helper API 已移除；IL 本地 `IntSet` / `BitIntSet` 已替代 CoreLib 版本；IL 和 SimpleC 项目均已移除 CoreLib include 路径和 project reference。 |
| 3. 加强最小回归测试 | 进行中 | 60% | `--corelib-self-test` 已建立，但仍不是完整编译器测试体系。 |
| 4. 压缩兼容层暴露面 | 少量开始 | 30% | IL 已清掉 CoreLib `Exception/String/IntSet/BitIntSet/Math` 边界；CoreLib 自测已拆到独立 `CoreLibTests` 目标，SimpleC 主可执行文件不再绑定 CoreLib。 |
| 5. 公共接口替换 | 暂缓 | 0% | 应放在内部依赖和测试护栏更多之后。 |

## 已完成的主线工作桶

- 建立 `CoreLibTests --corelib-self-test` 最小回归入口，并从 SimpleC 主目标拆出。
- 将多个前端 AST 容器从项目自定义容器替换为 `std::vector`。
- 将 SimpleC 前端 AST 所有权从 `RefPtr` 替换为 `std::shared_ptr`。
- 将 SimpleC parser AST 节点创建替换为 `std::make_shared`。
- 将 SimpleC `SyntaxVisitor` 工厂所有权替换为 `std::unique_ptr`。
- 将 SimpleC `SyntaxNode::Accept` visitor 入口从 raw pointer 改为引用接口。
- 将 SimpleC `SyntaxVisitor::VisitXxx` node 参数从 raw pointer 改为引用接口。
- 将 SimpleC code generator 输出所有权替换为 `std::unique_ptr`。
- 删除旧的、未参与构建的 `ParserTest.cpp`。
- 将 SimpleC code generator 的局部变量字典替换为 `std::unordered_map`。
- 将 `IL/CompileError.h` 从 CoreLib `String/List/Exception` 改为 `std::wstring` / 标准异常边界。
- 将 SimpleC `Lexer` 从 CoreLib `String/StringBuilder` 迁移到 `std::wstring`。
- 将 SimpleC AST 文本字段和 `ExpressionType::ToString()` 从 CoreLib `String` 迁移到 `std::wstring`。
- 将 Parser、SemanticsVisitor、SyntaxPrinter 适配为标准库宽字符串；CodeGenerator 只在进入 IL 时转换为 CoreLib `String`。
- 将 IL `Function::Name`、`Variable::Name` 和 x86 `Function_x86::Name` 迁移为 `std::wstring`，保留 CoreLib `String` 兼容入口。
- 将 IL optimizer pipeline 子 pass 所有权从 `RefPtr` 迁移到 `std::unique_ptr`。
- 将 IL optimizer 工厂返回值从 raw pointer 迁移到 `std::unique_ptr`。
- 将 x86 code generator 工厂返回值从 raw pointer 迁移到 `std::unique_ptr`。
- 将 IL/x86 指令文本输出从 CoreLib `String/StringBuilder` 迁移到 `std::wstring`。
- 将 IL `Instruction::Operands` 从 CoreLib `List<Operand>` 迁移到 `std::vector<Operand>`，覆盖 CFG、optimizer、SSA 转换和 x86 codegen 调用点。
- 将 `ControlFlowGraph::Variables` 从 CoreLib `List<RefPtr<Variable>>` 迁移到 `std::vector<RefPtr<Variable>>`，覆盖 SSA、out-of-SSA、optimizer、register allocation 和 interference analysis 调用点。
- 将 `ControlFlowGraph::Nodes` 从 CoreLib `List<RefPtr<ControlFlowNode>>` 迁移到 `std::vector<RefPtr<ControlFlowNode>>`，覆盖 CFG 构建、支配树、liveness、SSA、dead-code/control-flow cleanup 和 register allocation 调用点。
- 将 `ControlFlowNode` 的支配树和支配边界列表迁移到 `std::vector<ControlFlowNode*>`。
- 将 `ControlFlowNode::Entries` CFG edge 列表迁移到 `std::vector<ControlFlowNode*>`。
- 将 optimizer 管线中的 CFG 所有权边界从 `RefPtr<ControlFlowGraph>` 迁移到 `std::shared_ptr<ControlFlowGraph>`。
- 将 CFG 节点所有权从 `RefPtr<ControlFlowNode>` 迁移到 `std::shared_ptr<ControlFlowNode>`。
- 将 out-of-SSA 内部 `PhiClasses` 从 `RefPtr<EnumerableIntSet>` 迁移到 `std::shared_ptr<EnumerableIntSet>`。
- 将 IL `Variable` 所有权从 `RefPtr<Variable>` 迁移到 `std::shared_ptr<Variable>`。
- 当前 `IL` 目录已经没有 `RefPtr<...>` 命中。
- 将 CFG liveness 和 SSA phi placement 的局部 `List<IntSet>` 工作集迁移到 `std::vector<IntSet>`。
- 清掉 SimpleC/IL 主链路中的 broad `using namespace CoreLib::Basic`。
- 将 SimpleC codegen 的 unsupported 错误从 CoreLib `NotSupportedException` 改为标准 `std::runtime_error`。
- `SimpleC/compiler_pipeline.cpp` 不再直接 include `Basic.h`。
- 将 x86 `Function_x86::Code` 迁移到 `std::list<Instruction>`。
- 将 IL `Function::Instructions` 和 `ControlFlowNode::Code` 迁移到本地 `InstructionList`，内部使用 `std::list`，主链路不再依赖 CoreLib `LinkedList` / `LinkedNode`。
- 将 IL `IntSet` / `BitIntSet` 迁移到 IL 本地标准库 backed 实现，并移除 IL 对 CoreLib 项目的直接引用。
- 将 IL 旧 CoreLib 异常边界改为 `std::runtime_error` / `std::logic_error`。
- 将 `ScopeDictionary` 的 key 存储从 CoreLib hash helper 迁移到标准 `std::unordered_map` 默认 hash/equality，并从 IL 公共头收缩为 optimizer 私有 helper。
- 清掉 IL 非测试路径中的 CoreLib `StringBuilder` 残留。
- 将 interference analysis 的 `LiveRange` 结果所有权从 CoreLib `RefPtr` 迁移到 `std::shared_ptr`。
- 将若干 IL 和 optimizer 内部容器替换为 `std::vector`、`std::unordered_map`、`std::unordered_set`。
- 将 x86 emitter 的字节缓冲、常量表、重定位表等局部存储替换为 `std::vector`。
- 将部分 CoreLib 内部缓冲替换为 `std::vector` 或 `std::string`。
- 收缩 `SimpleC` 公共头中的 broad namespace import，改为精确引用 `CompileError` 和实际需要的 IL 类型。
- 清掉 `InstructionList` 旧全局 helper API 的全部使用点，并删除兼容 helper 定义；IL pass 现在通过 `InstructionList` / `InstructionNode` 成员接口操作指令链表。

## 已完成的 correctness 工作桶

- 修复多处 `String`、`StringBuilder` 边界和语义问题。
- 修复 `FileStream`、`StreamReader`、`StreamWriter`、`Path`、`File` 的一批边界和编码行为。
- 修复 Unicode `StreamWriter` path 构造时 BOM 输出，使 `StreamReader(path)` 能识别非 ASCII UTF-16 文本。
- 修复 `Encoding::Ansi/Unicode::GetString` 负长度保护并加入自测。
- 修复多个 `List`、`LinkedList`、`Dictionary`、`HashSet`、`RefPtr` 自赋值或所有权问题。
- 修复 SimpleC 命令行入口对 CoreLib 异常的报告路径。
- 修复 CFG compactors 对 `Exits[1]` 的 stale edge 保留问题。
- 修复 IL 常量折叠中的 subtraction、modulo、floating divide、double 类型保留、D2I 常量折叠、compare folding 问题。
- 修复 x86 logical-not codegen。
- 修复 optimizer 简单赋值消除死循环。
- 修复 parser 括号表达式错误恢复中的赋值/比较笔误。
- 增加整数 unary negation codegen。
- 增加一小段 x87 double codegen。

## 与原始目标的偏离情况

原始目标：

- 先收拾 SimpleC 前端和 CoreLib。
- 尽量把能替换的局部自定义容器、智能指针、流和字符串实现替换成标准库。
- 保持构建和最小回归为绿。
- 不优先做大后端功能。

已经偏离的地方：

- 曾经从标准库替换主线偏到 IL/x86 double codegen。
- x87 double 修复虽然解决真实 correctness 问题，但不应继续扩展。
- 一些早期改动包含低价值机械清理倾向，例如 cast 风格替换，后续应避免。

已经纠偏的地方：

- 最新一批转向更高价值的主链路 CoreLib 依赖拆除，已完成 IL `Instruction::Operands`、`ControlFlowGraph::Variables`、`ControlFlowGraph::Nodes`、CFG 支配树列表、CFG edge `Entries`、CFG 局部 liveness/phi 工作集的 `std::vector` 化，IL/x86 指令链表的标准库链表迁移，optimizer CFG、CFG 节点、IL 变量和 out-of-SSA `PhiClasses` 所有权的 `std::shared_ptr` 化，IR/x86 文本输出的 `std::wstring` 化，IL 项目对 CoreLib 的直接引用拆除，以及 `InstructionList` 旧全局 helper API 移除。
- 后续不再优先做 visitor/cast/格式类小修，除非它们直接服务于 CoreLib 依赖移除。

## 当前焦点

- 继续优先压缩 SimpleC/IL 主链路 CoreLib 依赖，先拆 `List`、`LinkedList`、`RefPtr`、`String` 的真实接口边界。
- CoreLib 内部实现替换保留，但低于主链路甩掉 CoreLib。
- 只做能通过最小回归验证的小批次。

## 下一小目标

1. 继续扫描 SimpleC 非测试源码的自定义容器/资源管理残留，优先处理会阻塞 CoreLib 脱钩的接口边界。
2. 继续检查解决方案和项目文件，防止 SimpleC/IL 重新引入 CoreLib include 或 project reference。
3. 不做机械 cast、visitor 小修或 x86 后端功能扩展，除非它直接阻塞 CoreLib 依赖移除。
4. 每批保持 `SimpleC Debug|Win32`、`CoreLibTests --corelib-self-test`、`SimpleC/in.txt` smoke 通过。

## 暂缓事项

- 大范围替换 `CoreLib::Basic::String` 公共接口。
- 全面替换 `RefPtr`、`List`、`Dictionary`、`HashSet` 的公共接口。
- LLVM 风格 IR 重做。
- CFG 可视化重写。
- 仓库目录结构重组。
- 继续新增 x86 后端功能。
