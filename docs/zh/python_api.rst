Python API 边界
===============

导入 ``hikyuu`` 不再产生运行时副作用：不会打开数据、注册退出清理，也不会隐式导入 pandas
或绘图库。包顶层只保留不超过 30 个经过审核的值类型和 Engine 入口；``sm``、
``StockManager``、``hikyuu_init`` 已不再是 Python 属性。

数据会话
--------

普通数据访问从显式持有的 Session 开始：

.. code-block:: python

    from hikyuu import Query, open_session

    with open_session() as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))

原生执行账户在打开 Session 时配置：

.. code-block:: python

    from hikyuu import open_session
    from hikyuu.execution import AccountConfig, AccountId

    account = AccountConfig(initial_cash=100000, name="research", account_id=AccountId(1))
    with open_session(account_config=account) as session:
        session.wait_ready()
        snapshot = session.execution.snapshot()

``ExecutionEngine(TradeManager)`` 和 ``session.bind_execution`` 不再属于 Python API。

领域模块
--------

Engine 专属接口分别位于 ``hikyuu.data``、``hikyuu.execution`` 和 ``hikyuu.strategy``。
扩展协议位于 ``hikyuu.spi``；数据服务与导入控制位于 ``hikyuu.advanced``。指标、工厂、绘图和
DataFrame 等宽研究接口改为显式导入 ``hikyuu.interactive``，不再由包导入隐式加载。

``SessionOptions``、``DataRuntime``、``ExecutionRuntime`` 和 ``StrategyRuntime`` 等运行时实现
类型不属于 Python API。

旧接口迁移
----------

``hikyuu.trade_manage`` 与 ``hikyuu.trade_sys`` 包已经删除，请直接使用对应领域模块：

===============================  ================================================
旧接口                           替代接口
===============================  ================================================
``TradeManager`` / ``crtTM``     ``AccountConfig`` 与 ``ExecutionEngine``
``System`` / ``SYS_Simple``      ``StrategyDefinition`` 与 ``StrategyEngine``
``hikyuu.trade_manage`` 记录类型 ``hikyuu.execution`` 记录类型
券商辅助函数                     ``hikyuu.execution`` 券商辅助函数
``hikyuu.trade_sys`` 组件工厂    ``hikyuu.strategy`` 组件工厂
记录的 ``to_np`` / ``to_df``     ``hikyuu.execution`` 中的显式转换函数
===============================  ================================================

回测现在显式接收 ``BacktestRequest`` 并返回 ``BacktestResult``；账户读取使用不可变的
``AccountSnapshot`` 与 ``AccountView``。已经退役的 Manager 和 System 类型不会再通过兼容别名
重新暴露。

Python 券商回调在账户配置时显式接入：

.. code-block:: python

    from hikyuu.execution import AccountConfig, OrderBrokerAdapter, TestOrderBroker

    broker = OrderBrokerAdapter(TestOrderBroker(), name="audit")
    account = AccountConfig(initial_cash=100000, brokers=[broker])
