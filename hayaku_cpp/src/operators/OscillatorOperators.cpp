#include "MomentumOperators.h"

/*
 * IRoc.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

// Rate of change indicator ((price / prevPrice)-1)*100
class IRoc : public hayaku::IndicatorImp {
  INDICATOR_IMP(IRoc)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRoc();
  virtual ~IRoc() override;
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IRoc.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRoc)
#endif

namespace hayaku {

IRoc::IRoc() : IndicatorImp("ROC", 1) { setParam<int>("n", 10); }

IRoc::~IRoc() {}

void IRoc::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IRoc::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  discard_ = ind.discard() + n;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  if (0 == n) {
    price_t pre_price = src[discard_];
    if (pre_price != 0.0) {
      dst[discard_] = 0.0;
      for (size_t i = discard_ + 1; i < total; i++) {
        dst[i] = (src[i] / pre_price - 1.0) * 100.0;
      }
    } else {
      for (size_t i = discard_; i < total; i++) {
        dst[i] = 0.0;
      }
    }
    return;
  }

  for (size_t i = discard_; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = pre_price != 0.0 ? ((src[i] / pre_price) - 1.0) * 100.0 : 0.0;
  }
}

bool IRoc::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

void IRoc::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  auto const* src = ind.data();
  auto* dst = this->data();

  for (size_t i = start_pos; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = pre_price != 0.0 ? ((src[i] / pre_price) - 1.0) * 100.0 : 0.0;
  }
}

void IRoc::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step) {
    return;
  } else {
    start = curPos - step;
  }

  _set(ind[start] != 0.0 ? ((ind[curPos] / ind[start]) - 1.0) * 100 : 0.0,
       curPos);
}

Indicator HAYAKU_API ROC(int n) {
  IndicatorImpPtr p = make_shared<IRoc>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ROC(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IRoc>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IRocp.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

class IRocp : public hayaku::IndicatorImp {
  INDICATOR_IMP(IRocp)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRocp();
  virtual ~IRocp() override;
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IRocp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRocp)
#endif

namespace hayaku {

IRocp::IRocp() : IndicatorImp("ROCP", 1) { setParam<int>("n", 10); }

IRocp::~IRocp() {}

void IRocp::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IRocp::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  discard_ = ind.discard() + n;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  if (0 == n) {
    price_t pre_price = src[discard_];
    if (pre_price != 0.0) {
      dst[discard_] = 0.0;
      for (size_t i = discard_ + 1; i < total; i++) {
        dst[i] = (src[i] - pre_price) / pre_price;
      }
    } else {
      for (size_t i = discard_; i < total; i++) {
        dst[i] = 0.0;
      }
    }
    return;
  }

  for (size_t i = discard_; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? (src[i] - pre_price) / pre_price : 0.0;
  }
}

bool IRocp::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

void IRocp::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  auto const* src = ind.data();
  auto* dst = this->data();

  for (size_t i = start_pos; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? (src[i] - pre_price) / pre_price : 0.0;
  }
}

void IRocp::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                              size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step) {
    return;
  } else {
    start = curPos - step;
  }

  _set(ind[start] != 0.0 ? (ind[curPos] - ind[start]) / ind[start] : 0.0,
       curPos);
}

Indicator HAYAKU_API ROCP(int n) {
  IndicatorImpPtr p = make_shared<IRocp>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ROCP(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IRocp>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IRocr.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

class IRocr : public hayaku::IndicatorImp {
  INDICATOR_IMP(IRocr)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRocr();
  virtual ~IRocr() override;
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IRocr.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRocr)
#endif

namespace hayaku {

IRocr::IRocr() : IndicatorImp("ROCR", 1) { setParam<int>("n", 10); }

IRocr::~IRocr() {}

void IRocr::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IRocr::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  discard_ = ind.discard() + n;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  if (0 == n) {
    price_t pre_price = src[discard_];
    if (pre_price != 0.0) {
      dst[discard_] = 1.0;
      for (size_t i = discard_ + 1; i < total; i++) {
        dst[i] = src[i] / pre_price;
      }
    } else {
      for (size_t i = discard_; i < total; i++) {
        dst[i] = 0.0;
      }
    }
    return;
  }

  for (size_t i = discard_; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? src[i] / pre_price : 0.0;
  }
}

bool IRocr::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

void IRocr::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  auto const* src = ind.data();
  auto* dst = this->data();

  for (size_t i = start_pos; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? src[i] / pre_price : 0.0;
  }
}

void IRocr::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                              size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step) {
    return;
  } else {
    start = curPos - step;
  }

  _set(ind[start] != 0.0 ? ind[curPos] / ind[start] : 0.0, curPos);
}

Indicator HAYAKU_API ROCR(int n) {
  IndicatorImpPtr p = make_shared<IRocr>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ROCR(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IRocr>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IRocr100.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

namespace hayaku {

class IRocr100 : public hayaku::IndicatorImp {
  INDICATOR_IMP(IRocr100)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRocr100();
  virtual ~IRocr100() override;
  virtual void _checkParam(const string& name) const override;

  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IRocr100.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRocr100)
#endif

namespace hayaku {

IRocr100::IRocr100() : IndicatorImp("ROCR100", 1) { setParam<int>("n", 10); }

IRocr100::~IRocr100() {}

void IRocr100::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IRocr100::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  discard_ = ind.discard() + n;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  if (0 == n) {
    price_t pre_price = src[discard_];
    if (pre_price != 0.0) {
      dst[discard_] = 100.0;
      for (size_t i = discard_ + 1; i < total; i++) {
        dst[i] = src[i] / pre_price * 100.0;
      }
    } else {
      for (size_t i = discard_; i < total; i++) {
        dst[i] = 0.0;
      }
    }
    return;
  }

  for (size_t i = discard_; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? (src[i] / pre_price) * 100.0 : 0.0;
  }
}

bool IRocr100::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

void IRocr100::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");

  auto const* src = ind.data();
  auto* dst = this->data();

  for (size_t i = start_pos; i < total; i++) {
    price_t pre_price = src[i - n];
    dst[i] = (pre_price != 0.0) ? (src[i] / pre_price) * 100.0 : 0.0;
  }
}

void IRocr100::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                 size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step) {
    return;
  } else {
    start = curPos - step;
  }

  _set(ind[start] != 0.0 ? ind[curPos] / ind[start] * 100.0 : 0.0, curPos);
}

Indicator HAYAKU_API ROCR100(int n) {
  IndicatorImpPtr p = make_shared<IRocr100>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API ROCR100(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IRocr100>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * RSI.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */

#include "common/Log.h"

namespace hayaku {

/**
 * Relative Strength Index
 * @ingroup Indicator
 */
Indicator HAYAKU_API RSI(int n) {
  // Indicator data = Indicator();
  // Indicator diff = REF(data, 0) - REF(data, 1);
  Indicator diff = REF(0) - REF(1);
  Indicator u = IF(diff > 0, diff, 0);
  Indicator d = IF(diff < 0, (-1) * diff, 0);
  Indicator ema_u = EMA(u, n);
  Indicator ema_d = EMA(d, n);
  ema_d = IF(ema_d == 0.0, 1, ema_d);
  Indicator rs = ema_u / ema_d;
  Indicator _1 = CVAL(1);
  Indicator rsi = (_1 - _1 / (_1 + rs)) * CVAL(100);
  rsi.name("RSI");
  rsi.setParam<int>("n", n);
  return rsi;
}

Indicator HAYAKU_API RSI(const Indicator& data, int n) {
  return RSI(n)(data)(data.getContext());
}

}  // namespace hayaku

/*
 * IVigor.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Alexander Elder's force index
 * See "Come Into My Trading Room" (2007, Earthquake Press) (Alexander Elder)
 * P131 Calculation formula: (today's close price - yesterday's close price) *
 * today's volume n: the period window used for the EMA smoothing, it must be an
 * integer greater than 0
 */
class IVigor : public IndicatorImp {
  INDICATOR_IMP(IVigor)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IVigor();
  virtual ~IVigor() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

/*
 * IVigor.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "SeriesOperators.h"
#include "WindowOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IVigor)
#endif

namespace hayaku {

IVigor::IVigor() : IndicatorImp("VIGOR", 1) {
  need_context_ = true;
  setParam<int>("n", 2);
}

IVigor::~IVigor() {}

void IVigor::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 1);
  }
}

void IVigor::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& kdata = getContext();
  size_t total = kdata.size();
  _readyBuffer(total, 1);

  int n = getParam<int>("n");

  discard_ = 1;
  if (0 == total) {
    return;
  }

  auto const* ks = kdata.data();
  PriceList tmp(total, Null<price_t>());
  for (size_t i = 1; i < total; ++i) {
    tmp[i] = (ks[i].closePrice - ks[i - 1].closePrice) * ks[i].transCount;
  }

  Indicator ema = EMA(PRICELIST(tmp, 1), n);
  auto const* src = ema.data();
  auto* dst = this->data();
  for (size_t i = 0; i < total; ++i) {
    dst[i] = src[i];
  }
}

Indicator HAYAKU_API VIGOR(int n) {
  IndicatorImpPtr p = make_shared<IVigor>();
  p->setParam<int>("n", n);
  return p->calculate();
}

Indicator HAYAKU_API VIGOR(const KData& k, int n) {
  Indicator v = VIGOR(n);
  v.setContext(k);
  return v;
}

} /* namespace hayaku */
