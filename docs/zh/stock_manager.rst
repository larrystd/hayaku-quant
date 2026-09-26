.. py:currentmodule:: hayaku
.. highlight:: python

证券管理
========

构建K线查询条件
-----------------
    
.. py:class:: Query

    K线数据查询条件，一般在Python中使用 Query 即可，不用指明 Query。

    简化 :py:data:`Query.KType` 枚举值
    
    - Query.DAY - 日线类型
    - Query.WEEK - 周线类型
    - Query.MONTH - 月线类型
    - Query.QUARTER - 季线类型
    - Query.HALFYEAR - 半年线类型
    - Query.YEAR - 年线类型
    - Query.MIN - 1分钟线类型
    - Query.MIN5 - 5分钟线类型
    - Query.MIN15 - 15分钟线类型
    - Query.MIN30 - 30分钟线类型
    - Query.MIN60 - 60分钟线类型
    
    简化 :py:data:`Query.RecoverType` 枚举值
    
    - Query.NO_RECOVER      - 不复权
    - Query.FORWARD         - 前向复权
    - Query.BACKWARD        - 后向复权
    - Query.EQUAL_FORWARD   - 等比前向复权
    - Query.EQUAL_BACKWARD  - 等比后向复权

    .. py:attribute:: start 
    
        起始索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: end

        结束索引，当按日期查询方式创建时无效，为 constant.null_int64
        
    .. py:attribute:: start_datetime
    
        起始日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: end_datetime
    
        结束日期，当按索引查询方式创建时无效，为 constant.null_datetime
        
    .. py:attribute:: query_type
    
        查询方式
        
    .. py:attribute:: ktype
    
        查询的K线类型
        
    .. py:attribute:: recover_type
    
        查询的复权类型
        
    .. py:attribute:: ktype_in_sec
    
        获取ktype对应的秒数，返回 TimeDelta 对象
        
        :rtype: TimeDelta
    
    .. py:data:: QueryType
    
        查询方式定义
        
        - DATE  - 按日期方式查询
        - INDEX - 按索引方式查询
    
    .. py:data:: KType
    
        K线类型枚举定义
        
        - DAY      - 日线类型
        - WEEK     - 周线类型
        - MONTH    - 月线类型
        - QUARTER  - 季线类型 
        - HALFYEAR - 半年线类型 
        - YEAR     - 年线类型 
        - MIN      - 1分钟线类型
        - MIN5     - 5分钟线类型
        - MIN15    - 15分钟线类型
        - MIN30    - 30分钟线类型
        - MIN60    - 60分钟线类型    
        
    .. py:data:: RecoverType
    
        K线复权类别枚举定义
    
        - NO_RECOVER      - 不复权
        - FORWARD         - 前向复权
        - BACKWARD        - 后向复权
        - EQUAL_FORWARD   - 等比前向复权
        - EQUAL_BACKWARD  - 等比后向复权

    .. py:method:: is_right_opening(self)

        判断是否为右开区间，即未指定结束时间

    .. py:method:: is_valid_ktype(self, ktype)

        判断指定的K线类型是否有效

        :param KType ktype: K线类型
        :return: 是否有效
        :rtype: bool

    .. py:staticmethod:: is_base_ktype(ktype)

        判断指定的K线类型是否为基础K线类型

        :param KType ktype: K线类型
        :return: 是否为基础K线类型
        :rtype: bool

    .. py:staticmethod:: is_extra_ktype(ktype)

        判断指定的K线类型是否为扩展K线类型

        :param KType ktype: K线类型
        :return: 是否为扩展K线类型
        :rtype: bool

    .. py:staticmethod:: get_base_ktype_list()

        获取所有基础K线类型

        :return: 基础K线类型列表
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_extra_ktype_list()

        获取所有扩展K线类型

        :return: 扩展K线类型列表
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_ktype_in_min(ktype)

        获取指定K线类型对应的分钟数

        :rtype: int


HayakuSession/DataEngine
------------------------

``HayakuSession`` 是显式的运行会话，负责数据引擎的初始化和作用域访问。旧的
``hayaku_init``、``StockManager`` 和全局 ``sm`` 已删除。关闭 Session 会立即使其
DataEngine 句柄失效；最后一个 Session 关闭时会停止数据加载并释放内部数据运行时。
不要让同一 Session 的 ``close`` 与查询并发执行。

