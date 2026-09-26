#include "MarketOperators.h"

/*
 * IAdvance.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-30
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

class IAdvance : public IndicatorImp {
  INDICATOR_IMP(IAdvance)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAdvance();
  virtual ~IAdvance();
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IAdvance.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-20
 *      Author: fasiondog
 */

#include "SeriesOperators.h"
#include "data/DataRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAdvance)
#endif

namespace hayaku {

IAdvance::IAdvance() : IndicatorImp("ADVANCE", 1) {
  setParam<KQuery>("query", KQueryByIndex(-100));
  setParam<string>("market", "SH");
  setParam<int>("stk_type", STOCKTYPE_A);
  setParam<bool>("ignore_context", false);
  setParam<bool>("fill_null", true);
}

IAdvance::~IAdvance() {}

void IAdvance::_checkParam(const string& name) const {
  if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  } else if ("stk_type" == name) {
    int stk_type = getParam<int>("stk_type");
    HAYAKU_ASSERT(stk_type >= 0);
  }
}

void IAdvance::_calculate(const Indicator& ind) {
  // The ref_date_list parameter affects the IndicatorImp globally, do not
  // modify it at will
  string market;
  KQuery q;
  int stk_type = STOCKTYPE_A;

  DataRuntime& runtime = getDataRuntime();
  DatetimeList dates;

  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  if (!ignore_context && k.getStock().type() != STOCKTYPE_INDEX) {
    q = k.getQuery();
    Stock stk = k.getStock();
    market = stk.market();
    stk_type = stk.type();
    dates = k.getDatetimeList();
  } else {
    market = getParam<string>("market");
    q = getParam<KQuery>("query");
    stk_type = getParam<int>("stk_type");
    dates = runtime.getTradingCalendar(q, market);
  }

  size_t total = dates.size();
  if (total == 0) {
    m_discard = 0;
    _readyBuffer(0, 1);
    return;
  }

  m_discard = 1;
  _readyBuffer(total, 1);

  // The Query needs to be converted into KQueryByDate
  q = KQueryByDate(dates.front(),
                   dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                   q.kType(), q.recoverType());

  auto* dst = this->data();
  Indicator x = ALIGN(CLOSE() > REF(CLOSE(), 1), std::move(dates),
                      getParam<bool>("fill_null"));
  for (auto iter = runtime.begin(); iter != runtime.end(); ++iter) {
    if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
        (market != "" && iter->market() != market)) {
      continue;
    }
    x.setContext(*iter, q);
    auto const* xdata = x.data();
    if (x.empty()) {
      continue;
    }
    for (size_t i = x.discard(); i < x.size(); i++) {
      if (x.getDatetime(i) > iter->lastDatetime()) {
        break;
      }

      if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
        dst[i] = std::isnan(dst[i]) ? 1 : dst[i] + 1;
      }
    }
  }
}

bool IAdvance::supportIncrementCalculate() const {
  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  return !ignore_context && k.getStock().type() != STOCKTYPE_INDEX;
}

void IAdvance::_increment_calculate(const Indicator& data, size_t start_pos) {
  SPEND_TIME(_increment_calculate);
  const auto& k = getContext();
  auto q = k.getQuery();
  auto stk = k.getStock();
  const auto& market = stk.market();
  auto stk_type = stk.type();
  DatetimeList old_dates = k.getDatetimeList();
  DatetimeList dates;
  for (size_t i = start_pos - 1; i < old_dates.size(); i++) {
    dates.push_back(old_dates[i]);
  }
  q = KQueryByDate(dates.front(),
                   dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                   q.kType(), q.recoverType());

  DataRuntime& runtime = getDataRuntime();
  auto* dst = this->data();
  // Force clearing the dst range that is about to be recalculated, to prevent
  // the old summary value copied by increment_execute_leaf_or_op from being
  // accumulated twice by the whole market traversal (a dirty read double
  // accumulation: when the old value is not NaN, isnan ? 1 : +1 adds one more).
  // The clearing starts from start_pos (not start_pos-1) because start_pos-1 is
  // the correct summary of the last bar of the old window; the incremental
  // inner loop writes dst[i+start_pos-1] starting from x.discard()>=1, writing
  // dst[start_pos] at the earliest, and never writes dst[start_pos-1], so its
  // old value is kept.
  for (size_t i = start_pos; i < this->size(); ++i) {
    dst[i] = Null<value_t>();
  }
  Indicator x = ALIGN(CLOSE() > REF(CLOSE(), 1), std::move(dates),
                      getParam<bool>("fill_null"));
  for (auto iter = runtime.begin(); iter != runtime.end(); ++iter) {
    if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
        (market != "" && iter->market() != market)) {
      continue;
    }
    x.setContext(*iter, q);
    auto const* xdata = x.data();
    if (x.empty()) {
      continue;
    }
    for (size_t i = x.discard(); i < x.size(); i++) {
      if (x.getDatetime(i) > iter->lastDatetime()) {
        break;
      }

      if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
        dst[i + start_pos - 1] =
            std::isnan(dst[i + start_pos - 1]) ? 1 : dst[i + start_pos - 1] + 1;
      }
    }
  }
}

