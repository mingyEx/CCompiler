# 现代化计划

## 总目标

把这个教学型、偏 Windows 的 C++ 编译器项目，逐步整理成更依赖标准库、更容易阅读和维护的工程。

目标不是推倒重写，也不是短期内重做全部架构，而是在当前项目中原地推进。

## 推荐阶段

1. 稳住编译流程和文档。

   当前已经完成一部分：编译 pipeline 已从入口函数拆出，架构和进度文档已建立。

2. 移除外围平台依赖。

   目标是把文件读取、路径处理、基础输出等外围代码优先转向标准库。当前 `SimpleC` 文件加载、部分 `CoreLib::IO::File/Path/FileStream` 已进入这个方向。

3. 迁移历史基础设施类型。

   这是当前最重要的主线。重点是逐步替换或压缩：

   - `String`
   - `List`
   - `LinkedList`
   - `Dictionary`
   - `HashSet`
   - `RefPtr`
   - 自定义 stream/text I/O 包装

   推荐顺序是先替换内部实现和局部所有权，再考虑公共接口替换。

4. 增加最低限度回归测试。

   当前已经有 `SimpleC --corelib-self-test`。它不是完整测试体系，但能覆盖 CoreLib/String/Stream/TextIO/IL 若干已修复问题。

5. 压缩兼容层暴露面。

   当内部依赖减少后，再逐步减少外部直接接触 `CoreLib` 自定义容器和指针的地方。

6. 再考虑更大结构调整。

   例如 LLVM 风格 IR、CFG 工具重做、跨平台化、命名和目录大调整，都应放在更后面。

## 当前优先级

当前应优先做：

- `SimpleC` 前端中还残留的 `RefPtr`、raw `new`、旧容器依赖。
- `CoreLib` 内部可安全替换成标准库的实现。
- `String`、`Stream`、`TextIO`、`LibIO` 的真实 correctness 问题。
- 每批都保持 `SimpleC Debug|Win32`、`--corelib-self-test` 和主 smoke 通过。

当前不应优先做：

- 继续扩展 x86 后端功能。
- 大范围机械 cast 替换。
- 纯格式化或低收益命名调整。
- 公共接口一次性大破坏。

## IR 方向

长期可以向“LLVM-like”靠拢，但当前不应该作为主线：

- 可以保留自定义 IR。
- 可以逐步改善 SSA/value 约定。
- 不应过早追求 LLVM 文本兼容或整体重写。

当前阶段真正该做的是先减少自定义基础设施依赖和手写资源管理。
