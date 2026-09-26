"""Low-level runtime and data-maintenance controls.

These functions are intentionally separated from the ordinary three-engine
workflow and must be imported from this module explicitly.
"""

from importlib import import_module as _import_module


__all__ = (
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


def __getattr__(name):
    if name not in __all__:
        raise AttributeError(f"module {__name__!r} has no attribute {name!r}")

    try:
        native = _import_module("hayaku_realtime_native")
    except ModuleNotFoundError as error:
        if error.name != "hayaku_realtime_native":
            raise
        raise ImportError(
            "Hayaku realtime API requires the optional hayaku-realtime package"
        ) from error

    value = getattr(native, name)
    globals()[name] = value
    return value
