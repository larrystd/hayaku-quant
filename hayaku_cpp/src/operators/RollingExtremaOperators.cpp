#include "WindowOperators.h"

/*
 * IHighLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016-4-1
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHIGHLINE_H_
#define INDICATOR_IMP_IHIGHLINE_H_

#include "Indicator.h"

namespace hayaku {

/*
 * The highest price within N days, the high price data is generally used as the
 * input Parameters: n: N-day time window
 */
class IHighLine : public IndicatorImp {
  INDICATOR_IMP(IHighLine)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IHighLine();
  virtual ~IHighLine() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

#endif /* INDICATOR_IMP_IHIGHLINE_H_ */

/*
 * IHhvbars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IHHVBARS_H_
#define INDICATOR_IMP_IHHVBARS_H_

namespace hayaku {

class IHhvbars : public IndicatorImp {
  INDICATOR_IMP(IHhvbars)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IHhvbars();
  virtual ~IHhvbars() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IHHVBARS_H_ */

/*
 * ILowLine.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILOWLINE_H_
#define INDICATOR_IMP_ILOWLINE_H_

namespace hayaku {

class ILowLine : public IndicatorImp {
  INDICATOR_IMP(ILowLine)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ILowLine();
  virtual ~ILowLine() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

#endif /* INDICATOR_IMP_ILOWLINE_H_ */

/*
 * ILowLineBars.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ILOWLINEBARS_H_
#define INDICATOR_IMP_ILOWLINEBARS_H_

namespace hayaku {

class ILowLineBars : public IndicatorImp {
  INDICATOR_IMP(ILowLineBars)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ILowLineBars();
  virtual ~ILowLineBars() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ILOWLINEBARS_H_ */

/*
 * IHighLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016-4-1
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IHighLine)
#endif

namespace hayaku {

IHighLine::IHighLine() : IndicatorImp("HHV", 1) { setParam<int>("n", 20); }

IHighLine::~IHighLine() {}

void IHighLine::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IHighLine::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  if (0 == total) {
    discard_ = 0;
    return;
  }

  if (ind.discard() >= total) {
    discard_ = total;
    return;
  }

  discard_ = ind.discard();
  if (1 == total) {
    if (0 == discard_) {
      _set(ind[0], 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (n <= 0) {
    n = total - discard_;
  } else if (n > total) {
    n = total;
  }

  size_t startPos = discard_;
  size_t first_end = startPos + n >= total ? total : startPos + n;

  auto const* src = ind.data();
  auto* dst = this->data();

  price_t max = src[startPos];
  size_t pre_pos = startPos;
  for (size_t i = startPos; i < first_end; i++) {
    if (src[i] >= max) {
      max = src[i];
      pre_pos = i;
    }
    dst[i] = max;
  }

  for (size_t i = first_end; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      max = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] >= max) {
          max = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] >= max) {
        max = src[i];
        pre_pos = i;
      }
    }
    dst[i] = max;
  }
}

bool IHighLine::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

size_t IHighLine::min_increment_start() const { return getParam<int>("n"); }

void IHighLine::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  discard_ = ind.discard();
  if (1 == total) {
    if (0 == discard_) {
      _set(ind[0], 0);
    }
    return;
  }

  int n = getParam<int>("n");
  auto const* src = ind.data();
  auto* dst = this->data();

  if (n >= total) {
    dst[discard_] = src[discard_];
    for (size_t i = discard_ + 1; i < total; i++) {
      if (src[i] > dst[i - 1]) {
        dst[i] = src[i];
      }
    }
  }

  price_t max = src[start_pos];
  size_t pre_pos = start_pos - n;
  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      max = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] >= max) {
          max = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] >= max) {
        max = src[i];
        pre_pos = i;
      }
    }
    dst[i] = max;
  }
}

void IHighLine::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                  size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  price_t max_val = ind[start];
  for (size_t i = start + 1; i <= curPos; i++) {
    if (ind[i] > max_val) {
      max_val = ind[i];
    }
  }
  _set(max_val, curPos);
}

Indicator HAYAKU_API HHV(int n) {
  IndicatorImpPtr p = make_shared<IHighLine>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API HHV(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IHighLine>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IHhvbars.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-11
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IHhvbars)
#endif

namespace hayaku {

IHhvbars::IHhvbars() : IndicatorImp("HHVBARS", 1) { setParam<int>("n", 20); }

IHhvbars::~IHhvbars() {}

void IHhvbars::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IHhvbars::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  if (0 == total) {
    discard_ = 0;
    return;
  }

  if (ind.discard() >= total) {
    discard_ = total;
    return;
  }

  discard_ = ind.discard();
  if (1 == total) {
    if (0 == discard_) {
      _set(0, 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (0 == n) {
    n = total - discard_;
  } else if (n > total) {
    n = total;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  price_t max = src[discard_];
  size_t pre_pos = discard_;
  size_t start_pos = discard_ + n < total ? discard_ + n : total;
  for (size_t i = discard_; i < start_pos; i++) {
    if (src[i] >= max) {
      max = src[i];
      pre_pos = i;
    }
    dst[i] = i - pre_pos;
  }

  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      max = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] >= max) {
          max = src[k];
          pre_pos = k;
        }
      }

    } else {
      if (src[i] >= max) {
        max = src[i];
        pre_pos = i;
      }
    }
    dst[i] = i - pre_pos;
  }
}

bool IHhvbars::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

size_t IHhvbars::min_increment_start() const { return getParam<int>("n"); }

void IHhvbars::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  if (1 == total) {
    if (0 == discard_) {
      _set(0, 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (n > total) {
    n = total;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  price_t max = src[start_pos - n];
  size_t pre_pos = start_pos - n;
  for (size_t i = start_pos - n; i < start_pos; i++) {
    if (src[i] >= max) {
      max = src[i];
      pre_pos = i;
    }
    // dst[i] = i - pre_pos;
  }

  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      max = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] >= max) {
          max = src[k];
          pre_pos = k;
        }
      }

    } else {
      if (src[i] >= max) {
        max = src[i];
        pre_pos = i;
      }
    }
    dst[i] = i - pre_pos;
  }
}

void IHhvbars::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                 size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  price_t maxVal = ind[start];
  size_t maxPos = start;
  for (size_t i = start + 1; i <= curPos; i++) {
    if (ind[i] > maxVal) {
      maxVal = ind[i];
      maxPos = i;
    }
  }
  _set(curPos - maxPos, curPos);
}

Indicator HAYAKU_API HHVBARS(int n) {
  IndicatorImpPtr p = make_shared<IHhvbars>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API HHVBARS(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IHhvbars>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * ILowLine.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2016-4-2
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILowLine)
#endif

namespace hayaku {

ILowLine::ILowLine() : IndicatorImp("LLV", 1) { setParam<int>("n", 20); }

ILowLine::~ILowLine() {}

void ILowLine::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void ILowLine::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  if (0 == total) {
    discard_ = 0;
    return;
  }

  if (ind.discard() >= total) {
    discard_ = total;
    return;
  }

  discard_ = ind.discard();
  if (1 == total) {
    if (0 == discard_) {
      _set(ind[0], 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (n <= 0) {
    n = total - discard_;
  } else if (n > total) {
    n = total;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  size_t startPos = discard_;
  size_t first_end = startPos + n >= total ? total : startPos + n;

  price_t min = src[startPos];
  size_t pre_pos = startPos;
  for (size_t i = startPos; i < first_end; i++) {
    if (src[i] <= min) {
      min = src[i];
      pre_pos = i;
    }
    dst[i] = min;
  }

  for (size_t i = first_end; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      min = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] <= min) {
          min = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] <= min) {
        min = src[i];
        pre_pos = i;
      }
    }
    dst[i] = min;
  }
}

bool ILowLine::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

size_t ILowLine::min_increment_start() const { return getParam<int>("n"); }

void ILowLine::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  int n = getParam<int>("n");
  auto const* src = ind.data();
  auto* dst = this->data();

  if (n >= total) {
    dst[discard_] = src[discard_];
    for (size_t i = discard_ + 1; i < total; i++) {
      if (src[i] < dst[i - 1]) {
        dst[i] = src[i];
      }
    }
  }

  price_t min = src[start_pos - n];
  size_t pre_pos = start_pos - n;
  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      min = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] <= min) {
          min = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] <= min) {
        min = src[i];
        pre_pos = i;
      }
    }
    dst[i] = min;
  }
}

void ILowLine::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                 size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  price_t min_val = ind[start];
  for (size_t i = start + 1; i <= curPos; i++) {
    if (ind[i] < min_val) {
      min_val = ind[i];
    }
  }
  _set(min_val, curPos);
}

Indicator HAYAKU_API LLV(int n) {
  IndicatorImpPtr p = make_shared<ILowLine>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API LLV(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ILowLine>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * ILowLineBars.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-14
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ILowLineBars)
#endif

namespace hayaku {

ILowLineBars::ILowLineBars() : IndicatorImp("LLVBARS", 1) {
  setParam<int>("n", 20);
}

ILowLineBars::~ILowLineBars() {}

void ILowLineBars::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void ILowLineBars::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  if (0 == total) {
    discard_ = 0;
    return;
  }

  if (ind.discard() >= total) {
    discard_ = total;
    return;
  }

  discard_ = ind.discard();
  if (1 == total) {
    if (0 == discard_) {
      _set(0, 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (0 == n) {
    n = total - discard_;
  } else if (n > total) {
    n = total;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  price_t min = src[discard_];
  size_t pre_pos = discard_;
  size_t start_pos = discard_ + n < total ? discard_ + n : total;
  for (size_t i = discard_; i < start_pos; i++) {
    if (src[i] <= min) {
      min = src[i];
      pre_pos = i;
    }
    dst[i] = i - pre_pos;
  }

  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      min = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] <= min) {
          min = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] <= min) {
        min = src[i];
        pre_pos = i;
      }
    }
    dst[i] = i - pre_pos;
  }
}

bool ILowLineBars::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

size_t ILowLineBars::min_increment_start() const { return getParam<int>("n"); }

void ILowLineBars::_increment_calculate(const Indicator& ind,
                                        size_t start_pos) {
  size_t total = ind.size();
  if (1 == total) {
    if (0 == discard_) {
      _set(0, 0);
    }
    return;
  }

  int n = getParam<int>("n");
  if (n > total) {
    n = total;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  price_t min = src[start_pos - n];
  size_t pre_pos = start_pos - n;
  for (size_t i = start_pos - n; i < start_pos; i++) {
    if (src[i] <= min) {
      min = src[i];
      pre_pos = i;
    }
    // dst[i] = i - pre_pos;
  }

  for (size_t i = start_pos; i < total; i++) {
    size_t j = i + 1 - n;
    if (pre_pos < j) {
      pre_pos = j;
      min = src[j];
      for (size_t k = pre_pos + 1; k <= i; k++) {
        if (src[k] <= min) {
          min = src[k];
          pre_pos = k;
        }
      }
    } else {
      if (src[i] <= min) {
        min = src[i];
        pre_pos = i;
      }
    }
    dst[i] = i - pre_pos;
  }
}

void ILowLineBars::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                     size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  price_t minVal = ind[start];
  size_t minPos = start;
  for (size_t i = start + 1; i <= curPos; i++) {
    if (ind[i] < minVal) {
      minVal = ind[i];
      minPos = i;
    }
  }
  _set(curPos - minPos, curPos);
}

Indicator HAYAKU_API LLVBARS(int n) {
  IndicatorImpPtr p = make_shared<ILowLineBars>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API LLVBARS(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ILowLineBars>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */
