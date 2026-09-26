"""Opt-in enhancements for native data types."""

from hayaku.core import *
from datetime import timedelta

Stock.__hash__ = lambda self: self.id

# ------------------------------------------------------------------
# Redefine Query
# ------------------------------------------------------------------

Query.INDEX = Query.QueryType.INDEX
Query.DATE = Query.QueryType.DATE
Query.DAY = "DAY"
Query.WEEK = "WEEK"
Query.MONTH = "MONTH"
Query.QUARTER = "QUARTER"
Query.HALFYEAR = "HALFYEAR"
Query.YEAR = "YEAR"
Query.MIN = "MIN"
Query.MIN3 = "MIN3"
Query.MIN5 = "MIN5"
Query.MIN15 = "MIN15"
Query.MIN30 = "MIN30"
Query.MIN60 = "MIN60"
Query.HOUR2 = "HOUR2"
Query.HOUR4 = "HOUR4"
Query.HOUR6 = "HOUR6"
Query.HOUR12 = "HOUR12"
Query.NO_RECOVER = Query.RecoverType.NO_RECOVER
Query.FORWARD = Query.RecoverType.FORWARD
Query.BACKWARD = Query.RecoverType.BACKWARD
Query.EQUAL_FORWARD = Query.RecoverType.EQUAL_FORWARD
Query.EQUAL_BACKWARD = Query.RecoverType.EQUAL_BACKWARD

old_Query_init = Query.__init__


def new_Query_init(self, start=0, end=None, ktype=Query.DAY, recover_type=Query.NO_RECOVER):
    """
        Build the condition to get the K-line data by index in the [start, end) way. start and end should both be int or both be Datetime.

        :param int|Datetime start: the start index position or the start date
        :param int|Datetime end: the end index position or the end date
        :param Query.KType ktype: the K-line data type (such as daily, minute, etc.)
        :param Query.RecoverType recover_type: the recovery type
        :return: the query condition
        :rtype: KQuery
        """
    if isinstance(start, int):
        end_pos = constant.null_int64 if end is None else end
    elif isinstance(start, Datetime):
        end_pos = constant.null_datetime if end is None else end
    else:
        raise TypeError('Incorrect parameter type error!')
    old_Query_init(self, start, end_pos, ktype, recover_type)


Query.__init__ = new_Query_init

TimeLineList.to_np = lambda data: timeline_to_np(data)
TimeLineList.to_df = lambda data: timeline_to_df(data)
TimeLineList.to_numpy = TimeLineList.to_np
TimeLineList.to_pandas = TimeLineList.to_df


TransList.to_np = lambda data: translist_to_np(data)
TransList.to_df = lambda data: translist_to_df(data)
TransList.to_numpy = TransList.to_np
TransList.to_pandas = TransList.to_df

StockWeightList.to_np = lambda data: weights_to_np(data)
StockWeightList.to_df = lambda data: weights_to_df(data)
StockWeightList.to_numpy = StockWeightList.to_np
StockWeightList.to_pandas = StockWeightList.to_df

KRecordList.to_np = lambda data: krecords_to_np(data)
KRecordList.to_df = lambda data: krecords_to_df(data)
KRecordList.to_numpy = KRecordList.to_np
KRecordList.to_pandas = KRecordList.to_df

KData.to_numpy = KData.to_np
KData.to_pandas = KData.to_df
