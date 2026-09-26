#include "Indicator.h"
#include "MomentumOperators.h"

namespace hayaku {

// Implementation class of the ADX average directional index
// The original formula of Wilder is used, with the period N = 14
// Smoothing coefficient = 1/N, the initial value is the simple average of N
// periods
class IAdx : public IndicatorImp {
  INDICATOR_IMP(IAdx)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAdx();
  virtual ~IAdx() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAdx)
#endif

namespace hayaku {

IAdx::IAdx() : IndicatorImp("ADX", 3) {
  need_context_ = true;
  setParam<int>("n", 14);
}

IAdx::~IAdx() {}

void IAdx::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 2);
  }
}

void IAdx::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 3);

  if (total < 2) {
    return;
  }

  auto* adx = this->data(0);
  auto* pdi = this->data(1);
  auto* mdi = this->data(2);

  int n = getParam<int>("n");
  size_t period = static_cast<size_t>(n);
  size_t adx_start = period + period - 1;

  if (total <= adx_start) {
    discard_ = total;
    return;
  }

  value_t smooth_factor = 1.0 / n;
  value_t prev_factor = 1.0 - smooth_factor;

  value_t tr_sum = 0.0;
  value_t pdm_sum = 0.0;
  value_t mdm_sum = 0.0;

  for (size_t i = 1; i <= period; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    tr_sum += std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    if (up_move > down_move && up_move > 0) {
      pdm_sum += up_move;
    }
    if (down_move > up_move && down_move > 0) {
      mdm_sum += down_move;
    }
  }

  value_t tr_smooth = tr_sum / n;
  value_t pdm_smooth = pdm_sum / n;
  value_t mdm_smooth = mdm_sum / n;

  if (tr_smooth != 0.0) {
    pdi[period] = 100.0 * pdm_smooth / tr_smooth;
    mdi[period] = 100.0 * mdm_smooth / tr_smooth;
  } else {
    pdi[period] = 0.0;
    mdi[period] = 0.0;
  }

  for (size_t i = period + 1; i < adx_start; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    value_t tr_i = std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
    value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

    tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
    pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
    mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

    if (tr_smooth != 0.0) {
      pdi[i] = 100.0 * pdm_smooth / tr_smooth;
      mdi[i] = 100.0 * mdm_smooth / tr_smooth;
    } else {
      pdi[i] = 0.0;
      mdi[i] = 0.0;
    }
  }

  for (size_t i = adx_start; i < total; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    value_t tr_i = std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
    value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

    tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
    pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
    mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

    if (tr_smooth != 0.0) {
      pdi[i] = 100.0 * pdm_smooth / tr_smooth;
      mdi[i] = 100.0 * mdm_smooth / tr_smooth;
    } else {
      pdi[i] = 0.0;
      mdi[i] = 0.0;
    }
  }

  discard_ = adx_start;

  if (adx_start < total) {
    value_t dx_sum = 0.0;
    for (size_t i = period; i < adx_start; i++) {
      value_t sum = pdi[i] + mdi[i];
      dx_sum += (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
    }
    adx[adx_start] = dx_sum / (n - 1);

    for (size_t i = adx_start + 1; i < total; i++) {
      value_t sum = pdi[i] + mdi[i];
      value_t dx = (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
      adx[i] = adx[i - 1] * prev_factor + dx * smooth_factor;
    }
  }

  for (size_t i = 0; i < period; i++) {
    pdi[i] = Null<value_t>();
    mdi[i] = Null<value_t>();
  }
}

Indicator HAYAKU_API ADX(int n) {
  auto p = make_shared<IAdx>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ADX(const KData& kdata, int n) {
  auto p = make_shared<IAdx>();
  p->setParam<int>("n", n);
  p->setContext(kdata);
  return Indicator(p);
}

} /* namespace hayaku */

#include <operators/IndicatorImp.h>

namespace hayaku {

class IAdx2 : public IndicatorImp {
  INDICATOR_IMP(IAdx2)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION
 public:
  IAdx2();
  virtual ~IAdx2() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAdx2)
#endif

namespace hayaku {

IAdx2::IAdx2() : IndicatorImp("ADX2", 3) {
  need_context_ = true;
  setParam<int>("n", 14);
}

IAdx2::~IAdx2() {}

void IAdx2::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 2);
  }
}

