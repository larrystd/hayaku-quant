/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <fmt/format.h>

#include "data/KQuery.h"
#include "common/ini_parser/IniParser.h"
#include "common/os.h"
#include "ConfigLoader.h"

namespace hku {

void getConfigFromIni(const string& configFileName, Parameter& baseParam, Parameter& blockParam,
                      Parameter& kdataParam, Parameter& preloadParam, Parameter& hkuParam) {
    IniParser config;
    config.read(configFileName);

    hkuParam.set<string>("tmpdir", config.get("hikyuu", "tmpdir", "."));
    hkuParam.set<string>("datadir", config.get("hikyuu", "datadir", "."));
    hkuParam.set<string>("quotation_server",
                         config.get("hikyuu", "quotation_server", "ipc:///tmp/hikyuu_real.ipc"));
    hkuParam.set<string>("reload_time", config.get("hikyuu", "reload_time", "00:00"));
    hkuParam.set<string>("lazy_preload", config.get("hikyuu", "lazy_preload", "False"));
    hkuParam.set<bool>("load_stock_weight",
                       config.getBool("hikyuu", "load_stock_weight", "True"));
    hkuParam.set<bool>("load_history_finance",
                       config.getBool("hikyuu", "load_history_finance", "True"));
    hkuParam.set<string>("plugindir", config.get("hikyuu", "plugindir",
                                                 fmt::format("{}/.hikyuu/plugin", getUserDir())));

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
        preloadParam.set<bool>(lowKtype, config.getBool("preload", lowKtype, "False"));
        const string preloadLimit = fmt::format("{}_max", lowKtype);
        preloadParam.set<int64_t>(preloadLimit,
                                  config.getInt("preload", preloadLimit, "4096"));
    }
}

}  // namespace hku