Indicator HAYAKU_API ADVANCE(const KQuery& query, const string& market,
                             int stk_type, bool ignore_context,
                             bool fill_null) {
  IndicatorImpPtr p = make_shared<IAdvance>();
  p->setParam<KQuery>("query", query);
  p->setParam<string>("market", market);
  p->setParam<int>("stk_type", stk_type);
  p->setParam<bool>("ignore_context", ignore_context);
  p->setParam<bool>("fill_null", fill_null);
  p->calculate();
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IDecline.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

namespace hayaku {

class IDecline : public IndicatorImp {
  INDICATOR_IMP(IDecline)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IDecline();
  virtual ~IDecline();
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IDecline.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-6-3
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDecline)
#endif

namespace hayaku {

IDecline::IDecline() : IndicatorImp("DECLINE", 1) {
  setParam<KQuery>("query", KQueryByIndex(-100));
  setParam<string>("market", "SH");
  setParam<int>("stk_type", STOCKTYPE_A);
  setParam<bool>("ignore_context", false);
  setParam<bool>("fill_null", true);
}

IDecline::~IDecline() {}

void IDecline::_checkParam(const string& name) const {
  if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  } else if ("stk_type" == name) {
    int stk_type = getParam<int>("stk_type");
    HAYAKU_ASSERT(stk_type >= 0);
  }
}

void IDecline::_calculate(const Indicator& ind) {
  // The ref_date_list parameter affects the IndicatorImp globally, do not
  // modify it at will
  string market;
  KQuery q;
  int stk_type = STOCKTYPE_A;

  DataRuntime& runtime = getDataRuntime();
  DatetimeList dates;

  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  if (!ignore_context && !k.empty() && k.getStock().type() != STOCKTYPE_INDEX) {
    q = k.getQuery();
    Stock stk = k.getStock();
    market = stk.market();
    stk_type = stk.type();
    dates = k.getDatetimeList();
  } else {
    market = getParam<string>("market");
    q = getParam<KQuery>("query");
    stk_type = getParam<int>("stk_type");
    dates = runtime.getTradingCalendar(q, market);
  }

  size_t total = dates.size();
  if (total == 0) {
    m_discard = 0;
    _readyBuffer(0, 1);
    return;
  }

  // The Query needs to be converted into KQueryByDate
  q = KQueryByDate(dates.front(),
                   dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                   q.kType(), q.recoverType());

  m_discard = 1;
  _readyBuffer(total, 1);
  auto* dst = this->data();
  Indicator x = ALIGN(CLOSE() < REF(CLOSE(), 1), std::move(dates),
                      getParam<bool>("fill_null"));
  for (auto iter = runtime.begin(); iter != runtime.end(); ++iter) {
    if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
        (market != "" && iter->market() != market)) {
      continue;
    }
    x.setContext(*iter, q);
    if (x.empty()) {
      continue;
    }
    auto const* xdata = x.data();
    for (size_t i = x.discard(); i < total; i++) {
      if (x.getDatetime(i) > iter->lastDatetime()) {
        break;
      }

      if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
        dst[i] = std::isnan(dst[i]) ? 1 : dst[i] + 1;
      }
    }
  }
}

