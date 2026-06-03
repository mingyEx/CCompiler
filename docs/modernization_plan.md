# 现代化计划

## 总目标

把这个教学型、偏 Windows 的 C++ 编译器项目，逐步整理成更依赖标准库、更容易阅读和维护的工程。

目标不是推倒重写，而是在当前项目中原地推进，并保持每批可构建、可验证。

## 推荐阶段

1. 稳住编译流程和文档。

   已完成：编译 pipeline 已从入口函数拆出，架构和进度文档已建立。

2. 移除外围平台依赖。

   已完成一批：SimpleC 文件加载、dump 输出、IL/x86 文本输出和 DevTools 生成器输出已转向标准库路径与文件流。

3. 迁移历史基础设施类型。

   主链路已完成关键目标：`SimpleC`、`IL`、`DevTools/X86_InstrCodeGen` 不再依赖 CoreLib，`CoreLib/` 和 `CoreLibTests/` 已删除。

4. 增加最低限度回归测试。

   当前最低验证组合为：

   - `SimpleC Debug|Win32`
   - `scripts\check_mainchain_no_corelib.ps1`
   - `Debug\SimpleC.exe SimpleC\in.txt`
   - 涉及生成器时额外构建并运行 `DevTools/X86_InstrCodeGen`

5. 压缩兼容层暴露面。

   CoreLib 兼容层已经从主仓库删除。后续只需要避免新代码重新引入等价的自定义基础设施。

6. 再考虑更大结构调整。

   例如 LLVM 风格 IR、CFG 工具重做、跨平台化、命名和目录大调整，都应放在更后面。

## 当前优先级

当前应优先做：

- 检查 `DevTools/X86_InstrCodeGen` 与生成产物的关系，整理真正影响维护性的部分。
- 保持 no-CoreLib 主链路验证持续为绿。
- 对 SimpleC/IL/DevTools 的改动只做能明确降低耦合、提升可验证性的低风险小批次。

当前不应优先做：

- 继续完善或修补已删除的 CoreLib。
- 大范围机械 cast 替换。
- 纯格式化或低收益命名调整。
- 新增 x86 后端功能。
- IL 深层结构收口，除非它阻塞当前验证或 DevTools 整理。

## IR 方向

长期可以向“LLVM-like”靠拢，但当前不作为主线：

- 可以保留自定义 IR。
- 可以逐步改善 SSA/value 约定。
- 不应过早追求 LLVM 文本兼容或整体重写。

当前阶段真正该做的是保持主链路脱离 CoreLib，并把附属工具和生成器关系整理清楚。
