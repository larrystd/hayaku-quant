#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Stable identifiers for optional plugins. Keep this header free of plugin
 * interface includes.
 */

namespace hayaku {

inline constexpr const char* HAYAKU_PLUGIN_DEVICE = "device";
inline constexpr const char* HAYAKU_PLUGIN_DATASERVER = "dataserver";
inline constexpr const char* HAYAKU_PLUGIN_IMPORTKDATATOHDF5 = "import2hdf5";
inline constexpr const char* HAYAKU_PLUGIN_IMPORTKDATATOCLICKHOUSE =
    "import2ch";
inline constexpr const char* HAYAKU_PLUGIN_IMPORTKDATATOMYSQL = "import2mysql";
inline constexpr const char* HAYAKU_PLUGIN_EXTEND_INDICATOR = "extind";
inline constexpr const char* HAYAKU_PLUGIN_TMREPORT = "tmreport";
inline constexpr const char* HAYAKU_PLUGIN_CLICKHOUSE_DRIVER =
    "clickhousedriver";
inline constexpr const char* HAYAKU_PLUGIN_HAYAKU_EXTRA = "hayakuextra";
inline constexpr const char* HAYAKU_PLUGIN_CHECK_DATA = "checkdata";
inline constexpr const char* HAYAKU_PLUGIN_SHM_SERVER = "shmserver";

}  // namespace hayaku
