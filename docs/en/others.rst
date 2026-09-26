Research Utilities
==================

The indicator helpers ``concat_to_df`` and ``df_to_ind`` live in
``hayaku.indicator.indicator``. Import that module explicitly when converting
between Hayaku indicators and pandas data frames; importing the base
``hayaku`` package does not load pandas.

.. py:function:: concat_to_df(dates, ind_list, head_stock_code=True, head_ind_name=False)

    Combine evaluated indicators into a pandas data frame indexed by the
    supplied trading dates.

.. py:function:: df_to_ind(df, col_name, col_date=None)

    Convert a numeric data-frame column into an indicator. Supply ``col_date``
    when the dates are stored in a column rather than the index.

The native core exposes ``multi_regression`` and ``multi_regression_full`` for
research on several indicators and one security. Open a data session before
querying market data; see :ref:`quickstart`.

Functions supplied by an external ``hayaku_plugin`` package are documented
with that plugin and are not part of the core research API.
