.. py:currentmodule:: hayaku
.. highlight:: python

Security Management
===================

Building Bar (Candlestick) Query Conditions
--------------------------------------------

.. py:class:: Query

    Query criteria for bar (candlestick) data. In Python, ``Query`` is readily available in the
    hayaku namespace and can be used directly.

    Shortcut aliases for the :py:data:`Query.KType` enumeration values

    - Query.DAY - daily bar
    - Query.WEEK - weekly bar
    - Query.MONTH - monthly bar
    - Query.QUARTER - quarterly bar
    - Query.HALFYEAR - half-yearly bar
    - Query.YEAR - yearly bar
    - Query.MIN - 1-minute bar
    - Query.MIN5 - 5-minute bar
    - Query.MIN15 - 15-minute bar
    - Query.MIN30 - 30-minute bar
    - Query.MIN60 - 60-minute bar

    Shortcut aliases for the :py:data:`Query.RecoverType` enumeration values

    - Query.NO_RECOVER      - no adjustment (raw prices)
    - Query.FORWARD         - forward adjustment
    - Query.BACKWARD        - backward adjustment
    - Query.EQUAL_FORWARD   - proportional forward adjustment
    - Query.EQUAL_BACKWARD  - proportional backward adjustment

    .. py:attribute:: start

        Start index. Invalid when the query is created by date, in which case it is
        constant.null_int64

    .. py:attribute:: end

        End index. Invalid when the query is created by date, in which case it is
        constant.null_int64

    .. py:attribute:: start_datetime

        Start date/time. Invalid when the query is created by positional index, in which case it
        is constant.null_datetime

    .. py:attribute:: end_datetime

        End date/time. Invalid when the query is created by positional index, in which case it is
        constant.null_datetime

    .. py:attribute:: query_type

        Query mode (by date or by positional index)

    .. py:attribute:: ktype

        Requested bar type (candlestick period)

    .. py:attribute:: recover_type

        Requested price adjustment type

    .. py:attribute:: ktype_in_sec

        Duration, in seconds, of the bar type given by ``ktype``; returned as a TimeDelta object

        :rtype: TimeDelta

    .. py:data:: QueryType

        Available query modes

        - DATE  - query by date
        - INDEX - query by positional index

    .. py:data:: KType

        Bar type (candlestick period) enumeration

        - DAY      - daily bar
        - WEEK     - weekly bar
        - MONTH    - monthly bar
        - QUARTER  - quarterly bar
        - HALFYEAR - half-yearly bar
        - YEAR     - yearly bar
        - MIN      - 1-minute bar
        - MIN5     - 5-minute bar
        - MIN15    - 15-minute bar
        - MIN30    - 30-minute bar
        - MIN60    - 60-minute bar

    .. py:data:: RecoverType

        Price adjustment type enumeration for bar data

        - NO_RECOVER      - no adjustment (raw prices)
        - FORWARD         - forward adjustment
        - BACKWARD        - backward adjustment
        - EQUAL_FORWARD   - proportional forward adjustment
        - EQUAL_BACKWARD  - proportional backward adjustment

    .. py:method:: is_right_opening(self)

        Return whether the query covers a right-open interval, i.e. whether no end time is
        specified

    .. py:method:: is_valid_ktype(self, ktype)

        Check whether the given bar type is valid

        :param KType ktype: bar type
        :return: True if the bar type is valid
        :rtype: bool

    .. py:staticmethod:: is_base_ktype(ktype)

        Check whether the given bar type is a built-in (base) bar type

        :param KType ktype: bar type
        :return: True if it is a built-in bar type
        :rtype: bool

    .. py:staticmethod:: is_extra_ktype(ktype)

        Check whether the given bar type is an extended bar type

        :param KType ktype: bar type
        :return: True if it is an extended bar type
        :rtype: bool

    .. py:staticmethod:: get_base_ktype_list()

        Return all built-in (base) bar types

        :return: list of built-in bar types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_extra_ktype_list()

        Return all extended bar types

        :return: list of extended bar types
        :rtype: list[Query.KType]

    .. py:staticmethod:: get_ktype_in_min(ktype)

        Return the duration, in minutes, of the given bar type

        :rtype: int


HayakuSession/DataEngine
------------------------

