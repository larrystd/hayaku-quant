#include "TalibOperators.h"
#include "TalibSupport.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-21
 *      Author: fasiondog
 */

#include "operators/Indicator.h"

namespace hayaku {

class TaMa : public IndicatorImp {
  INDICATOR_IMP(TaMa)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaMa();
  virtual ~TaMa() = default;
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
BOOST_CLASS_EXPORT(hayaku::TaMa)
#endif

namespace hayaku {

TaMa::TaMa() : IndicatorImp("TA_MA", 1) {
  setParam<int>("n", 30);
  setParam<int>("matype", 0);
}

void TaMa::_checkParam(const string& name) const {
  if (name == "n") {
    int n = getParam<int>(name);
    HAYAKU_ASSERT(n >= 1 && n <= 100000);
  } else if (name == "matype") {
    int matype = getParam<int>("matype");
    HAYAKU_ASSERT(matype >= 0 && matype <= 8);
  }
}

void TaMa::_calculate(const Indicator& data) {
  int n = getParam<int>("n");
  TA_MAType matype = (TA_MAType)getParam<int>("matype");
  size_t total = data.size();
  int lookback = TA_MA_Lookback(n, matype);
  if (lookback < 0) {
    m_discard = total;
    return;
  }

  m_discard = data.discard() + lookback;
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  const double* src = data.data();
  auto* dst = this->data();

  int outBegIdx;
  int outNbElement;
  ::TA_MA(m_discard, total - 1, src, n, matype, &outBegIdx, &outNbElement,
          dst + m_discard);
  HAYAKU_ASSERT(outBegIdx == m_discard && (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_MA(int n, int matype) {
  auto p = make_shared<TaMa>();
  p->setParam<int>("n", n);
  p->setParam<int>("matype", matype);
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

class TaT3 : public IndicatorImp {
  INDICATOR_IMP(TaT3)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaT3();
  virtual ~TaT3() = default;
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
BOOST_CLASS_EXPORT(hayaku::TaT3)
#endif

namespace hayaku {

TaT3::TaT3() : IndicatorImp("TA_T3", 1) {
  setParam<int>("n", 5);
  setParam<double>("vfactor", 0.7);
}

void TaT3::_checkParam(const string& name) const {
  if (name == "n") {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "n must >= 1 and <= 100000 ");
  } else if (name == "vfactor") {
    double vfactor = getParam<double>("vfactor");
    HAYAKU_ASSERT(vfactor >= 0.0 && vfactor <= 1.0);
  }
}

void TaT3::_calculate(const Indicator& data) {
  int n = getParam<int>("n");
  double vfactor = getParam<double>("vfactor");
  size_t total = data.size();
  int lookback = TA_T3_Lookback(n, vfactor);
  if (lookback < 0) {
    m_discard = total;
    return;
  }

  m_discard = data.discard() + lookback;
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  const double* src = data.data();
  auto* dst = this->data();
  int outBegIdx;
  int outNbElement;
  ::TA_T3(m_discard, total - 1, src, n, vfactor, &outBegIdx, &outNbElement,
          dst + m_discard);
  HAYAKU_ASSERT((outBegIdx == m_discard) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_T3(int n, double vfactor) {
  auto p = make_shared<TaT3>();
  p->setParam<int>("n", n);
  p->setParam<double>("vfactor", vfactor);
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

class TaMama : public IndicatorImp {
  INDICATOR_IMP(TaMama)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaMama();
  virtual ~TaMama() = default;
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
BOOST_CLASS_EXPORT(hayaku::TaMama)
#endif

namespace hayaku {

TaMama::TaMama() : IndicatorImp("TA_MAMA", 2) {
  setParam<double>("fast_limit", 0.5);
  setParam<double>("slow_limit", 0.05);
}

void TaMama::_checkParam(const string& name) const {
  if (name == "fast_limit" || name == "slow_limit") {
    double limit = getParam<double>(name);
    HAYAKU_CHECK(limit >= 0.01 && limit <= 0.99, "{} must be in [0.01, 0.99]!",
                 name);
  }
}

void TaMama::_calculate(const Indicator& data) {
  double fast_limit = getParam<double>("fast_limit");
  double slow_limit = getParam<double>("slow_limit");
  size_t total = data.size();
  int lookback = TA_MAMA_Lookback(fast_limit, slow_limit);
  if (lookback < 0) {
    m_discard = total;
    return;
  }

  m_discard = data.discard() + lookback;
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  const double* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);

  int outBegIdx;
  int outNbElement;
  ::TA_MAMA(m_discard, total - 1, src, fast_limit, slow_limit, &outBegIdx,
            &outNbElement, dst0 + m_discard, dst1 + m_discard);
  HAYAKU_ASSERT((outBegIdx == m_discard) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_MAMA(double fast_limit, double slow_limit) {
  auto p = make_shared<TaMama>();
  p->setParam<double>("fast_limit", fast_limit);
  p->setParam<double>("slow_limit", slow_limit);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include "operators/Indicator.h"
#include "operators/Indicator2InImp.h"

namespace hayaku {

class TaMavp : public Indicator2InImp {
  INDICATOR2IN_IMP(TaMavp)
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  TaMavp();
  TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype,
         bool fill_null);
  virtual ~TaMavp();

  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#include <ta-lib/ta_func.h>

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TaMavp)
#endif

namespace hayaku {

TaMavp::TaMavp() : Indicator2InImp("TA_MAVP", 1) {
  setParam<int>("min_n", 2);
  setParam<int>("max_n", 30);
  setParam<int>("matype", 0);
}

TaMavp::TaMavp(const Indicator& ref_ind, int min_n, int max_n, int matype,
               bool fill_null)
    : Indicator2InImp("TA_MAVP", ref_ind, fill_null, 1) {
  setParam<int>("min_n", min_n);
  setParam<int>("max_n", max_n);
  setParam<int>("matype", matype);
}

TaMavp::~TaMavp() {}

void TaMavp::_checkParam(const string& name) const {
  if ("min_n" == name || "max_n" == name) {
    int n = getParam<int>(name);
    HAYAKU_CHECK(n >= 2 && n <= 100000, "{} must in [2, 100000]", name);
  } else if (name == "matype") {
    int matype = getParam<int>(name);
    HAYAKU_ASSERT(matype >= 0 && matype <= 8);
  }
}

void TaMavp::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int min_n = getParam<int>("min_n");
  int max_n = getParam<int>("max_n");
  TA_MAType matype = (TA_MAType)getParam<int>("matype");
  int lookback = TA_MAVP_Lookback(min_n, max_n, matype);
  if (lookback < 0) {
    m_discard = total;
    return;
  }

  m_discard = lookback + std::max(ind.discard(), ref.discard());
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  const auto* src0 = ind.data();
  const auto* src1 = ref.data();
  auto* dst = this->data();
  int outBegIdx;
  int outNbElement;
  ::TA_MAVP(m_discard, total - 1, src0, src1, min_n, max_n, matype, &outBegIdx,
            &outNbElement, dst + m_discard);
  HAYAKU_ASSERT((outBegIdx == m_discard) &&
                (outBegIdx + outNbElement) <= total);
}

Indicator HAYAKU_API TA_MAVP(const Indicator& ref_ind, int min_n, int max_n,
                             int matype, bool fill_null) {
  return Indicator(
      make_shared<TaMavp>(ref_ind, min_n, max_n, matype, fill_null));
}

}  // namespace hayaku