void IAdx2::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 3);

  if (total < 2) {
    return;
  }

  auto* adx = this->data(0);
  auto* pdi = this->data(1);
  auto* mdi = this->data(2);

  int n = getParam<int>("n");
  size_t period = static_cast<size_t>(n);
  size_t adx_start = period + period - 1;

  if (total <= adx_start) {
    discard_ = total;
    return;
  }

  value_t smooth_factor = 2.0 / (n + 1.0);
  value_t prev_factor = 1.0 - smooth_factor;

  value_t tr_sum = 0.0;
  value_t pdm_sum = 0.0;
  value_t mdm_sum = 0.0;

  for (size_t i = 1; i <= period; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    tr_sum += std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    if (up_move > down_move && up_move > 0) {
      pdm_sum += up_move;
    }
    if (down_move > up_move && down_move > 0) {
      mdm_sum += down_move;
    }
  }

  value_t tr_smooth = tr_sum / n;
  value_t pdm_smooth = pdm_sum / n;
  value_t mdm_smooth = mdm_sum / n;

  if (tr_smooth != 0.0) {
    pdi[period] = 100.0 * pdm_smooth / tr_smooth;
    mdi[period] = 100.0 * mdm_smooth / tr_smooth;
  } else {
    pdi[period] = 0.0;
    mdi[period] = 0.0;
  }

  for (size_t i = period + 1; i < adx_start; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    value_t tr_i = std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
    value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

    tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
    pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
    mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

    if (tr_smooth != 0.0) {
      pdi[i] = 100.0 * pdm_smooth / tr_smooth;
      mdi[i] = 100.0 * mdm_smooth / tr_smooth;
    } else {
      pdi[i] = 0.0;
      mdi[i] = 0.0;
    }
  }

  for (size_t i = adx_start; i < total; i++) {
    const KRecord& r = k[i];
    const KRecord& prev_r = k[i - 1];

    value_t h = r.highPrice;
    value_t l = r.lowPrice;
    value_t prev_c = prev_r.closePrice;

    value_t tr1 = h - l;
    value_t tr2 = std::abs(h - prev_c);
    value_t tr3 = std::abs(l - prev_c);
    value_t tr_i = std::max({tr1, tr2, tr3});

    value_t up_move = h - prev_r.highPrice;
    value_t down_move = prev_r.lowPrice - l;

    value_t pdm_i = (up_move > down_move && up_move > 0) ? up_move : 0.0;
    value_t mdm_i = (down_move > up_move && down_move > 0) ? down_move : 0.0;

    tr_smooth = tr_smooth * prev_factor + tr_i * smooth_factor;
    pdm_smooth = pdm_smooth * prev_factor + pdm_i * smooth_factor;
    mdm_smooth = mdm_smooth * prev_factor + mdm_i * smooth_factor;

    if (tr_smooth != 0.0) {
      pdi[i] = 100.0 * pdm_smooth / tr_smooth;
      mdi[i] = 100.0 * mdm_smooth / tr_smooth;
    } else {
      pdi[i] = 0.0;
      mdi[i] = 0.0;
    }
  }

  discard_ = adx_start;

  value_t dx_sum = 0.0;
  for (size_t i = period; i < adx_start; i++) {
    value_t sum = pdi[i] + mdi[i];
    dx_sum += (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
  }
  adx[adx_start] = dx_sum / (n - 1);

  for (size_t i = adx_start + 1; i < total; i++) {
    value_t sum = pdi[i] + mdi[i];
    value_t dx = (sum != 0.0) ? 100.0 * std::abs(pdi[i] - mdi[i]) / sum : 0.0;
    adx[i] = adx[i - 1] * prev_factor + dx * smooth_factor;
  }

  for (size_t i = 0; i < period; i++) {
    pdi[i] = Null<value_t>();
    mdi[i] = Null<value_t>();
  }
}

Indicator HAYAKU_API ADX2(int n) {
  auto p = make_shared<IAdx2>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ADX2(const KData& kdata, int n) {
  auto p = make_shared<IAdx2>();
  p->setParam<int>("n", n);
  p->setContext(kdata);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IAtr.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

namespace hayaku {

class IAtr : public IndicatorImp {
  INDICATOR_IMP(IAtr)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAtr();
  virtual ~IAtr() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 * IAtr.cpp
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAtr)
#endif

namespace hayaku {

IAtr::IAtr() : IndicatorImp("ATR", 1) {
  need_context_ = true;
  setParam<int>("n", 14);
}

IAtr::~IAtr() {}

void IAtr::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 1);
  }
}

void IAtr::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  int n = getParam<int>("n");

  // Use n+1 instead of n, to avoid the inconsistency between the first values
  // of MA(TR) and ATR
  discard_ = n + 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto* k = kdata.data();
  vector<value_t> buf(total);
  for (size_t i = 1; i < total; ++i) {
    value_t v1 = k[i].highPrice - k[i].lowPrice;
    value_t v2 = std::abs(k[i].highPrice - k[i - 1].closePrice);
    value_t v3 = std::abs(k[i].lowPrice - k[i - 1].closePrice);
    buf[i] = std::max(std::max(v1, v2), v3);
  }

  value_t sum = 0.0;
  for (size_t i = 1, end = n + 1; i < end; ++i) {
    sum += buf[i];
  }

  auto* dst = this->data();
  dst[n] = sum / n;

  for (size_t i = n + 1; i < total; ++i) {
    sum = buf[i] + sum - buf[i - n];
    dst[i] = sum / n;
  }
}

