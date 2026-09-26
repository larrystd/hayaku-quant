#pragma once

/*
 * MarketInfoTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include "common/database/SQLStatementBase.h"

namespace hayaku {

class MarketInfoTable {
 public:
  MarketInfoTable()
      : marketid_(0),
        last_date_(0),
        open_time1_(0),
        close_time1_(0),
        open_time2_(0),
        close_time2_(0) {}

  uint64_t id() const { return marketid_; }

  const string& market() const { return market_; }

  const string& name() const { return name_; }

  const string& description() const { return description_; }

  const string& code() const { return code_; }

  Datetime lastDate() const {
    HAYAKU_CHECK(last_date_ <= 99999999, "Invalid lastDate: {}!", last_date_);
    return Datetime(last_date_ * 10000LL);
  }

  TimeDelta openTime1() const { return _transTimeDelta(open_time1_); }

  TimeDelta closeTime1() const { return _transTimeDelta(close_time1_); }

  TimeDelta openTime2() const { return _transTimeDelta(open_time2_); }

  TimeDelta closeTime2() const { return _transTimeDelta(close_time2_); }

 private:
  TimeDelta _transTimeDelta(uint64_t time) const {
    int64_t hours = time / 100;
    HAYAKU_CHECK(hours >= 0 && hours <= 23, "Invalid time: {}!", time);
    int64_t mins = time - hours * 100;
    HAYAKU_CHECK(mins >= 0 && mins <= 59, "Invalid time: {}!", time);
    return TimeDelta(0, hours, mins);
  }

 public:
  static const char* getInsertSQL() {
    return "insert into `market` "
           "(`marketid`, `market`, `name`, `description`, `code`, `lastDate`,"
           " `openTime1`, `closeTime1`, `openTime2`, `closeTime2`) "
           "values (?,?,?,?,?,?,?,?,?,?)";
  }

  static const char* getUpdateSQL() {
    return "update `market` set `market`=?, `name`=?, `description`=?, "
           "`code`=?, `lastDate`=?, `openTime1`=?, `closeTime1`=?, "
           "`openTime2=`=?, `closeTime2`=? where `marketid`=?";
  }

  static const char* getSelectSQL() {
    return "select `marketid`,`market`,`name`, `description`, `code`, "
           "`lastDate`, "
           "`openTime1`, `closeTime1`, `openTime2`, `closeTime2` from `market`";
  }

  void save(const SQLStatementPtr& st) const {
    st->bind(0, marketid_, market_, name_, description_, code_, last_date_,
             open_time1_, close_time1_, open_time2_, close_time2_);
  }

  void update(const SQLStatementPtr& st) const {
    st->bind(0, market_, name_, description_, code_, last_date_,
             open_time1_, close_time1_, open_time2_, close_time2_, marketid_);
  }

  void load(const SQLStatementPtr& st) {
    st->getColumn(0, marketid_, market_, name_, description_, code_,
                  last_date_, open_time1_, close_time1_, open_time2_,
                  close_time2_);
  }

 private:
  uint64_t marketid_;
  string market_;
  string name_;
  string description_;
  string code_;
  uint64_t last_date_;
  uint64_t open_time1_;
  uint64_t close_time1_;
  uint64_t open_time2_;
  uint64_t close_time2_;
};

}  // namespace hayaku
