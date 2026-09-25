"""Explicit conversion helpers for execution-domain value records.

The old ``trade_manage`` package installed methods on native extension types at import time. These
helpers keep the useful conversions without mutating shared classes or importing pandas eagerly.
"""

from hikyuu.core import PositionRecordList, TradeRecordList
from hikyuu.core import positions_to_df as _positions_to_df
from hikyuu.core import positions_to_np as _positions_to_np
from hikyuu.core import trades_to_df as _trades_to_df
from hikyuu.core import trades_to_np as _trades_to_np


def _as_native_list(records, record_list_type):
    """Accept either the bound vector type or an ordinary Python iterable."""

    return records if isinstance(records, record_list_type) else record_list_type(records)


def trades_to_numpy(records):
    """Return trade records as a NumPy array."""

    return _trades_to_np(_as_native_list(records, TradeRecordList))


def trades_to_dataframe(records):
    """Return trade records as a pandas DataFrame."""

    return _trades_to_df(_as_native_list(records, TradeRecordList))


def positions_to_numpy(records):
    """Return position records as a NumPy array."""

    return _positions_to_np(_as_native_list(records, PositionRecordList))


def positions_to_dataframe(records):
    """Return position records as a pandas DataFrame."""

    return _positions_to_df(_as_native_list(records, PositionRecordList))


__all__ = (
    "positions_to_dataframe",
    "positions_to_numpy",
    "trades_to_dataframe",
    "trades_to_numpy",
)
