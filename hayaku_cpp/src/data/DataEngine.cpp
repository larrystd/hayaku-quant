/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "DataEngine.h"

#include "DataRuntime.h"

namespace hayaku {

DataEngine::DataEngine(std::shared_ptr<std::atomic_bool> active)
    : active_(std::move(active)) {}

void DataEngine::_attach(DataRuntime& backend) noexcept {
  backend_ = &backend;
}

DataRuntime& DataEngine::_backend() const {
  HAYAKU_CHECK(active_ && active_->load(std::memory_order_acquire),
               "DataEngine belongs to a closed HayakuSession");
  HAYAKU_CHECK(backend_, "DataEngine is not attached to a data runtime");
  return *backend_;
}

bool DataEngine::ready() const { return _backend().dataReady(); }

bool DataEngine::initializing() const { return _backend().initializing(); }

void DataEngine::waitReady() const { _backend().waitDataReady(); }

size_t DataEngine::size() const { return _backend().size(); }

Stock DataEngine::getStock(const string& marketCode) const {
  return _backend().getStock(marketCode);
}

StockList DataEngine::getStockList() const { return _backend().getStockList(); }

KData DataEngine::getKData(const string& marketCode,
                           const KQuery& query) const {
  return _backend().getStock(marketCode).getKData(query);
}

MarketInfo DataEngine::getMarketInfo(const string& market) const {
  return _backend().getMarketInfo(market);
}

Stock DataEngine::getMarketStock(const string& market) const {
  return _backend().getMarketStock(market);
}

StringList DataEngine::getMarketList() const {
  return _backend().getAllMarket();
}

StockTypeInfo DataEngine::getStockTypeInfo(uint32_t type) const {
  return _backend().getStockTypeInfo(type);
}

vector<StockTypeInfo> DataEngine::getStockTypeInfoList() const {
  return _backend().getStockTypeInfoList();
}

StringList DataEngine::getBlockCategoryList() const {
  return _backend().getAllCategory();
}

Block DataEngine::getBlock(const string& category, const string& name) const {
  return _backend().getBlock(category, name);
}

BlockList DataEngine::getBlockList(const string& category) const {
  return _backend().getBlockList(category);
}

BlockList DataEngine::getStockBelongs(const Stock& stock,
                                      const string& category) const {
  return _backend().getStockBelongs(stock, category);
}

DatetimeList DataEngine::getTradingCalendar(const KQuery& query,
                                            const string& market) const {
  return _backend().getTradingCalendar(query, market);
}

DatetimeList DataEngine::getTradingCalendar(const StockList& stocks,
                                            const KQuery& query) const {
  return _backend().getTradingCalendar(stocks, query);
}

bool DataEngine::isHoliday(const Datetime& datetime) const {
  return _backend().isHoliday(datetime);
}

bool DataEngine::isTradingHours(const Datetime& datetime,
                                const string& market) const {
  return _backend().isTradingHours(datetime, market);
}

const ZhBond10List& DataEngine::getZhBond10() const {
  return _backend().getZhBond10();
}

StockWeightList DataEngine::getStockWeightList(const Stock& stock,
                                               Datetime start,
                                               Datetime end) const {
  return _backend().getStockWeightList(stock, start, end);
}

const string& DataEngine::getHistoryFinanceFieldName(size_t index) const {
  return _backend().getHistoryFinanceFieldName(index);
}

size_t DataEngine::getHistoryFinanceFieldIndex(const string& name) const {
  return _backend().getHistoryFinanceFieldIndex(name);
}

vector<std::pair<size_t, string>> DataEngine::getHistoryFinanceAllFields()
    const {
  return _backend().getHistoryFinanceAllFields();
}

vector<HistoryFinanceInfo> DataEngine::getHistoryFinance(const Stock& stock,
                                                         Datetime start,
                                                         Datetime end) const {
  return _backend().getHistoryFinance(stock, start, end);
}

}  // namespace hayaku
