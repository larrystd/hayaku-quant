# AGENTS.md — Hayaku Project AI Development Guide

> This file provides the core context needed to work in the hayaku repository for AI coding agents (and newly joined developers):
> the project structure, the build/test commands, the code conventions and the common caveats. **Read this file first, before changing any code.**

> **Refactor note (Step 5, 2026-09-26):** the C++ source and test roots are
> `hayaku_cpp/src` and `hayaku_cpp/test`. The physical layout uses the eight business modules
> defined in `docs/arch/refactor/step-5-progress.md`; that document supersedes the historical
> `crt/imp/internal` organization described by older code.
> **Refactor note (Step 6, 2026-09-26):** Python bindings and packages follow
> `docs/arch/refactor/step-6-progress.md`; old `hayaku.fetcher`, `hayaku.util`,
> `hayaku.flat`, `hayaku.extend`, and `hayaku.gui.data` paths are removed.
> **Refactor note (Step 6B, 2026-09-26):** Python tests and examples moved outside the
> installable package to `tests/python` and `examples/python`.
> **Refactor note (Step 6C, 2026-09-26):** the installable Python package follows the eight
> C++ domains: `common`, `data`, `operators`, `execution`, `metrics`, `strategy`,
> `application`, and `extensions`. Optional ingestion, realtime, visualization, and
> SPI modules live under `hayaku.extensions`; GUI, CLI, and session entry points live
> under `hayaku.application`. The former `hayaku.indicator`, `hayaku.analysis`,
> `hayaku.apps`, `hayaku.session`, `hayaku.ingest`, `hayaku.realtime`,
> `hayaku.visualization`, and `hayaku.spi` paths are removed.

## 1. Project Overview

