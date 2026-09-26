#include "BooleanOperators.h"

// ---- Merged implementation type from IIsInf.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

class IIsInf : public IndicatorImp {
  INDICATOR_IMP(IIsInf)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsInf();
  virtual ~IIsInf() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIsInfa.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

namespace hayaku {

class IIsInfa : public IndicatorImp {
  INDICATOR_IMP(IIsInfa)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsInfa();
  virtual ~IIsInfa() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIsLastBar.h ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */

namespace hayaku {

class IIsLastBar : public IndicatorImp {
  INDICATOR_IMP(IIsLastBar)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsLastBar();
  virtual ~IIsLastBar() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIsLimitDown.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

namespace hayaku {

class IIsLimitDown : public IndicatorImp {
  INDICATOR_IMP(IIsLimitDown)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsLimitDown();
  virtual ~IIsLimitDown() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIsLimitUp.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-25
 *      Author: fasiondog
 */

namespace hayaku {

class IIsLimitUp : public IndicatorImp {
  INDICATOR_IMP(IIsLimitUp)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsLimitUp();
  virtual ~IIsLimitUp() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IIsNa.h ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */

namespace hayaku {

class IIsNa : public IndicatorImp {
  INDICATOR_IMP(IIsNa)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IIsNa();
  virtual ~IIsNa() override;
};

} /* namespace hayaku */

// ---- Merged implementation from IIsInf.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsInf)
#endif

namespace hayaku {

IIsInf::IIsInf() : IndicatorImp("ISINF", 1) {}

IIsInf::~IIsInf() {}

void IIsInf::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IIsInf::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  value_t positive_inf = std::numeric_limits<value_t>::infinity();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = (src[i] == positive_inf) ? 1.0 : 0.0;
  }
}

Indicator HAYAKU_API ISINF() { return Indicator(make_shared<IIsInf>()); }

} /* namespace hayaku */

// ---- Merged implementation from IIsInfa.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsInfa)
#endif

namespace hayaku {

IIsInfa::IIsInfa() : IndicatorImp("ISINFA", 1) {}

IIsInfa::~IIsInfa() {}

void IIsInfa::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IIsInfa::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  value_t negative_inf = -std::numeric_limits<value_t>::infinity();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = (src[i] == negative_inf) ? 1.0 : 0.0;
  }
}

Indicator HAYAKU_API ISINFA() { return Indicator(make_shared<IIsInfa>()); }

} /* namespace hayaku */

// ---- Merged implementation from IIsLastBar.cpp ----
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-12-16
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsLastBar)
#endif

namespace hayaku {

IIsLastBar::IIsLastBar() : IndicatorImp("ISLASTBAR", 1) {}

IIsLastBar::~IIsLastBar() {}

void IIsLastBar::_calculate(const Indicator& data) {
  size_t total = data.size();
  if (isLeaf()) {
    const KData& k = getContext();
    total = k.size();
    _readyBuffer(total, 1);
    if (total >= 1) {
      auto* dst = this->data();
      memset(dst, 0, sizeof(value_t) * (total - 1));
      dst[total - 1] = 1.;
    }
    return;
  }

  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  size_t len = total - discard_;
  HAYAKU_IF_RETURN(len == 0, void());

  auto* dst = this->data() + discard_;
  memset(dst, 0, sizeof(value_t) * len);
  dst[total - 1] = 1.;
}

Indicator HAYAKU_API ISLASTBAR() {
  return Indicator(make_shared<IIsLastBar>());
}

Indicator HAYAKU_API ISLASTBAR(const KData& kdata) {
  auto p = make_shared<IIsLastBar>();
  p->setContext(kdata);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IIsLimitDown.cpp ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-26
 *      Author: fasiondog
 */

#include "data/StockTypeInfo.h"
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsLimitDown)
#endif

namespace hayaku {

IIsLimitDown::IIsLimitDown() : IndicatorImp("ISLIMITDOWN", 1) {
  need_context_ = true;
}

IIsLimitDown::~IIsLimitDown() {}

void IIsLimitDown::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 getParam<string>("kpart"));

  size_t total = getContext().size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);
  discard_ =
      1;  // No previous K-line, so the limit down cannot be judged; discard it
  _increment_calculate(ind, 0);
}

void IIsLimitDown::_increment_calculate(const Indicator& data,
                                        size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  value_t limit_down = 0.0;
  const Stock& stock = kdata.getStock();
  if (stock.type() == STOCKTYPE_A) {
    limit_down = 0.9;  // 10% for the A-shares, but 5% for the ST stocks (not
                       // handled: no ST date)
  } else if (stock.type() == STOCKTYPE_A_BJ) {
    limit_down = 0.7;  // 30% for the Beijing Stock Exchange
  } else if (stock.type() == STOCKTYPE_GEM || stock.type() == STOCKTYPE_START) {
    limit_down = 0.8;  // 20% for the ChiNext and the STAR Market
  }

  auto const* ks = kdata.data();
  auto* dst = this->data();
  if (limit_down > 0.0) {
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] =
          value_t(ks[i].closePrice <= roundEx(ks[i - 1].closePrice * limit_down,
                                              stock.precision()));
    }
  } else {
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = 0.0;  // An unsupported security type, regarded as not limit down
    }
  }
}

Indicator HAYAKU_API ISLIMITDOWN() {
  return make_shared<IIsLimitDown>()->calculate();
}

Indicator HAYAKU_API ISLIMITDOWN(const KData& k) {
  auto p = make_shared<IIsLimitDown>();
  p->setContext(k);
  return Indicator(p);
}

}  // namespace hayaku

// ---- Merged implementation from IIsLimitUp.cpp ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-25
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsLimitUp)
#endif

namespace hayaku {

IIsLimitUp::IIsLimitUp() : IndicatorImp("ISLIMITUP", 1) {
  need_context_ = true;
}

IIsLimitUp::~IIsLimitUp() {}

void IIsLimitUp::_calculate(const Indicator& ind) {
  HAYAKU_WARN_IF(!isLeaf() && !ind.empty(),
                 "The input is ignored because {} depends on the context!",
                 getParam<string>("kpart"));

  size_t total = getContext().size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);
  discard_ =
      1;  // No previous K-line, so the limit up cannot be judged; discard it
  _increment_calculate(ind, 0);
}

void IIsLimitUp::_increment_calculate(const Indicator& data, size_t start_pos) {
  const KData& kdata = getContext();
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(total == 0, void());

  value_t limit_up = 0.0;
  const Stock& stock = kdata.getStock();
  if (stock.type() == STOCKTYPE_A) {
    limit_up = 1.1;  // 10% for the A-shares, but 5% for the ST stocks (not
                     // handled: no ST date)
  } else if (stock.type() == STOCKTYPE_A_BJ) {
    limit_up = 1.3;  // 30% for the Beijing Stock Exchange
  } else if (stock.type() == STOCKTYPE_GEM || stock.type() == STOCKTYPE_START) {
    limit_up = 1.2;  // 20% for the ChiNext and the STAR Market
  }

  auto const* ks = kdata.data();
  auto* dst = this->data();
  if (limit_up > 0.0) {
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] =
          value_t(ks[i].closePrice >=
                  roundEx(ks[i - 1].closePrice * limit_up, stock.precision()));
    }
  } else {
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = 0.0;  // An unsupported security type, regarded as not limit up
    }
  }
}

Indicator HAYAKU_API ISLIMITUP() {
  return make_shared<IIsLimitUp>()->calculate();
}

Indicator HAYAKU_API ISLIMITUP(const KData& k) {
  auto p = make_shared<IIsLimitUp>();
  p->setContext(k);
  return Indicator(p);
}

}  // namespace hayaku

// ---- Merged implementation from IIsNa.cpp ----
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-01-08
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIsNa)
#endif

namespace hayaku {

IIsNa::IIsNa() : IndicatorImp("ISNA", 1) {
  setParam<bool>("ignore_discard", false);
}

IIsNa::~IIsNa() {}

void IIsNa::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = getParam<bool>("ignore_discard") ? 0 : data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IIsNa::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = std::isnan(src[i]);
  }
}

Indicator HAYAKU_API ISNA(bool ignore_discard) {
  auto p = make_shared<IIsNa>();
  p->setParam<bool>("ignore_discard", ignore_discard);
  return Indicator(p);
}

} /* namespace hayaku */
