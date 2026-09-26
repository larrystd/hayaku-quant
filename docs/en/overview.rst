.. figure:: _static/00000-title.png

About Hayaku
============

Hayaku Quant Framework is an open-source, extremely fast quantitative trading research framework
written in C++/Python. It focuses on strategy analysis, backtesting and the extension to live
trading (currently deeply adapted to the Chinese A-share market).

Hayaku is derived from `Hikyuu <https://github.com/fasiondog/hikyuu>`_. This repository is a
breaking architecture-refactor POC, not a drop-in replacement for the upstream project.

The project is dedicated to the core technologies of quantitative trading, with capabilities in four
dimensions: **trading model development, an extremely fast computation engine, an efficient
backtesting system, and live-trading extension**.

Built around mature systematic trading concepts, the framework decomposes a complete trading system
into independent parts: **market environment, condition, signal generator, stop-loss /
take-profit, money management, profit goal, slippage, multi-factor model, portfolio and fund
allocation**. You can build your own strategy library for each module, combine them freely in
research and backtest flexibly, and analyze the effectiveness and robustness of a single strategy as
well as the overall return of combined strategies.

Repositories:

* `Hayaku <https://github.com/larrystd/hayaku-quant>`_ — active refactor repository
* `Hikyuu <https://github.com/fasiondog/hikyuu>`_ — upstream source and history

Project home page: `https://github.com/larrystd/hayaku-quant <https://github.com/larrystd/hayaku-quant>`_

Getting started: :ref:`quickstart`.

Tutorial notebooks: `English notebook series <https://nbviewer.org/github/larrystd/hayaku-quant/blob/poc/examples/python/notebook/en/000-Index.ipynb?flush_cache=True>`_.

Upstream strategy part library: `https://gitee.com/fasiondog/hikyuu_hub <https://gitee.com/fasiondog/hikyuu_hub>`_

Example code:

::

    from hayaku import Query, open_session
    from hayaku.execution import AccountConfig

    # Runtime state is explicitly owned by the session.
    account = AccountConfig(initial_cash=300000, name="research")
    with open_session(account_config=account) as session:
        session.wait_ready()
        bars = session.data.get_kdata("sz000001", Query(-150))
        snapshot = session.execution.snapshot()

    # Build and run component graphs with StrategyDefinition, BacktestRequest and StrategyEngine;
    # see the Strategy engine chapter for the complete flow.

.. figure:: _static/10000-overview.png
        :width: 600px

The current session flow is described in :ref:`quickstart`.


Why Hayaku?
-------------------------

**Flexible composition: build a categorized strategy asset library**

Hayaku provides a lightweight abstraction of systematic trading methods, covering market environment,
condition, signal generator, stop-loss / take-profit, money management, profit goal, slippage,
selector and fund allocation. You can assemble your own strategy library from these parts,
combine and backtest them efficiently, and focus on the effect and impact of a single module while
exploring strategies, which greatly improves research productivity. The main functional architecture
is shown below:

.. figure:: _static/10002-function-arc.png
        :width: 800px

**Extreme performance: build your own quantitative application with ease**

The project consists of three parts: a high-performance C++ core library, a Python interface layer
(hayaku), and interactive exploration tools.

* **Measured performance:** on an AMD 7950x, loading the full A-share market (19.13 million daily
  bars) and computing and summing the 20-day moving average for the first time takes only 6 seconds;
  once the data is warm, the same operation takes only 166 milliseconds. See the
  `benchmark article <https://mp.weixin.qq.com/s?__biz=MzkwMzY1NzYxMA==&mid=2247483768&idx=1&sn=33e40aa9633857fa7b4c7ded51c95ae7&chksm=c093a09df7e4298b3f543121ba01334c0f8bf76e75c643afd6fc53aea1792ebb92de9a32c2be&mpshare=1&scene=23&srcid=05297ByHT6DEv6XAmyje1oOr&sharer_shareinfo=b38f5f91b4efd8fb60303a4ef4774748&sharer_shareinfo_first=b38f5f91b4efd8fb60303a4ef4774748#rd>`_

* **C++ core library:** ships a complete strategy framework with native multi-threading and
  multi-core acceleration, leaving room to scale for very high computing demands. The core library
  can also be used standalone, helping developers build custom quantitative tools quickly.

* **Python interface layer (hayaku):** a lightweight wrapper around the C++ core with TA-Lib
  integrated; converts seamlessly to and from numpy and pandas, so it plugs into the mainstream
  Python data analysis ecosystem.

* **hayaku.interactive exploration tool:** built-in visualization of candlesticks, indicators and
  signals, suitable for rapid strategy validation and backtest analysis.

**Concise syntax: explore strategies faster and more freely**

Both object-oriented and command-line styles are supported. Especially during strategy exploration,
the command-line style is minimal and expressive, letting you validate ideas and iterate faster.

