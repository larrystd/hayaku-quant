#pragma once

/*
 * MySQLBaseInfoDriver.h
 *
 *  Created on: 2014-8-27
 *      Author: fasiondog
 */


#include "common/database/ResourcePool.h"
#include "extensions/mysql/MySQLConnect.h"
#include "data/storage/BaseInfoDriver.h"

namespace hayaku {

class MySQLBaseInfoDriver : public BaseInfoDriver {
public:
    MySQLBaseInfoDriver();
    virtual ~MySQLBaseInfoDriver() override;

    virtual bool _init() override;
    virtual vector<StockInfo> getAllStockInfo() override;
    virtual vector<MarketInfo> getAllMarketInfo() override;
    virtual vector<StockTypeInfo> getAllStockTypeInfo() override;

    virtual Parameter getFinanceInfo(const string& market, const string& code) override;
    virtual StockWeightList getStockWeightList(const string& market, const string& code,
                                               Datetime start, Datetime end) override;
    virtual unordered_map<string, StockWeightList> getAllStockWeightList() override;
    virtual MarketInfo getMarketInfo(const string& market) override;
    virtual StockTypeInfo getStockTypeInfo(uint32_t type) override;
    virtual StockInfo getStockInfo(string market, const string& code) override;
    virtual std::unordered_set<Datetime> getAllHolidays() override;
    virtual ZhBond10List getAllZhBond10() override;

    virtual vector<std::pair<size_t, string>> getHistoryFinanceField() override;
    virtual vector<HistoryFinanceInfo> getHistoryFinance(const string& market, const string& code,
                                                         Datetime start, Datetime end) override;

private:
    ResourcePool<MySQLConnect>* m_pool;
};

} /* namespace hayaku */