bool IDecline::supportIncrementCalculate() const {
  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  return !ignore_context && k.getStock().type() != STOCKTYPE_INDEX;
}

void IDecline::_increment_calculate(const Indicator& data, size_t start_pos) {
  const auto& k = getContext();
  auto q = k.getQuery();
  auto stk = k.getStock();
  const auto& market = stk.market();
  auto stk_type = stk.type();
  DatetimeList old_dates = k.getDatetimeList();
  DatetimeList dates;
  for (size_t i = start_pos - 1; i < old_dates.size(); i++) {
    dates.push_back(old_dates[i]);
  }
  q = KQueryByDate(dates.front(),
                   dates.back() + Seconds(KQuery::getKTypeInSeconds(q.kType())),
                   q.kType(), q.recoverType());

  DataRuntime& runtime = getDataRuntime();
  auto* dst = this->data();
  // Force clearing the dst range that is about to be recalculated, to prevent
  // the old summary value copied by increment_execute_leaf_or_op from being
  // accumulated twice by the whole market traversal (a dirty read double
  // accumulation: when the old value is not NaN, isnan ? 1 : +1 adds one more).
  // The clearing starts from start_pos (not start_pos-1) because start_pos-1 is
  // the correct summary of the last bar of the old window; the incremental
  // inner loop writes dst[i+start_pos-1] starting from x.discard()>=1, writing
  // dst[start_pos] at the earliest, and never writes dst[start_pos-1], so its
  // old value is kept.
  for (size_t i = start_pos; i < this->size(); ++i) {
    dst[i] = Null<value_t>();
  }
  Indicator x = ALIGN(CLOSE() < REF(CLOSE(), 1), std::move(dates),
                      getParam<bool>("fill_null"));
  for (auto iter = runtime.begin(); iter != runtime.end(); ++iter) {
    if ((stk_type <= STOCKTYPE_TMP && iter->type() != stk_type) ||
        (market != "" && iter->market() != market)) {
      continue;
    }
    x.setContext(*iter, q);
    if (x.empty()) {
      continue;
    }
    auto const* xdata = x.data();
    for (size_t i = x.discard(); i < x.size(); i++) {
      if (x.getDatetime(i) > iter->lastDatetime()) {
        break;
      }

      if (!std::isnan(xdata[i]) && xdata[i] > 0.0) {
        dst[i + start_pos - 1] =
            std::isnan(dst[i + start_pos - 1]) ? 1 : dst[i + start_pos - 1] + 1;
      }
    }
  }
}

