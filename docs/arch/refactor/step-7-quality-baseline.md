# 第 7 步质量基线（2026-09-26）

本记录基于 `hikyuu-auto` 的 `poc` 分支，在 macOS arm64、AppleClang 17、
Xmake 3.0.8、Python 3.10.21 上执行。格式和静态检查工具为 Homebrew
LLVM 20.1.8。

## 格式和编译数据库

- `./op.sh fmt-check`：初始 968 个、删除失效 `demo2.cpp` 后原工作树
  967 个文件通过；rebase 远程 `poc` 的 Python 绑定目录重组后，当前
  902 个手写 C/C++ 文件通过。清单由 Git 跟踪且当前存在的文件生成，不包含
  生成代码、第三方包或 `*.h.in` 模板。
- `HAYAKU_PYTHON=/opt/homebrew/opt/python@3.10/bin/python3.10 xmake project
  -k compile_commands --lsp=clangd`：初始生成 623 条、删除失效示例后
  重新生成 622 条编译记录；rebase 后当前为 558 条。已核对
  `common/Lang.cpp`、`extensions/ingest/TdxKDataDriver.cpp`、
  `extensions/realtime/SpotAgent.cpp` 和两个可选 Python 扩展入口均在清单中。
- 以单个编译单元运行 `./op.sh tidy-strict <source.cpp>` 是新增文件的严格
  检查入口；已有告警尚未清零，不对全仓设置 `WarningsAsErrors: '*'`。

## `clang-tidy` 首轮抽样

复现命令：

```sh
./op.sh tidy hayaku_cpp/src/common/Lang.cpp \
  hayaku_cpp/src/extensions/ingest/TdxKDataDriver.cpp \
  hayaku_cpp/src/extensions/realtime/SpotAgent.cpp
```

| 编译单元 | 首轮项目告警 | 处理结论 |
|---|---:|---|
| `common/Lang.cpp` | 0 | 可直接作为严格检查样本 |
| `extensions/ingest/TdxKDataDriver.cpp` | 47 | 28 处旧式强制转换、14 处文件偏移由无符号数转为 `streamoff`、5 处相邻同类型参数；偏移来自文件大小限定的记录数，需单独核验极端文件边界；参数签名留待接口收敛审议 |
| `extensions/realtime/SpotAgent.cpp` | 5 | 以限定名替换全局命名空间导入、显式转换 NNG 接收缓冲区参数，并将两个按值接收的回调移入容器；析构期间 `stop()` 可能抛异常的诊断需结合线程生命周期单独审议 |

首轮共 52 条项目告警。上述 `SpotAgent.cpp` 的 4 处已修复；
`FuncWrapper.h` 中一处分析器提示的潜在泄漏改为 `std::make_unique`
管理构造期间的所有权。复跑 `./op.sh tidy SpotAgent.cpp` 对应的完整路径后，
该编译单元仅剩析构函数的 1 条 `bugprone-exception-escape` 告警。当前
`stop()` 会 join 线程及线程池，析构期间的异常处理需要与生命周期一起审议，
暂不以空的 catch 块掩盖未完成的线程清理。
更广的编译单元覆盖记录在进度文档中。
未对第三方头文件的海量诊断施加
项目告警门禁，也未自动执行 `clang-tidy --fix`。

扩展抽查编译数据库前 11 个编译单元时另发现两处编译错误：
`demo2.cpp` 使用现行代码中不存在的 `Strategy` 类型，因此连同唯一的
Xmake target 一起删除；`application/SystemInfo.cpp` 在 unity build 之外
缺少 `fmt/format.h` 和 `common/Log.h` 的直接包含，已补齐。
独立构建剩余示例时，`demo3` 缺少 `hayaku-realtime` 链接依赖，已补齐。
`application/IniParser.cpp` 三处默认值解析只用于验证，结果会被当前选项值
覆盖；已移除无用赋值，保留验证与异常行为。复查仅剩 2 条相邻参数
容易交换的 API 风格告警；`SystemInfo.cpp` 严格 tidy 已通过。
`demo1`、`demo3` 最终均可单独构建。`GlobalInitializer.cpp`
的空 catch 是明确的 `noexcept` 进程退出路径，`HayakuSession::close()`
和 `SpotAgent` 析构的异常逃逸告警需要单独审议线程停止顺序。

曾尝试并发扫描编译数据库中 608 个受跟踪编译单元；本机出现明显内存换页，
完成前 11 个后停止。全量告警统计不能从这份抽样外推，未将其设为全仓
质量门禁。

## ASan 和回归

`./op.sh asan-test` 在独立 `build/asan` 目录构建项目目标并在退出时恢复
原 Xmake 配置。macOS 关闭 LeakSanitizer；Linux 才启用泄漏检查。本机
复跑完整命令以 0 退出，`small-test` 为 51/3356，`unit-test` 为
805/208437，均无 ASan 诊断。已用 `otool -L` 验证两个测试程序、core、
插件 ABI 测试库、ingest 和 realtime 均链接 ASan 运行时。Linux 的
LeakSanitizer 和 Sphinx HTML 构建未在本机执行；普通构建恢复状态记入
`step-7-progress.md` 的执行记录。
