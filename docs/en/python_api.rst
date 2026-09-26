Python API Boundaries
=====================

Importing ``hayaku`` is side-effect free: it does not open data, register shutdown cleanup, or
import pandas and plotting packages. The package root contains at most 30 reviewed value types and
engine entry points. Legacy ``sm``, ``StockManager`` and ``hayaku_init`` are not Python attributes.

Data sessions
-------------

Ordinary data access starts with an explicitly owned session:

.. code-block:: python

    from hayaku import Query, open_session

    with open_session() as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))

Native execution accounts are configured while the session is opened:

.. code-block:: python

    from hayaku import open_session
    from hayaku.execution import AccountConfig, AccountId

    account = AccountConfig(initial_cash=100000, name="research", account_id=AccountId(1))
    with open_session(account_config=account) as session:
        session.wait_ready()
        snapshot = session.execution.snapshot()

``ExecutionEngine(TradeManager)`` and ``session.bind_execution`` are no longer Python APIs.

Domain modules
--------------

Use ``hayaku.data``, ``hayaku.execution`` and ``hayaku.strategy`` for engine-specific APIs.
Extension protocols live in ``hayaku.spi``; data-service controls live in ``hayaku.advanced``.
Historical data import is available through the optional ``hayaku.ingest`` module. The broad
indicator, factory, drawing and dataframe research surface is an
explicit opt-in through ``hayaku.interactive`` rather than a package-import side effect.

Internal runtime types such as ``SessionOptions``, ``DataRuntime``, ``ExecutionRuntime`` and
``StrategyRuntime`` are not Python APIs.

Historical data import
----------------------

The ingestion extension is not loaded when importing the core package. When building from source,
use ``xmake ingest`` to build the optional Python extension. ``import hayaku`` still works without
it; calling ingestion functionality then raises an actionable missing-extension error.

.. code-block:: python

    from hayaku.ingest import open_kdata_importer

    importer = open_kdata_importer("hdf5", datapath="/path/to/data", markets=["SH"])
    if importer is None:
        raise RuntimeError("The importer plugin or license is unavailable")

Supported backends are ``hdf5``, ``mysql``, and ``clickhouse``. Legacy import names such as
``hayaku.core.KDataToHdf5Importer`` and the ingestion aliases in ``hayaku.advanced`` have been
removed; import explicitly from ``hayaku.ingest``. MySQL and ClickHouse are storage backends, not
additional product capability groups.

Real-time quotes
----------------

The optional ``hayaku-realtime`` package provides real-time quote reception and data services.
Build it from source with ``xmake realtime``. With only the core installed, ``import hayaku``
and research/backtesting do not require the realtime extension. Import runtime controls such as
``start_spot_agent`` and ``stop_spot_agent`` explicitly from ``hayaku.advanced`` when needed.
Calling them without the optional extension gives an installation hint.

Migration from the legacy surface
---------------------------------

The old ``hayaku.trade_manage`` and ``hayaku.trade_sys`` packages have been removed. Use the
domain modules directly:

===============================  ================================================
Legacy API                       Replacement
===============================  ================================================
``TradeManager`` / ``crtTM``     ``AccountConfig`` and ``ExecutionEngine``
``System`` / ``SYS_Simple``      ``StrategyDefinition`` and ``StrategyEngine``
``hayaku.trade_manage`` records  ``hayaku.execution`` records
broker helpers                   ``hayaku.execution`` broker helpers
``hayaku.trade_sys`` factories   ``hayaku.strategy`` component factories
record ``to_np`` / ``to_df``     explicit ``hayaku.execution`` conversion helpers
===============================  ================================================

Backtests now receive an explicit ``BacktestRequest`` and return ``BacktestResult``. Account
inspection uses immutable ``AccountSnapshot`` and ``AccountView`` values. The retired manager and
system classes are deliberately not re-exported as compatibility aliases.

Python broker callbacks are attached explicitly when the account is configured:

.. code-block:: python

    from hayaku.execution import AccountConfig, OrderBrokerAdapter, TestOrderBroker

    broker = OrderBrokerAdapter(TestOrderBroker(), name="audit")
    account = AccountConfig(initial_cash=100000, brokers=[broker])