.. code-block:: python

    from hayaku import Query, open_session

    with open_session() as session:
        stock = session.data.get_stock("sh000001")
        kdata = session.data.get_kdata("sh000001", Query(-100))

.. py:function:: open_session(filename=None, ignore_preload=False, context=None, account_config=None)

    打开运行会话。返回值支持 Python 上下文管理器协议，退出 ``with`` 时自动关闭该会话。

    :param str filename: 配置文件路径；缺省为 ``~/.hayaku/hayaku.ini``
    :param bool ignore_preload: 是否忽略预加载配置
    :param StrategyContext context: 数据加载范围
    :param AccountConfig account_config: 可选的原生执行账户配置
    :rtype: HayakuSession

.. py:class:: HayakuSession

    .. py:attribute:: opened

        会话是否仍然打开。

    .. py:attribute:: ready

        数据是否加载完成。

    .. py:attribute:: data

        当前会话持有的只读 :py:class:`DataEngine`。

    .. py:method:: close()

        关闭并使当前会话句柄失效，重复调用是安全的。最后一个显式 Session 会释放内部
        数据运行时，对应的数据运行时。

.. py:class:: DataEngine

    面向普通用户的只读数据入口，提供证券、K 线、市场、交易日历、板块、权重和财务数据查询。
    Driver、插件、预加载线程和 IPC 控制不属于该公共接口。它直接访问当前
    :py:class:`HayakuSession` 持有的内部数据运行时。

    常用方法包括 ``get_stock``、``get_stock_list``、``get_kdata``、
    ``get_market_info``、``get_trading_calendar``、``get_block`` 和
    ``get_history_finance_all_fields``。


Stock 与 Block 值类型
---------------------