``HayakuSession`` is the explicit runtime session responsible for initialization and scoped access
to the data engine. The legacy ``hayaku_init`` function, ``StockManager`` class, and global ``sm``
object have been removed. Closing a session immediately invalidates its DataEngine handle. Closing
the final session stops data loading and releases the internal data runtime. Do not call ``close``
concurrently with a query on the same session.

.. code-block:: python

    from hayaku import Query, open_session

    with open_session() as session:
        stock = session.data.get_stock("sh000001")
        kdata = session.data.get_kdata("sh000001", Query(-100))

.. py:function:: open_session(filename=None, ignore_preload=False, context=None, account_config=None)

    Open a runtime session. The returned object implements the Python context-manager protocol and
    closes the session when leaving the ``with`` block.

    :param str filename: configuration file path; defaults to ``~/.hayaku/hayaku.ini``
    :param bool ignore_preload: whether to ignore preloading configuration
    :param StrategyContext context: data loading scope
    :param AccountConfig account_config: optional native execution account
    :rtype: HayakuSession

.. py:class:: HayakuSession

    .. py:attribute:: opened

        Whether the session is open.

    .. py:attribute:: ready

        Whether data loading has completed.

    .. py:attribute:: data

        The read-only :py:class:`DataEngine` owned by this session.

    .. py:method:: close()

        Close and invalidate this session handle. Repeated calls are safe. The final explicit
        session releases the internal data runtime.

.. py:class:: DataEngine

    The read-only data entry point for ordinary users. It provides security, bar-data, market,
    trading-calendar, sector, weight and financial-data queries. Driver, plugin, preload-thread and
    IPC controls are not part of this public interface. It talks directly to the internal data
    runtime owned by its :py:class:`HayakuSession`.

    Common methods include ``get_stock``, ``get_stock_list``, ``get_kdata``, ``get_market_info``,
    ``get_trading_calendar``, ``get_block`` and ``get_history_finance_all_fields``.


Stock and Block values
----------------------

