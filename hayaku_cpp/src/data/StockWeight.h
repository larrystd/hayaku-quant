#pragma once

/*
 * StockWeight.h
 *
 *  Created on: 2011-12-9
 *      Author: fasiondog
 */

#include "MarketTypes.h"

namespace hayaku {

/**
 * Ex-rights/ex-dividend data structure
 * @ingroup StockManage
 */
class HAYAKU_API StockWeight {
 public:
  /** Default constructor, returns Null<StockWeight>() */
  StockWeight() = default;

  explicit StockWeight(const Datetime& datetime);

  StockWeight(const Datetime& datetime, price_t countAsGift,
              price_t countForSell, price_t priceForSell, price_t bonus,
              price_t increasement, price_t totalCount, price_t freeCount,
              price_t suogu);

  /** Ex-rights/ex-dividend date */
  Datetime datetime() const noexcept { return datetime_; }

  /** Bonus shares per 10 shares (X shares given per 10 shares) */
  price_t countAsGift() const noexcept { return count_as_gift_; }

  /** Rights shares per 10 shares (X shares allotted per 10 shares) */
  price_t countForSell() const noexcept { return count_for_sell_; }

  /** Rights issue price */
  price_t priceForSell() const noexcept { return price_for_sell_; }

  /** Dividend per 10 shares */
  price_t bonus() const noexcept { return bonus_; }

  /** Capitalized shares per 10 shares (X shares converted per 10 shares) */
  price_t increasement() const noexcept { return increasement_; }

  /** Total share capital (in units of 10 thousand shares) */
  price_t totalCount() const noexcept { return total_count_; }

  /** Outstanding shares (in units of 10 thousand shares) */
  price_t freeCount() const noexcept { return free_count_; }

  /** Share expansion/contraction ratio (suogu) */
  price_t suogu() const noexcept { return suogu_; }

 private:
  Datetime datetime_;         // Ex-rights/ex-dividend date
  price_t count_as_gift_{0.};   // Bonus shares per 10 shares
  price_t count_for_sell_{0.};  // Rights shares per 10 shares
  price_t price_for_sell_{0.};  // Rights issue price
  price_t bonus_{0.};         // Dividend per 10 shares
  price_t increasement_{0.};  // Capitalized shares per 10 shares
  price_t total_count_{0.};    // Total share capital (10 thousand shares)
  price_t free_count_{0.};     // Outstanding shares (10 thousand shares)
  price_t suogu_{0.};         // Share expansion/contraction ratio
};

/** @ingroup StockManage */
typedef vector<StockWeight> StockWeightList;

/**
 * Output the ex-rights/ex-dividend information, e.g.: Weight(datetime,
 * countAsGift, countForSell, priceForSell, bonus, increasement, totalCount,
 * freeCount)
 * @ingroup StockManage
 */
HAYAKU_API std::ostream& operator<<(std::ostream&, const StockWeight&);

///////////////////////////////////////////////////////////////////////////////
//
// Relational comparison functions
//
///////////////////////////////////////////////////////////////////////////////
bool operator==(const StockWeight&, const StockWeight&);
bool operator!=(const StockWeight&, const StockWeight&);
bool operator>(const StockWeight&, const StockWeight&);
bool operator<(const StockWeight&, const StockWeight&);
bool operator>=(const StockWeight&, const StockWeight&);
bool operator<=(const StockWeight&, const StockWeight&);

/* Equal comparison, judged by the date only */
inline bool operator==(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() == m2.datetime();
}

/* Unequal comparison, judged by the date only */
inline bool operator!=(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() != m2.datetime();
}

/* Greater-than comparison, judged by the date only */
inline bool operator>(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() > m2.datetime();
}

/* Less-than comparison, judged by the date only */
inline bool operator<(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() < m2.datetime();
}

/* Greater-than-or-equal comparison, judged by the date only */
inline bool operator>=(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() >= m2.datetime();
}

/* Less-than-or-equal comparison, judged by the date only */
inline bool operator<=(const StockWeight& m1, const StockWeight& m2) {
  return m1.datetime() <= m2.datetime();
}

/** @} */
}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::StockWeight> : ostream_formatter {};
#endif
