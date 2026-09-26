/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-05-06
 *      Author: fasiondog
 */

#include "KDataToClickHouseImporter.h"

#include "application/PluginRuntime.h"
#include "application/plugins/PluginIds.h"

namespace hayaku {

KDataToClickHouseImporter::KDataToClickHouseImporter() {
  plugin_ = getPlugin<ImportKDataToClickHousePluginInterface>(
      HAYAKU_PLUGIN_IMPORTKDATATOCLICKHOUSE);
  if (!plugin_) {
    HAYAKU_ERROR(
        htr("Can't find {} plugin!", HAYAKU_PLUGIN_IMPORTKDATATOCLICKHOUSE));
  }
}

KDataToClickHouseImporter::~KDataToClickHouseImporter() {}

bool KDataToClickHouseImporter::setConfig(const string& host, int port,
                                          const string& user, const string& pwd,
                                          const string& baseinfo_db) {
  return plugin_ ? plugin_->setConfig(host, port, user, pwd, baseinfo_db)
                  : false;
}

Datetime KDataToClickHouseImporter::getLastDatetime(
    const string& market, const string& code, const KQuery::KType& ktype) {
  return plugin_ ? plugin_->getLastDatetime(market, code, ktype)
                  : Null<Datetime>();
}

void KDataToClickHouseImporter::addKRecordList(const string& market,
                                               const string& code,
                                               const vector<KRecord>& krecords,
                                               const KQuery::KType& ktype) {
  if (plugin_) {
    plugin_->addKRecordList(market, code, krecords, ktype);
  }
}

void KDataToClickHouseImporter::updateIndex(const string& market,
                                            const string& code,
                                            const KQuery::KType& ktype) {
  if (plugin_) {
    plugin_->updateIndex(market, code, ktype);
  }
}

void KDataToClickHouseImporter::remove(const string& market, const string& code,
                                       const KQuery::KType& ktype,
                                       Datetime start) {
  if (plugin_) {
    plugin_->remove(market, code, ktype, start);
  }
}

void KDataToClickHouseImporter::addTimeLineList(const string& market,
                                                const string& code,
                                                const TimeLineList& timeline) {
  if (plugin_) {
    plugin_->addTimeLineList(market, code, timeline);
  }
}

void KDataToClickHouseImporter::addTransList(const string& market,
                                             const string& code,
                                             const TransRecordList& translist) {
  if (plugin_) {
    plugin_->addTransList(market, code, translist);
  }
}

bool KDataToClickHouseImporter::addMarket(const string& market,
                                          const string& name,
                                          const string& description,
                                          const string& index_code,
                                          uint64_t open1, uint64_t close1,
                                          uint64_t open2, uint64_t close2) {
  return plugin_ ? plugin_->addMarket(market, name, description, index_code,
                                        open1, close1, open2, close2)
                  : false;
}

bool KDataToClickHouseImporter::addStockType(
    uint32_t type_id, const string& description, uint32_t precision,
    double tick, double tick_value, double min_trade, double max_trade) {
  return plugin_
             ? plugin_->addStockType(type_id, description, precision, tick,
                                      tick_value, min_trade, max_trade)
             : false;
}

}  // namespace hayaku
