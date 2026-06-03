# 重构进度看板

这个文件是短版进度表。详细状态见 [refactor_status.md](</C:/Users/mingy/Documents/New project/CCompiler/docs/refactor_status.md>)。

## 当前检查点

- 日期：2026-06-03
- 构建：`SimpleC Debug|Win32` 通过，`0 Warning(s), 0 Error(s)`
- 主链路护栏：`scripts\check_mainchain_no_corelib.ps1` 通过
- 主 smoke：`Debug\SimpleC.exe SimpleC\in.txt` 通过
- DevTools：`DevTools/X86_InstrCodeGen Debug|Win32` 通过，生成器运行通过
- 当前阶段：CoreLib 主链路替换完成，仓库内 CoreLib/CoreLibTests 已删除
- 当前焦点：补充生成器漂移检查和更系统的 SimpleC 前端回归入口，IL 深层结构收口暂缓

## 阶段进度

| 阶段 | 状态 | 粗略进度 | 说明 |
| --- | --- | ---: | --- |
| 1. 稳定旧 CoreLib 内部 | 已结束 | 100% | 旧库已经不再作为主线继续维护；`CoreLib/` 已删除。 |
| 2. 减少自定义容器和资源管理依赖 | 已完成主链路 | 100% | `SimpleC`、`IL`、`DevTools/X86_InstrCodeGen` 已脱离 CoreLib include/project reference。 |
| 3. 加强最小回归测试 | 进行中 | 75% | 主链路已有统一 smoke 入口、no-CoreLib 守卫、多样例 SimpleC smoke 和 x86 生成器漂移检查；仍缺少系统性 parser/semantic/backend regression。 |
| 4. 压缩兼容层暴露面 | 已完成主目标 | 100% | `CoreLib/` 和 `CoreLibTests/` 已从仓库删除。 |
| 5. 后续结构整理 | 未开始 | 0% | DevTools 生成器、IL/x86 边界、CFG 工具等后续分批处理。 |

## 已完成的主线工作桶

- 建立并启用 `scripts\check_mainchain_no_corelib.ps1` 主链路依赖护栏。
- 将 SimpleC 前端 AST 容器、字符串和所有权迁移到标准库类型。
- 将 SimpleC parser、semantic、printer、codegen 适配到标准库字符串和智能指针边界。
- 将 IL 主要指令、CFG、变量、节点、optimizer 所有权和若干局部容器从 CoreLib 类型迁移到标准库类型。
- 将 IL 本地 `IntSet` / `BitIntSet` 替代 CoreLib 版本。
- 将 IL/x86 文本输出从 CoreLib `String/StringBuilder` 迁移到 `std::wstring`。
- 将 `SimpleC.vcxproj`、`IL.vcxproj`、`DevTools/X86_InstrCodeGen` 从 CoreLib include/project reference 中拆出。
- 删除 `CoreLib/` 和 `CoreLibTests/`。
- `DevTools/X86_InstrCodeGen` 生成器已修到输出当前 `std::vector` API，不再生成旧 `code.Add(...)`。
- 新增 `scripts\check_smoke.ps1` 统一验证入口和 `scripts\check_simplec_smoke.ps1` 多样例 SimpleC smoke。

## 与原始目标的偏离情况

原始目标：

- 先收拾 SimpleC 前端和 CoreLib。
- 尽量把能替换的局部自定义容器、智能指针、流和字符串实现替换成标准库。
- 保持构建和最小回归为绿。
- 不优先做大后端功能。

已经纠偏：

- 主链路已完成 CoreLib 脱钩。
- CoreLib 不再继续完善，已删除。
- 后续不再优先做 visitor/cast/格式类小修，除非它直接服务于当前维护目标。

## 当前焦点

- 保持 `scripts\check_smoke.ps1` 为绿。
- 用生成器漂移检查避免误覆盖 `IL/Instruction_x86.cpp` 的手工差异。
- 暂不推进 IL 深层结构收口。

## 下一小目标

1. 继续增加更系统的 SimpleC 前端 parser/semantic regression 样例。
2. 运行并维护 `scripts\check_smoke.ps1`。
3. 继续记录 `DevTools/X86_InstrCodeGen` 与 `IL/Instruction_x86.cpp` 的手工差异，避免误把生成文件直接覆盖进主链路。
