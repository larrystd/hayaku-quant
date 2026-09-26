#include "BooleanOperators.h"

// ---- Merged implementation type from IEvery.h ----
/*
 * IEvery.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */
#include "Indicator.h"

namespace hayaku {

/*
 * Always existing, EVERY (X,N) means the condition X always exists within N
 * periods
 */
class IEvery : public IndicatorImp {
  INDICATOR_IMP(IEvery)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IEvery();
  virtual ~IEvery() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IExist.h ----
/*
 * IExist.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Existence, EXIST(X,N) means the condition X exists within N periods
 */
class IExist : public IndicatorImp {
  INDICATOR_IMP(IExist)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IExist();
  virtual ~IExist() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

// ---- Merged implementation type from IFilter.h ----
/*
 * IFilter.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */

namespace hayaku {

class IFilter : public IndicatorImp {
  INDICATOR_IMP(IFilter)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IFilter();
  virtual ~IFilter() override;

  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

// ---- Merged implementation type from INot.h ----
/*
 * INot.h
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */

namespace hayaku {

class INot : public IndicatorImp {
  INDICATOR_IMP(INot)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  INot();
  virtual ~INot() override;
};

} /* namespace hayaku */

// ---- Merged implementation from IEvery.cpp ----
/*
 * IEvery.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-28
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IEvery)
#endif

namespace hayaku {

IEvery::IEvery() : IndicatorImp("EVERY", 1) { setParam<int>("n", 20); }

IEvery::~IEvery() {}

void IEvery::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IEvery::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total, void());

  int n = getParam<int>("n");
  if (0 == n) {
    auto const* src = ind.data();
    auto* dst = this->data();
    m_discard = ind.discard();
    for (size_t i = m_discard; i < total; i++) {
      price_t every = 1.0;
      for (size_t j = m_discard; j <= i; j++) {
        if (src[j] == 0.0) {
          every = 0.0;
          break;
        }
      }
      dst[i] = every;
    }
    return;
  }

  m_discard = ind.discard() + n - 1;
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(ind, m_discard);
}

bool IEvery::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IEvery::min_increment_start() const { return getParam<int>("n") - 1; }

void IEvery::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");
  auto const* src = ind.data();
  auto* dst = this->data();

  price_t every = 1;
  size_t pre_pos = start_pos + n - 1;
  for (size_t i = start_pos + 1 - n; i <= start_pos; i++) {
    if (src[i] == 0) {
      pre_pos = i;
      every = 0;
    }
  }

  dst[start_pos] = every;
  for (size_t i = start_pos + 1; i < total - 1; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      every = src[j] == 0 ? 0 : 1;
    }
    if (src[i] == 0) {
      pre_pos = i;
      every = 0;
    }
    dst[i] = every;
  }

  every = 1;
  for (size_t i = total - n; i < total; i++) {
    if (src[i] == 0) {
      every = 0;
      break;
    }
  }
  dst[total - 1] = every;
}

void IEvery::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                               size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step - 1) {
    return;
  } else {
    start = curPos + 1 - step;
  }

  price_t every = 1.0;
  for (size_t i = start; i <= curPos; i++) {
    if (ind[i] == 0.0) {
      every = 0.0;
      break;
    }
  }
  _set(every, curPos);
}

Indicator HAYAKU_API EVERY(int n) {
  IndicatorImpPtr p = make_shared<IEvery>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API EVERY(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IEvery>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IExist.cpp ----
/*
 * IExist.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-19
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IExist)
#endif

namespace hayaku {

IExist::IExist() : IndicatorImp("EXIST", 1) { setParam<int>("n", 20); }

IExist::~IExist() {}

void IExist::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IExist::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  if (n == 0) {
    auto const* src = ind.data();
    auto* dst = this->data();
    m_discard = ind.discard();
    for (size_t i = m_discard; i < total; i++) {
      price_t exist = 0.0;
      for (size_t j = m_discard; j <= i; j++) {
        if (src[j] != 0.0) {
          exist = 1.0;
          break;
        }
      }
      dst[i] = exist;
    }
    return;
  }

  m_discard = ind.discard() + n - 1;
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(ind, m_discard);
}

bool IExist::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IExist::min_increment_start() const { return getParam<int>("n") - 1; }

void IExist::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  auto const* src = ind.data();
  auto* dst = this->data();
  int n = getParam<int>("n");

  price_t exist = 0;
  size_t pre_pos = start_pos + n - 1;
  for (size_t i = start_pos + 1 - n; i <= m_discard; i++) {
    if (src[i] != 0) {
      pre_pos = i;
      exist = 1.0;
    }
  }

  dst[start_pos] = exist;

  for (size_t i = start_pos + 1; i < total - 1; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      exist = src[j] != 0 ? 1 : 0;
    }
    if (src[i] != 0) {
      pre_pos = i;
      exist = 1;
    }
    dst[i] = exist;
  }

  exist = 0;
  for (size_t i = total - n; i < total; i++) {
    if (src[i] != 0) {
      exist = 1;
      break;
    }
  }
  dst[total - 1] = exist;
}

void IExist::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                               size_t step) {
  size_t start = 0;
  if (0 == step) {
    start = ind.discard();
  } else if (curPos < ind.discard() + step - 1) {
    return;
  } else {
    start = curPos + 1 - step;
  }

  price_t exist = 0.0;
  for (size_t i = start; i <= curPos; i++) {
    if (ind[i] != 0.0) {
      exist = 1.0;
      break;
    }
  }
  _set(exist, curPos);
}

Indicator HAYAKU_API EXIST(int n) {
  IndicatorImpPtr p = make_shared<IExist>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API EXIST(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IExist>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from IFilter.cpp ----
/*
 * IFilter.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-4
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IFilter)
#endif

namespace hayaku {

IFilter::IFilter() : IndicatorImp("FILTER", 1) {
  m_is_serial = true;
  setParam<int>("n", 5);
}

IFilter::~IFilter() {}

void IFilter::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IFilter::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  m_discard = ind.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  int n = getParam<int>("n");
  if (0 == n) {
    for (size_t i = m_discard; i < total; i++) {
      dst[i] = src[i] != 0.0 ? 1.0 : 0.0;
    }
    return;
  }

  _increment_calculate(ind, m_discard);
}

bool IFilter::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

void IFilter::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");
  auto const* src = ind.data();
  auto* dst = this->data();

  size_t i = start_pos;
  while (i < total) {
    if (src[i] == 0.0) {
      dst[i] = 0.0;
      i++;
    } else {
      dst[i] = 1.0;
      size_t end = i + n + 1;
      if (end > total) {
        end = total;
      }
      for (size_t j = i + 1; j < end; j++) {
        dst[j] = 0.0;
      }
      i = end;
    }
  }
}

void IFilter::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                size_t step) {
  price_t val = get(curPos);
  HAYAKU_IF_RETURN(!std::isnan(val) && val == 0.0, void());
  if (ind[curPos] == 0.0) {
    _set(0.0, curPos);
  } else {
    _set(1.0, curPos);
    size_t end = curPos + step + 1;
    if (end > ind.size()) {
      end = ind.size();
    }
    for (size_t i = curPos + 1; i < end; i++) {
      _set(0.0, i);
    }
  }
}

Indicator HAYAKU_API FILTER(int n) {
  IndicatorImpPtr p = make_shared<IFilter>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API FILTER(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IFilter>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

// ---- Merged implementation from INot.cpp ----
/*
 * INot.cpp
 *
 *  Created on: 2019-4-2
 *      Author: fasiondog
 */
#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::INot)
#endif

namespace hayaku {

INot::INot() : IndicatorImp("NOT", 1) {}

INot::~INot() {}

void INot::_calculate(const Indicator& data) {
  size_t total = data.size();
  m_discard = data.discard();
  if (m_discard >= total) {
    m_discard = total;
    return;
  }

  _increment_calculate(data, m_discard);
}

void INot::_increment_calculate(const Indicator& data, size_t start_pos) {
  auto const* src = data.data();
  auto* dst = this->data();
  for (size_t i = start_pos, total = data.size(); i < total; ++i) {
    dst[i] = (src[i] <= 0.0) ? 1.0 : 0.0;
  }
}

Indicator HAYAKU_API NOT() { return Indicator(make_shared<INot>()); }

} /* namespace hayaku */
