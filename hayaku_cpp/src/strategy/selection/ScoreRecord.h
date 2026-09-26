#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-17
 *      Author: fasiondog
 */

#include "data/Stock.h"
#include "operators/Indicator.h"

namespace hayaku {

struct ScoreRecord {
  typedef Indicator::value_t value_t;

  Stock stock;
  value_t value{0.0};

  ScoreRecord() = default;
  ScoreRecord(const Stock& stock_, value_t value_);

  ScoreRecord(const ScoreRecord&);
  ScoreRecord(ScoreRecord&&);

  ScoreRecord& operator=(const ScoreRecord&);
  ScoreRecord& operator=(ScoreRecord&&);
};

typedef vector<ScoreRecord> ScoreRecordList;
typedef vector<ScoreRecord> ScoreList;

std::ostream& operator<<(std::ostream& out, const ScoreRecord& td);

std::ostream& operator<<(std::ostream& out, const ScoreRecordList& td);

std::ostream& operator<<(std::ostream& out, const vector<ScoreRecordList>& td);

}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::ScoreRecord> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::ScoreRecordList> : ostream_formatter {};

template <>
struct fmt::formatter<std::vector<hayaku::ScoreRecordList>>
    : ostream_formatter {};
#endif
