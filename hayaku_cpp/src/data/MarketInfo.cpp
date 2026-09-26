/*
 * MarketInfo.cpp
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketInfo.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const MarketInfo& market) {
  os << market.toString();
  return os;
}

string MarketInfo::toString() const {
  std::stringstream os;
  if (market_ == "") {
    os << "MarketInfo()";
    return os.str();
  }

  string sp(", ");
  os << "MarketInfo(" << market_ << sp << name_ << sp << description_ << sp
     << code_ << sp << last_date_ << sp << open_time1_.hours() << ":"
     << open_time1_.minutes() << sp << close_time1_.hours() << ":"
     << close_time1_.minutes() << sp << open_time2_.hours() << ":"
     << open_time2_.minutes() << sp << close_time2_.hours() << ":"
     << close_time2_.minutes() << ")";
  return os.str();
}

MarketInfo::MarketInfo() {}

MarketInfo::MarketInfo(const string& market, const string& name,
                       const string& description, const string& code,
                       const Datetime& lastDate, TimeDelta openTime1,
                       TimeDelta closeTime1, TimeDelta openTime2,
                       TimeDelta closeTime2)
    : market_(market),
      name_(name),
      description_(description),
      code_(code),
      last_date_(lastDate),
      open_time1_(openTime1),
      close_time1_(closeTime1),
      open_time2_(openTime2),
      close_time2_(closeTime2) {}

MarketInfo::MarketInfo(MarketInfo&& rhs) noexcept
    : market_(std::move(rhs.market_)),
      name_(std::move(rhs.name_)),
      description_(std::move(rhs.description_)),
      code_(std::move(rhs.code_)),
      last_date_(std::move(rhs.last_date_)),
      open_time1_(std::move(rhs.open_time1_)),
      close_time1_(std::move(rhs.close_time1_)),
      open_time2_(std::move(rhs.open_time2_)),
      close_time2_(std::move(rhs.close_time2_)) {}

MarketInfo& MarketInfo::operator=(MarketInfo&& rhs) noexcept {
  if (this != &rhs) {
    market_ = std::move(rhs.market_);
    name_ = std::move(rhs.name_);
    description_ = std::move(rhs.description_);
    code_ = std::move(rhs.code_);
    last_date_ = std::move(rhs.last_date_);
    open_time1_ = std::move(rhs.open_time1_);
    close_time1_ = std::move(rhs.close_time1_);
    open_time2_ = std::move(rhs.open_time2_);
    close_time2_ = std::move(rhs.close_time2_);
  }
  return *this;
}

}  // namespace hayaku