.. py:class:: Stock

    证券对象

    .. py:attribute:: id : 内部id，一般用于作为map的键值使用
    .. py:attribute:: market : 获取所属市场简称，市场简称是市场的唯一标识
    .. py:attribute:: code : 获取证券代码
    .. py:attribute:: market_code : 市场简称+证券代码，如: sh000001
    .. py:attribute:: name : 获取证券名称
    .. py:attribute:: type 
    
        获取证券类型，参见：:py:data:`constant`
        
    .. py:attribute:: valid : 该证券当前是否有效
    .. py:attribute:: start_datetime : 证券起始日期
    .. py:attribute:: last_datetime : 证券最后日期
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tick_value : 最小跳动量价值
    .. py:attribute:: unit : 每单位价值 = tickValue / tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: atom : 最小交易数量，同minTradeNumber
    .. py:attribute:: min_trade_number : 最小交易数量
    .. py:attribute:: max_trade_number : 最大交易数量

    .. py:method:: is_null(self)
    
        是否为Null
    
        :rtype: bool
    
    .. py:method:: get_kdata(self, query)
    
        获取K线数据
        
        :param Query query: 查询条件
        :return: 满足查询条件的K线数据
        :rtype: KData
    
    .. py:method:: get_count(self[, ktype=Query.DAY])
    
        获取不同类型K线数据量
        
        :param Query.KType ktype: K线数据类别
        :return: K线记录数
        :rtype: int
    
    .. py:method:: get_market_value(self, date, ktype)
    
        获取指定时刻的市值，即小于等于指定时刻的最后一条记录的收盘价
        
        :param Datetime date: 指定时刻
        :param Query.KType ktype: K线数据类别
        :return: 指定时刻的市值
        :rtype: float
    
    .. py:method:: get_krecord(self, pos[, ktype=Query.DAY])
    
        获取指定索引的K线数据记录，未作越界检查
        
        :param int pos | Datetime datetime: 指定的索引位置，或日期
        :param Query.KType ktype: K线数据类别
        :return: K线记录
        :rtype: KRecord
    
    
    .. py:method:: get_krecord_list(self, start, end, ktype)
    
        获取K线记录 [start, end)，一般不直接使用，用getKData替代
        
        :param int start: 起始位置
        :param int end: 结束位置
        :param Query.KType ktype: K线类别
        :return: K线记录列表
        :rtype: KRecordList
    
    .. py:method:: get_datetime_list(self, query)
    
        获取日期列表
        
        :param Query query: 查询条件
        :rtype: DatetimeList

    .. py:method:: get_timeline_list(self, query)
    
        获取分时线数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TimeLineList
    
    .. py:method:: get_trans_list(self, query)
    
        获取历史分笔数据
        
        :param Query query: 查询条件（查询条件中的K线类型、复权类型参数此时无用）
        :rtype: TransList

    .. py:method:: get_weight(self[, start, end])
    
        获取指定时间段[start,end)内的权息信息。未指定起始、结束时刻时，获取全部权息记录。
        
        :param Datetime start: 起始时刻
        :param Datetime end: 结束时刻
        :rtype: StockWeightList
        
    .. py:method:: get_finance_info(self)
    
        获取当前财务信息
        
        :rtype: Parameter
        
    .. py:method:: get_history_finance(self)
    
        获取所有历史财务信息列表，字段信息可参考 DataEngine 的 get_history_finance_all_fields/get_history_finance_field_index/get_history_finance_field_name 方法
        
        日常建议直接使用指标 FINANCE 获取财务数据
        
        :param Datetime date: 指定日期必须是0331、0630、0930、1231，如 Datetime(201109300000)
        :rtype: list
    
    .. py:method:: set_krecord_list(self, krecord_list[, ktype=Query.DAY])

        谨慎调用！！！直接设置当前内存 KRecordList, 仅供需临时增加的外部 Stock 设置 K 线数据
        如果数据格式为 pandas.DataFrame, 可以使用 set_kdata_from_df 方法。

        :param sequence krecord_list: 一个可迭代变量获取 KRecord 实例的对象，如: list (仅包含 KRecord 实例)
        :param Query.KType ktype: K线类别

    .. py:method:: set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

        谨慎调用！！！直接设置当前内存数据，意味着 Stock 的基础数据变更。
        从 DataFrame 中获取 KRecordList, 并设置给当前Stock。df, 必须按顺序指定列名，默认为: ("datetime", "open", "high", "low", "close", "amount", "volume"))")

        .. code-block:: python

            import baostock as bs
            import pandas as pd
            lg = bs.login()

            rs = bs.query_history_k_data_plus("sh.600246",
                                            "date,code,open,high,low,close,volume,amount,adjustflag",
                                            start_date='2020-01-01', end_date='2025-12-31')
            print('query_history_k_data_plus respond error_code:'+rs.error_code)
            print('query_history_k_data_plus respond  error_msg:'+rs.error_msg)

            #### 打印结果集 ####
            data_list = []
            while (rs.error_code == '0') & rs.next():
                # 获取一条记录，将记录合并在一起
                data_list.append(rs.get_row_data())
            result = pd.DataFrame(data_list, columns=rs.fields)
            print(result)
            result['datetime'] = pd.to_datetime(result['date'])
            print(result)

            stock = Stock('TMP', '600246', 'test')
            stock.set_kdata_from_df(result)
            print(stock)        

        :param DataFrame df: 输入数据
        :param list cols: 列名
        :param Query.KType ktype: K线类别


    .. py:method:: realtime_update(self, krecord)
    
        （临时函数）只用于更新内存缓存中的日线数据

        单机数据服务客户端模式下：普通证券（本地无缓冲）的更新经 IPC 转发至主进程应用并
        镜像至共享内存（全体客户端可读）；临时证券（经 set_krecord_list 建有本地缓冲）则
        就地更新本地缓存、不外发。
        
        :param KRecord krecord: 新增的实时K线记录
        
    .. py:method:: get_last_update_time(self[, ktype=Query.DAY])

        获取指定类型 K 线数据的最后更新时刻。单机数据服务客户端模式下，普通证券转发至
        主进程取其缓冲刷新时刻，临时证券（经 set_krecord_list 指定外部数据）则返回本地写入时刻。

        :param Query.KType ktype: K线类型
        :rtype: Datetime

    .. py:method:: load_kdata_to_buffer(self, ktype)
    
        将指定类别的K线数据加载至内存缓存
        
        :param Query.KType ktype: K线类型

    .. py:method:: release_kdata_buffer(self, ktype)
    
        释放指定类别的内存K线数据
        
        :param Query.KType ktype: K线类型

    .. py:method:: get_belong_to_block_list(self[, category=None])
    
        获取所属板块列表

        :param str category: 指定的板块分类，为 None 时，返回所有板块分类下的所属板块
        :rtype: list    
    
    