.. py:class:: Stock

    A tradable security

    .. py:attribute:: id : internal id, typically used as a map key
    .. py:attribute:: market : market abbreviation of the market it belongs to; the market abbreviation uniquely identifies a market
    .. py:attribute:: code : the security code
    .. py:attribute:: market_code : market abbreviation + security code, e.g. sh000001
    .. py:attribute:: name : the security name
    .. py:attribute:: type

        The security type; see :py:data:`constant`

    .. py:attribute:: valid : whether the security is currently valid (tradable)
    .. py:attribute:: start_datetime : the first date for which data is available
    .. py:attribute:: last_datetime : the last date for which data is available
    .. py:attribute:: tick : the minimum price tick
    .. py:attribute:: tick_value : the monetary value of one tick
    .. py:attribute:: unit : value per unit = tickValue / tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: atom : the minimum tradable quantity; the same as minTradeNumber
    .. py:attribute:: min_trade_number : the minimum tradable quantity
    .. py:attribute:: max_trade_number : the maximum tradable quantity

    .. py:method:: is_null(self)

        Whether this is a null instance

        :rtype: bool

    .. py:method:: get_kdata(self, query)

        Return the bars matching the query

        :param Query query: the query criteria
        :return: the bar data satisfying the query
        :rtype: KData

    .. py:method:: get_count(self[, ktype=Query.DAY])

        Return the number of available bars of the specified type

        :param Query.KType ktype: the bar type
        :return: the number of bar records
        :rtype: int

    .. py:method:: get_market_value(self, date, ktype)

        Return the market value at the specified time, i.e. the close price of the last bar whose
        time is less than or equal to the given time

        :param Datetime date: the specified time
        :param Query.KType ktype: the bar type
        :return: the market value at the specified time
        :rtype: float

    .. py:method:: get_krecord(self, pos[, ktype=Query.DAY])

        Return the bar record at the specified index, without bounds checking

        :param int pos | Datetime datetime: the zero-based index position, or a date
        :param Query.KType ktype: the bar type
        :return: the bar record
        :rtype: KRecord


    .. py:method:: get_krecord_list(self, start, end, ktype)

        Return the bar records in the half-open range [start, end). Rarely called directly;
        use get_kdata instead

        :param int start: the start position
        :param int end: the end position
        :param Query.KType ktype: the bar type
        :return: the list of bar records
        :rtype: KRecordList

    .. py:method:: get_datetime_list(self, query)

        Return the list of bar dates

        :param Query query: the query criteria
        :rtype: DatetimeList

    .. py:method:: get_timeline_list(self, query)

        Return the intraday timeline data

        :param Query query: the query criteria (the bar type and price adjustment type carried by
                            the query are ignored here)
        :rtype: TimeLineList

    .. py:method:: get_trans_list(self, query)

        Return the historical tick-by-tick transaction data

        :param Query query: the query criteria (the bar type and price adjustment type carried by
                            the query are ignored here)
        :rtype: TransList

    .. py:method:: get_weight(self[, start, end])

        Return the dividend and corporate-action records within the half-open range
        [start, end). When neither bound is specified, all records are returned.

        :param Datetime start: the start time
        :param Datetime end: the end time
        :rtype: StockWeightList

    .. py:method:: get_finance_info(self)

        Return the current fundamental data

        :rtype: Parameter

    .. py:method:: get_history_finance(self)

        Return all historical fundamental records. For the field layout, refer to the related
        DataEngine methods: get_history_finance_all_fields /
        get_history_finance_field_index / get_history_finance_field_name

        For day-to-day work, the FINANCE indicator is the more convenient way to access
        fundamental data

        :param Datetime date: the report date must be one of 0331, 0630, 0930, 1231,
                              e.g. Datetime(201109300000)
        :rtype: list

    .. py:method:: set_krecord_list(self, krecord_list[, ktype=Query.DAY])

        Use with caution!!! Directly replaces the in-memory KRecordList. Intended only for
        supplying bar data to temporary external Stocks.
        If the data is a pandas.DataFrame, use the set_kdata_from_df method instead.

        :param sequence krecord_list: an iterable that yields KRecord instances, e.g. a list
                                      containing only KRecord instances
        :param Query.KType ktype: the bar type

    .. py:method:: set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

        Use with caution!!! Directly replaces the in-memory data, which means the underlying data
        of the Stock is changed.
        Builds a KRecordList from the DataFrame and assigns it to this Stock. df must provide the
        column names in the given order; the default columns are:
        ("datetime", "open", "high", "low", "close", "amount", "volume")

        .. code-block:: python

            import baostock as bs
            import pandas as pd
            lg = bs.login()

            rs = bs.query_history_k_data_plus("sh.600246",
                                            "date,code,open,high,low,close,volume,amount,adjustflag",
                                            start_date='2020-01-01', end_date='2025-12-31')
            print('query_history_k_data_plus respond error_code:'+rs.error_code)
            print('query_history_k_data_plus respond  error_msg:'+rs.error_msg)

            #### Print the result set ####
            data_list = []
            while (rs.error_code == '0') & rs.next():
                # Fetch one row at a time and accumulate the rows
                data_list.append(rs.get_row_data())
            result = pd.DataFrame(data_list, columns=rs.fields)
            print(result)
            result['datetime'] = pd.to_datetime(result['date'])
            print(result)

            stock = Stock('TMP', '600246', 'test')
            stock.set_kdata_from_df(result)
            print(stock)

        :param DataFrame df: the input data
        :param list cols: the column names
        :param Query.KType ktype: the bar type


    .. py:method:: realtime_update(self, krecord)

        (Temporary helper) Only updates the daily-bar data in the in-memory cache

        In client mode of the standalone data server, updates for ordinary securities (which
        have no local buffer) are forwarded over IPC to the master process, applied there, and
        mirrored into shared memory where every client can read them; temporary securities
        (those with a local buffer created through set_krecord_list) are updated in place in the
        local cache and are not forwarded.

        :param KRecord krecord: the newly arrived real-time bar record

    .. py:method:: get_last_update_time(self[, ktype=Query.DAY])

        Return the last update time for bars of the specified type. In client mode of the
        standalone data server, ordinary securities are forwarded to the master process and
        return its buffer refresh time; temporary securities (external data supplied through
        set_krecord_list) return the local write time.

        :param Query.KType ktype: the bar type
        :rtype: Datetime

    .. py:method:: load_kdata_to_buffer(self, ktype)

        Load bars of the specified type into the in-memory cache

        :param Query.KType ktype: the bar type

    .. py:method:: release_kdata_buffer(self, ktype)

        Release the in-memory bars of the specified type from the cache

        :param Query.KType ktype: the bar type

    .. py:method:: get_belong_to_block_list(self[, category=None])

        Return the sectors this security belongs to

        :param str category: restrict the result to this sector category; when None, sectors from
                             all categories are returned
        :rtype: list