void IAtr::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();

  int n = getParam<int>("n");

  auto* k = kdata.data();
  vector<value_t> buf(total);
  for (size_t i = start_pos - n; i < total; ++i) {
    value_t v1 = k[i].highPrice - k[i].lowPrice;
    value_t v2 = std::abs(k[i].highPrice - k[i - 1].closePrice);
    value_t v3 = std::abs(k[i].lowPrice - k[i - 1].closePrice);
    buf[i] = std::max(std::max(v1, v2), v3);
  }

  auto* dst = this->data();

  value_t sum = dst[start_pos - 1] * n;

  for (size_t i = start_pos; i < total; ++i) {
    sum = buf[i] + sum - buf[i - n];
    dst[i] = sum / n;
  }
}

Indicator HAYAKU_API ATR(int n) {
  auto p = make_shared<IAtr>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ATR(const KData& kdata, int n) {
  Indicator ret = ATR(n);
  ret.setContext(kdata);
  return ret;
}

} /* namespace hayaku */

/*
 * IDiff.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Difference indicator, i.e. a[i] - a[i-n]
 * Parameters: n: difference period, 1 by default
 */
class IDiff : public hayaku::IndicatorImp {
  INDICATOR_IMP(IDiff)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IDiff();
  virtual ~IDiff() override;

  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 * IDiff.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDiff)
#endif

namespace hayaku {

IDiff::IDiff() : IndicatorImp("DIFF", 1) { setParam<int>("n", 1); }

IDiff::~IDiff() {}

void IDiff::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") > 0);
  }
}

void IDiff::_calculate(const Indicator& data) {
  size_t total = data.size();
  int n = getParam<int>("n");

  discard_ = data.discard() + n;
  if (total <= discard_) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IDiff::_increment_calculate(const Indicator& data, size_t start_pos) {
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = src[i] - src[i - n];
  }
}

Indicator HAYAKU_API DIFF(int n) {
  IndicatorImpPtr p = make_shared<IDiff>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API DIFF(const Indicator& data, int n) {
  return DIFF(n)(data);
}

} /* namespace hayaku */

/*
 * IMacd.h
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * MACD moving average convergence / divergence

 * Parameters: n1: short-term EMA time window

 *         n2: long-term EMA time window

 *         n3: EMA smoothing time window of (short-term EMA - long-term EMA)

 * Returns: 1) MACD BAR: MACD histogram, i.e. MACD fast line - MACD slow line

 *      2) DIFF: fast line, i.e. (short-term EMA - long-term EMA)

 *      3) DEA: slow line, i.e. the n3-period EMA smoothing of the fast line

 */
class IMacd : public IndicatorImp {
  INDICATOR_IMP(IMacd)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IMacd();
  virtual ~IMacd() override;

  virtual void _checkParam(const string& name) const override;
  virtual void _dyn_calculate(const Indicator&) override;
  virtual size_t min_increment_start() const override;

 private:
  void _dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2,
                       int n3);
};

} /* namespace hayaku */

/*
 * IMacd.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include "SeriesOperators.h"
#include "WindowOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IMacd)
#endif

namespace hayaku {

IMacd::IMacd() : IndicatorImp("MACD", 3) {
  setParam<int>("n1", 12);
  setParam<int>("n2", 26);
  setParam<int>("n3", 9);
}

IMacd::~IMacd() {}

void IMacd::_checkParam(const string& name) const {
  if ("n1" == name) {
    HAYAKU_ASSERT(getParam<int>("n1") >= 0);
  } else if ("n2" == name) {
    HAYAKU_ASSERT(getParam<int>("n2") >= 0);
  } else if ("n3" == name) {
    HAYAKU_ASSERT(getParam<int>("n3") >= 0);
  }
}

void IMacd::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 3);

  discard_ = data.discard();
  if (total <= discard_) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_ + 1);
}

size_t IMacd::min_increment_start() const { return 1; }

void IMacd::_increment_calculate(const Indicator& data, size_t start_pos) {
  int n1 = getParam<int>("n1");
  int n2 = getParam<int>("n2");
  int n3 = getParam<int>("n3");

  auto const* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  auto* dst2 = this->data(2);

  price_t m1 = 2.0 / (n1 + 1);
  price_t m2 = 2.0 / (n2 + 1);
  price_t m3 = 2.0 / (n3 + 1);
  price_t ema1 = src[start_pos - 1];
  price_t ema2 = src[start_pos - 1];
  price_t diff = 0.0;
  price_t dea = 0.0;
  price_t bar = 0.0;
  dst0[start_pos - 1] = bar;
  dst1[start_pos - 1] = diff;
  dst2[start_pos - 1] = dea;

  size_t total = data.size();
  for (size_t i = start_pos; i < total; ++i) {
    ema1 = (src[i] - ema1) * m1 + ema1;
    ema2 = (src[i] - ema2) * m2 + ema2;
    diff = ema1 - ema2;
    dea = diff * m3 + dea - dea * m3;
    bar = diff - dea;
    dst0[i] = bar;
    dst1[i] = diff;
    dst2[i] = dea;
  }
}

void IMacd::_dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2,
                            int n3) {
  HAYAKU_IF_RETURN(n1 <= 0 || n2 <= 0 || n3 <= 0, void());
  Indicator slice = SLICE(ind, 0, curPos + 1);
  Indicator macd = MACD(slice, n1, n2, n3);
  if (macd.size() > 0) {
    size_t index = macd.size() - 1;
    _set(macd.get(index, 0), curPos, 0);
    _set(macd.get(index, 1), curPos, 1);
    _set(macd.get(index, 2), curPos, 2);
  }
}

void IMacd::_dyn_calculate(const Indicator& ind) {
  auto iter = ind_params_.find("n1");
  Indicator n1 = iter != ind_params_.end() ? Indicator(iter->second)
                                            : CVAL(ind, getParam<int>("n1"));
  iter = ind_params_.find("n2");
  Indicator n2 = iter != ind_params_.end() ? Indicator(iter->second)
                                            : CVAL(ind, getParam<int>("n2"));
  iter = ind_params_.find("n3");
  Indicator n3 = iter != ind_params_.end() ? Indicator(iter->second)
                                            : CVAL(ind, getParam<int>("n3"));

  HAYAKU_CHECK(n1.size() == ind.size(),
               "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
               ind.size());
  HAYAKU_CHECK(n2.size() == ind.size(),
               "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
               ind.size());
  HAYAKU_CHECK(n3.size() == ind.size(),
               "ind_param(n3).size()={}, ind.size()={}!", n3.size(),
               ind.size());

  discard_ = std::max(ind.discard(), n2.discard());
  discard_ = std::max(discard_, n3.discard());
  discard_ = std::max(discard_, n1.discard());
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total || discard_ >= total, void());

  global_parallel_for_index_void(ind.discard(), total, [&](size_t i) {
    _dyn_one_circle(ind, i, n1[i], n2[i], n3[i]);
  });

  updateDiscard();
}

Indicator HAYAKU_API MACD(int n1, int n2, int n3) {
  IndicatorImpPtr p = make_shared<IMacd>();
  p->setParam<int>("n1", n1);
  p->setParam<int>("n2", n2);
  p->setParam<int>("n3", n3);
  return Indicator(p);
}

Indicator HAYAKU_API MACD(const IndParam& n1, const IndParam& n2,
                          const IndParam& n3) {
  IndicatorImpPtr p = make_shared<IMacd>();
  p->setIndParam("n1", n1);
  p->setIndParam("n2", n2);
  p->setIndParam("n3", n3);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-05
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * The true range (TR) is the maximum of the following three values:
 *  1. the difference between the high price and the low price of the current
 * period
 *  2. the absolute value of the difference between the high price of the
 * current period and the close price of the previous period
 *  3. the absolute value of the difference between the low price of the current
 * period and the close price of the previous period
 */
class ITr : public IndicatorImp {
  INDICATOR_IMP(ITr)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ITr();
  virtual ~ITr() override;
};

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-05
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ITr)
#endif

namespace hayaku {

ITr::ITr() : IndicatorImp("TR", 1) { need_context_ = true; }

ITr::~ITr() {}

void ITr::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);
  discard_ = 1;

  _increment_calculate(data, 0);
}

void ITr::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  auto* k = kdata.data();
  auto* dst = this->data();
  for (size_t i = start_pos; i < total; ++i) {
    value_t v1 = k[i].highPrice - k[i].lowPrice;
    value_t v2 = std::abs(k[i].highPrice - k[i - 1].closePrice);
    value_t v3 = std::abs(k[i].lowPrice - k[i - 1].closePrice);
    dst[i] = std::max(std::max(v1, v2), v3);
  }
}

Indicator HAYAKU_API TR() { return make_shared<ITr>()->calculate(); }

Indicator HAYAKU_API TR(const KData& k) {
  auto p = make_shared<ITr>();
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */
