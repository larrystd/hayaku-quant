#pragma once

/*
 * MarketInfo.h
 *
 *  Created on: 2011-12-5
 *      Author: fasiondog
 */

#include "MarketTypes.h"

namespace hayaku {

/**
 * Market information record
 * @ingroup StockManage
 */
class MarketInfo {
 public:
  /** Default constructor, returns Null<MarketInfo>() */
  MarketInfo();

  /**
   * @param market market abbreviation
   * @param name market name
   * @param description market description
   * @param code base index: used to read the trading calendar of the market
   * @param lastDate current last date of the market
   * @param openTime1 start time of trading session 1
   * @param closeTime1 end time of trading session 1
   * @param openTime2 start time of trading session 2
   * @param closeTime2 end time of trading session 2
   */
  MarketInfo(const string& market, const string& name,
             const string& description, const string& code,
             const Datetime& lastDate, TimeDelta openTime1,
             TimeDelta closeTime1, TimeDelta openTime2, TimeDelta closeTime2);

  MarketInfo(const MarketInfo&) = default;
  MarketInfo& operator=(const MarketInfo&) = default;

  MarketInfo(MarketInfo&&) noexcept;
  MarketInfo& operator=(MarketInfo&&) noexcept;

  /** Get the market abbreviation */
  const string& market() const noexcept { return market_; }

  /** Get the market name */
  const string& name() const noexcept { return name_; }

  /** Get the market description */
  const string& description() const noexcept { return description_; }

  /** Get the index code corresponding to the market */
  const string& code() const noexcept { return code_; }

  /** Get the last update date of the market data */
  Datetime lastDate() const noexcept { return last_date_; }

  /** Opening time of session 1 */
  TimeDelta openTime1() const noexcept { return open_time1_; }

  /** Closing time of session 1 */
  TimeDelta closeTime1() const noexcept { return close_time1_; }

  /** Opening time of session 2 */
  TimeDelta openTime2() const noexcept { return open_time2_; }

  /** Closing time of session 2 */
  TimeDelta closeTime2() const noexcept { return close_time2_; }

  /** Used by __str__ of python only */
  string toString() const;

 private:
  string market_;       // Market identifier
  string name_;         // Market name
  string description_;  // Description
  string code_;  // Index code of the market, used to get the trading calendar
  Datetime last_date_;     // Current last date of the market
  TimeDelta open_time1_;   // Morning opening time
  TimeDelta close_time1_;  // Morning closing time
  TimeDelta open_time2_;   // Afternoon opening time
  TimeDelta close_time2_;  // Afternoon closing time
};

/**
 * Output the market information, e.g.:
 * MarketInfo(SH, Shanghai Stock Exchange, Shanghai market, 000001, 2011-Dec-06
 * 00:00:00)
 * @ingroup StockManage
 */
std::ostream& operator<<(std::ostream&, const MarketInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const MarketInfo&, const MarketInfo&);
bool operator!=(const MarketInfo&, const MarketInfo&);

/** Equal comparison */
inline bool operator==(const MarketInfo& m1, const MarketInfo& m2) {
  return m1.market() == m2.market();
}

/** Unequal comparison */
inline bool operator!=(const MarketInfo& m1, const MarketInfo& m2) {
  return m1.market() != m2.market();
}

}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::MarketInfo> : ostream_formatter {};
#endif
