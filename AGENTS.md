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
hayaku-quant/
├── MODULE.bazel / MODULE.bazel.lock  # Pinned C++ dependencies
├── .bazelversion / .bazelrc          # Bazel version and compiler defaults
├── BUILD.bazel                       # Shared test fixtures
├── bazel/                            # Third-party builds, feature configuration, packaging tests
├── hayaku_cpp/src/BUILD.bazel        # Core, ingest and realtime C++ libraries
├── hayaku_cpp/test/BUILD.bazel       # C++ regression tests
├── hayaku_cpp/demo/BUILD.bazel       # C++ examples
├── hayaku_pywrap/BUILD.bazel         # Python 3.10 native extensions
├── hayaku/                           # Python interface package
├── hayaku_ingest_native/             # Optional ingestion package
├── hayaku_realtime_native/           # Optional realtime package
├── tools/wheels/                     # Optional wheel packaging definitions
├── tests/python/                     # Python regression suite
├── docs/                             # English and Chinese Sphinx documentation
└── .github/workflows/                # Bazel, docs and architecture checks
```

## 3. Build System (Bazel)

Bazel is the build system for macOS and Linux. Windows is outside the supported
Bazel platforms. Bazelisk reads `.bazelversion`; dependencies are fixed in
`MODULE.bazel` and its lockfile. The default feature set is in `bazel/config/`,
and FlatBuffers generates `spot_generated.h` from `spot.fbs`.

```bash
./op.sh build        # Build and stage the shared libraries and Python 3.10 extensions
./op.sh test         # Full C++ suite and Python package loading tests
./op.sh python-test  # Source-tree Python regression suite
bazel build //...    # All Bazel targets
```

`./op.sh build` copies the six native outputs into the source Python packages.
`./op.sh wheel`, `wheel-ingest`, and `wheel-realtime` create binary wheels. See
[BAZEL.md](BAZEL.md) for direct Bazel commands and output paths.

For clangd or clang-tidy, generate `compile_commands.json` with `./op.sh compdb`.
Regenerate it when targets or compiler flags change. Do not edit generated
headers under `bazel-bin/`.

## 4. Testing

### Python Tests (tests/python/)

```bash
export PYTHONPATH=.
./op.sh python-test           # the entry used by the CI
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
8. **The coverage requirements**: overall, aim for the branch coverage, with the line coverage as the minimum requirement. **The code paths that must be mocked to simulate are exempted** (e.g. the branches that can only be triggered by the external dependencies such as the network, the database and the live trading connections). `bazel coverage //hayaku_cpp/test:cval_test` can generate a coverage report for a self-check.

#### The Run Targets

- `//hayaku_cpp/test:cval_test`: focused indicator and serialization checks.
- `//hayaku_cpp/test:unit_test`: full C++ suite (798 doctest cases on current fixtures).
- `//bazel:python_package_smoke_test`: staged native package import checks.
- `tests/python/test.py`: broader Python regression suite, run with Python 3.10.

## 5. Code Conventions

| Language | Convention                                   | Key points                                                                                                                       |
| -------- | -------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------- |
| C++      | `.clang-format` (Google style as the base) | a 4-space indent, a column width of 100, the attached braces; compiler options are in the Bazel `BUILD.bazel` files |
| Python   | `hayaku/.style.yapf` (yapf) + `.flake8`  | a 4-space indent, a column width of 120 (flake8`max-line-length=120`)                                                          |

- Format the changed files with `clang-format` / `yapf` before committing, to avoid deviating from the existing style.
- Adding a new public API requires synchronizing generated `.pyi` stubs through the release workflow and the documentation (`docs/zh/` and `docs/en/`; the two trees must be updated in pairs with a consistent structure).

### Naming Conventions (C++)

The conventions below describe the existing public API. New and renamed instance members use Google C++ style: lowercase words with a trailing underscore. Existing public method names stay stable for source compatibility:

| The identifier category                   | The convention                                                                                                                                               | The examples                                                                                                                                                                                                        |
| ----------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| Namespace                                 | all lowercase                                                                                                                                                | `namespace hayaku;`                                                                                                                                                                                                  |
| Class / struct                            | `PascalCase`, the business domain + the core concept; the exported classes carry the `HAYAKU_API` macro                                                     | `class HAYAKU_API StockManager`, `class SignalBase`, `struct ParamItemRecord`                                                                                                                                    |
| Public member functions                   | `camelCase`, starting with a verb (`should/get/set/is/has/reload…`)                                                                                     | `shouldBuy()`, `getBuyValue()`, `reloadWith()`, `isIpcClientMode()`, `setTO()`, `nextTimeShouldBuy()`                                                                                                   |
| Protected / private member functions      | the`_` prefix + `camelCase` (the hooks that the subclasses need to override start with `_`)                                                            | `_calculate()`, `_reset()`, `_clone()`, `_addBuySignal()`, `_testingSetIpcClientMode()`                                                                                                                   |
| Member variables                          | `snake_case_` (Google C++ style; no `m_` prefix) | `impl_`, `runtime_`, `base_info_driver_`, `buy_sig_` |
| Class static member variables             | New names use `snake_case_`; existing `ms_` names are legacy | `sm_`, `init_mutex_`; legacy `ms_init_mutex` |
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
   ./op.sh build
   # The wrapper stages the native libraries into the Python packages.
   ```

   The Python package loads `core310.so` and `libhayaku.so` from `hayaku/cpp/`.
3. **When only changing the Python layer, there is no need to recompile the C++**, but note that the generated `.pyi` stubs must stay in sync with the implementations, and `hayaku/core.py` plus each domain's `_extensions.py` carry the Python enhancements.
4. **Do not commit the compiled artifacts**: `*.so`, `*.pyd`, `*.dll`, `build/` are all in `.gitignore`; the `core3xx.so`, etc. under `hayaku/cpp/` are the local build artifacts.
5. **Adding new dependencies**: the C++ dependencies go into `MODULE.bazel` and the Bazel `BUILD.bazel` files; the Python dependencies go into `requirements.txt`.
6. **Tests first**: when the change involves the C++ core, run `./op.sh test` and the relevant Python tests; when a specific module is involved, run its corresponding test file.
7. **The CI will verify**: the macOS/Linux Bazel pipeline and the docs pipeline under `.github/workflows/`; the PRs must pass the builds and the tests before merging into `master`.
8. **The git commit messages uniformly use English**: in the conventional commits style, e.g. `fix(data): fix cross-period aggregation of derived K-lines in the SQL backend`; the historical early commits have Chinese messages, but all the new commits use English, and the body text is also in English.
9. **The AI must not commit proactively**: an AI coding agent is forbidden to execute `git commit`, and should also avoid `git add`; after completing each step, list "the list of the files to be committed + the suggested English commit message (a directly copyable `git commit -m "..."`)" and inform the user, letting the user decide the commit timing and the granularity.
10. **Handle with care**: keep Bazel target source lists and dependency edges current when adding C++ or binding files.

## 9. The Quick Self-check Checklist (before committing)

- [ ] The changed files have been formatted with `clang-format` / `yapf`
- [ ] The C++ changes have compiled successfully and the Python side can `import hayaku` normally
- [ ] The related unit tests have been run (C++: `./op.sh test`; Python: `python3 tests/python/test.py`)
- [ ] No compiled artifacts/local data files have been committed
