# 重构详细状态

这个文件记录当前原地重构的详细状态。

## 当前策略

重构必须保持渐进：

- 保留现有项目结构和构建目标。
- 当前最高优先级是拆掉 SimpleC/IL 主编译链路里能替换的 CoreLib 依赖。
- 优先替换主链路接口和内部实现中已经可验证的 `String`、`List`、`LinkedList`、`RefPtr` 使用点。
- 每批保持 `SimpleC Debug|Win32` 可构建。
- 每批尽量跑 `CoreLibTests --corelib-self-test` 和 `SimpleC/in.txt` smoke。
- 能用标准库表达清楚的局部容器、所有权和 I/O 实现，应逐步替换。

## 当前验证状态

- `SimpleC Debug|Win32` 当前构建通过。
- 最新构建结果：`0 Warning(s), 0 Error(s)`。
- `Debug\CoreLibTests.exe --corelib-self-test` 通过。
- `Debug\SimpleC.exe SimpleC\in.txt` 通过。

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
- CodeGenerator 现在可以把前端 `std::wstring` 名称直接交给 IL，名称字段不再以 CoreLib `String` 为主载体。
- CodeGenerator unsupported-codegen 错误已改用标准 `std::runtime_error`，不再依赖 CoreLib `NotSupportedException`。
- SimpleC 非测试代码已无 `CoreLib::Basic` 直接引用；CoreLib 最小回归已拆到独立 `CoreLibTests` 项目。
- compiler pipeline 顶层 x86 generator 和 optimizer 局部所有权已改为 `std::unique_ptr`。
- compiler pipeline 输出 `.code/.cfg/.asm` dump 路径已改为走 `std::filesystem::path` 重载。
- IL `Function::Name`、`Variable::Name` 已迁移为 `std::wstring`，旧 CoreLib `String` 构造和添加入口保留为兼容转发。
- 旧的、未参与构建的 `ParserTest.cpp` 已删除；它还使用 `SmartPointer` 和旧 `List::Add` API，会干扰后续判断。
- Parser、SemanticsVisitor、SyntaxPrinter、CodeGenerator 已适配新的 AST 容器和所有权模型。

当前残留：

- `SimpleC` 主目标已经移除 CoreLib include directory 和 project reference；后续重点转为防止新代码重新引入 CoreLib，并继续扫描非测试源码中的自定义容器/资源管理残留。

## IL 和分析工具状态

已完成：

