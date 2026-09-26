.. _quickstart:

Getting Started
===============

Hayaku keeps data loading inside an explicit session. Importing ``hayaku``
defines types but does not open a data source. Start with :doc:`install`, then
prepare a local ``hayaku.ini`` and market data through the optional ingest
tools. A data session does not download data from the network.

Open a research session
-----------------------

The configuration file selects the local data drivers. If ``filename`` is
omitted, ``open_session`` uses ``~/.hayaku/hayaku.ini``.

.. code-block:: python

    from hayaku import Query, open_session

    with open_session(filename="/path/to/hayaku.ini") as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))
        print(stock.market_code, len(bars))

The data handle belongs to the session. Close the session after the query;
the ``with`` block does this even when an exception occurs. The data APIs are
described in :doc:`stock_manager`.

Add an execution account
------------------------

For strategy research or backtesting, pass an ``AccountConfig`` when opening
the session. This enables its native execution engine.

.. code-block:: python

    from hayaku import open_session
    from hayaku.execution import AccountConfig

    account = AccountConfig(initial_cash=100000, name="research")
    with open_session(filename="/path/to/hayaku.ini", account_config=account) as session:
        session.wait_ready()
        print(session.execution.snapshot())

See :doc:`strategy` for assembling and running a strategy. The optional
``hayaku.application.interactive`` module provides a broader exploratory interface and
loads additional plotting dependencies only when explicitly imported.