**Modular and extensible data storage**

The core supports local HDF5 and SQLite data sources. MySQL and ClickHouse are
optional adapters. Prepare data with the optional ingest capability before
opening a research session.



Dependencies
---------------------------------------------------------------

The C++ core of Hayaku depends directly on the following open-source projects (indirect dependencies
and Python-side dependencies are not listed; see requirements.txt for the Python dependencies).
Thanks to all the open-source authors for their contributions.

.. raw:: html

    <table>
    <thead>
    <tr>
    <th>Name</th>
    <th>Project</th>
    <th>License</th>
    </tr>
    </thead>
    <tbody>
    <tr>
    <td>xmake</td>
    <td><a href="https://github.com/xmake-io/xmake">https://github.com/xmake-io/xmake</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>hdf5</td>
    <td><a href="https://github.com/HDFGroup/hdf5">https://github.com/HDFGroup/hdf5</a></td>
    <td><a href="https://github.com/HDFGroup/hdf5?tab=License-1-ov-file#License-1-ov-file">hdf5 license</a></td>
    </tr>
    <tr>
    <td>mysql(client)</td>
    <td><a href="https://github.com/mysql/mysql-server">https://github.com/mysql/mysql-server</a></td>
    <td><a href="https://github.com/mysql/mysql-server?tab=License-1-ov-file#readme">mysql license</a></td>
    </tr>
    <tr>
    <td>fmt</td>
    <td><a href="https://github.com/fmtlib/fmt">https://github.com/fmtlib/fmt</a></td>
    <td><a href="https://github.com/fmtlib/fmt?tab=License-1-ov-file#readme">fmt license</a></td>
    </tr>
    <tr>
    <td>spdlog</td>
    <td><a href="https://github.com/gabime/spdlog">https://github.com/gabime/spdlog</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>sqlite</td>
    <td><a href="https://www.sqlite.org/">https://www.sqlite.org/</a></td>
    <td><a href="https://www.sqlite.org/copyright.html">sqlite license</a></td>
    </tr>
    <tr>
    <td>flatbuffers</td>
    <td><a href="https://github.com/google/flatbuffers">https://github.com/google/flatbuffers</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>nng</td>
    <td><a href="https://github.com/nanomsg/nng">https://github.com/nanomsg/nng</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>nlohmann_json</td>
    <td><a href="https://github.com/nlohmann/json">https://github.com/nlohmann/json</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>boost</td>
    <td><a href="https://www.boost.org/">https://www.boost.org/</a></td>
    <td><a href="https://www.boost.org/users/license.html">Boost Software License</a></td>
    </tr>
    <tr>
    <td>python</td>
    <td><a href="https://www.python.org/">https://www.python.org/</a></td>
    <td><a href="https://docs.python.org/3/license.html">Python license</a></td>
    </tr>
    <tr>
    <td>pybind11</td>
    <td><a href="https://github.com/pybind/pybind11">https://github.com/pybind/pybind11</a></td>
    <td><a href="https://github.com/pybind/pybind11?tab=License-1-ov-file#readme">pybind11 license</a></td>
    </tr>
    <tr>
    <td>gzip-hpp</td>
    <td><a href="https://github.com/mapbox/gzip-hpp">https://github.com/mapbox/gzip-hpp</a></td>
    <td>BSD-2-Clause license</td>
    </tr>
    <tr>
    <td>doctest</td>
    <td><a href="https://github.com/doctest/doctest">https://github.com/doctest/doctest</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>ta-lib</td>
    <td><a href="https://github.com/TA-Lib/ta-lib.git">https://github.com/TA-Lib/ta-lib.git</a></td>
    <td>BSD-3-Clause license</td>
    </tr>
    <tr>
    <td>clickhouse</td>
    <td><a href="https://github.com/ClickHouse/ClickHouse">https://github.com/ClickHouse/ClickHouse</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>xxhash</td>
    <td><a href="https://github.com/Cyan4973/xxHash">https://github.com/Cyan4973/xxHash</a></td>
    <td>BSD 2-Clause License</td>
    </tr>
    <tr>
    <td>utf8proc</td>
    <td><a href="https://github.com/JuliaStrings/utf8proc">https://github.com/JuliaStrings/utf8proc</a></td>
    <td>MIT</td>
    </tr>
    <tr>
    <td>arrow</td>
    <td><a href="https://github.com/apache/arrow">https://github.com/apache/arrow</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>eigen</td>
    <td><a href="https://gitlab.com/libeigen/eigen">https://gitlab.com/libeigen/eigen</a></td>
    <td>Apache 2.0</td>
    </tr>
    <tr>
    <td>mimalloc</td>
    <td><a href="https://github.com/microsoft/mimalloc">https://github.com/microsoft/mimalloc</a></td>
    <td>MIT</td>
    </tr>
    </tbody>
    </table>    
