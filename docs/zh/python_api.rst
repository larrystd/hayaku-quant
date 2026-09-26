Python API 边界
===============

导入 ``hayaku`` 不再产生运行时副作用：不会打开数据、注册退出清理，也不会隐式导入 pandas
或绘图库。包顶层只保留不超过 30 个经过审核的值类型和 Engine 入口；``sm``、
``StockManager``、``hayaku_init`` 已不再是 Python 属性。

数据会话
--------

普通数据访问从显式持有的 Session 开始：

.. code-block:: python

    from hayaku import Query, open_session

    with open_session() as session:
        session.wait_ready()
        stock = session.data.get_stock("sh000001")
        bars = session.data.get_kdata("sh000001", Query(-100))

原生执行账户在打开 Session 时配置：

.. code-block:: python

    from hayaku import open_session
    from hayaku.execution import AccountConfig, AccountId

    account = AccountConfig(initial_cash=100000, name="research", account_id=AccountId(1))
    with open_session(account_config=account) as session:
        session.wait_ready()
        snapshot = session.execution.snapshot()

``ExecutionEngine(TradeManager)`` 和 ``session.bind_execution`` 不再属于 Python API。

领域模块
--------

Engine 专属接口分别位于 ``hayaku.data``、``hayaku.execution`` 和 ``hayaku.strategy``。
扩展协议位于 ``hayaku.extensions.spi``；数据服务控制位于 ``hayaku.extensions.realtime``，历史数据导入使用可选的
``hayaku.extensions.ingest``。指标、工厂、绘图和
DataFrame 等宽研究接口改为显式导入 ``hayaku.application.interactive``，不再由包导入隐式加载。

``SessionOptions``、``DataRuntime``、``ExecutionRuntime`` 和 ``StrategyRuntime`` 等运行时实现
类型不属于 Python API。

历史数据导入
------------

导入能力不随核心导入自动加载。源码构建时可使用 ``./op.sh build`` 构建可选的 Python 扩展；
未安装该扩展时，``import hayaku`` 仍可使用，调用导入功能会得到明确的缺失扩展错误。

.. code-block:: python

    from hayaku.extensions.ingest import open_kdata_importer

    importer = open_kdata_importer("hdf5", datapath="/path/to/data", markets=["SH"])
    if importer is None:
        raise RuntimeError("导入插件或许可证不可用")

后端可选 ``hdf5``、``mysql``、``clickhouse``。旧的
``hayaku.core.KDataToHdf5Importer`` 等名称和原有的导入别名已删除；
请从 ``hayaku.extensions.ingest`` 显式导入。MySQL 与 ClickHouse 是存储后端，不是额外的产品能力面。

实时行情
--------

实时行情接收与数据服务由可选的 ``hayaku-realtime`` 提供。源码构建使用
``./op.sh build``；仅安装核心包时，``import hayaku`` 和研究/回测不需要实时扩展。
需要使用时，从 ``hayaku.extensions.realtime`` 显式导入 ``start_spot_agent``、
``stop_spot_agent`` 等运行期入口。未安装实时扩展时，调用这些入口会提示安装可选模块。

旧接口迁移
----------

``hayaku.trade_manage`` 与 ``hayaku.trade_sys`` 包已经删除，请直接使用对应领域模块：

.. list-table::
   :header-rows: 1

   * - 旧接口
     - 替代接口
   * - ``TradeManager`` / ``crtTM``
     - ``AccountConfig`` 与 ``ExecutionEngine``
   * - ``System`` / ``SYS_Simple``
     - ``StrategyDefinition`` 与 ``StrategyEngine``
   * - ``hayaku.trade_manage`` 记录类型
     - ``hayaku.execution`` 记录类型
   * - 券商辅助函数
     - ``hayaku.execution`` 券商辅助函数
   * - ``hayaku.trade_sys`` 组件工厂
     - ``hayaku.strategy`` 组件工厂
   * - 记录的 ``to_np`` / ``to_df``
     - ``hayaku.execution`` 中的显式转换函数

回测现在显式接收 ``BacktestRequest`` 并返回 ``BacktestResult``；账户读取使用不可变的
``AccountSnapshot`` 与 ``AccountView``。已经退役的 Manager 和 System 类型不会再通过兼容别名
重新暴露。

Python 券商回调在账户配置时显式接入：

.. code-block:: python

    from hayaku.execution import AccountConfig, OrderBrokerAdapter, TestOrderBroker

    broker = OrderBrokerAdapter(TestOrderBroker(), name="audit")
    account = AccountConfig(initial_cash=100000, brokers=[broker])
