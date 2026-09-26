from typing import Any, List, Optional


KDataToHdf5Importer: Any
KDataToMySQLImporter: Any
KDataToClickHouseImporter: Any
check_data: Any


def open_kdata_importer(
    backend: str,
    *,
    datapath: Optional[str] = ...,
    markets: Optional[List[str]] = ...,
    ktypes: Optional[List[str]] = ...,
    baseinfo_path: str = ...,
    host: Optional[str] = ...,
    port: Optional[int] = ...,
    usr: Optional[str] = ...,
    pwd: str = ...,
    baseinfo_db: str = ...,
) -> Optional[Any]: ...
