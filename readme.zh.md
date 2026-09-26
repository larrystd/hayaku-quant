<p align="center">
  <img src="docs/zh/_static/00000-title.png" width="200" alt="Hayaku">
</p>

<p align="center">
  用 C++ 和 Python 研究行情数据与策略回测<br>
  <strong>显式会话 · 可组合策略 · Bazel 构建</strong>
</p>

# Hayaku Quant

[English](readme.md) | 简体中文

Hayaku 是面向行情数据研究、指标计算、策略组合与回测的 C++/Python 框架。C++ 核心负责数据访问和计算，Python API 提供显式会话和按领域划分的使用入口。

本仓库源自 [Hikyuu](https://github.com/fasiondog/hikyuu)，目前处于破坏式架构重构的概念验证阶段，原有 Hikyuu 程序需要适配。当前 Bazel 构建支持 **macOS、Linux 和 Python 3.10**；此构建不支持 Windows。

## 仓库提供什么？

- **数据与指标**：查询本地行情数据，组合指标计算。
- **策略研究**：定义信号、资金管理等组件，运行回测并检查结果。
- **执行账户**：通过会话拥有的执行引擎提交模拟订单，查看现金、持仓和成交记录。
- **可选模块**：历史数据导入和实时行情服务使用独立的原生包。

导入 `hayaku` 只加载公开类型，不会打开数据源。调用 `open_session()` 后才启动运行时；退出 `with` 代码块时会关闭会话。

## 从源码快速开始

需要 Bazelisk、CMake、支持 C++20 的编译器和 Python 3.10。Bazelisk 使用 [`.bazelversion`](.bazelversion) 指定的版本；C++ 依赖固定在 [`MODULE.bazel`](MODULE.bazel) 及其锁文件中。

~~~bash
git clone https://github.com/larrystd/hayaku-quant.git
cd hayaku-quant
python3.10 -m pip install -r requirements.txt
./op.sh build
./op.sh import-test
~~~

`./op.sh build` 会编译核心库与两个可选原生模块，再将六个库放入源码树中的 Python 包。下面的示例在仓库根目录运行，**不需要行情数据**：

~~~bash
python3.10 - <<'PY'
from hayaku.operators import MA, PRICELIST

prices = PRICELIST([1, 2, 3, 4, 5])
print(list(MA(prices, 3))[2:])  # [2.0, 3.0, 4.0]
PY
~~~

### 使用本地行情数据

先准备兼容的本地数据源和 `hayaku.ini` 配置文件。`open_session()` 默认读取 `~/.hayaku/hayaku.ini`，也可以显式传入路径。打开会话不会下载数据。

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

可选的数据导入 API 位于 `hayaku.extensions.ingest`。运行策略时，先用组件组成 `StrategyDefinition`，把所需 K 线放入 `BacktestRequest`，再使用绑定当前会话执行账户的 `StrategyEngine`。参见[策略指南](docs/zh/strategy.rst)和[订单示例](examples/python/execution_engine.py)。

## 架构

~~~text
Python 脚本 / Notebook
        |
        v
hayaku/                    会话、数据、指标、策略与执行 API
        |
        v
hayaku_pywrap/             pybind11 绑定层
        |
        v
hayaku_cpp/src/            C++ 数据、指标、策略与执行引擎
        |
        v
本地数据驱动                HDF5、SQLite、TDX 及已配置的扩展

可选：hayaku.extensions.ingest   -> hayaku_ingest_native
      hayaku.extensions.realtime -> hayaku_realtime_native
~~~

| Python 入口 | 职责 |
| --- | --- |
| `hayaku.data` | 证券、K 线数据和查询 |
| `hayaku.operators` | 指标与序列变换 |
| `hayaku.strategy` | 策略组件定义与回测引擎 |
| `hayaku.execution` | 订单、账户、持仓和成交记录 |
| `hayaku.metrics` | 结果转换与分析辅助函数 |
| `hayaku.application` | 会话、配置、命令行和交互工具 |
| `hayaku.extensions` | 显式启用数据导入、实时行情服务、可视化和扩展协议 |

核心 wheel 包含 `hayaku` 及其原生库；数据导入和实时行情服务分别打包为独立 wheel。默认 Bazel 配置包含 HDF5、SQLite、TDX 和 TA-Lib；MySQL 和 Windows 不在此构建配置内。构建目标、生成文件及依赖版本详见 [Bazel 指南](BAZEL.md)。

## 构建、测试与打包

~~~bash
./op.sh ci            # 提交检查：编译并测试 C++ 目标
./op.sh test          # C++ 测试和原生包导入检查
./op.sh python-test   # Python 回归测试
./op.sh all           # 构建并运行上述两组测试
~~~

生成三个 Python 3.10 wheel：

~~~bash
python3.10 -m pip install wheel
./op.sh wheel
./op.sh wheel-ingest
./op.sh wheel-realtime
python3.10 bazel/check_wheels.py
~~~

wheel 输出到 `dist/`。C++ 开发工具可运行 `./op.sh compdb`，从 Bazel 目标生成 `compile_commands.json`；运行 `./op.sh doctor` 可查看当前工具和路径。

## 文档与项目状态

- [快速入门](docs/zh/quickstart.rst)和[开发指南](docs/zh/developer.rst)
- [Bazel 构建指南](BAZEL.md)和 [Python 示例](examples/python/)
- [第三方许可证](THIRD_PARTY_LICENSES.md)和[项目许可证](LICENSE)

Hayaku 是研究工具，不提供投资建议或内置证券交易服务。用户自行接入的外部交易通道及其使用由用户负责。
