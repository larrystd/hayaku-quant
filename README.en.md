# Hayaku Quant

[简体中文](README.md)

Hayaku is a C++ framework for market data research, indicator calculation, strategy composition, and backtesting. The C++ core handles data access, calculations, and strategy execution. The existing optional Python integration lives under `python/`.

This repository originated from [Hikyuu](https://github.com/fasiondog/hikyuu). The default Bazel build focuses on C++ on macOS and Linux. The retained Python integration targets Python 3.10. This build does not support Windows.

## C++ call chain

An application opens a `HayakuSession`, then obtains data and an execution account from it. For a backtest, C++ code combines a signal, money management, and optional components into a `StrategyDefinition` and passes it to `StrategyEngine`:

~~~text
C++ application
  -> HayakuSession::open(SessionOptions, AccountConfig)
  -> DataEngine::getKData() -> local market data driver (HDF5 / SQLite / TDX)
  -> indicators (CLOSE / MA / ...) -> signal + money management -> StrategyDefinition
  -> HayakuSession::bindStrategy() -> StrategyEngine::run(BacktestRequest)
  -> StrategyRuntime processes each bar -> ExecutionEngine / account execution
  -> BacktestResult + ExecutionEngine::snapshot() / history()
~~~

`StrategyDefinition` describes the trading rules. `BacktestRequest` supplies the bars and reset options for one run. `StrategyEngine` applies those rules and returns a snapshot of the trades. The same definition can run on different bar data; construct a new definition to change rule parameters. `ExecutionEngine` maintains cash, positions, and trade history.

The live path uses the C++ `Strategy` class and the optional realtime module. `Strategy::start()` starts a `SpotAgent` through `RealtimePort`. Incoming quotes update stock data and trigger `onChange()`, `onReceivedSpot()`, or scheduled tasks. A strategy can submit orders through its execution account. The live and backtest paths use the C++ data, indicator, and execution components, while their entry points and run loops currently differ. The realtime module requires a separate build and a configured quote service.

## Quick start from source

Install Bazelisk, CMake, and a C++20 compiler. Bazelisk reads the version in [`.bazelversion`](.bazelversion); C++ dependencies are pinned in [`MODULE.bazel`](MODULE.bazel) and its lockfile.

~~~bash
git clone https://github.com/larrystd/hayaku-quant.git
cd hayaku-quant
./op.sh build
./op.sh test
~~~

The following C++ example shows how to connect a session, local market data, a strategy, and an execution account. Configure `hayaku.ini` and its data paths for your machine before running it:

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

See [`hayaku_cpp/demo/`](hayaku_cpp/demo/) for more C++ examples. The backtest and live implementations are under `hayaku_cpp/src/strategy/` and `hayaku_cpp/src/extensions/realtime/`. The default Bazel configuration includes HDF5, SQLite, TDX, and TA-Lib. MySQL and Windows are outside this build configuration.

## Optional Python integration

The retained Python code is under `python/`. Its call chain is `python/hayaku` → `python/hayaku_pywrap` (pybind11) → `hayaku_cpp/src`. Development and the default build focus on C++; use `./op.sh python-build` and `./op.sh python-test` when you need Python. Data ingestion and realtime Python extensions are separate native packages. See the [Bazel guide](BAZEL.md).

## Build and test

~~~bash
./op.sh test          # C++ tests
./op.sh build         # C++ core and extension libraries
./op.sh all           # C++ build and tests
./op.sh ci            # CI gate: build all C++ targets and run tests
~~~

Run `./op.sh compdb` to generate `compile_commands.json` from Bazel targets, or `./op.sh doctor` to inspect the local toolchain and paths. See the [Bazel guide](BAZEL.md) for optional Python build, test, and packaging commands.

## Documentation

- [C++ architecture](docs/arch/new_architecture.md) and [API inventory](docs/arch/api-inventory.md)
- [Bazel build guide](BAZEL.md) and [Python examples](python/examples/)
- [Third-party licenses](THIRD_PARTY_LICENSES.md) and [project license](LICENSE)

Hayaku is a research tool. It does not provide investment advice or a built-in brokerage service. Users are responsible for any external trading connections they configure.
