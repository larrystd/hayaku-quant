/*
 * StockTypeInfo.cpp
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#include "StockTypeInfo.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const StockTypeInfo& stockTypeInfo) {
  if (Null<StockTypeInfo>() == stockTypeInfo) {
    os << "StockTypeInfo()";
    return os;
  }

  string sp(", ");
  os << "StockTypeInfo(" << stockTypeInfo.type() << sp
     << stockTypeInfo.description() << sp << stockTypeInfo.tick() << sp
     << stockTypeInfo.tickValue() << sp << stockTypeInfo.unit() << sp
     << stockTypeInfo.precision() << sp << stockTypeInfo.minTradeNumber() << sp
     << stockTypeInfo.maxTradeNumber() << ")";
  return os;
}

string StockTypeInfo::toString() const {
  std::stringstream os;
  if (Null<StockTypeInfo>() == *this) {
    os << "StockTypeInfo()";
    return os.str();
  }

  string sp(", ");
  os << "StockTypeInfo(" << type_ << sp << description_ << sp << tick_ << sp
     << tick_value_ << sp << unit_ << sp << precision_ << sp
     << min_trade_number_ << sp << max_trade_number_ << ")";
  return os.str();
}

StockTypeInfo::StockTypeInfo()
    : type_(Null<uint32_t>()),
      tick_(0.0),
      tick_value_(0.0),
      unit_(1.0),
      precision_(0),
      min_trade_number_(0),
      max_trade_number_(0) {}

StockTypeInfo::StockTypeInfo(uint32_t type, const string& description,
                             price_t tick, price_t tickValue, int precision,
                             double minTradeNumber, double maxTradeNumber)
    : type_(type),
      description_(description),
      tick_(tick),
      tick_value_(tickValue),
      precision_(precision),
      min_trade_number_(minTradeNumber),
      max_trade_number_(maxTradeNumber) {
  if (tick_ == 0.0) {
    unit_ = 1.0;
    HAYAKU_WARN("tick should not be zero!");
  } else {
    unit_ = tick_value_ / tick_;
  }
}

StockTypeInfo::StockTypeInfo(StockTypeInfo&& rhs) noexcept
    : type_(rhs.type_),
      description_(std::move(rhs.description_)),
      tick_(rhs.tick_),
      tick_value_(rhs.tick_value_),
      unit_(rhs.unit_),
      precision_(rhs.precision_),
      min_trade_number_(rhs.min_trade_number_),
      max_trade_number_(rhs.max_trade_number_) {}

StockTypeInfo& StockTypeInfo::operator=(StockTypeInfo&& rhs) noexcept {
  HAYAKU_IF_RETURN(this == &rhs, *this);
  type_ = rhs.type_;
  description_ = std::move(rhs.description_);
  tick_ = rhs.tick_;
  tick_value_ = rhs.tick_value_;
  unit_ = rhs.unit_;
  precision_ = rhs.precision_;
  min_trade_number_ = rhs.min_trade_number_;
  max_trade_number_ = rhs.max_trade_number_;
  return *this;
};

}  // namespace hayaku
