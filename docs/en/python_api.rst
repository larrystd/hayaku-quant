Python API Boundaries
=====================

Importing ``hikyuu`` is side-effect free: it does not open data, register shutdown cleanup, or
import pandas and plotting packages. The package root contains at most 30 reviewed value types and
engine entry points. Legacy ``sm``, ``StockManager`` and ``hikyuu_init`` are not Python attributes.

Data sessions
-------------

Ordinary data access starts with an explicitly owned session:

.. code-block:: python

    from hikyuu import Query, open_session

    with open_session() as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))

Native execution accounts are configured while the session is opened:

.. code-block:: python

    from hikyuu import open_session
    from hikyuu.execution import AccountConfig, AccountId

    account = AccountConfig(initial_cash=100000, name="research", account_id=AccountId(1))
    with open_session(account_config=account) as session:
        session.wait_ready()
        snapshot = session.execution.snapshot()

``ExecutionEngine(TradeManager)`` and ``session.bind_execution`` are no longer Python APIs.

Domain modules
--------------

Use ``hikyuu.data``, ``hikyuu.execution`` and ``hikyuu.strategy`` for engine-specific APIs.
Extension protocols live in ``hikyuu.spi``; data-service and importer controls live in
``hikyuu.advanced``. The broad indicator, factory, drawing and dataframe research surface is an
explicit opt-in through ``hikyuu.interactive`` rather than a package-import side effect.

Internal runtime types such as ``SessionOptions``, ``DataRuntime``, ``ExecutionRuntime`` and
``StrategyRuntime`` are not Python APIs.

Migration from the legacy surface
---------------------------------

The old ``hikyuu.trade_manage`` and ``hikyuu.trade_sys`` packages have been removed. Use the
domain modules directly:

===============================  ================================================
Legacy API                       Replacement
===============================  ================================================
``TradeManager`` / ``crtTM``     ``AccountConfig`` and ``ExecutionEngine``
``System`` / ``SYS_Simple``      ``StrategyDefinition`` and ``StrategyEngine``
``hikyuu.trade_manage`` records  ``hikyuu.execution`` records
broker helpers                   ``hikyuu.execution`` broker helpers
``hikyuu.trade_sys`` factories   ``hikyuu.strategy`` component factories
record ``to_np`` / ``to_df``     explicit ``hikyuu.execution`` conversion helpers
===============================  ================================================

Backtests now receive an explicit ``BacktestRequest`` and return ``BacktestResult``. Account
inspection uses immutable ``AccountSnapshot`` and ``AccountView`` values. The retired manager and
system classes are deliberately not re-exported as compatibility aliases.

Python broker callbacks are attached explicitly when the account is configured:

.. code-block:: python

    from hikyuu.execution import AccountConfig, OrderBrokerAdapter, TestOrderBroker

    broker = OrderBrokerAdapter(TestOrderBroker(), name="audit")
    account = AccountConfig(initial_cash=100000, brokers=[broker])
