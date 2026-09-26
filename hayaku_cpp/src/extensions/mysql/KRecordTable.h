#pragma once

/*
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2020-9-27
 *      Author: fasiondog
 */

#include "common/database/SQLStatementBase.h"
#include "data/KQuery.h"
#include "data/KRecord.h"

namespace hayaku {

class KRecordTable {
 public:
  KRecordTable()
      : date_(0),
        open_(0.0),
        high_(0.0),
        low_(0.0),
        close_(0.0),
        amount_(0.0),
        count_(0.0) {}

  KRecordTable(const string& market, const string& code,
               const KQuery::KType& ktype)
      : db_name_(fmt::format("{}_{}", market, KQuery::getKTypeName(ktype))),
        code_(code),
        date_(0),
        open_(0.0),
        high_(0.0),
        low_(0.0),
        close_(0.0),
        amount_(0.0),
        count_(0.0) {
    // m_db_name = fmt::format("{}_{}", market, KQuery::getKTypeName(ktype));
    to_lower(db_name_);
  };

  KRecordTable(const string& market, const string& code,
               const KQuery::KType& ktype, const KRecord& record)
      : KRecordTable(market, code, ktype) {
    date_ = record.datetime.ymdhm();
    open_ = record.openPrice;
    high_ = record.highPrice;
    low_ = record.lowPrice;
    close_ = record.closePrice;
    count_ = record.transCount;
    amount_ = record.transAmount;
  }

  KRecordTable(const KRecordTable&) = default;
  KRecordTable& operator=(const KRecordTable&) = default;
  KRecordTable(KRecordTable&& rhs)
      : db_name_(std::move(rhs.db_name_)),
        code_(std::move(rhs.code_)),
        date_(rhs.date_),
        open_(rhs.open_),
        high_(rhs.high_),
        low_(rhs.low_),
        close_(rhs.close_),
        amount_(rhs.amount_),
        count_(rhs.count_) {}

  KRecordTable& operator=(KRecordTable&& rhs) {
    if (&rhs != this) {
      db_name_ = std::move(rhs.db_name_);
      code_ = std::move(rhs.code_);
      date_ = rhs.date_;
      open_ = rhs.open_;
      high_ = rhs.high_;
      low_ = rhs.low_;
      close_ = rhs.close_;
      amount_ = rhs.amount_;
      count_ = rhs.count_;
    }
    return *this;
  }

  Datetime date() const {
    return date_ == 0 ? Null<Datetime>() : Datetime((uint64_t)date_);
  }

  price_t open() const { return open_; }

  price_t high() const { return high_; }

  price_t low() const { return low_; }

  price_t close() const { return close_; }

  price_t amount() const { return amount_; }

  price_t count() const { return count_; }

  string str() const {
    return fmt::format(
        "KRecordTable({}(date), {}(open), {}(high), {}(low), {}(close), "
        "{}(amount), {}(count))",
        date_, open_, high_, low_, close_, amount_, count_);
  }

 public:
  string getInsertSQL() {
    return fmt::format(
        "insert into `{}`.`{}` "
        "(`date`, `open`, `high`, `low`, `close`, `amount`, `count`) "
        "values (?,?,?,?,?,?,?)",
        db_name_, code_);
  }

  string getUpdateSQL() {
    return fmt::format(
        "update `{}`.`{}` set `open`=?, `high`=?, `low`=?, "
        "`close`=?, `amount`=? `count`=? where `date`=?",
        db_name_, code_);
  }

  string getSelectSQL() {
    return fmt::format(
        "select `date`,`open`,`high`, `low`, `close`, `amount`, `count` from "
        "`{}`.`{}`",
        db_name_, code_);
  }

  string getSelectSQLNoDB() {
    return fmt::format(
        "select `date`,`open`,`high`, `low`, `close`, `amount`, `count` from "
        "`{}`",
        code_);
  }

  void save(const SQLStatementPtr& st) const {
    st->bind(0, date_, open_, high_, low_, close_, amount_, count_);
  }

  void update(const SQLStatementPtr& st) const {
    st->bind(0, open_, high_, low_, close_, amount_, count_);
  }

  void load(const SQLStatementPtr& st) {
    st->getColumn(0, date_, open_, high_, low_, close_, amount_, count_);
  }

 private:
  string db_name_;
  string code_;

  int64_t date_;
  price_t open_;
  price_t high_;
  price_t low_;
  price_t close_;
  price_t amount_;
  price_t count_;
};

}  // namespace hayaku
