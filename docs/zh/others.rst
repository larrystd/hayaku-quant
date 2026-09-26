研究辅助函数
============

指标辅助函数 ``concat_to_df`` 和 ``df_to_ind`` 位于
``hayaku.indicator.indicator``。需要在 Hayaku 指标与 pandas DataFrame
之间转换时显式导入该模块；导入基础 ``hayaku`` 包不会加载 pandas。

.. py:function:: concat_to_df(dates, ind_list, head_stock_code=True, head_ind_name=False)

    将计算后的多个指标按指定交易日期合并为 pandas DataFrame。

.. py:function:: df_to_ind(df, col_name, col_date=None)

    将 DataFrame 的数值列转换为指标。日期存放在列中而非索引中时，传入
    ``col_date``。

原生 core 提供 ``multi_regression`` 和 ``multi_regression_full``，可用
单只证券及多个指标进行研究。查询行情数据前应打开数据 Session，参见
:ref:`quickstart`。

外部 ``hayaku_plugin`` 包提供的函数由对应插件单独维护文档，不属于 core
研究接口。
