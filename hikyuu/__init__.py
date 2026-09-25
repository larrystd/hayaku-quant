"""Hikyuu's small, side-effect-free public package surface.

Importing :mod:`hikyuu` only loads the native type definitions. It does not open market data,
register shutdown hooks, import plotting/dataframe stacks, or create process-global managers.
Use :func:`open_session` for runtime work and :mod:`hikyuu.interactive` for the broad research API.
"""

import os as _os
import sys as _sys
from pathlib import Path as _Path


_PACKAGE_DIR = _Path(__file__).resolve().parent
if _sys.platform == "win32":
    _dll_handle = _os.add_dll_directory(str(_PACKAGE_DIR / "cpp"))

from . import core as _core  # noqa: E402
from .core import (AccountSnapshot, BacktestRequest, BacktestResult, Block, DataEngine, Datetime, ExecutionEngine,
                   ExecutionReport, ExecutionStatus, FundsRecord, HKUException, HikyuuSession,
                   Indicator, KData, KRecord, MarketInfo, OrderRequest, OrderSide, Parameter,
                   PositionRecord, Query, Stock, StockTypeInfo, StockWeight,
                   TradeRecord,
                   StrategyContext, StrategyEngine, TimeDelta)
from .session import open_session


__version__ = _core.get_version()

from ._public_api import resolve_public_api as _resolve_public_api


__all__ = _resolve_public_api(globals())


def __dir__():
    return list(__all__)


del _resolve_public_api
