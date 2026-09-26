"""Optional historical-data import entry points.

Importing this module does not open a storage connection or load an importer
plugin. Call :func:`open_kdata_importer` when a write target is needed.
"""

import importlib
import sys


_NATIVE_EXPORTS = frozenset((
    "KDataToHdf5Importer",
    "KDataToMySQLImporter",
    "KDataToClickHouseImporter",
    "check_data",
))


def _load_native():
    suffix = f"{sys.version_info.major}{sys.version_info.minor}"
    module_name = f"hayaku_ingest_native.ingest{suffix}"
    try:
        return importlib.import_module(module_name)
    except ModuleNotFoundError as exc:
        if exc.name not in ("hayaku_ingest_native", module_name):
            raise
        raise ImportError(
            "The optional hayaku ingestion extension is not installed. "
            "Build it with `xmake ingest` or install the ingestion package."
        ) from exc


def __getattr__(name):
    if name in _NATIVE_EXPORTS:
        return getattr(_load_native(), name)
    raise AttributeError(f"module {__name__!r} has no attribute {name!r}")


def __dir__():
    return sorted(set(globals()) | _NATIVE_EXPORTS)


def open_kdata_importer(backend, *, datapath=None, markets=None, ktypes=None,
                        baseinfo_path="", host=None, port=None, usr=None, pwd="",
                        baseinfo_db="hayaku_base"):
    """Create and configure a K-line importer for a storage backend.

    Return ``None`` when the selected importer cannot be configured, including
    when its optional plugin or license is unavailable. The returned object
    exposes the common K-line import methods (``add_krecord_list``,
    ``get_last_datetime``, ``update_index`` and ``remove``).

    ``datapath``, ``markets`` and ``ktypes`` configure HDF5. ``host``,
    ``port``, ``usr``, ``pwd`` and ``baseinfo_db`` configure MySQL or
    ClickHouse. Only the selected backend's importer is constructed.
    """
    backend = backend.lower()
    if backend == "hdf5":
        if datapath is None:
            raise ValueError("datapath is required for the HDF5 importer")
        importer = _load_native().KDataToHdf5Importer()
        configured = importer.set_config(
            datapath,
            ["SH", "SZ", "BJ"] if markets is None else markets,
            ["DAY", "MIN", "MIN5", "TIMELINE", "TRANSDATA"] if ktypes is None else ktypes,
            baseinfo_path,
        )
    elif backend in ("mysql", "clickhouse"):
        if backend == "mysql":
            importer = _load_native().KDataToMySQLImporter()
            default_port = 3306
            default_usr = "root"
        else:
            importer = _load_native().KDataToClickHouseImporter()
            default_port = 9000
            default_usr = "default"
        if host is None:
            raise ValueError(f"host is required for the {backend} importer")
        configured = importer.set_config(
            host, default_port if port is None else port,
            default_usr if usr is None else usr, pwd, baseinfo_db,
        )
    else:
        raise ValueError(f"unsupported K-line import backend: {backend}")

    return importer if configured else None


__all__ = ("open_kdata_importer", "KDataToHdf5Importer", "KDataToMySQLImporter",
           "KDataToClickHouseImporter", "check_data")