.. py:class:: Block

    The sector class; a Block can be viewed as a container of securities

    .. py:attribute:: category : the sector category
    .. py:attribute:: name : the sector name
    .. py:attribute:: index_stock: the associated index (may be a null Stock)

    .. py:method:: __init__(self, category, name):

        Construct a new empty sector with the given category and name

        :param str category: the sector category
        :param str name: the sector name

    .. py:method:: __init__(self, block):

        Construct a new sector as a copy of another sector

        :param Block block: the source sector instance

    .. py:method:: size(self)

        The number of securities contained in the sector

    .. py:method:: empty(self)

        Whether the sector is empty

    .. py:method:: get(self, market_code)

        Return the security identified by "market abbreviation + security code"

        :param str market_code: market abbreviation followed by the security code, e.g.
                                "sh000001"
        :return: the matching security; returns Null<Stock>() if it does not exist, without
                 raising an exception
        :rtype: Stock

    .. py:method:: add(self, stock)

        Add the specified security to the sector

        :param Stock stock: the security to add
        :return: True if it was added successfully
        :rtype: bool

        add(self, market_code)

        Add the specified security by "market abbreviation + security code"

        :param str market_code: market abbreviation + security code
        :return: True if it was added successfully
        :rtype: bool

    .. py:method:: remove(self, stock)

        Remove the specified security from the sector

        :param Stock stock: the security to remove
        :return: True on success
        :rtype: bool

        remove(self, market_code)

        Remove the specified security from the sector

        :param str market_code: market abbreviation + security code
        :return: True on success
        :rtype: bool

    .. py:method:: clear(self)

        Remove all securities from the sector

    .. py:method:: __len__(self)

        The number of securities contained in the sector

    .. py:method:: __getitem__(self, market_code)

        Return the security identified by "market abbreviation + security code"

        :param str market_code: market abbreviation followed by the security code, e.g.
                                "sh000001"
        :return: the matching security; returns Null<Stock>() if it does not exist, without
                 raising an exception
        :rtype: Stock


Other Security Information Definitions
---------------------------------------

.. py:class:: StockTypeInfo

    The detailed record describing a security type

    .. py:attribute:: type : the security type
    .. py:attribute:: description : descriptive information
    .. py:attribute:: tick : the minimum price tick
    .. py:attribute:: tick_value : the price of one tick
    .. py:attribute:: unit : the price per minimum increment, i.e. the unit price = tickValue/tick
    .. py:attribute:: precision : the price precision
    .. py:attribute:: min_trade_num : the minimum order quantity
    .. py:attribute:: max_trade_num : the maximum order quantity


.. py:class:: StockWeight

    The dividend and corporate-action record

    .. py:attribute:: datetime : the dividend/corporate-action date
    .. py:attribute:: count_as_gift : X bonus shares granted per 10 shares held
    .. py:attribute:: count_for_sell : X rights-issue shares allotted per 10 shares held
    .. py:attribute:: price_for_sell : the rights-issue subscription price
    .. py:attribute:: bonus : the cash dividend per 10 shares
    .. py:attribute:: increasement : X shares converted from capital reserves per 10 shares
    .. py:attribute:: total_count : total share capital (in 10,000 shares)
    .. py:attribute:: free_count : freely tradable (floating) shares (in 10,000 shares)


.. py:class:: StockWeightList

    A wrapper around std::vector<StockWeight>; see :py:class:`StockWeight`

    .. py:method:: to_numpy(self)

        Convert to a numpy array

    .. py:method:: to_pandas(self)

        Convert to a pandas DataFrame

    .. py:method:: to_pyarrow(self)

        Convert to a pyarrow Table


.. py:class:: MarketInfo

    The market information record

    .. py:attribute:: market : the market abbreviation (e.g. "SH" for the Shanghai market, "SZ" for the Shenzhen market)
    .. py:attribute:: name : the full name of the market
    .. py:attribute:: description : descriptive information
    .. py:attribute:: code : the main benchmark index of this market, used to derive the trading calendar
    .. py:attribute:: last_datetime : the latest trading date for which bar data is available in this market
