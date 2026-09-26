#pragma once

/*
 * StockTypeInfo.h
 *
 *  Created on: 2011-12-12
 *      Author: fasiondog
 */

#include "MarketTypes.h"

namespace hayaku {

#define STOCKTYPE_BLOCK 0    /// Block (sector)
#define STOCKTYPE_A 1        /// A-share
#define STOCKTYPE_INDEX 2    /// Index
#define STOCKTYPE_B 3        /// B-share
#define STOCKTYPE_FUND 4     /// Fund
#define STOCKTYPE_ETF 5      /// ETF
#define STOCKTYPE_ND 6       /// Treasury bond
#define STOCKTYPE_BOND 7     /// Bond
#define STOCKTYPE_GEM 8      /// ChiNext (Growth Enterprise Market)
#define STOCKTYPE_START 9    /// STAR Market
#define STOCKTYPE_CRYPTO 10  /// Cryptocurrency
#define STOCKTYPE_A_BJ \
  11  /// Beijing Stock Exchange (its minimum trading unit is not 100 shares)

#define STOCKTYPE_TMP 999  /// Used for a temporary Stock

/**
 * Security type information
 * @ingroup StockManage
 */
class HAYAKU_API StockTypeInfo {
 public:
  /** Default constructor, returns Null<StockTypeInfo>() */
  StockTypeInfo();
  StockTypeInfo(uint32_t, const string&, price_t, price_t, int, double, double);

  StockTypeInfo(const StockTypeInfo&) = default;
  StockTypeInfo& operator=(const StockTypeInfo&) = default;

  StockTypeInfo(StockTypeInfo&&) noexcept;
  StockTypeInfo& operator=(StockTypeInfo&&) noexcept;

  /** Get the security type */
  uint32_t type() const noexcept { return type_; }

  /** Get the description of the security type */
  const string& description() const noexcept { return description_; }

  /** Get the minimum tick size */
  price_t tick() const noexcept { return tick_; }

  /** Price per tick */
  price_t tickValue() const noexcept { return tick_value_; }

  /** Price per unit = tickValue / tick */
  price_t unit() const noexcept { return unit_; }

  /** Get the price precision */
  int precision() const noexcept { return precision_; }

  /** Get the minimum trade quantity per order */
  double minTradeNumber() const noexcept { return min_trade_number_; }

  /** Get the maximum trade quantity per order */
  double maxTradeNumber() const noexcept { return max_trade_number_; }

  /** Used by __str__ of python only */
  string toString() const;

 private:
  uint32_t type_;       // Security type
  string description_;  // Description
  price_t tick_;        // Minimum tick size
  price_t tick_value_;   // Price of every tick
  price_t
      unit_;  // Price per minimum change, i.e. unit price = tickValue / tick
  int precision_;          // Price precision
  double min_trade_number_;  // Minimum trade quantity per order
  double max_trade_number_;  // Maximum trade quantity per order
};

/**
 * Output the security type information, e.g. StockTypeInfo(type, description,
 * tick, precision, minTradeNumber, maxTradeNumber)
 * @ingroup StockManage
 */
HAYAKU_API std::ostream& operator<<(std::ostream&, const StockTypeInfo&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockTypeInfo&, const StockTypeInfo&);
bool operator!=(const StockTypeInfo&, const StockTypeInfo&);

/** Equal comparison */
inline bool operator==(const StockTypeInfo& m1, const StockTypeInfo& m2) {
  return m1.type() == m2.type();
}

/** Unequal comparison */
inline bool operator!=(const StockTypeInfo& m1, const StockTypeInfo& m2) {
  return m1.type() != m2.type();
}

}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::StockTypeInfo> : ostream_formatter {};
#endif
