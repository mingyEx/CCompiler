# 当前检查点

这个文件记录当前可以安全续接的状态。

## 时间

- 保存日期：2026-06-03

## 最新状态

- `CoreLib/` 已删除。
- `CoreLibTests/` 已删除。
- `SimpleC`、`IL`、`DevTools/X86_InstrCodeGen` 主链路已经不再依赖 CoreLib include/project reference。
- `scripts\check_mainchain_no_corelib.ps1` 仍保留，用于防止主链路重新出现 CoreLib、RefPtr、SmartPointer、LinkedList 等旧基础设施依赖。

## 最新已验证项

删除 CoreLib 后已经重新执行并通过：

1. `SimpleC Debug|Win32`，`0 Warning(s), 0 Error(s)`。
2. `scripts\check_mainchain_no_corelib.ps1`。
3. `Debug\SimpleC.exe SimpleC\in.txt`。
4. `DevTools/X86_InstrCodeGen Debug|Win32`，`0 Warning(s), 0 Error(s)`。
5. `DevTools/X86_InstrCodeGen` 生成器运行。

## 最近完成的主线工作

- SimpleC 前端 AST 容器、字符串和所有权已迁移到标准库类型。
- SimpleC parser、semantic、printer、codegen 已适配标准库字符串和智能指针边界。
- IL 主要指令、CFG、变量、节点、optimizer 所有权和若干局部容器已从 CoreLib 类型迁移到标准库类型。
- IL 本地 `IntSet` / `BitIntSet` 已替代 CoreLib 版本。
- IL/x86 文本输出已从 CoreLib `String/StringBuilder` 迁移到 `std::wstring`。
- `SimpleC.vcxproj`、`IL.vcxproj`、`DevTools/X86_InstrCodeGen` 已移除 CoreLib include/project reference。
- `CoreLib/` 和 `CoreLibTests/` 已从仓库删除。

## 与原始目标的关系

原始目标是尽快把能替换的自定义基础设施改成 STL/标准库，并最终甩掉 CoreLib。

当前结论：

- 主链路甩掉 CoreLib 的目标已经完成。
- 旧的 `--corelib-self-test` 入口随 CoreLib 删除一起退役。
- 后续验证以 `SimpleC Debug|Win32`、no-CoreLib 守卫、SimpleC smoke 和必要的 DevTools 构建/运行检查为准。

## 续接步骤

1. 打开 `C:\Users\mingy\Documents\New project\CCompiler`。
2. 每批继续执行 `SimpleC Debug|Win32`、no-CoreLib 守卫和 `SimpleC/in.txt` smoke。
3. 涉及生成器时额外执行 `scripts\check_x86_generator_drift.ps1`。
4. 下一块优先补 SimpleC 前端 parser/semantic regression。

## 下一小目标

- 优先处理 `DevTools/X86_InstrCodeGen` 中真正影响维护性或误用风险的低风险问题。
- 暂停 IL 深层结构收口。
- 不再完善或修复已删除的 CoreLib。
