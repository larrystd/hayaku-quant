# Hayaku Quant

[English](README.en.md)

Hayaku 是面向行情数据研究、指标计算、策略组合与回测的 C++ 框架。C++ 核心负责数据访问、计算与策略执行；现有 Python 接口作为可选集成单独放在 `python/`。

本仓库源自 [Hikyuu](https://github.com/fasiondog/hikyuu)。默认 Bazel 构建在 macOS 和 Linux 上以 C++ 为主；保留的 Python 3.10 集成集中在 `python/`；此构建不支持 Windows。

## C++ 调用链

应用调用 `HayakuSession::open()` 创建会话，并从会话取得数据和执行账户。回测时，C++ 代码把信号、资金管理等组件组合成 `StrategyDefinition`，交给 `StrategyEngine` 执行：

~~~text
C++ 应用
  -> HayakuSession::open(SessionOptions, AccountConfig)
  -> DataEngine::getKData() -> 本地行情驱动 (HDF5 / SQLite / TDX)
  -> 指标 (CLOSE / MA / ...) -> 信号 + 资金管理 -> StrategyDefinition
  -> HayakuSession::bindStrategy() -> StrategyEngine::run(BacktestRequest)
  -> StrategyRuntime 逐根 K 线计算 -> ExecutionEngine / 账户执行与账本更新
  -> BacktestResult + ExecutionEngine::snapshot() / history()
~~~

`StrategyDefinition` 描述策略规则；`BacktestRequest` 提供本次回测的 K 线和重置选项。`StrategyEngine` 运行这些规则，返回交易结果快照。同一个策略定义可以使用不同的 K 线运行多次；调整规则参数时创建新的定义。`ExecutionEngine` 维护现金、持仓和成交记录。

实时路径使用 C++ 的 `Strategy` 和可选实时模块。`Strategy::start()` 经 `RealtimePort` 启动 `SpotAgent`，接收行情后更新证券数据并触发 `onChange()`、`onReceivedSpot()` 或定时任务；策略可以通过执行账户提交订单。实时行情和回测使用同一套 C++ 数据、指标与执行组件，但目前入口和运行流程不同。实时模块需要单独构建并接入行情服务。

## 从源码快速开始

需要 Bazelisk、CMake 和支持 C++20 的编译器。Bazelisk 使用 [`.bazelversion`](.bazelversion) 指定的版本；C++ 依赖固定在 [`MODULE.bazel`](MODULE.bazel) 及其锁文件中。

~~~bash
git clone https://github.com/larrystd/hayaku-quant.git
cd hayaku-quant
./op.sh build
./op.sh test
~~~

使用已配置的本地行情数据运行 C++ 回测。下面展示会话、数据、策略、执行账户的连接方式；`hayaku.ini` 及其中的数据路径需按本机环境配置：

~~~cpp
#include <iostream>

#include <hayaku.h>
#include <execution/pricing/TradeCosts.h>
#include <operators/SeriesOperators.h>
#include <operators/WindowOperators.h>
#include <strategy/decision/Signals.h>
#include <strategy/risk/MoneyManagers.h>

using namespace hayaku;

int main() {
  auto options = SessionOptions::fromIni("/path/to/hayaku.ini");
  auto account = AccountConfig(Datetime(199001010000LL), 100000.0,
                               TC_Zero(), "research");
  auto session = HayakuSession::open(options, account);
  session.waitReady();

  auto kdata = session.data().getKData("sh600000", KQuery(-100));
  auto definition = StrategyDefinition(
      MM_FixedCount(100), SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)));
  auto& strategy = session.bindStrategy(definition);
  auto result = strategy.run(BacktestRequest(kdata));
  auto funds = session.execution().snapshot().funds();
  std::cout << result.tradeCount() << " trades, cash " << funds.cash << '\n';
}
~~~

更完整的 C++ 调用示例在 [`hayaku_cpp/demo/`](hayaku_cpp/demo/)；策略回测与实时接口的实现分别位于 `hayaku_cpp/src/strategy/` 和 `hayaku_cpp/src/extensions/realtime/`。默认 Bazel 配置包含 HDF5、SQLite、TDX 和 TA-Lib；MySQL 和 Windows 不在此构建配置内。

## 可选的 Python 接口

保留的 Python 代码集中在 `python/`，调用链是 `python/hayaku` → `python/hayaku_pywrap`（pybind11）→ `hayaku_cpp/src`。当前开发和默认构建以 C++ 为主；需要 Python 接口时再使用 `./op.sh python-build` 和 `./op.sh python-test`。数据导入与实时 Python 扩展是独立原生包，详见 [Bazel 指南](BAZEL.md)。

## 构建与测试

~~~bash
./op.sh test          # C++ 测试
./op.sh build         # C++ 核心与扩展库
./op.sh all           # C++ 构建和测试
./op.sh ci            # CI 检查：构建所有 C++ 目标并运行测试
~~~

C++ 开发工具可运行 `./op.sh compdb`，从 Bazel 目标生成 `compile_commands.json`；运行 `./op.sh doctor` 可查看当前工具和路径。可选 Python 扩展的构建、测试与打包命令见 [Bazel 指南](BAZEL.md)。

## 文档

- [C++ 架构总览](docs/arch/new_architecture.md)和[API 清单](docs/arch/api-inventory.md)
- [Bazel 构建指南](BAZEL.md)和 [Python 示例](python/examples/)
- [第三方许可证](THIRD_PARTY_LICENSES.md)和[项目许可证](LICENSE)

Hayaku 是研究工具，不提供投资建议或内置证券交易服务。用户自行接入的外部交易通道及其使用由用户负责。
