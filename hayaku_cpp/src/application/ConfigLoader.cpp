/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "ConfigLoader.h"

#include <fmt/format.h>

#include "application/IniParser.h"
#include "common/Os.h"
#include "data/KQuery.h"

namespace hayaku {

void getConfigFromIni(const string& configFileName, Parameter& baseParam,
                      Parameter& blockParam, Parameter& kdataParam,
                      Parameter& preloadParam, Parameter& hayakuParam) {
  IniParser config;
  config.read(configFileName);

  hayakuParam.set<string>("tmpdir", config.get("hayaku", "tmpdir", "."));
  hayakuParam.set<string>("datadir", config.get("hayaku", "datadir", "."));
  hayakuParam.set<string>(
      "quotation_server",
      config.get("hayaku", "quotation_server", "ipc:///tmp/hayaku_real.ipc"));
  hayakuParam.set<bool>("auto_reload",
                        config.getBool("hayaku", "auto_reload", "False"));
  hayakuParam.set<string>("reload_time",
                          config.get("hayaku", "reload_time", "00:00"));
  hayakuParam.set<bool>("use_shm_server",
                        config.getBool("hayaku", "use_shm_server", "False"));
  hayakuParam.set<int64_t>(
      "shm_server_wait_timeout",
      config.getInt("hayaku", "shm_server_wait_timeout", "600"));
  hayakuParam.set<string>("lazy_preload",
                          config.get("hayaku", "lazy_preload", "False"));
  hayakuParam.set<bool>("load_stock_weight",
                        config.getBool("hayaku", "load_stock_weight", "True"));
  hayakuParam.set<bool>(
      "load_history_finance",
      config.getBool("hayaku", "load_history_finance", "True"));
  hayakuParam.set<string>(
      "plugindir", config.get("hayaku", "plugindir",
                              fmt::format("{}/.hayaku/plugin", getUserDir())));

  auto option = config.getOptionList("baseinfo");
  for (const auto& name : *option) {
    baseParam.set<string>(name, config.get("baseinfo", name));
  }

  const auto blockConfig = config.getOptionList("block");
  for (const auto& name : *blockConfig) {
    blockParam.set<string>(name, config.get("block", name));
  }

  option = config.getOptionList("kdata");
  for (const auto& name : *option) {
    if (name == "convert") {
      kdataParam.set<bool>(name, config.getBool("kdata", name));
    } else {
      kdataParam.set<string>(name, config.get("kdata", name));
    }
  }

  for (const auto& ktype : KQuery::getBaseKTypeList()) {
    string lowKtype = ktype;
    to_lower(lowKtype);
    preloadParam.set<bool>(lowKtype,
                           config.getBool("preload", lowKtype, "False"));
    const string preloadLimit = fmt::format("{}_max", lowKtype);
    preloadParam.set<int64_t>(preloadLimit,
                              config.getInt("preload", preloadLimit, "4096"));
  }
}

}  // namespace hayaku
