#pragma once

/*
 * SQLiteBaseInfoDriver.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include "common/database/ResourcePool.h"
#include "common/database/SQLiteConnect.h"
#include "data/storage/BaseInfoDriver.h"

namespace hayaku {

class SQLiteBaseInfoDriver : public BaseInfoDriver {
 public:
  SQLiteBaseInfoDriver();
  virtual ~SQLiteBaseInfoDriver() override;

  virtual bool _init() override;
  virtual vector<MarketInfo> getAllMarketInfo() override;
  virtual vector<StockTypeInfo> getAllStockTypeInfo() override;

  virtual Parameter getFinanceInfo(const string& market,
                                   const string& code) override;
  virtual StockWeightList getStockWeightList(const string& market,
                                             const string& code, Datetime start,
                                             Datetime end) override;
  virtual unordered_map<string, StockWeightList> getAllStockWeightList()
      override;
  virtual MarketInfo getMarketInfo(const string& market) override;
  virtual StockTypeInfo getStockTypeInfo(uint32_t type) override;
  virtual StockInfo getStockInfo(string market, const string& code) override;
  virtual vector<StockInfo> getAllStockInfo() override;
  virtual std::unordered_set<Datetime> getAllHolidays() override;
  virtual ZhBond10List getAllZhBond10() override;

  virtual vector<std::pair<size_t, string>> getHistoryFinanceField() override;
  virtual vector<HistoryFinanceInfo> getHistoryFinance(const string& market,
                                                       const string& code,
                                                       Datetime start,
                                                       Datetime end) override;

 private:
  // The database instance of the stock basic information
  ResourcePool<SQLiteConnect>* m_pool;
};

} /* namespace hayaku */
