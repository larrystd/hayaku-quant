/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Stable facade for read-only market data access.
 */

#pragma once
#ifndef HIKYUU_DATA_DATAENGINE_H
#define HIKYUU_DATA_DATAENGINE_H

#include <atomic>
#include <memory>

#include "data/Block.h"
#include "data/HistoryFinanceInfo.h"
#include "data/KData.h"
#include "data/MarketInfo.h"
#include "data/StockTypeInfo.h"
#include "data/StockWeight.h"
#include "data/ZhBond10.h"

namespace hku {

class HikyuuSession;
class DataRuntime;

class HKU_API DataEngine {
public:
    DataEngine(const DataEngine&) = delete;
    DataEngine& operator=(const DataEngine&) = delete;
    DataEngine(DataEngine&&) noexcept = default;
    DataEngine& operator=(DataEngine&&) noexcept = default;
    ~DataEngine() = default;

    [[nodiscard]] bool ready() const;
    [[nodiscard]] bool initializing() const;
    void waitReady() const;

    [[nodiscard]] size_t size() const;
    [[nodiscard]] Stock getStock(const string& marketCode) const;
    [[nodiscard]] StockList getStockList() const;
    [[nodiscard]] KData getKData(const string& marketCode, const KQuery& query) const;

    [[nodiscard]] MarketInfo getMarketInfo(const string& market) const;
    [[nodiscard]] Stock getMarketStock(const string& market) const;
    [[nodiscard]] StringList getMarketList() const;
    [[nodiscard]] StockTypeInfo getStockTypeInfo(uint32_t type) const;
    [[nodiscard]] vector<StockTypeInfo> getStockTypeInfoList() const;

    [[nodiscard]] StringList getBlockCategoryList() const;
    [[nodiscard]] Block getBlock(const string& category, const string& name) const;
    [[nodiscard]] BlockList getBlockList(const string& category = "") const;
    [[nodiscard]] BlockList getStockBelongs(const Stock& stock, const string& category = "") const;

    [[nodiscard]] DatetimeList getTradingCalendar(const KQuery& query,
                                                  const string& market = "SH") const;
    [[nodiscard]] DatetimeList getTradingCalendar(const StockList& stocks,
                                                  const KQuery& query) const;
    [[nodiscard]] bool isHoliday(const Datetime& datetime) const;
    [[nodiscard]] bool isTradingHours(const Datetime& datetime, const string& market = "SH") const;

    [[nodiscard]] const ZhBond10List& getZhBond10() const;
    [[nodiscard]] StockWeightList getStockWeightList(const Stock& stock, Datetime start,
                                                     Datetime end) const;
    [[nodiscard]] const string& getHistoryFinanceFieldName(size_t index) const;
    [[nodiscard]] size_t getHistoryFinanceFieldIndex(const string& name) const;
    [[nodiscard]] vector<std::pair<size_t, string>> getHistoryFinanceAllFields() const;
    [[nodiscard]] vector<HistoryFinanceInfo> getHistoryFinance(const Stock& stock, Datetime start,
                                                               Datetime end) const;

private:
    friend class HikyuuSession;

    explicit DataEngine(std::shared_ptr<std::atomic_bool> active);
    void _attach(DataRuntime& backend) noexcept;
    [[nodiscard]] DataRuntime& _backend() const;

private:
    std::shared_ptr<std::atomic_bool> m_active;
    DataRuntime* m_backend{nullptr};
};

}  // namespace hku

#endif /* HIKYUU_DATA_DATAENGINE_H */
