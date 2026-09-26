<p align="center">
  <img src="docs/en/_static/00000-title.png" width="200" alt="Hayaku">
</p>

<p align="center">
  Market data research and backtesting in C++ and Python<br>
  <strong>Explicit sessions · Composable strategies · Bazel builds</strong>
</p>

# Hayaku Quant

[English] | [简体中文](readme.zh.md)

Hayaku is a C++ and Python framework for market data research, indicators, strategy composition, and backtesting. The C++ core performs data access and calculation; the Python API provides explicit sessions and domain-specific entry points.

This repository is an architecture refactor derived from [Hikyuu](https://github.com/fasiondog/hikyuu). It is a breaking proof of concept, so existing Hikyuu programs need adaptation. The current Bazel build targets **macOS and Linux with Python 3.10**. Windows is not supported by this build.

## What is in the repository?

- **Data and indicators:** query local market data and compose indicator calculations.
- **Strategy research:** define signal and money management components, run a backtest, and inspect its result.
- **Execution accounts:** submit simulated orders and inspect cash, positions, and trade records through a session-owned execution engine.
- **Optional modules:** historical data ingestion and real-time data services have separate native packages.

Importing `hayaku` defines the public types without opening a data source. A runtime starts when you call `open_session()`, and the context manager closes it when the block exits.

## Quick start from source

Install Bazelisk, CMake, a C++20 compiler, and Python 3.10. Bazelisk uses the version in [`.bazelversion`](.bazelversion); C++ dependencies are pinned in [`MODULE.bazel`](MODULE.bazel) and its lockfile.

~~~bash
git clone https://github.com/larrystd/hayaku-quant.git
cd hayaku-quant
python3.10 -m pip install -r requirements.txt
./op.sh build
./op.sh import-test
~~~

`./op.sh build` compiles the core and both optional native modules, then stages the six libraries in the source-tree Python packages. Run the following example from the repository root. It does **not** need market data:

~~~bash
python3.10 - <<'PY'
from hayaku.operators import MA, PRICELIST

prices = PRICELIST([1, 2, 3, 4, 5])
print(list(MA(prices, 3))[2:])  # [2.0, 3.0, 4.0]
PY
~~~

### Work with local market data

Prepare a compatible local data source and a `hayaku.ini` configuration first. By default, `open_session()` reads `~/.hayaku/hayaku.ini`; you can also pass a path explicitly. Opening a session does not download data.

~~~python
from hayaku import Query, open_session
from hayaku.execution import AccountConfig

account = AccountConfig(name="research", initial_cash=100_000)
with open_session(filename="/path/to/hayaku.ini", account_config=account) as session:
    session.wait_ready()
    bars = session.data.get_kdata("sh600000", Query(-100))
    snapshot = session.execution.snapshot()
    print(len(bars), snapshot.funds)
~~~

The optional ingestion API is under `hayaku.extensions.ingest`. To run a strategy, compose a `StrategyDefinition` from components, pass the requested bars in a `BacktestRequest`, and execute it with a `StrategyEngine` bound to the session account. See the [strategy guide](docs/en/strategy.rst) and the [order example](examples/python/execution_engine.py).

## Architecture

~~~text
Python script / notebook
        |
        v
hayaku/                    Session, data, operators, strategy, execution APIs
        |
        v
hayaku_pywrap/             pybind11 bindings
        |
        v
hayaku_cpp/src/            C++ data, indicator, strategy, and execution engines
        |
        v
local data drivers          HDF5, SQLite, TDX, and configured extensions

Optional: hayaku.extensions.ingest   -> hayaku_ingest_native
          hayaku.extensions.realtime -> hayaku_realtime_native
~~~

| Python entry point | Responsibility |
| --- | --- |
| `hayaku.data` | Securities, K-line (candlestick) data, and queries |
| `hayaku.operators` | Indicators and series transformations |
| `hayaku.strategy` | Component definitions and backtest engine |
| `hayaku.execution` | Orders, accounts, positions, and trade records |
| `hayaku.metrics` | Result conversion and analysis helpers |
| `hayaku.application` | Session, configuration, CLI, and interactive tools |
| `hayaku.extensions` | Explicit opt-ins for ingest, real-time services, visualization, and SPI |

The core wheel contains `hayaku` and its native library. Ingest and real-time services are packaged as separate wheels. The default Bazel configuration includes HDF5, SQLite, TDX, and TA-Lib; MySQL and Windows are outside this configuration. See the [Bazel guide](BAZEL.md) for targets, generated files, dependency pins, and build options.

## Build, test, and package

~~~bash
./op.sh ci            # submission gate: build and test C++ targets
./op.sh test          # C++ tests and native package smoke tests
./op.sh python-test   # Python regression suite
./op.sh all           # build, then both test suites
~~~

To produce the three Python 3.10 wheels:

~~~bash
python3.10 -m pip install wheel
./op.sh wheel
./op.sh wheel-ingest
./op.sh wheel-realtime
python3.10 bazel/check_wheels.py
~~~

Wheels appear in `dist/`. For C++ tooling, run `./op.sh compdb` to generate `compile_commands.json` from Bazel targets. Use `./op.sh doctor` to inspect selected tools and paths.

## Documentation and project status

- [Getting started](docs/en/quickstart.rst) and [developer guide](docs/en/developer.rst)
- [Bazel build guide](BAZEL.md) and [Python examples](examples/python/)
- [Third-party licenses](THIRD_PARTY_LICENSES.md) and [project license](LICENSE)

Hayaku is a research tool. It does not provide investment advice or an embedded securities trading service. Users are responsible for any external trading connection they add.