- 若干局部 `Dictionary` / `HashSet` 使用已换成 `std::unordered_map` / `std::unordered_set`。
- optimizer pipeline 子 pass 存储已换成 `std::vector`。
- optimizer pipeline 子 pass 所有权已从 CoreLib `RefPtr<IntraProcOptimizer>` 改为 `std::unique_ptr`。
- optimizer 工厂返回值已从 raw `IntraProcOptimizer*` 改为 `std::unique_ptr<IntraProcOptimizer>`。
- scoped dictionary 已有标准库 backed 实现。
- 低层 set 操作引入了 `BitIntSet` 风格辅助结构。
- x86 binary emitter 字节缓冲已从 `CoreLib::List<unsigned char>` 换成 `std::vector<unsigned char>`。
- x86 emitter 重定位表、函数元数据表、浮点常量表等多处局部存储已换成 `std::vector`。
- 删除了无消费者的旧 x86 函数元数据和局部 link-point 表。
- `Program_x86` 生成函数列表已换成 `std::vector<Function_x86>`。
- x86 `Function_x86::Name` 已迁移为 `std::wstring`，和 IL 函数名保持标准库字符串边界。
- x86 code generator 工厂返回值已从 raw `X86CodeGenerator*` 改为 `std::unique_ptr<X86CodeGenerator>`。
- CFG post-order traversal 结果容器已换成 `std::vector<ControlFlowNode*>`。
- interference analysis live-range 结果存储已换成 `std::vector<std::shared_ptr<LiveRange>>`。
- `Intermediate::Program` 函数列表已换成 `std::vector<Function>`。
- `Intermediate::Function` 变量和参数存储已换成 `std::vector<RefPtr<Variable>>`。
- `ControlFlowGraph::VarDefs` 已换成 `std::vector<InstructionNode*>`。
- `Operand::ToString()`、`Instruction::ToString()` 和 x86 `Instruction::ToString()` 已从 CoreLib `String/StringBuilder` 改为 `std::wstring`。
- CFG dump 的 IR 文本缓冲已从 CoreLib `StringBuilder` 改为 `std::wstring`。
- `Intermediate::Instruction::Operands` 已从 CoreLib `List<Operand>` 改为 `std::vector<Operand>`，相关 CFG、optimizer、out-of-SSA、x86 codegen 调用点已同步更新。
- `ControlFlowGraph::Variables` 已从 CoreLib `List<RefPtr<Variable>>` 改为 `std::vector<RefPtr<Variable>>`，相关 SSA、out-of-SSA、optimizer、register allocation 和 interference analysis 调用点已同步更新。
- `ControlFlowGraph::Nodes` 已从 CoreLib `List<RefPtr<ControlFlowNode>>` 改为 `std::vector<RefPtr<ControlFlowNode>>`，相关 CFG 构建、支配树、liveness、SSA、dead-code/control-flow cleanup 和 register allocation 调用点已同步更新。
- `ControlFlowNode` 的支配树和支配边界列表已从 CoreLib `List<ControlFlowNode*>` 改为 `std::vector<ControlFlowNode*>`。
- `ControlFlowNode::Entries` CFG edge 列表已从 CoreLib `List<ControlFlowNode*>` 改为 `std::vector<ControlFlowNode*>`。
- optimizer 管线中的 CFG 所有权边界已从 CoreLib `RefPtr<ControlFlowGraph>` 改为 `std::shared_ptr<ControlFlowGraph>`。
- CFG 节点所有权已从 CoreLib `RefPtr<ControlFlowNode>` 改为 `std::shared_ptr<ControlFlowNode>`。
- out-of-SSA 内部 `PhiClasses` 已从 CoreLib `RefPtr<EnumerableIntSet>` 改为 `std::shared_ptr<EnumerableIntSet>`。
- IL `Variable` 所有权已从 CoreLib `RefPtr<Variable>` 改为 `std::shared_ptr<Variable>`。
- 当前 `IL` 目录已经没有 `RefPtr<...>` 命中。
- out-of-SSA parallel-copy 临时变量已从裸 `new Variable` 改为 `std::unique_ptr` 托管。
- CFG liveness 和 SSA phi placement 的局部 `List<IntSet>` 工作集已改为 `std::vector<IntSet>`。
- CFG dominator visitor 的旧 `FakedList` 辅助容器已改为 `std::span<ControlFlowNode*>`。
- SimpleC/IL 主链路中的 broad `using namespace CoreLib::Basic` 已清掉，剩余 CoreLib 类型通过精确 using 或显式限定暴露。
- x86 `Function_x86::Code` 已从 CoreLib `LinkedList<Instruction>` 改为 `std::list<Instruction>`。
- IL `Function::Instructions` 和 `ControlFlowNode::Code` 已从 CoreLib `LinkedList<Instruction>` 改为本地 `InstructionList` 过渡层；该层内部使用 `std::list`，保留稳定 `InstructionNode*` 以兼容现有 SSA/out-of-SSA 和优化器节点引用。
- 当前 SimpleC/IL 主链路已经没有 CoreLib `LinkedList` / `LinkedNode` 类型命中；剩余 `LinkedList` 命中只在 `corelib_regression_tests.cpp` 中测试 CoreLib 自身。
- IL `IntSet` / `BitIntSet` 已迁移为 IL 本地标准库 backed 实现，不再从 CoreLib 引入。
- interference analysis 已去掉 CoreLib `Math` / `LibMath` 依赖，改用标准库 min/max。
- `IntermediateCode.h` 的 `InvalidProgramException` 已改为 `std::runtime_error`，`InstructionList` / parameter ordering 错误改为标准异常，IL 旧 CoreLib `Exception/String` 边界已清掉。
- `IL.vcxproj` 已移除 CoreLib include 路径和 CoreLib project reference；当前 `IL` 目录没有 `CoreLib` 命中。
- `ScopeDictionary` 已从 CoreLib `Dictionary/GetHashCode` helper 解耦，当前表达式 key 使用 `std::wstring`，并已从 IL 公共头收缩为 optimizer 私有 helper。
- IL 非测试路径中的 CoreLib `StringBuilder` 已清掉，若干无调用的 CoreLib `String` 兼容重载已删除。
- interference analysis 的 `LiveRange` 结果已从 CoreLib `RefPtr` 改为 `std::shared_ptr`。

