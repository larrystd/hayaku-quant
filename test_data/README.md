# Local test fixtures

The default C++ and Python tests use local files only. Test runners create
`tmp/` as needed; files written there are generated results and are not source
fixtures.

| Scenario and assertion | Required content | Files |
| --- | --- | --- |
| Security, market, block, holiday, finance and bond queries | `Market`, `Stock`, `Block`, `Holiday`, `HistoryFinance`, `zh_bond10` and related lookup tables | `stock.db`, the five configured `block/*.ini` files |
| Day bars and period aggregation for Shanghai and Shenzhen | OHLCV records plus week, month, quarter, half-year and year index tables | `sh_day.h5`, `sz_day.h5` |
| Minute and derived period queries for both exchanges | OHLCV minute records and MIN15/MIN30/MIN60 index tables for `SH000001`, `SH600000` and `SZ000001`; `SH600004` five-minute records cover index operators | `sh_1min.h5`, `sz_1min.h5`, `sh_5min.h5`, `sz_5min.h5` |
| Intraday time line and trade records | Timestamp, price, volume and trade side | `sh_time.h5`, `sz_time.h5`, `sh_trans.h5`, `sz_trans.h5` |
| Temporary CSV security import | 100 daily bars; 578 minute rows across the first partial day, a full day and the last day | `test_day_data.csv`, `test_min_data.csv` |
| TDX input parsing | Small vendor-format source records | `vipdoc/` |
| Platform path syntax | The same local database and HDF5 paths in platform syntax | `hayaku_linux.ini`, `hayaku_win.ini` |

The minute HDF5 files retain only the securities used by the minute
query assertions. Their exact historical ranges remain because `test_Stock.cpp`
and period tests assert counts and positional records across them. The four
files shrank from 47.3 MB to 23.7 MB without changing the retained records or
index tables. Further date-range trimming requires coordinated assertion
changes. No default fixture depends on a network download.
