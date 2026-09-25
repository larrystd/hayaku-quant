策略引擎
========

Python 策略边界由 ``hikyuu.strategy`` 中的三个显式值类型/门面构成：

``StrategyDefinition``
    不可变的策略组件图。至少显式传入资金管理组件和信号组件；环境、条件、止损、止盈、
    盈利目标与滑点组件均为可选参数。

``BacktestRequest``
    单次运行请求，包含输入 ``KData`` 与重置策略。

``StrategyEngine``
    使用显式传入的 ``ExecutionEngine`` 执行定义，并返回稳定的 ``BacktestResult`` 快照。

基本流程
--------

.. code-block:: python

    from hikyuu import Query, open_session
    from hikyuu.execution import AccountConfig
    from hikyuu.strategy import BacktestRequest, StrategyDefinition, StrategyEngine

    # money_manager 与 signal 可由内置组件构造器或显式 crtMM/crtSG 扩展工厂创建。
    definition = StrategyDefinition(money_manager, signal, name="demo")

    with open_session(account_config=AccountConfig(initial_cash=100000)) as session:
        session.wait_ready()
        bars = session.data.get_kdata("sh000001", Query(-500))
        engine = StrategyEngine(definition, session.execution)
        result = engine.run(BacktestRequest(bars))
        print(result.trade_count)

组件扩展
--------

组件基类以及 ``crtCN``、``crtEV``、``crtMM``、``crtPG``、``crtSG``、``crtSP``、
``crtST``、``crtMF``、``crtSCFilter`` 和 ``crtNorm`` 工厂位于
``hikyuu.strategy``，不会再注入包顶层。更底层的扩展协议位于 ``hikyuu.spi``。

已删除的运行时
--------------

Python 的 ``Strategy``/``System``/``Portfolio`` 运行时绑定以及 ``hikyuu.trade_sys`` 包已经
删除。数据服务和导入控制通过 ``hikyuu.advanced`` 显式使用；账户执行属于
``hikyuu.execution``。新接口不再提供 ``TradeManager`` 属性，也不再隐式创建进程级账户。
