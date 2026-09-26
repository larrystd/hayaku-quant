.. _quickstart:

快速入门
========

Hayaku 通过显式 Session 加载数据。导入 ``hayaku`` 只定义类型，不会打开数据源。
先按 :doc:`install` 完成安装，再用可选的 ingest 工具准备本地 ``hayaku.ini``
和行情数据。打开数据 Session 不会从网络下载数据。

打开研究会话
------------

配置文件指定本地数据驱动。省略 ``filename`` 时，``open_session`` 使用
``~/.hayaku/hayaku.ini``。

.. code-block:: python

    from hayaku import Query, open_session

    with open_session(filename="/path/to/hayaku.ini") as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))
        print(stock.market_code, len(bars))

数据句柄属于当前 Session。查询结束后应关闭 Session；``with`` 代码块会在
异常发生时也执行关闭。数据接口见 :doc:`stock_manager`。

加入执行账户
------------

进行策略研究或回测时，在打开 Session 时传入 ``AccountConfig``，即可启用
当前会话的原生执行引擎。

.. code-block:: python

    from hayaku import open_session
    from hayaku.execution import AccountConfig

    account = AccountConfig(initial_cash=100000, name="research")
    with open_session(filename="/path/to/hayaku.ini", account_config=account) as session:
        session.wait_ready()
        print(session.execution.snapshot())

策略的组合和运行方式见 :doc:`strategy`。可选的 ``hayaku.application.interactive``
模块提供更广的交互式研究接口，仅在显式导入时加载绘图等额外依赖。