已修复：

- `Intermediate::Instruction`、`Variable`、`Function` move assignment 现在保留 CFG node、flags、location 和 stack-size 元数据。
- `ControlFlowNode` move assignment 现在保留 dominator、frontier 和 liveness 元数据。
- `ConstIndirectionRemoval` pointer-value 状态已简化，base pointer 默认初始化，copy/move 使用默认语义。
- CFG compaction 已修复，不再只检查 `Exits[0]` 而漏掉 `Exits[1]`。
- `UselessInstructionOptimizer` 简单赋值消除不再无限循环。
- IL 常量折叠修复了 `0 - x`、`x % 1`、`1 % x`、浮点除零检查、double 类型保留、constant `D2I` 等问题。
- IL comparison folding 修复了 raw `Compare` 未初始化结果和 integer self `<=` / `>=` 折叠。
- x86 codegen 对不支持的 IR operation 会显式失败，不再静默跳过。
- `X86CodeGenerator` 基类已补虚析构，避免通过基类指针释放派生实现时的析构风险。
- x86 codegen 已移除固定写入 `d:\programdump.asm` 的旧调试副作用。
- x86 logical-not codegen 已改为正确 `SETZ` 到 8 位目标。
- integer unary negation codegen 已补。
- integer bit-not 现在通过临时寄存器写回结果，不再直接修改源操作数。
- SimpleC/SSA double 类型传播已修复。
- x86 register allocator 不再把非 4 字节值分配到通用寄存器。
- 增加一小段安全 x87 double codegen。

注意：

- x87 double codegen 属于真实 correctness 修复，但已经接近后端扩展。后续不应继续沿这条线扩展，除非阻塞固定验证。

## CoreLib 状态

已完成的现代化：

- `TextIO` buffer 部分改用 `std::vector<char>`。
- `IntSet` 存储改用 `std::vector<int>`。
- `String` multibyte cache 改用 `std::string`。
- `StringBuilder` 改用 `std::vector<wchar_t>`。
- `BinaryReader::ReadString()` 不再依赖局部 `new[]` / `delete[]`。
- `TextIO` 编码转换临时宽字符缓冲改用标准容器。
- 多处 `_Move` / 手写 rvalue cast 已替换为 `std::move`。
- 多处 `NULL` 和裸 0 指针逐步替换为 `nullptr`。
- 多处 C 字符串函数调用改成显式 `std::` 版本。
- 部分 `memcpy` 依赖改为显式 `<cstring>` 和 `std::memcpy`。
- `List` 的 memcpy 快路径限制到 `std::is_trivially_copyable`。

已修复的 correctness：