.. py:class:: Block

    板块类，可视为证券的容器
    
    .. py:attribute:: category : 板块分类
    .. py:attribute:: name : 板块名称
    .. py:attribute:: index_stock: 对应指数（可能为空 Stock）
    
    .. py:method:: __init__(self, category, name):
    
        构建一个新的板块实例，并指定其板块分类及板块名称
    
        :param str category: 板块分类
        :param str name: 板块名称

    .. py:method:: __init__(self, block):
    
        通过其他板块实例构建新的板块实例
    
        :param Block block: 板块实例
    
    .. py:method:: size(self)
    
        包含的证券数量
        
    .. py:method:: empty(self)
    
        是否为空
        
    .. py:method:: get(self, market_code)

        根据"市场简称证券代码"获取对应的证券实例

        :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock

    .. py:method:: add(self, stock)
    
        加入指定的证券
        
        :param Stock stock: 待加入的证券
        :return: 是否成功加入
        :rtype: bool
        
        add(self, market_code)
    
        根据"市场简称证券代码"加入指定的证券
        
        :param str market_code: 市场简称证券代码
        :return: 是否成功加入
        :rtype: bool

    .. py:method:: remove(self, stock)
    
        移除指定证券
        
        :param Stock stock: 指定的证券
        :return: 是否成功
        :rtype: bool
        
        remove(self, market_code)
    
        移除指定证券
        
        :param str market_code: 市场简称证券代码
        :return: 是否成功
        :rtype: bool
        
    .. py:method:: clear(self)

        移除包含的所有证券
        
    .. py:method:: __len__(self)  

        包含的证券数量
        
    .. py:method:: __getitem__(self, market_code)
    
        根据"市场简称证券代码"获取对应的证券实例
        
        :param str market_code: 格式：“市场简称证券代码”，如"sh000001"
        :return: 对应的证券实例，如果实例不存在，则Null<Stock>()，不抛出异常
        :rtype: Stock        

     

其它证券信息定义
------------------

.. py:class:: StockTypeInfo

    股票类型详情记录
    
    .. py:attribute:: type : 证券类型
    .. py:attribute:: description : 描述信息
    .. py:attribute:: tick : 最小跳动量
    .. py:attribute:: tick_value : 每一个tick价格
    .. py:attribute:: unit : 每最小变动量价格，即单位价格 = tickValue/tick
    .. py:attribute:: precision : 价格精度
    .. py:attribute:: min_trade_num : 每笔最小交易量
    .. py:attribute:: max_trade_num : 每笔最大交易量


.. py:class:: StockWeight

    权息记录
    
    .. py:attribute:: datetime : 权息日期
    .. py:attribute:: count_as_gift : 每10股送X股
    .. py:attribute:: count_for_sell : 每10股配X股
    .. py:attribute:: price_for_sell : 配股价
    .. py:attribute:: bonus : 每10股红利
    .. py:attribute:: increasement : 每10股转增X股
    .. py:attribute:: total_count : 总股本（万股）
    .. py:attribute:: free_count : 流通股（万股）
    

.. py:class:: StockWeightList

    std::vector<StockWeight> 包装，见 :py:class:`StockWeight`

    .. py:method:: to_numpy(self)

        转为 numpy 数组

    .. py:method:: to_pandas(self)

        转为 pandas DataFrame

    .. py:method:: to_pyarrow(self)

        转为 pyarrow Table


.. py:class:: MarketInfo

    市场信息记录
    
    .. py:attribute:: market : 市场简称（如：沪市“SH”, 深市“SZ”）
    .. py:attribute:: name : 市场全称
    .. py:attribute:: description :描述说明
    .. py:attribute:: code : 该市场对应的主要指数，用于获取交易日历
    .. py:attribute:: last_datetime : 该市场K线数据最后交易日期
