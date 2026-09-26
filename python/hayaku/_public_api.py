"""Declarative package-boundary policy for :mod:`hayaku`."""


EXTENSION_SPI_API = frozenset(
    (
        "BaseInfoDriver",
        "BlockInfoDriver",
        "ConditionBase",
        "DataDriverFactory",
        "EnvironmentBase",
        "IndicatorImp",
        "KDataDriver",
        "MoneyManagerBase",
        "MultiFactorBase",
        "NormalizeBase",
        "OrderBrokerBase",
        "ProfitGoalBase",
        "ScoresFilterBase",
        "SignalBase",
        "SlippageBase",
        "StoplossBase",
        "TradeCostBase",
    )
)


ADVANCED_API = frozenset(
    (
        "get_data_from_buffer_server",
        "get_spot_from_buffer_server",
        "is_shm_server_running",
        "start_data_server",
        "start_shm_server",
        "start_spot_agent",
        "stop_data_server",
        "stop_shm_server",
        "stop_spot_agent",
    )
)


TOP_LEVEL_PUBLIC_API = frozenset(
    (
        "__version__",
        "HAYAKUException",
        "open_session",
        "HayakuSession",
        "DataEngine",
        "ExecutionEngine",
        "StrategyEngine",
        "Datetime",
        "TimeDelta",
        "Stock",
        "Block",
        "KData",
        "Query",
        "Indicator",
        "Parameter",
        "StrategyContext",
        "MarketInfo",
        "StockTypeInfo",
        "StockWeight",
        "KRecord",
        "TradeRecord",
        "FundsRecord",
        "PositionRecord",
        "OrderSide",
        "OrderRequest",
        "ExecutionStatus",
        "ExecutionReport",
        "AccountSnapshot",
        "BacktestRequest",
        "BacktestResult",
    )
)


INTERNAL_TOP_LEVEL_API = frozenset(
    (
        "StockManager",
        "TradeManager",
        "System",
        "hayaku_init",
        "sm",
        "SessionOptions",
        "DataRuntime",
        "ExecutionRuntime",
        "StrategyRuntime",
        "Ledger",
        "OrderValidator",
    )
)


def resolve_public_api(namespace):
    """Validate and return the frozen package-level allow-list."""

    missing = TOP_LEVEL_PUBLIC_API.difference(namespace)
    if missing:
        missing_names = ", ".join(sorted(missing))
        raise RuntimeError(f"Incomplete hayaku public API: {missing_names}")
    leaked = INTERNAL_TOP_LEVEL_API.intersection(namespace)
    if leaked:
        leaked_names = ", ".join(sorted(leaked))
        raise RuntimeError(f"Internal hayaku API leaked at package root: {leaked_names}")
    return tuple(sorted(TOP_LEVEL_PUBLIC_API))


__all__ = (
    "ADVANCED_API",
    "EXTENSION_SPI_API",
    "INTERNAL_TOP_LEVEL_API",
    "TOP_LEVEL_PUBLIC_API",
    "resolve_public_api",
)
