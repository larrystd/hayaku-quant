#pragma once

/*
 * StockTypeInfoTable.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-8-11
 *      Author: fasiondog
 */

#include "common/database/SQLStatementBase.h"

namespace hayaku {

class StockTypeInfoTable {
 public:
  StockTypeInfoTable()
      : id_(0),
        type_(Null<uint32_t>()),
        precision_(0),
        tick_(0.0),
        tick_value_(0.0),
        min_trade_number_(0),
        max_trade_number_(0) {}

  int64_t id() const { return id_; }

  uint32_t type() const { return type_; }

  uint32_t precision() const { return precision_; }

  double tick() const { return tick_; }

  double tickValue() const { return tick_value_; }

  double minTradeNumber() const { return min_trade_number_; }

  double maxTradeNumber() const { return max_trade_number_; }

  const string& description() const { return description_; }

 public:
  static const char* getInsertSQL() {
    return "insert into `stocktypeinfo` "
           "(`id`, `type`, `precision`, `tick`, `tickValue`, "
           "`minTradeNumber`, `maxTradeNumber`, `description`) "
           "values (?,?,?,?,?,?,?,?)";
  }

  static const char* getUpdateSQL() {
    return "update `stocktypeinfo` set `type`=?, `precision`=?, `tick`=?, "
           "`tickValue`=?, `minTradeNumber`=?, `maxTradeNumber`=?"
           "`description`=? where `id`=?";
  }

  static const char* getSelectSQL() {
    return "select `id`,`type`,`precision`, `tick`, `tickValue`, "
           "`minTradeNumber`, `maxTradeNumber`, `description` from "
           "`stocktypeinfo`";
  }

  void save(const SQLStatementPtr& st) const {
    st->bind(0, id_, type_, precision_, tick_, tick_value_,
             min_trade_number_, max_trade_number_, description_);
  }

  void update(const SQLStatementPtr& st) const {
    st->bind(0, type_, precision_, tick_, tick_value_, min_trade_number_,
             max_trade_number_, description_, id_);
  }

  void load(const SQLStatementPtr& st) {
    st->getColumn(0, id_, type_, precision_, tick_, tick_value_,
                  min_trade_number_, max_trade_number_, description_);
  }

 private:
  int64_t id_;
  uint32_t type_;          // Security type
  uint32_t precision_;     // Price precision
  double tick_;            // Minimum tick size
  double tick_value_;       // Price of every tick
  double min_trade_number_;  // Minimum trade quantity per order
  double max_trade_number_;  // Maximum trade quantity per order
  string description_;     // Description
};

}  // namespace hayaku
