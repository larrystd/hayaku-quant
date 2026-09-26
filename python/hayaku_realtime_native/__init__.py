"""Native binaries for the optional Hayaku realtime extension.

Use :mod:`hayaku.extensions.realtime` for the supported Python API. Importing this
package explicitly loads the live extension; importing :mod:`hayaku` does not.
"""

from importlib import import_module as _import_module
from sys import version_info as _version_info


_module_name = f"realtime{_version_info.major}{_version_info.minor}"
try:
    _native = _import_module(f".{_module_name}", __name__)
except ModuleNotFoundError as _error:
    if _error.name == f"{__name__}.{_module_name}":
        raise ImportError(
            "Hayaku realtime native extension is not installed for this Python version"
        ) from _error
    raise


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
    "spot_agent_is_running",
    "spot_agent_is_connected",
)

for _name in __all__:
    globals()[_name] = getattr(_native, _name)

del _import_module, _module_name, _native, _name, _version_info