Indicator HAYAKU_API DECLINE(const KQuery& query, const string& market,
                             int stk_type, bool ignore_context,
                             bool fill_null) {
  IndicatorImpPtr p = make_shared<IDecline>();
  p->setParam<KQuery>("query", query);
  p->setParam<string>("market", market);
  p->setParam<int>("stk_type", stk_type);
  p->setParam<bool>("ignore_context", ignore_context);
  p->setParam<bool>("fill_null", fill_null);
  p->calculate();
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

namespace hayaku {

class IBlockSetNum : public IndicatorImp {
  INDICATOR_IMP(IBlockSetNum)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBlockSetNum();
  virtual ~IBlockSetNum();
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 * IBlockSetNum.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2024-5-21
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBlockSetNum)
#endif

namespace hayaku {

IBlockSetNum::IBlockSetNum() : IndicatorImp("BLOCKSETNUM", 1) {
  setParam<KQuery>("query", KQueryByIndex(-100));
  setParam<Block>("block", Block());
  setParam<string>("market", "SH");
  setParam<bool>("ignore_context", false);
}

IBlockSetNum::~IBlockSetNum() {}

void IBlockSetNum::_checkParam(const string& name) const {
  if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  }
}

void IBlockSetNum::_calculate(const Indicator& ind) {
  const Block block = getParam<const Block&>("block");
  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  DatetimeList dates;
  if (!ignore_context && !k.empty()) {
    dates = k.getDatetimeList();
  } else {
    const KQuery& q = getParam<const KQuery&>("query");
    if (q != KQuery(0, 0)) {
      dates =
          getDataRuntime().getTradingCalendar(q, getParam<string>("market"));
    }
  }

  size_t total = dates.size();
  m_discard = 0;
  _readyBuffer(total, 1);
  HAYAKU_IF_RETURN(total == 0, void());

  value_t zero = 0.0;
  auto* dst = this->data();
  for (size_t i = 0; i < total; i++) {
    dst[i] = zero;
  }

  for (auto iter = block.begin(); iter != block.end(); ++iter) {
    const Datetime& start_date = iter->startDatetime();
    Datetime last_date =
        iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
    for (size_t i = 0; i < total; i++) {
      if (dates[i] >= start_date && dates[i] <= last_date) {
        dst[i]++;
      }
    }
  }
}

bool IBlockSetNum::supportIncrementCalculate() const {
  return !getParam<bool>("ignore_context");
}

void IBlockSetNum::_increment_calculate(const Indicator& ind,
                                        size_t start_pos) {
  const Block block = getParam<const Block&>("block");
  const KData& k = getContext();
  DatetimeList dates = k.getDatetimeList();

  size_t total = dates.size();

  value_t zero = 0.0;
  auto* dst = this->data();
  for (size_t i = start_pos; i < total; i++) {
    dst[i] = zero;
  }

  for (auto iter = block.begin(); iter != block.end(); ++iter) {
    const Datetime& start_date = iter->startDatetime();
    Datetime last_date =
        iter->lastDatetime().isNull() ? Datetime::max() : iter->lastDatetime();
    for (size_t i = start_pos; i < total; i++) {
      if (dates[i] >= start_date && dates[i] <= last_date) {
        dst[i]++;
      }
    }
  }
}

Indicator HAYAKU_API BLOCKSETNUM(const Block& block, const KQuery& query) {
  IndicatorImpPtr p = make_shared<IBlockSetNum>();
  p->setParam<KQuery>("query", query);
  p->setParam<Block>("block", block);
  p->setParam<bool>("ignore_context", false);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API BLOCKSETNUM(const Block& block) {
  return BLOCKSETNUM(block, KQuery(0, 0));
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

namespace hayaku {

/* Return whether it is in the given block, in the form of an indicator */
class IInBlock : public IndicatorImp {
  INDICATOR_IMP(IInBlock)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IInBlock();
  virtual ~IInBlock() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-26
 *      Author: fasiondog
 */

#include "data/Block.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IInBlock)
#endif

namespace hayaku {

IInBlock::IInBlock() : IndicatorImp("INBLOCK", 1) {
  m_need_context = true;
  setParam<string>("category", "");
  setParam<string>("name", "");
}

IInBlock::~IInBlock() {}

void IInBlock::_calculate(const Indicator& data) {
  HAYAKU_IF_RETURN(!isLeaf() && !data.empty(), void());

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  Block block =
      getBlock(getParam<string>("category"), getParam<string>("name"));
  value_t in = block.have(k.getStock()) ? 1.0 : 0.0;
  auto* dst = this->data();
  for (size_t i = 0; i < total; ++i) {
    dst[i] = in;
  }
}

void IInBlock::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& k = getContext();
  size_t total = k.size();
  Block block =
      getBlock(getParam<string>("category"), getParam<string>("name"));
  value_t in = block.have(k.getStock()) ? 1.0 : 0.0;
  auto* dst = this->data();
  for (size_t i = start_pos; i < total; ++i) {
    dst[i] = in;
  }
}

Indicator HAYAKU_API INBLOCK(const string& category, const string& name) {
  auto p = make_shared<IInBlock>();
  p->setParam<string>("category", category);
  p->setParam<string>("name", name);
  return Indicator(p);
}

Indicator HAYAKU_API INBLOCK(const KData& k, const string& category,
                             const string& name) {
  auto p = make_shared<IInBlock>();
  p->setParam<string>("category", category);
  p->setParam<string>("name", name);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-21
 *      Author: fasiondog
 */

namespace hayaku {

class IInSum : public IndicatorImp {
  INDICATOR_IMP(IInSum)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IInSum();
  virtual ~IInSum();
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 * IInSum.cpp
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2024-5-21
 *      Author: fasiondog
 */

#include "common/concurrency/thread.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IInSum)
#endif

namespace hayaku {

IInSum::IInSum() : IndicatorImp("INSUM", 1) {
  setParam<KQuery>("query", KQuery(0, 0));
  setParam<Block>("block", Block());
  setParam<int>("mode", 0);
  setParam<string>("market", "SH");
  setParam<bool>("ignore_context", false);
  setParam<bool>("fill_null", false);
}

IInSum::~IInSum() {}

void IInSum::_checkParam(const string& name) const {
  if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  } else if ("mode" == name) {
    int mode = getParam<int>("mode");
    HAYAKU_ASSERT(mode == 0 || mode == 1 || mode == 2 || mode == 3 ||
                  mode == 4 || mode == 5);
  }
}

static IndicatorList getAllIndicators(const Block& block, const KQuery& query,
                                      const DatetimeList& dates,
                                      const Indicator& ind, bool fill_null) {
  auto stks = block.getStockList();
  return global_parallel_for_index(
      0, stks.size(),
      [nind = ind.clone(), fill_null, &stks, &query, &dates](size_t index) {
        auto k = stks[index].getKData(query);
        return ALIGN(nind, dates, fill_null)(k).getResult(0)();
      });
}

static void insum_cum(const IndicatorList& inds, Indicator::value_t* dst,
                      size_t len) {
  for (const auto& value : inds) {
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();
    for (size_t i = 0; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (std::isnan(dst[i])) {
          dst[i] = data[i];
        } else {
          dst[i] += data[i];
        }
      }
    }
  }
}

static void insum_mean(const IndicatorList& inds, Indicator::value_t* dst,
                       size_t len) {
  vector<size_t> count(len, 0);
  for (const auto& value : inds) {
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();
    for (size_t i = 0; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (std::isnan(dst[i])) {
          dst[i] = data[i];
        } else {
          dst[i] += data[i];
        }
        count[i]++;
      }
    }
  }

  for (size_t i = 0; i < len; i++) {
    if (!std::isnan(dst[i])) {
      dst[i] = dst[i] / count[i];
    }
  }
}

static void insum_max(const IndicatorList& inds, Indicator::value_t* dst,
                      size_t len) {
  for (const auto& value : inds) {
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();
    for (size_t i = 0; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (std::isnan(dst[i])) {
          dst[i] = data[i];
        } else if (data[i] > dst[i]) {
          dst[i] = data[i];
        }
      }
    }
  }
}

static void insum_min(const IndicatorList& inds, Indicator::value_t* dst,
                      size_t len) {
  for (const auto& value : inds) {
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();
    // Traverse all the data; dst is set to data[i] when it is empty, and to
    // data[i] when data[i] is smaller than the original value
    for (size_t i = 0; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (std::isnan(dst[i])) {
          dst[i] = data[i];
        } else if (data[i] < dst[i]) {
          dst[i] = data[i];
        }
      }
    }
  }
}

// The ranking is in the descending order, the highest indicator value has the
// rank 1
static void insum_rank_desc(const IndicatorList& inds, Indicator::value_t* dst,
                            const Indicator& ind, size_t len) {
  size_t discard = ind.discard();
  for (size_t i = discard; i < len; i++) {
    if (std::isnan(dst[i])) {
      dst[i] = 1;  // It is equivalent to the initialization
    }
  }
  for (const auto& value : inds) {  // A single ind
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();    // The data of the compared stock
    const auto* data_ind = ind.data();  // The data of this stock

    for (size_t i = discard; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (data[i] > data_ind[i]) {
          dst[i]++;
        }
      }
    }
  }
}

// The ranking is in the ascending order, the lowest indicator value has the
// rank 1 and a higher indicator value means a higher rank
static void insum_rank_asc(const IndicatorList& inds, Indicator::value_t* dst,
                           const Indicator& ind, size_t len) {
  size_t discard = ind.discard();
  for (size_t i = discard; i < len; i++) {
    if (std::isnan(dst[i])) {
      dst[i] = 1;  // It is equivalent to the initialization
    }
  }
  for (const auto& value : inds) {  // A single ind
    if (value.empty()) {
      continue;
    }
    if (value.size() != len) {
      HAYAKU_WARN("Ignore stock: {}, value len: {}, dst len: {}",
                  value.getContext().getStock().market_code(), value.size(),
                  len);
      continue;
    }
    const auto* data = value.data();    // The data of the compared stock
    const auto* data_ind = ind.data();  // The data of this stock

    for (size_t i = discard; i < len; i++) {
      if (!std::isnan(data[i])) {
        if (data[i] < data_ind[i]) {  // Rank + 1 when it is smaller than
                                      // dst_tmp, unchanged otherwise
          dst[i]++;
        }
      }
    }
  }
}

void IInSum::_calculate(const Indicator& ind) {
  const Block block = getParam<const Block&>("block");
  bool ignore_context = getParam<bool>("ignore_context");
  const KData& k = getContext();
  KQuery q;
  DatetimeList dates;
  if (!ignore_context && !k.empty()) {
    q = k.getQuery();
    dates = k.getDatetimeList();
  } else {
    q = getParam<KQuery>("query");
    if (q != KQuery(0, 0)) {
      dates =
          getDataRuntime().getTradingCalendar(q, getParam<string>("market"));
    }
  }

  size_t total = dates.size();
  m_discard = 0;
  _readyBuffer(total, 1);
  HAYAKU_IF_RETURN(total == 0, void());

  int mode = getParam<int>("mode");
  // Modes 4/5 depend on the context
  if (mode == 4 || mode == 5) {
    if (ind.size() == 0) {
      m_discard = total;
      return;
    }
  }

  auto inds =
      getAllIndicators(block, q, dates, ind, getParam<bool>("fill_null"));
  auto* dst = this->data();

  if (0 == mode) {
    insum_cum(inds, dst, total);
  } else if (1 == mode) {
    insum_mean(inds, dst, total);
  } else if (2 == mode) {
    insum_max(inds, dst, total);
  } else if (3 == mode) {
    insum_min(inds, dst, total);
  } else if (4 == mode) {
    // A larger indicator value means a lower rank, i.e. the largest indicator
    // value has the rank 1
    auto nind = ind;
    if (ind.size() != total) {
      nind = ALIGN(ind, std::move(dates), getParam<bool>("fill_null"));
      HAYAKU_CHECK(nind.size() == total, "ind size: {}  != total: {}",
                   ind.size(), total);
    }
    insum_rank_desc(inds, dst, nind, total);
  } else if (5 == mode) {
    // A higher indicator value means a higher rank, i.e. the lowest indicator
    // value has the rank 1
    auto nind = ind;
    if (ind.size() != total) {
      nind = ALIGN(ind, std::move(dates), getParam<bool>("fill_null"));
      HAYAKU_CHECK(nind.size() == total, "ind size: {}  != total: {}",
                   ind.size(), total);
    }
    insum_rank_asc(inds, dst, nind, total);
  } else {
    HAYAKU_ERROR("Not support mode: {}", mode);
  }

  for (size_t i = m_discard; i < total; i++) {
    if (!std::isnan(dst[i])) {
      break;
    }
    m_discard++;
  }
}

Indicator HAYAKU_API INSUM(const Block& block, int mode, bool fill_null) {
  IndicatorImpPtr p = make_shared<IInSum>();
  p->setParam<Block>("block", block);
  p->setParam<int>("mode", mode);
  p->setParam<bool>("ignore_context", false);
  p->setParam<bool>("fill_null", fill_null);
  return Indicator(p);
}

Indicator HAYAKU_API INSUM(const Block& block, const KQuery& query,
                           const Indicator& ind, int mode, bool fill_null) {
  IndicatorImpPtr p = make_shared<IInSum>();
  p->setParam<KQuery>("query", query);
  p->setParam<Block>("block", block);
  p->setParam<int>("mode", mode);
  p->setParam<bool>("ignore_context", false);
  p->setParam<bool>("fill_null", fill_null);
  return Indicator(p)(ind);
}

Indicator HAYAKU_API INSUM(const Block& block, const Indicator& ind, int mode,
                           bool fill_null) {
  return INSUM(block, KQuery(0, 0), ind, mode, fill_null);
}

} /* namespace hayaku */
