#include "ScalarMathOperators.h"

// ---- Merged implementation type from IAbs.h ----
/*
 * IAbs.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

class IAbs : public IndicatorImp {
  INDICATOR_IMP(IAbs)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAbs();
  virtual ~IAbs() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IExp.h ----
/*
 * IExp.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

namespace hayaku {

class IExp : public IndicatorImp {
  INDICATOR_IMP(IExp)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IExp();
  virtual ~IExp() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ILn.h ----
/*
 * ILn.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

namespace hayaku {

class ILn : public IndicatorImp {
  INDICATOR_IMP(ILn)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ILn();
  virtual ~ILn() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ILog.h ----
/*
 * ILog.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

namespace hayaku {

class ILog : public IndicatorImp {
  INDICATOR_IMP(ILog)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ILog();
  virtual ~ILog() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IPow.h ----
/*
 * IPow.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Power
 */
class IPow : public IndicatorImp {
  INDICATOR_IMP(IPow)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IPow();
  virtual ~IPow() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ISqrt.h ----
/*
 * ISqrt.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Power
 */
class ISqrt : public IndicatorImp {
  INDICATOR_IMP(ISqrt)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISqrt();
  virtual ~ISqrt() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ISign.h ----
/*
 * ISign.h
 *
 *  Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */

namespace hayaku {

class ISign : public IndicatorImp {
  INDICATOR_IMP(ISign)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISign();
  virtual ~ISign() override;
};

} /* namespace hayaku */

// ---- Merged implementation type from ISignedPower.h ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Power
 */
class ISignedPower : public IndicatorImp {
  INDICATOR_IMP(ISignedPower)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISignedPower();
  virtual ~ISignedPower() override;
  virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */

// ---- Merged implementation from IAbs.cpp ----
/*
 * IAbs.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAbs)
#endif

namespace hayaku {

IAbs::IAbs() : IndicatorImp("ABS", 1) {}

IAbs::~IAbs() {}

void IAbs::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void IAbs::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::abs(src[i]);
  }
}

Indicator HAYAKU_API ABS() { return Indicator(make_shared<IAbs>()); }

} /* namespace hayaku */

// ---- Merged implementation from IExp.cpp ----
/*
 * IExp.cpp
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IExp)
#endif

namespace hayaku {

IExp::IExp() : IndicatorImp("EXP", 1) {}

IExp::~IExp() {}

void IExp::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void IExp::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::exp(src[i]);
  }
}

Indicator HAYAKU_API EXP() { return Indicator(make_shared<IExp>()); }

} /* namespace hayaku */

// ---- Merged implementation from ILn.cpp ----
/*
 * ILn.cpp
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILn)
#endif

namespace hayaku {

ILn::ILn() : IndicatorImp("LN", 1) {}

ILn::~ILn() {}

void ILn::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void ILn::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::log(src[i]);
  }
}

Indicator HAYAKU_API LN() { return Indicator(make_shared<ILn>()); }

} /* namespace hayaku */

// ---- Merged implementation from ILog.cpp ----
/*
 * ILog.cpp
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILog)
#endif

namespace hayaku {

ILog::ILog() : IndicatorImp("LOG", 1) {}

ILog::~ILog() {}

void ILog::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void ILog::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, end = data.size(); i < end; ++i) {
    dst[i] = std::log10(src[i]);
  }
}

Indicator HAYAKU_API LOG() { return Indicator(make_shared<ILog>()); }

} /* namespace hayaku */

// ---- Merged implementation from IPow.cpp ----
/*
 * IPow.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IPow)
#endif

namespace hayaku {

IPow::IPow() : IndicatorImp("POW", 1) { setParam<int>("n", 3); }

IPow::~IPow() {}

void IPow::_checkParam(const string& name) const {}

void IPow::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void IPow::_increment_calculate(const Indicator& data, size_t start_pos) {
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos; i < data.size(); ++i) {
    dst[i] = std::pow(src[i], n);
  }
}

void IPow::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
  _set(std::pow(ind[curPos], step), curPos);
}

Indicator HAYAKU_API POW(int n) {
  IndicatorImpPtr p = make_shared<IPow>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API POW(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IPow>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from ISqrt.cpp ----
/*
 * ISqrt.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-04-14
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISqrt)
#endif

namespace hayaku {

ISqrt::ISqrt() : IndicatorImp("SQRT", 1) {}

ISqrt::~ISqrt() {}

void ISqrt::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void ISqrt::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = std::sqrt(src[i]);
  }
}

Indicator HAYAKU_API SQRT() { return Indicator(make_shared<ISqrt>()); }

} /* namespace hayaku */

// ---- Merged implementation from ISign.cpp ----
/*
 * ISign.cpp
 *
 * Copyright (c) 2019 fasiondog
 *
 *  Created on: 2019-4-3
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISign)
#endif

namespace hayaku {

ISign::ISign() : IndicatorImp("SGN", 1) {}

ISign::~ISign() {}

void ISign::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  m_discard = ind.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(ind, m_discard);
  return;
}

void ISign::_increment_calculate(const Indicator& ind, size_t start_pos) {
  auto const* src = ind.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = ind.size(); i < total; i++) {
    value_t s = src[i];
    if (std::isnan(s)) continue;

    value_t pos = s > 0.0 ? 1.0 : 0.0;
    value_t neg = s < 0.0 ? 1.0 : 0.0;
    dst[i] = pos - neg;
  }
}

Indicator HAYAKU_API SGN() { return Indicator(make_shared<ISign>()); }

} /* namespace hayaku */

// ---- Merged implementation from ISignedPower.cpp ----
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-06-09
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISignedPower)
#endif

namespace hayaku {

ISignedPower::ISignedPower() : IndicatorImp("SIGNED_POWER", 1) {
  setParam<int>("n", 3);
}

ISignedPower::~ISignedPower() {}

void ISignedPower::_checkParam(const string& name) const {}

void ISignedPower::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void ISignedPower::_increment_calculate(const Indicator& data,
                                        size_t start_pos) {
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos; i < data.size(); ++i) {
    dst[i] =
        (std::signbit(src[i]) ? -1.0 : 1.0) * std::pow(std::abs(src[i]), n);
  }
}

void ISignedPower::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                     size_t step) {
  _set((std::signbit(ind[curPos]) ? -1.0 : 1.0) *
           std::pow(std::abs(ind[curPos]), step),
       curPos);
}

Indicator HAYAKU_API SIGNED_POWER(int n) {
  IndicatorImpPtr p = make_shared<ISignedPower>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API SIGNED_POWER(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ISignedPower>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */
