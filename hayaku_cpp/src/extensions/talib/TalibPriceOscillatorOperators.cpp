#include "TalibOperators.h"
#include "TalibSupport.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaAdosc : public IndicatorImp {
  INDICATOR_IMP(TaAdosc)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaAdosc();
  virtual ~TaAdosc() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaAdosc)
#endif

namespace hayaku {

TaAdosc::TaAdosc() : IndicatorImp("TA_ADOSC", 1) {
  need_context_ = true;
  setParam<int>("fast_n", 3);
  setParam<int>("slow_n", 10);
}

void TaAdosc::_checkParam(const string& name) const {
  if (name == "fast_n") {
    int fast_n = getParam<int>("fast_n");
    HAYAKU_ASSERT(fast_n >= 2 && fast_n <= 100000);
  } else if (name == "slow_n") {
    int slow_n = getParam<int>("slow_n");
    HAYAKU_ASSERT(slow_n >= 2 && slow_n <= 100000);
  }
}

void TaAdosc::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  if (total == 0) {
    return;
  }

  _readyBuffer(total, 1);

  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");
  int back = TA_ADOSC_Lookback(fast_n, slow_n);
  if (back < 0 || back >= total) {
    discard_ = total;
    return;
  }

  const KRecord* kptr = k.data();
  std::unique_ptr<double[]> buf = std::make_unique<double[]>(4 * total);
  double* high = buf.get();
  double* low = high + total;
  double* close = low + total;
  double* vol = close + total;
  for (size_t i = 0; i < total; ++i) {
    high[i] = kptr[i].highPrice;
    low[i] = kptr[i].lowPrice;
    close[i] = kptr[i].closePrice;
    vol[i] = kptr[i].transCount;
  }

  discard_ = back;
  auto* dst = this->data();
  int outBegIdx;
  int outNbElement;
  ::TA_ADOSC(discard_, total - 1, high, low, close, vol, fast_n, slow_n,
             &outBegIdx, &outNbElement, dst + discard_);
  HAYAKU_ASSERT(discard_ == outBegIdx);
}

Indicator HAYAKU_API TA_ADOSC(int fast_n, int slow_n) {
  auto p = make_shared<TaAdosc>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API TA_ADOSC(const KData& k, int fast_n, int slow_n) {
  auto p = make_shared<TaAdosc>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaApo : public IndicatorImp {
  INDICATOR_IMP(TaApo)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaApo();
  virtual ~TaApo() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaApo)
#endif

namespace hayaku {

TaApo::TaApo() : IndicatorImp("TA_APO", 1) {
  setParam<int>("fast_n", 12);
  setParam<int>("slow_n", 26);
  setParam<int>("matype", 0);
}

void TaApo::_checkParam(const string& name) const {
  if (name == "fast_n" || name == "slow_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "{} must >= 2 and <= 100000 ", name);
  } else if (name == "matype") {
    int matype = getParam<int>("matype");
    HAYAKU_ASSERT(matype >= 0 && matype <= 8);
  }
}

void TaApo::_calculate(const Indicator& data) {
  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");
  TA_MAType matype = (TA_MAType)getParam<int>("matype");
  size_t total = data.size();
  int lookback = TA_APO_Lookback(fast_n, slow_n, matype);
  if (lookback >= total || lookback < 0) {
    discard_ = total;
    return;
  }

  discard_ = data.discard() + lookback;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  const double* src = data.data();
  double* dst = this->data();
  int outBegIdx;
  int outNbElement;
  ::TA_APO(discard_, total - 1, src, fast_n, slow_n, matype, &outBegIdx,
           &outNbElement, dst + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_APO(int fast_n, int slow_n, int matype) {
  auto p = make_shared<TaApo>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  p->setParam<int>("matype", matype);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaMacd : public IndicatorImp {
  INDICATOR_IMP(TaMacd)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaMacd();
  virtual ~TaMacd() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaMacd)
#endif

namespace hayaku {

TaMacd::TaMacd() : IndicatorImp("TA_MACD", 3) {
  setParam<int>("fast_n", 12);
  setParam<int>("slow_n", 26);
  setParam<int>("signal_n", 9);
}

void TaMacd::_checkParam(const string& name) const {
  if (name == "fast_n" || name == "slow_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "{} must be in [2, 100000]!", name);
  } else if (name == "signal_n") {
    int signal_n = getParam<int>("signal_n");
    HAYAKU_ASSERT(signal_n >= 1 && signal_n <= 100000);
  }
}

void TaMacd::_calculate(const Indicator& data) {
  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");
  int signal_n = getParam<int>("signal_n");
  size_t total = data.size();
  int lookback = TA_MACD_Lookback(fast_n, slow_n, signal_n);
  if (lookback < 0) {
    discard_ = total;
    return;
  }

  discard_ = data.discard() + lookback;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  const double* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  auto* dst2 = this->data(2);

  int outBegIdx;
  int outNbElement;
  ::TA_MACD(discard_, total - 1, src, fast_n, slow_n, signal_n, &outBegIdx,
            &outNbElement, dst0 + discard_, dst1 + discard_,
            dst2 + discard_);
  HAYAKU_ASSERT(outBegIdx == discard_ && (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_MACD(int fast_n, int slow_n, int signal_n) {
  auto p = make_shared<TaMacd>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  p->setParam<int>("signal_n", signal_n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaMacdext : public IndicatorImp {
  INDICATOR_IMP(TaMacdext)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaMacdext();
  virtual ~TaMacdext() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaMacdext)
#endif

namespace hayaku {

TaMacdext::TaMacdext() : IndicatorImp("TA_MACDEXT", 3) {
  setParam<int>("fast_n", 12);
  setParam<int>("fast_matype", 0);
  setParam<int>("slow_n", 26);
  setParam<int>("slow_matype", 0);
  setParam<int>("signal_n", 9);
  setParam<int>("signal_matype", 0);
}

void TaMacdext::_checkParam(const string& name) const {
  if (name == "fast_n" || name == "slow_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "{} must be in [2, 100000]!", name);
  } else if (name == "signal_n") {
    int signal_n = getParam<int>("signal_n");
    HAYAKU_ASSERT(signal_n >= 1 && signal_n <= 100000);
  } else if (name == "fast_matype" || name == "slow_matype" ||
             name == "signal_matype") {
    int matype = getParam<int>(name);
    HAYAKU_CHECK(matype >= 0 && matype <= 8, "{} must be in [0, 8]!", name);
  }
}

void TaMacdext::_calculate(const Indicator& data) {
  int fast_n = getParam<int>("fast_n");
  TA_MAType fast_matype = (TA_MAType)getParam<int>("fast_matype");
  int slow_n = getParam<int>("slow_n");
  TA_MAType slow_matype = (TA_MAType)getParam<int>("slow_matype");
  int signal_n = getParam<int>("signal_n");
  TA_MAType signal_matype = (TA_MAType)getParam<int>("signal_matype");
  size_t total = data.size();
  int lookback = TA_MACDEXT_Lookback(fast_n, fast_matype, slow_n, slow_matype,
                                     signal_n, signal_matype);
  if (lookback < 0) {
    discard_ = total;
    return;
  }

  discard_ = data.discard() + lookback;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  const double* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  auto* dst2 = this->data(2);

  int outBegIdx;
  int outNbElement;
  ::TA_MACDEXT(discard_, total - 1, src, fast_n, fast_matype, slow_n,
               slow_matype, signal_n, signal_matype, &outBegIdx, &outNbElement,
               dst0 + discard_, dst1 + discard_, dst2 + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_MACDEXT(int fast_n, int slow_n, int signal_n,
                                int fast_matype, int slow_matype,
                                int signal_matype) {
  auto p = make_shared<TaMacdext>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("fast_matype", fast_matype);
  p->setParam<int>("slow_n", slow_n);
  p->setParam<int>("slow_matype", slow_matype);
  p->setParam<int>("signal_n", signal_n);
  p->setParam<int>("signal_matype", signal_matype);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaPpo : public IndicatorImp {
  INDICATOR_IMP(TaPpo)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaPpo();
  virtual ~TaPpo() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaPpo)
#endif

namespace hayaku {

TaPpo::TaPpo() : IndicatorImp("TA_PPO", 1) {
  setParam<int>("fast_n", 12);
  setParam<int>("slow_n", 26);
  setParam<int>("matype", 0);
}

void TaPpo::_checkParam(const string& name) const {
  if (name == "fast_n" || name == "slow_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "{} must >= 2 and <= 100000 ", name);
  } else if (name == "matype") {
    int matype = getParam<int>("matype");
    HAYAKU_ASSERT(matype >= 0 && matype <= 8);
  }
}

void TaPpo::_calculate(const Indicator& data) {
  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");
  TA_MAType matype = (TA_MAType)getParam<int>("matype");
  size_t total = data.size();
  int lookback = TA_PPO_Lookback(fast_n, slow_n, matype);
  if (lookback < 0) {
    discard_ = total;
    return;
  }

  discard_ = data.discard() + lookback;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  const double* src = data.data();
  auto* dst = this->data();

  int outBegIdx;
  int outNbElement;
  ::TA_PPO(discard_, total - 1, src, fast_n, slow_n, matype, &outBegIdx,
           &outNbElement, dst + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_PPO(int fast_n, int slow_n, int matype) {
  auto p = make_shared<TaPpo>();
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  p->setParam<int>("matype", matype);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaStoch : public IndicatorImp {
  INDICATOR_IMP(TaStoch)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaStoch();
  virtual ~TaStoch() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaStoch)
#endif

namespace hayaku {

TaStoch::TaStoch() : IndicatorImp("TA_STOCH", 2) {
  need_context_ = true;
  setParam<int>("fastk_n", 5);
  setParam<int>("slowk_n", 3);
  setParam<int>("slowk_matype", 0);
  setParam<int>("slowd_n", 3);
  setParam<int>("slowd_matype", 0);
}

void TaStoch::_checkParam(const string& name) const {
  if (name == "fastk_n" || name == "slowk_n" || name == "slowd_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
  } else if (name == "slowk_matype" || name == "slowd_matype") {
    int matype = getParam<int>(name);
    HAYAKU_CHECK(matype >= 0 && matype <= 8, "{} must in [0, 8]", name);
  }
}

void TaStoch::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 2);

  int fastk_n = getParam<int>("fastk_n");
  int slowk_n = getParam<int>("slowk_n");
  TA_MAType slowk_matype = (TA_MAType)getParam<int>("slowk_matype");
  int slowd_n = getParam<int>("slowd_n");
  TA_MAType slowd_matype = (TA_MAType)getParam<int>("slowd_matype");
  int back =
      TA_STOCH_Lookback(fastk_n, slowk_n, slowk_matype, slowd_n, slowd_matype);
  if (back < 0 || back >= total) {
    discard_ = total;
    return;
  }

  const KRecord* kptr = k.data();
  std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);
  double* high = buf.get();
  double* low = high + total;
  double* close = low + total;
  for (size_t i = 0; i < total; ++i) {
    high[i] = kptr[i].highPrice;
    low[i] = kptr[i].lowPrice;
    close[i] = kptr[i].closePrice;
  }

  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  discard_ = back;
  int outBegIdx;
  int outNbElement;
  ::TA_STOCH(discard_, total - 1, high, low, close, fastk_n, slowk_n,
             slowk_matype, slowd_n, slowd_matype, &outBegIdx, &outNbElement,
             dst0 + discard_, dst1 + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_STOCH(int fastk_n, int slowk_n, int slowk_matype,
                              int slowd_n, int slowd_matype) {
  auto p = make_shared<TaStoch>();
  p->setParam<int>("fastk_n", fastk_n);
  p->setParam<int>("slowk_n", slowk_n);
  p->setParam<int>("slowk_matype", slowk_matype);
  p->setParam<int>("slowd_n", slowd_n);
  p->setParam<int>("slowd_matype", slowd_matype);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API TA_STOCH(const KData& k, int fastk_n, int slowk_n,
                              int slowk_matype, int slowd_n, int slowd_matype) {
  auto p = make_shared<TaStoch>();
  p->setParam<int>("fastk_n", fastk_n);
  p->setParam<int>("slowk_n", slowk_n);
  p->setParam<int>("slowk_matype", slowk_matype);
  p->setParam<int>("slowd_n", slowd_n);
  p->setParam<int>("slowd_matype", slowd_matype);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaStochf : public IndicatorImp {
  INDICATOR_IMP(TaStochf)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaStochf();
  virtual ~TaStochf() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaStochf)
#endif

namespace hayaku {

TaStochf::TaStochf() : IndicatorImp("TA_STOCHF", 2) {
  need_context_ = true;
  setParam<int>("fastk_n", 5);
  setParam<int>("fastd_n", 3);
  setParam<int>("fastd_matype", 0);
}

void TaStochf::_checkParam(const string& name) const {
  if (name == "fastk_n" || name == "fastd_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
  } else if (name == "fastd_matype") {
    int fastd_matype = getParam<int>("fastd_matype");
    HAYAKU_ASSERT(fastd_matype >= 0 && fastd_matype <= 8);
  }
}

void TaStochf::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 2);

  int fastk_n = getParam<int>("fastk_n");
  int fastd_n = getParam<int>("fastd_n");
  TA_MAType fastd_matype = (TA_MAType)getParam<int>("fastd_matype");
  int back = TA_STOCHF_Lookback(fastk_n, fastd_n, fastd_matype);
  if (back < 0 || back >= total) {
    discard_ = total;
    return;
  }

  const KRecord* kptr = k.data();
  std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);
  double* high = buf.get();
  double* low = high + total;
  double* close = low + total;
  for (size_t i = 0; i < total; ++i) {
    high[i] = kptr[i].highPrice;
    low[i] = kptr[i].lowPrice;
    close[i] = kptr[i].closePrice;
  }

  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  discard_ = back;
  int outBegIdx;
  int outNbElement;
  ::TA_STOCHF(discard_, total - 1, high, low, close, fastk_n, fastd_n,
              fastd_matype, &outBegIdx, &outNbElement, dst0 + discard_,
              dst1 + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_STOCHF(int fastk_n, int fastd_n, int fastd_matype) {
  auto p = make_shared<TaStochf>();
  p->setParam<int>("fastk_n", fastk_n);
  p->setParam<int>("fastd_n", fastd_n);
  p->setParam<int>("fastd_matype", fastd_matype);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API TA_STOCHF(const KData& k, int fastk_n, int fastd_n,
                               int fastd_matype) {
  auto p = make_shared<TaStochf>();
  p->setParam<int>("fastk_n", fastk_n);
  p->setParam<int>("fastd_n", fastd_n);
  p->setParam<int>("fastd_matype", fastd_matype);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaStochrsi : public IndicatorImp {
  INDICATOR_IMP(TaStochrsi)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaStochrsi();
  virtual ~TaStochrsi() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-22
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaStochrsi)
#endif

namespace hayaku {

TaStochrsi::TaStochrsi() : IndicatorImp("TA_STOCHRSI", 2) {
  setParam<int>("n", 14);
  setParam<int>("fastk_n", 5);
  setParam<int>("fastd_n", 3);
  setParam<int>("matype", 0);
}

void TaStochrsi::_checkParam(const string& name) const {
  if (name == "n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "n must in [2, 100000]");
  } else if (name == "fastk_n" || name == "fastd_n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
  } else if (name == "matype") {
    int matype = getParam<int>("matype");
    HAYAKU_ASSERT(matype >= 0 && matype <= 8);
  }
}

void TaStochrsi::_calculate(const Indicator& data) {
  int n = getParam<int>("n");
  int fastk_n = getParam<int>("fastk_n");
  int fastd_n = getParam<int>("fastd_n");
  TA_MAType matype = (TA_MAType)getParam<int>("matype");
  size_t total = data.size();
  int lookback = TA_STOCHRSI_Lookback(n, fastk_n, fastd_n, matype);
  if (lookback < 0) {
    discard_ = total;
    return;
  }

  discard_ = data.discard() + lookback;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  const double* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);

  int outBegIdx;
  int outNbElement;
  ::TA_STOCHRSI(discard_, total - 1, src, n, fastk_n, fastd_n, matype,
                &outBegIdx, &outNbElement, dst0 + discard_, dst1 + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_STOCHRSI(int n, int fastk_n, int fastd_n, int matype) {
  auto p = make_shared<TaStochrsi>();
  p->setParam<int>("n", n);
  p->setParam<int>("fastk_n", fastk_n);
  p->setParam<int>("fastd_n", fastd_n);
  p->setParam<int>("matype", matype);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaUltosc : public IndicatorImp {
  INDICATOR_IMP(TaUltosc)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaUltosc();
  virtual ~TaUltosc() = default;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-20
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaUltosc)
#endif

namespace hayaku {

TaUltosc::TaUltosc() : IndicatorImp("TA_ULTOSC", 1) {
  need_context_ = true;
  setParam<int>("n1", 7);
  setParam<int>("n2", 14);
  setParam<int>("n3", 28);
}

void TaUltosc::_checkParam(const string& name) const {
  if (name == "n1" || name == "n2" || name == "n3") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 1 && n <= 100000, "{} must in [1, 100000]", name);
  }
}

void TaUltosc::_calculate(const Indicator& data) {
  HAYAKU_WARN_IF(!isLeaf() && !data.empty(),
                 "The input is ignored because {} depends on the context!",
                 name_);

  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  int n1 = getParam<int>("n1");
  int n2 = getParam<int>("n2");
  int n3 = getParam<int>("n3");
  int back = TA_ULTOSC_Lookback(n1, n2, n3);
  if (back < 0 || back >= total) {
    discard_ = total;
    return;
  }

  const KRecord* kptr = k.data();
  std::unique_ptr<double[]> buf = std::make_unique<double[]>(3 * total);
  double* high = buf.get();
  double* low = high + total;
  double* close = low + total;
  for (size_t i = 0; i < total; ++i) {
    high[i] = kptr[i].highPrice;
    low[i] = kptr[i].lowPrice;
    close[i] = kptr[i].closePrice;
  }

  auto* dst = this->data();
  discard_ = back;
  int outBegIdx;
  int outNbElement;
  ::TA_ULTOSC(discard_, total - 1, high, low, close, n1, n2, n3, &outBegIdx,
              &outNbElement, dst + discard_);
  HAYAKU_ASSERT((outBegIdx == discard_) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_ULTOSC(int n1, int n2, int n3) {
  auto p = make_shared<TaUltosc>();
  p->setParam<int>("n1", n1);
  p->setParam<int>("n2", n2);
  p->setParam<int>("n3", n3);
  p->calculate();
  return Indicator(p);
}

Indicator HAYAKU_API TA_ULTOSC(const KData& k, int n1, int n2, int n3) {
  auto p = make_shared<TaUltosc>();
  p->setParam<int>("n1", n1);
  p->setParam<int>("n2", n2);
  p->setParam<int>("n3", n3);
  p->setContext(k);
  return Indicator(p);
}

} /* namespace hayaku */
