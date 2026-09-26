#pragma once

/*
 * TransRecord.h
 *
 *  Created on: 2019-2-10
 *      Author: fasiondog
 */

#include "MarketTypes.h"

namespace hayaku {

/**
 * Tick (sub-transaction) record
 * @ingroup StockManage
 */
class TransRecord {
 public:
  Datetime datetime;  // Time
  price_t price;      // Average transaction price
  price_t vol;        // Trading volume
  int32_t direct;  // Order direction: 1 -- sell, 0 -- buy, 2 -- call auction,
                   // others unknown

  TransRecord();
  TransRecord(const Datetime& datetime, price_t price, price_t vol,
              int32_t direct);

  bool isValid() const noexcept;
};

/**
 * Time-sharing (tick) line
 * @ingroup StockManage
 */
typedef vector<TransRecord> TransList;
typedef vector<TransRecord> TransRecordList;

/**
 * Output the TransRecord information, e.g. TimeSharingRecord(datetime, price,
 * vol)
 * @ingroup StockManage
 */
std::ostream& operator<<(std::ostream&, const TransRecord&);

/**
 * Output the TransList information
 * @details
 * <pre>
 * TimeLine{
 *   size : 738501
 *   start: YYYY-MM-DD hh:mm:ss
 *   last : YYYY-MM-DD hh:mm:ss
 *  }
 * </pre>
 * @ingroup StockManage
 */
std::ostream& operator<<(std::ostream& os, const TransList&);

/**
 * Compare whether two TransRecord are equal, generally used in tests only
 * @ingroup StockManage
 */
bool operator==(const TransRecord& d1, const TransRecord& d2);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::TransRecord> : ostream_formatter {};
#endif
