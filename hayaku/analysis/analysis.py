"""Read-only helpers for stable execution and backtest result values."""

from hayaku.execution import positions_to_dataframe, trades_to_dataframe


def backtest_trades_to_dataframe(result):
    """Convert a :class:`BacktestResult` trade snapshot to a pandas DataFrame."""

    return trades_to_dataframe(result.trades)


def account_positions_to_dataframe(snapshot, *, short=False):
    """Convert an :class:`AccountSnapshot` position snapshot to a pandas DataFrame."""

    records = snapshot.short_positions if short else snapshot.positions
    return positions_to_dataframe(records)


__all__ = ("account_positions_to_dataframe", "backtest_trades_to_dataframe")