- `String` 自己 buffer 或内部指针赋值时不再提前破坏源文本。
- `String::IndexOf(wchar_t, int)` 失败路径的未定义行为已修复。
- 多个空字符串、非法索引、非法区间、trim、case conversion 边界已修复。
- `StringBuilder` 初始容量和 release 下 invalid `Remove()` 行为已修复。
- `StreamReader::Read(wchar_t*, count)` 现在保留换行，不再像 `ReadLine()`。
- `StreamReader` 对显式 Unicode encoding 的无 BOM 文本处理已修复。
- `StreamReader::Peak()` Unicode 状态保持已修复。
- `StreamWriter(path, Unicode)` 现在写 UTF-16LE BOM。
- `StreamWriter` null encoding 现在 fallback 到 Unicode，不再首写崩溃。
- `File::ReadAllText()` 共享 `StreamReader` BOM/encoding 行为。
- `FileStream` 空路径和读写权限检查增强。
- `Path` 空输入和 extension replacement 行为增强。
- `BinaryReader` fixed-size read 改为 exact-read 语义，短流会失败而不是返回半初始化值。
- `BinaryReader::ReadString()` 拒绝无法满足读取契约的异常长度前缀。
- `List`、`LinkedList`、`Dictionary`、`HashSet` 多个 copy/move self-assignment 问题已修复。
- `LinkedList::Count()` 已可在 const 对象上调用。
- `RefPtr` cross-type assignment、raw self-assignment、manual detach 行为已修复。
- `HashSet` copy assignment 和 const iteration 相关问题已修复。
- `Dictionary` hash helper 和 iterator const 行为已改进。
- `KeyValuePair` move self-assignment 和 copy assignment 问题已修复。

仍需继续：

- 继续压缩 `String`、`RefPtr`、`List`、`Dictionary`、`HashSet` 的公共暴露面。
- 继续替换 `Stream` / `TextIO` / `LibIO` 内部可安全替换成标准库的实现。
- 避免只做大范围风格化 cast 替换。

## 最小回归覆盖

`CoreLibTests --corelib-self-test` 已覆盖：

- 字符串非法索引、非法范围、trim、self-buffer assignment。
- `StringBuilder` 容量和 safe remove。
- path helper 空输入和扩展名替换。
- read-only / write-only stream misuse。
- `StreamReader` 负长度、UTF-16 BOM、Unicode `Peak()`、显式 Unicode 无 BOM 文本。
- `File::ReadAllText()` UTF-16 BOM 行为。
- `StreamWriter` Unicode BOM round-trip。
- `List`、`LinkedList`、`Dictionary`、`HashSet`、`KeyValuePair` 自赋值和 const iteration。
- `RefPtr` cross-type assignment、raw self-assignment、detach。
- `BinaryReader` 短流和异常字符串长度。
- IL move assignment 元数据保留。
- CFG node move assignment 元数据保留。
- IL operation folding 和 comparison folding。

不足：

- 这还不是完整编译器测试体系。
- 缺少系统性的前端 parser/semantic regression。
- 缺少稳定的后端行为测试。

## 与原始目标的偏离总结

原始目标的优先级：

1. 先稳定并现代化内部实现。
2. 优先缩减自定义容器和手写资源管理。
3. 增加最小回归测试。
4. 压缩兼容层暴露面。
5. 最后再考虑大接口替换或架构升级。

偏离：

- 有一段时间追进了 IL/x86 double codegen。
- 那批修复本身不是无效工作，但它不是“先收拾前端和 CoreLib 标准库替换”的主线。
- 后续不能继续在 x86 后端上扩展新功能。

纠偏：

- 最新一批已经切到主链路 CoreLib 依赖拆除，完成 IL/x86 文本输出的标准库字符串迁移，以及 `Instruction::Operands`、`ControlFlowGraph::Variables`、`ControlFlowGraph::Nodes`、CFG 支配树列表、CFG edge `Entries`、CFG 局部 liveness/phi 工作集的标准容器迁移，IL/x86 指令链表的标准库链表迁移，optimizer CFG / CFG 节点 / IL 变量 / out-of-SSA `PhiClasses` 所有权的 `std::shared_ptr` 迁移，并完成 IL 本身对 CoreLib include/project reference 的拆除。
- 下一步应继续拆 SimpleC/IL 的 `List`、`LinkedList`、`RefPtr`、`String` 边界，而不是继续后端功能或低价值风格化清理。

## 当前下一步建议

短期：

- 继续扫描 SimpleC 非测试源码中的自定义容器/资源管理残留，优先处理真正影响 CoreLib 脱钩的接口边界。
- 检查解决方案和项目文件，防止 SimpleC/IL 重新引入 CoreLib include 或 project reference。
- 每批继续执行固定验证。

暂不做：

- 更多 x86 codegen 功能。
- LLVM-style IR 重做。
- 大规模 public API 破坏。
- 大范围机械 cast 清理。
