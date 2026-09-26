Strategy engine
===============

The Python strategy boundary is centered on three explicit value/facade types from
``hayaku.strategy``:

``StrategyDefinition``
    An immutable component graph. At minimum it receives a money-management component and a
    signal component. Optional environment, condition, stop-loss, take-profit, profit-goal and
    slippage components are supplied explicitly.

``BacktestRequest``
    One run request containing the input ``KData`` and reset policy.

``StrategyEngine``
    Runs one definition against an explicitly supplied ``ExecutionEngine`` and returns a stable
    ``BacktestResult`` snapshot.

Basic flow
----------

.. code-block:: python

    from hayaku import Query, open_session
    from hayaku.execution import AccountConfig
    from hayaku.strategy import BacktestRequest, StrategyDefinition, StrategyEngine

    # ``money_manager`` and ``signal`` are component instances created by a built-in component
    # constructor or by the explicit crtMM/crtSG extension factories.
    definition = StrategyDefinition(money_manager, signal, name="demo")

    with open_session(account_config=AccountConfig(initial_cash=100000)) as session:
        session.wait_ready()
        bars = session.data.get_kdata("sh000001", Query(-500))
        engine = StrategyEngine(definition, session.execution)
        result = engine.run(BacktestRequest(bars))
        print(result.trade_count)

Component extensions
--------------------

The component base classes and the ``crtCN``, ``crtEV``, ``crtMM``, ``crtPG``, ``crtSG``,
``crtSP``, ``crtST``, ``crtMF``, ``crtSCFilter`` and ``crtNorm`` factories live in
``hayaku.strategy``. They are not injected into the package root. Lower-level extension protocols
are also available through ``hayaku.spi``.

Removed runtime
---------------

The Python ``Strategy``/``System``/``Portfolio`` runtime bindings and the
``hayaku.trade_sys`` package have been removed. Data services and importer controls are explicit
opt-ins in ``hayaku.advanced``; account execution belongs to
``hayaku.execution``. There is no ``TradeManager`` property or implicit process-global account.