- **Hayaku** is an open-source ultra-high-speed quantitative trading research framework based on **C++/Python**, focusing on the strategy analysis, the backtesting and the live trading capability extensions (deeply adapted to the domestic China A-share market).
- The core capabilities: the trading model development, the ultra-fast computing engine, the efficient backtesting system and the live trading extensions.
- The project composition: the **high-performance C++ core library** (`hayaku_cpp`) + the **pybind11 binding layer** (`hayaku_pywrap`) + the **Python interface layer** (the `hayaku` package) + the **interactive exploration tools** (`hayaku.application.interactive`).
- License: Apache License 2.0; the default branch is `master`, plus the `release`, `bugfix` and `feature/*` branches.
- Project documentation source: `docs/` (Sphinx, mainly in Chinese); upstream history remains at [hikyuu.readthedocs.io](https://hikyuu.readthedocs.io/zh-cn/latest/index.html).

## 2. Repository Structure

```
hayaku/
├── xmake.lua                 # The top-level build script (an xmake project, defining the global options/dependencies)
├── copy_dependents.lua       # The task that copies the third-party dependency headers/libraries
├── requirements.txt          # The Python-side dependencies
├── setup.py / sub_setup.py   # The Python package installation scripts
├── hayaku_cpp/               # The C++ core engine library
│   ├── src/                  # C++ core: common/data/operators/execution/metrics/strategy/application/extensions
│   │   └── xmake.lua         # The core and optional native target definitions
│   ├── test/                 # C++ tests (doctest), targets: unit-test / small-test / real-test
│   └── demo/                 # The C++ demos
├── hayaku_pywrap/            # The pybind11 bindings (the target "core" → core.so / core.pyd)
│   ├── main.cpp              # The binding registration entry
│   ├── common/ data/ operators/ execution/ metrics/ strategy/ application/ extensions/
│   ├── Bindings.h            # Domain registration declarations
│   └── xmake.lua
├── hayaku/                   # The Python interface package
│   ├── __init__.py           # The package entry: loading the compiled core.so and the dependency libraries
│   ├── core.py               # Native core imports; opt-in enhancements live in domain _extensions.py files
│   ├── common/ data/ operators/ execution/ metrics/ strategy/
│   ├── application/          # Sessions, interactive tools, GUI, CLI, and configuration
│   ├── extensions/           # Optional ingest, realtime, visualization, and SPI modules
│   ├── _support/             # Internal Python helpers
│   ├── plugin/               # The runtime plugins (the data import, the market data, etc.)
│   ├── cpp/                  # The compiled artifacts directory: core310~core313.so, lib*.dylib, etc. (gitignored)
├── tests/python/             # The Python tests (test.py is the entry)
├── examples/python/          # The examples and notebook tutorials
├── docs/                     # The Sphinx documentation (dual-source: docs/zh Chinese + docs/en English; docs/make.sh builds)
├── test_data/                # The C++ test data (copied automatically when running the tests)
├── i18n/                     # The internationalization/language files
├── docker/                   # The containerization configuration
└── .github/workflows/        # The CI: ubuntu.yml / windows.yml / macosx.yml
```

## 3. Build System (xmake)

- Build tool: **xmake** (the top-level `set_xmakever("3.0.0")`, and the CI uses 3.0.8). The C++ standard is **C++20**; Windows uses clang-cl.
- All the third-party dependencies are pulled through the xmake package management (`add_requires`): boost, hdf5, mysql, fmt, spdlog, sqlite3, flatbuffers, nng, nlohmann_json, eigen, xxhash, utf8proc, ta-lib, mimalloc, pybind11, doctest, etc.; the external repository is `hayaku-extern-libs` (github/gitee).
- The key configuration items (the `xmake f` options): `mysql`, `hdf5`, `sqlite`, `tdx`, `ta_lib`, `low_precision`, `omp`, `serialize`, `leak_check`, `stacktrace`, `log_level`, `async_log`, `feedback`, `spend_time`, etc.
- The artifacts are output to `build/{mode}/{plat}/{arch}/lib`; the `core.so` and the dependency libraries needed by the Python package at runtime must be copied to `hayaku/cpp/` (see the workflow below).

### Common Commands

```bash
# Configure (the first time, or after changing the dependencies/options)
xmake f -k shared -y -vD

# Build the C++ core library
xmake -b core

# Build and run the C++ unit tests (doctest; small-test does not depend on the real data)
xmake r small-test

# Run the full unit tests (covering the strategy, indicator, data, and execution modules)
xmake r unit-test

# The real data test (requiring HAYAKU_USE_REAL_DATA_TEST and the real market data; usually run only in the CI or locally with the data)
xmake r real-test

# The debug/coverage mode
xmake f -m debug -y          # debug
xmake f -m coverage -y       # coverage (generating the lcov/genhtml reports)
```

> Note: when running the `xmake r` series tests, the build system automatically copies `test_data`, `hayaku/plugin` and `i18n` to the directory of the executable (see the `prepare_run` in `hayaku_cpp/test/xmake.lua`).

### IDE / LSP Indexing (clangd)

The sources under `hayaku_cpp/src/` use module-root includes such as `#include "data/KData.h"`, with `hayaku_cpp/src` as the include root. Tests and `hayaku_pywrap/` use the same include root through their target configuration. Therefore, **clangd must get the compilation database**, otherwise it will degrade to fallback arguments and report false missing-header/type errors; do not try to "fix" them by changing source code.

```bash
# Generate compile_commands.json in the project root (the locations discovered natively by clangd)
xmake project -k compile_commands --lsp=clangd
```

- Re-run it after adding/deleting the source files or changing the `xmake f` options (the dependencies/switches); the generated artifacts are gitignored (`.vscode` and `.clangd` are both in the ignore list), do not commit them.
- Do not use the `-I.` of `.clangd` to replace the compilation database: the relative paths are resolved against the compilation directory, and for the fallback commands they point to the directory of the source file rather than the include root — **verified to be ineffective**.
- If you still want to put the database in `.vscode/` or another subdirectory, you can use `clangd.arguments: --compile-commands-dir=<dir>` to specify the directory.
- Manually editing `hayaku_cpp/src/config.h` and `version.h` has no lasting effect (both are generated at build time by `add_configfiles` from `config.h.in`/`version.h.in`, and are gitignored); the recognition of `HAYAKU_*` conditional compilation macros by clangd also depends on the `-D` definitions carried in the compilation database, so regenerate `compile_commands.json` after changing them.

## 4. Testing

### Python Tests (tests/python/)

```bash
export PYTHONPATH=.
python3 tests/python/test.py     # the entry used by the CI
```

- The independent test files of each module: `Indicator.py`, `KData.py`, `Signal.py`, `MoneyManager.py`, `Stoploss.py`, `AllocateFunds.py`, `Datetime.py`, `Parameter.py`, etc., which can be run individually (e.g. `python3 tests/python/Indicator.py`).
- The new Python features should add the corresponding tests under `tests/python/`.

### C++ Tests (hayaku_cpp/test/)

Based on **doctest**, the directory structure corresponds one-to-one with the core library modules. The test project organization must follow the principles below:

#### The Organization Principles

1. **Physical isolation with a parallel structure**: the test project and source project are physically isolated, using `hayaku_cpp/test/…` against `hayaku_cpp/src/…`, with tests mirroring business modules rather than implementation-detail directories.
2. **One module, one suite**: for one module (usually one class), establish a test suite, named `test_<module>_suite`, e.g. `test_iniparser_suite`, all in lowercase. Declare the suite at the top of the file with `@defgroup` / `@ingroup`, see the existing files such as `test_iniparser.cpp` and `test_Stock.cpp`.
3. **One suite, one file**: each test suite uses an independent test file, named `test_<module>.cpp`, e.g. `test_iniparser.cpp` and `test_Stock.cpp`.
4. **One function/method, one case**: for each function or class member method, establish an independent test case (`TEST_CASE`), named `test_<function_name>` or `test_<class_name>_<method_name>`. When the names collide, you can add `_case` or another identifier after them to distinguish.
5. **Cover the public interfaces as much as possible**: the public interfaces should have tests added as much as possible; the scenarios that must be mocked to simulate may be skipped.
6. **Mark the test points with @arg**: within each test case, use the `/** @arg … */` comments to clearly mark each test point, for the code review and the quick locating. Example:

```cpp
TEST_CASE("test_IniParser_hasSection") {
    IniParser ini_parser;
    // …prepare the data…
    /** @arg the specified section exists */
    CHECK_UNARY(ini_parser.hasSection("test1"));
    /** @arg the specified section does not exist */
    CHECK_UNARY(!ini_parser.hasSection("test2"));
}
```

7. **The boundary conditions must be covered**: in the tests of each function/method, the boundary conditions must be covered, paying special attention to:

   - **The loop boundaries**: 0, 1, exactly N, N-1, N+1 iterations (e.g. an empty container, a single element, multiple elements).
   - **The extreme value boundaries**: the minimum/maximum, an empty string, an empty range, `Null<T>()`, an out-of-bounds index, zero, a negative value (if allowed).
   - **The branch boundaries**: each branch of `if/else` and `switch`, both sides of the ternary expressions, and the paths of the early `return` / `break` / `continue`.
   - **The error/exception paths**: the invalid inputs, the missing files, the malformed formats, etc., which should trigger the exceptions.
8. **The coverage requirements**: overall, aim for the branch coverage, with the line coverage as the minimum requirement. **The code paths that must be mocked to simulate are exempted** (e.g. the branches that can only be triggered by the external dependencies such as the network, the database and the live trading connections). `xmake f -m coverage -y` can generate the lcov coverage report for a self-check.

#### The Run Targets

- `unit-test`: the complete unit test set covering most modules.
- `small-test`: the minimal regression set, used by the CI by default.
- `real-test`: requires the real market data, used together with `HAYAKU_USE_REAL_DATA_TEST`.

## 5. Code Conventions

| Language | Convention                                   | Key points                                                                                                                       |
| -------- | -------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| C++      | `.clang-format` (Google style, LLVM 20) | use the pinned formatter for indentation, line length and include order; warning switches such as `-Wno-sign-compare` are in xmake.lua |
| Python   | `hayaku/.style.yapf` (yapf) + `.flake8`  | a 4-space indent, a column width of 120 (flake8`max-line-length=120`)                                                          |
| Lua      | `.lua-format`                              | format the build scripts                                                                                                         |

- Format the changed files with `clang-format` / `yapf` before committing, to avoid deviating from the existing style.
- Adding a new public API requires synchronizing generated `.pyi` stubs through the release workflow and the documentation (`docs/zh/` and `docs/en/`; the two trees must be updated in pairs with a consistent structure).

### Naming Conventions (C++)

The conventions below are distilled from the existing code of `hayaku_cpp/src/`; the new/modified code must follow them:

| The identifier category                   | The convention                                                                                                                                               | The examples                                                                                                                                                                                                        |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Namespace                                 | all lowercase                                                                                                                                                | `namespace hayaku;`                                                                                                                                                                                                  |
| Class / struct                            | `PascalCase`, the business domain + the core concept; the exported classes carry the `HAYAKU_API` macro                                                     | `class HAYAKU_API StockManager`, `class SignalBase`, `struct ParamItemRecord`                                                                                                                                    |
| Public member functions                   | `camelCase`, starting with a verb (`should/get/set/is/has/reload…`)                                                                                     | `shouldBuy()`, `getBuyValue()`, `reloadWith()`, `isIpcClientMode()`, `setTO()`, `nextTimeShouldBuy()`                                                                                                   |
| Protected / private member functions      | the`_` prefix + `camelCase` (the hooks that the subclasses need to override start with `_`)                                                            | `_calculate()`, `_reset()`, `_clone()`, `_addBuySignal()`, `_testingSetIpcClientMode()`                                                                                                                   |
| Member variables                          | the`m_` prefix + `camelCase`                                                                                                                             | `m_name`, `m_kdata`, `m_is_python_object`, `m_buySig`, `m_cycle_start`, `m_ipc_client_mode`                                                                                                             |
| Class static member variables             | the`ms_` prefix + `camelCase` (distinguished from the non-static `m_`)                                                                                 | `ms_sm`, `ms_init_mutex`, `ms_stockDict` (note: the old code of `StockManager` still uses `m_sm`/`m_init_mutex`/`m_stockDict` as a historical legacy; all the newly added static members use `ms_`) |
| Global variables / file-scope statics     | the`g_` prefix + `camelCase`                                                                                                                             | `g_load_event`, `g_shm_server_role`, `g_all_base_ktype`, `g_ktype2min`, `g_log_level`                                                                                                                     |
| The static local variables in functions   | the`g_` prefix + `camelCase` (consistent with the global variables, easy to identify the long-lived storage)                                             | `static std::once_flag g_tz_set;`, `static long int g_timezone;`                                                                                                                                                |
| Type aliases / smart pointer aliases      | the business object name +`Ptr` (`typedef shared_ptr<T> XPtr;`)                                                                                          | `typedef shared_ptr<SignalBase> SignalPtr;`                                                                                                                                                                       |
| Enumeration types / enumeration values    | the enumeration type in`PascalCase`; the enumeration values in all uppercase + underscores                                                                 | `KQuery::QueryType { INDEX, DATE, INVALID }`                                                                                                                                                                      |
| Macros / compilation switches / constants | all uppercase + underscores                                                                                                                                  | `HAYAKU_API`, `HAYAKU_SUPPORT_SERIALIZATION`, `HAYAKU_ENABLE_NODE`, `IND_EQ_THRESHOLD`                                                                                                                               |
| Function parameters                       | `camelCase`                                                                                                                                                | `baseInfoParam`, `kdataParam`, `datetime`, `context`                                                                                                                                                        |
| Local variables                           | `camelCase`                                                                                                                                                | the local variable style inside`initParam()`                                                                                                                                                                      |
| Header/source file names                  | `PascalCase`; stateful or independently testable business classes normally retain a matching file, while related PODs and thin stateless factories may be grouped by business capability | `Stock.h`, `SignalBase.h`, `ScalarMathOperators.h` |
| Derived implementations                  | Place them in the owning business module; do not create `imp`, `crt`, `internal`, `logic`, `support`, or `utils` directories merely to describe implementation technique | `strategy/decision/CrossSignal.h`, `execution/pricing/SlippageModels.h` |

> Note: `hayaku` is the only top-level namespace of the entire C++ core library; newly added public classes must carry the `HAYAKU_API` export macro. Factory functions and derived implementations belong to capability-named files inside their owning business module.
>
> **The file organization constraint**: preserve one-file ownership for complex stateful classes. Closely related value types, declarations, thin factories, and stateless implementations may coexist when they share one business responsibility and test boundary. Do not merge unrelated code merely to reduce file count.

### Naming Conventions (Python)

The conventions below are distilled from the existing code of the `hayaku/` package (excluding the `cpp/` compiled artifacts):

| The identifier category                 | The convention                                                                                                                      | The examples                                                                                                                |
| --------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------- |
| Package / module file names             | `snake_case`                                                                                                                      | `operators/`, `extensions/ingest/backends/`, `_support/singleton.py`, `extensions/visualization/backends/matplotlib_draw.py` |
| Classes                                 | `PascalCase`                                                                                                                      | `class Spot`, `class OrderBrokerWrap`, `class SingletonType`, `class System`                                        |
| Public functions / methods              | `snake_case`, starting with a verb                                                                                                | `concat_to_df()`, `df_to_ind()`, `run_in_strategy()`, `get_part()`                                                  |
| Private / internal methods              | a single underscore prefix +`snake_case` (conventionally for internal use, not strictly enforced)                                 | `_buy()`, `_sell()`, `_get_asset_info()`, `_clone()`                                                                |
| Magic methods                           | wrapped in double underscores, the Python standard                                                                                  | `__init__`, `__iter__`, `__str__`, `__repr__`                                                                       |
| Module-level constants                  | `SCREAMING_SNAKE_CASE`                                                                                                            | `BASE_DIR`, `DRAWNULL`, `HDF5_COMPRESS_LEVEL`, `KDATA`, `CLOSE`, `OPEN`, `HIGH`, `LOW`                      |
| The short global context variables      | a single uppercase letter (the abbreviations of the K-line fields/objects)                                                          | `O`, `C`, `H`, `L`, `A`, `V`, `D`, `K`, `Q` (as the global convenience aliases in `hayaku/__init__.py`) |
| Class variables / enumeration constants | all uppercase                                                                                                                       | `System.ENVIRONMENT`, `System.SIGNAL`, `System.STOPLOSS`                                                              |
| Instance variables                      | `snake_case`; the private ones start with a single underscore                                                                     | `self._name`, `self._params`, `self._broker`, `self._instance_lock`, `self._stop_event`                           |
| Local variables                         | `snake_case`                                                                                                                      | `df`, `ind_list`, `head_stock_code`, `params`, `cloned`                                                           |
| Function parameters                     | `snake_case`; the annotated variable names are also `snake_case`                                                                | `head_stock_code`, `col_name`, `col_date`, `allocate_weight_func`, `get_real_buy_price`                           |
| Custom decorators                       | the`hayaku_` prefix + `snake_case`                                                                                                 | `@hayaku_catch`, `@hayaku_check_ignore`                                                                                       |
| property / accessors                    | `snake_case` (the `PascalCase` accessors in `application/gui/flat/Spot.py` are the flatbuffers generated code, **not** this convention) | the accessor methods of the`hayaku/` code itself are mainly `snake_case`                                                |

> Note: the stubs generated by pybind11-stubgen, such as `hayaku/cpp/core3xx.pyi`, may contain the naming inconsistent with the above; they belong to the binding layer generated artifacts and are not regarded as the Python-side handwritten conventions.

### Generating the .pyi Stubs (pybind11-stubgen)

The `.pyi` stubs of the C++ binding layer (`core.so` / `core.pyd`) are generated with **pybind11-stubgen**:

```bash
# 1. Install pybind11-stubgen (if it is not installed)
pip install pybind11-stubgen

# 2. Generate the stubs (run it in the repository root, outputting to the current directory; make sure the project directory
#    is in the PYTHONPATH, and the compiled artifacts are under hayaku/cpp/ so that import hayaku works)
pybind11-stubgen -o . hayaku
```

- Do not generate the stubs by hand; only generate them when releasing or when requested manually.
- After modifying the bindings under `hayaku_pywrap/` (adding/changing the classes, the functions and the parameters), the corresponding stubs should be regenerated and synchronized.

## 6. Architecture and Key Components

The core components of the systematic trading framework are implemented under `hayaku_cpp/src/strategy/` and `hayaku_cpp/src/execution/`; their Python-facing modules are under `hayaku/strategy/`, `hayaku/execution/`, and related packages:

| The level              | The components                                                                                               | The description                                                                                                                |
| ---------------------- | ------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------ |
| The portfolio level    | PortfolioPF / SelectorSE / AllocateFundsAF / MultiFactorMF                                                   | the multi-system scheduling, the strategy screening, the fund allocation, the multi-factor                                     |
| The trading system SYS | EnvironmentEV / ConditionCN / SignalSG / Stoploss·StopprofitST / MoneyManagerMM / ProfitGoalPG / SlippageSP | the market environment, the valid condition, the signal, the risk control, the money management, the profit goal, the slippage |
| The trade management   | TradeManagerTM / OrderBrokerOB                                                                               | the account funds and position records, the live trading order connection                                                      |
| The data level         | StockManagerSM / KDataKD / QueryQ                                                                            | the security management, the K-line data, the time range query                                                                 |

- The data storage supports: HDF5 (the default) / MySQL / ClickHouse / SQLite / TDX.
- For the new indicators/new strategy components, it is recommended to implement them in the C++ core first (including the bindings and the unit tests), and then expose them at the Python layer; the pure Python extensions go into the corresponding subpackages of `hayaku/`.

## 7. Documentation

- Sphinx + myst_parser, **dual-source and bilingual**: `docs/zh/` (Chinese) and `docs/en/` (English) are two **independent Sphinx trees**, each with its own `conf.py`, without using gettext.
- The files are still a mix of `.rst` and `.md` (the new files prefer `.md`).
- The local build: `cd docs && ./make.sh` (building both trees → `build/html/{en,zh}`); `./make.sh en` / `./make.sh zh` build only one tree.
- **They must be maintained in pairs**: when changing the documentation of either language, synchronize the other tree within the same PR, keeping the file sets / the toctree / the heading levels / the labels / the images / the code blocks consistent.
- When modifying the public interfaces/adding the parts, synchronize the corresponding sections under **both trees** (`indicator/`, `data/`, `execution/`, `strategy/`, `factor.md`, etc.).
- The RTD hosting configuration: `docs/en/.readthedocs.yaml`, `docs/zh/.readthedocs.yaml` (the configuration files are **not** placed at the repository root); the cross-language jumps are provided by the RTD Flyout, and hardcoding the `/en/`, `/zh-cn/` links in the sources is forbidden.
- **When doing Chinese-English translation (covering the C++/Python comment anglicization, the docstrings, the bilingual docs, the README, etc.), the wording must refer to the glossary `docs/tools/glossary.zh-en.md`**; new terms must be registered in the glossary first (via PR review), and then be used — do not invent synonymous translations.

## 8. The AI Development Workflow and Caveats

1. **Locate the code**: C++ logic → `hayaku_cpp/src/`; bindings → `hayaku_pywrap/`; Python layer → `hayaku/`; tests → `hayaku_cpp/test/` and `tests/python/`.
2. **After modifying the C++ code, you must recompile and let the Python package load the new artifacts**:

   ```bash
   xmake -b core
   # Synchronize the compiled artifacts to hayaku/cpp/ (for import hayaku to load)
   ```

   The Python package entry `hayaku/__init__.py` loads `core.so` and the dependency libraries from `hayaku/cpp/` (mac/linux sets the `LD_LIBRARY_PATH`).
3. **When only changing the Python layer, there is no need to recompile the C++**, but note that the generated `.pyi` stubs must stay in sync with the implementations, and `hayaku/core.py` plus each domain's `_extensions.py` carry the Python enhancements.
4. **Do not commit the compiled artifacts**: `*.so`, `*.pyd`, `*.dll`, `build/` are all in `.gitignore`; the `core3xx.so`, etc. under `hayaku/cpp/` are the local build artifacts.
5. **Adding new dependencies**: the C++ dependencies go into `xmake.lua` with `add_requires` (note the platform differences and the versions, e.g. hdf5 is 1.13.3 on Windows, and mysql varies by platform); the Python dependencies go into `requirements.txt`.
6. **Tests first**: when the change involves the C++ core, run at least `xmake r unit-test` + `python3 tests/python/test.py`; when a specific module is involved, run its corresponding test file.
7. **The CI will verify**: the three pipelines of ubuntu (aarch64/x86_64), windows and macosx under `.github/workflows/`; the PRs must pass the builds and the tests before merging into `master`.
8. **The git commit messages uniformly use English**: in the conventional commits style, e.g. `fix(data): fix cross-period aggregation of derived K-lines in the SQL backend`; the historical early commits have Chinese messages, but all the new commits use English, and the body text is also in English.
9. **The AI must not commit proactively**: an AI coding agent is forbidden to execute `git commit`, and should also avoid `git add`; after completing each step, list "the list of the files to be committed + the suggested English commit message (a directly copyable `git commit -m "..."`)" and inform the user, letting the user decide the commit timing and the granularity.
10. **Handle with care**: `hayaku_pywrap` uses a unity build (`c++.unity_build`); pay attention to the unity_group grouping when adding the .cpp files; after modifying `xmake.lua`, you need to reconfigure with `xmake f`.

## 9. The Quick Self-check Checklist (before committing)

- [ ] The changed files have been formatted with `clang-format` / `yapf`
- [ ] The C++ changes have compiled successfully and the Python side can `import hayaku` normally
- [ ] The related unit tests have been run (C++: `xmake r small-test`; Python: `python3 tests/python/test.py`)
- [ ] No compiled artifacts/local data files have been committed
