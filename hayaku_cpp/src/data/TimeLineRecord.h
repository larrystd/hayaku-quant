#pragma once

/*
 * TimeLineRecord.h
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include "MarketTypes.h"

namespace hayaku {

/**
 * Time-sharing (intraday) line record
 * @ingroup StockManage
 */
class TimeLineRecord {
 public:
  Datetime datetime;
  price_t price;
  price_t vol;

  TimeLineRecord();
  TimeLineRecord(const Datetime& datetime, price_t price, price_t vol);

  bool isValid() const noexcept;
};

/**
 * Time-sharing (intraday) line
 * @ingroup StockManage
 */
typedef vector<TimeLineRecord> TimeLineList;

/**
 * Output the TimeLineRecord information, e.g. TimeSharingRecord(datetime,
 * price, vol)
 * @ingroup StockManage
 */
std::ostream& operator<<(std::ostream&, const TimeLineRecord&);

/**
 * Output the TimeLine information
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
std::ostream& operator<<(std::ostream& os, const TimeLineList&);

/**
 * Compare whether two TimeLineRecord are equal, generally used in tests only
 * @ingroup StockManage
 */
bool operator==(const TimeLineRecord& d1, const TimeLineRecord& d2);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::TimeLineRecord> : ostream_formatter {};
#endif
