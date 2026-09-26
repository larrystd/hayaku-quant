#pragma once

/*
 * LoanRecord.h
 *
 *  Created on: 2013-5-24
 *      Author: fasiondog
 */

#include "config.h"
#include "data/MarketTypes.h"

namespace hayaku {

/**
 * Loan record (margin financing record)
 * @ingroup ExecutionAccount
 */
class HAYAKU_API LoanRecord {
 public:
  LoanRecord() : datetime(Null<Datetime>()), value(0.0) {}
  LoanRecord(const Datetime& datetime, price_t value)
      : datetime(datetime), value(value) {}

  Datetime datetime;
  price_t value;

#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    namespace bs = boost::serialization;
    hayaku::uint64_t date_number = datetime.number();
    ar& bs::make_nvp("datetime", date_number);
    ar& BOOST_SERIALIZATION_NVP(value);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    namespace bs = boost::serialization;
    hayaku::uint64_t date_number;
    ar& bs::make_nvp("datetime", date_number);
    datetime = Datetime(date_number);
    ar& BOOST_SERIALIZATION_NVP(value);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif
};

typedef vector<LoanRecord> LoanRecordList;

HAYAKU_API std::ostream& operator<<(std::ostream&, const LoanRecord&);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::LoanRecord> : ostream_formatter {};
#endif
