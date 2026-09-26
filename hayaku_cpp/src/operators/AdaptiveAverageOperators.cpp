#include "WindowOperators.h"

/*
 * Ama.h
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#pragma once
#ifndef IAMA_H_
#define IAMA_H_

#include "Indicator.h"

namespace hayaku {

/*
 * Perry J. Kaufman adaptive moving average, see "Smarter Trading" (2006,
 * Guangdong Economy Publishing House) Parameters: n: the period window for
 * calculating the average, it must be an integer greater than 2 fast_n: the
 * period of the fast trend, generally 2, it does not need to be changed slow_n:
 * the N value of the corresponding slow EMA line, Kaufman generally sets it to
 * 30; the indicator converges when it exceeds about 60 and there is not much
 * influence
 */
class IAma : public IndicatorImp {
  INDICATOR_IMP(IAma)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IAma();
  virtual ~IAma() override;

  virtual void _checkParam(const string& name) const override;
  virtual void _dyn_calculate(const Indicator&) override;

  virtual size_t min_increment_start() const override;

 private:
  void _dyn_one_circle(const Indicator& ind, size_t curPos, int n, int fast_n,
                       int slow_n);
};

} /* namespace hayaku */
#endif /* IAMA_H_ */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-09
 *      Author: fasiondog
 */

#pragma once

#include "Indicator2InImp.h"

namespace hayaku {

/*
 * Dynamic moving average
 * Usage: DMA(X,A) gives the dynamic moving average of X.
 * Algorithm: if Y=DMA(X,A) then Y=A*X+(1-A)*Y', where Y' is the Y value of the
 * previous period. For example: DMA(CLOSE,VOL/CAPITAL) gives the average price
 * with the turnover rate as the smoothing factor
 */
class IDma : public Indicator2InImp {
  INDICATOR2IN_IMP(IDma)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IDma();
  explicit IDma(const Indicator& ref_a, bool fill_null);
  virtual ~IDma() override;
  virtual size_t min_increment_start() const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IKALMAN_H_
#define INDICATOR_IMP_IKALMAN_H_

namespace hayaku {

/* Kalman filter */
class IKalman : public IndicatorImp {
  INDICATOR_IMP(IKalman)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IKalman();
  virtual ~IKalman() override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IKALMAN_H_ */

/*
 * IAma.cpp
 *
 *  Created on: 2013-4-7
 *      Author: fasiondog
 */

#include <cmath>

#include "SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IAma)
#endif

namespace hayaku {

IAma::IAma() : IndicatorImp("AMA", 2) {
  setParam<int>("n", 10);
  setParam<int>("fast_n", 2);
  setParam<int>("slow_n", 30);
}

IAma::~IAma() {}

void IAma::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 1);
  } else if ("fast_n" == name) {
    HAYAKU_ASSERT(getParam<int>("fast_n") >= 0);
  } else if ("slow_n" == name) {
    HAYAKU_ASSERT(getParam<int>("slow_n") >= 0);
  }
}

void IAma::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);

  int n = getParam<int>("n");
  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");

  size_t start = discard_;

  price_t fastest = 2.0 / (fast_n + 1);
  price_t slowest = 2.0 / (slow_n + 1);
  price_t delta = fastest - slowest;

  price_t prevol = 0.0, vol = 0.0, er = 1.0;
  price_t ama = src[start];
  size_t first_end = start + n + 1 >= total ? total : start + n + 1;
  _set(ama, start, 0);
  _set(er, start, 1);
  for (size_t i = start + 1; i < first_end; ++i) {
    vol += std::fabs(src[i] - src[i - 1]);
    er = (vol == 0.0) ? 1.0 : (src[i] - src[start]) / vol;
    if (er > 1.0) er = 1.0;
    price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
    ama += c * (src[i] - ama);
    dst0[i] = ama;
    dst1[i] = er;
  }

  prevol = vol;
  for (size_t i = first_end; i < total; ++i) {
    vol = prevol + std::fabs(src[i] - src[i - 1]) -
          std::fabs(src[i + 1 - n] - src[i - n]);
    er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
    if (er > 1.0) er = 1.0;
    if (er < -1.0) er = -1.0;
    price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
    ama += c * (src[i] - ama);
    prevol = vol;
    dst0[i] = ama;
    dst1[i] = er;
  }
}

size_t IAma::min_increment_start() const {
  int n = getParam<int>("n");
  // start_pos >= n + 1
  return n + 1;
}

void IAma::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  auto const* src = data.data();
  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);

  int n = getParam<int>("n");
  int fast_n = getParam<int>("fast_n");
  int slow_n = getParam<int>("slow_n");
  HAYAKU_CHECK(start_pos >= n + 1, "start_pos: {}, n: {}", start_pos, n);

  price_t fastest = 2.0 / (fast_n + 1);
  price_t slowest = 2.0 / (slow_n + 1);
  price_t delta = fastest - slowest;

  price_t prevol = 0.0, vol = 0.0, er = 1.0;
  for (size_t i = start_pos + 1 - n; i < start_pos; ++i) {
    vol += std::fabs(src[i] - src[i - 1]);
  }

  price_t ama = dst0[start_pos - 1];
  er = (vol == 0.0) ? 1.0 : (src[start_pos - 1] - src[start_pos - 1 - n]) / vol;
  if (er > 1.0) er = 1.0;
  if (er < -1.0) er = -1.0;

  prevol = vol;
  for (size_t i = start_pos; i < total; ++i) {
    vol = prevol + std::fabs(src[i] - src[i - 1]) -
          std::fabs(src[i + 1 - n] - src[i - n]);
    er = (vol == 0.0) ? 1.0 : (src[i] - src[i - n]) / vol;
    if (er > 1.0) er = 1.0;
    if (er < -1.0) er = -1.0;
    price_t c = std::pow((std::fabs(er) * delta + slowest), 2);
    ama += c * (src[i] - ama);
    prevol = vol;
    dst0[i] = ama;
    dst1[i] = er;
  }
}

void IAma::_dyn_one_circle(const Indicator& ind, size_t curPos, int n,
                           int fast_n, int slow_n) {
  if (n < 1) {
    n = 1;
  }

  if (fast_n < 0) {
    fast_n = 0;
  }

  if (slow_n < 0) {
    slow_n = 0;
  }

  Indicator slice = SLICE(ind, 0, curPos + 1);
  Indicator ama = AMA(slice, n, fast_n, slow_n);
  if (ama.size() > 0) {
    size_t index = ama.size() - 1;
    _set(ama.get(index, 0), curPos, 0);
    _set(ama.get(index, 1), curPos, 1);
  }
}

void IAma::_dyn_calculate(const Indicator& ind) {
  auto iter = ind_params_.find("fast_n");
  Indicator fast_n = iter != ind_params_.end()
                         ? Indicator(iter->second)
                         : CVAL(ind, getParam<int>("fast_n"));
  iter = ind_params_.find("slow_n");
  Indicator slow_n = iter != ind_params_.end()
                         ? Indicator(iter->second)
                         : CVAL(ind, getParam<int>("slow_n"));
  iter = ind_params_.find("n");
  Indicator n = iter != ind_params_.end() ? Indicator(iter->second)
                                           : CVAL(ind, getParam<int>("n"));

  HAYAKU_CHECK(fast_n.size() == ind.size(),
               "ind_param(fast_n).size()={}, ind.size()={}!", fast_n.size(),
               ind.size());
  HAYAKU_CHECK(slow_n.size() == ind.size(),
               "ind_param(slow_n).size()={}, ind.size()={}!", slow_n.size(),
               ind.size());

  discard_ = std::max(ind.discard(), fast_n.discard());
  discard_ = std::max(discard_, slow_n.discard());
  discard_ = std::max(discard_, n.discard());
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total || discard_ >= total, void());

  global_parallel_for_index_void(
      ind.discard(), total,
      [&](size_t i) { _dyn_one_circle(ind, i, n[i], fast_n[i], slow_n[i]); },
      400);

  updateDiscard();
}

Indicator HAYAKU_API AMA(int n, int fast_n, int slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setParam<int>("n", n);
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(int n, const IndParam& fast_n, int slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setParam<int>("n", n);
  p->setIndParam("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(int n, const IndParam& fast_n,
                         const IndParam& slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setParam<int>("n", n);
  p->setIndParam("fast_n", fast_n);
  p->setIndParam("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(int n, int fast_n, const IndParam& slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setParam<int>("n", n);
  p->setParam<int>("fast_n", fast_n);
  p->setIndParam("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(const IndParam& n, int fast_n, int slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setIndParam("n", n);
  p->setParam<int>("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(const IndParam& n, const IndParam& fast_n,
                         int slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setIndParam("n", n);
  p->setIndParam("fast_n", fast_n);
  p->setParam<int>("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(const IndParam& n, int fast_n,
                         const IndParam& slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setIndParam("n", n);
  p->setParam<int>("fast_n", fast_n);
  p->setIndParam("slow_n", slow_n);
  return Indicator(p);
}

Indicator HAYAKU_API AMA(const IndParam& n, const IndParam& fast_n,
                         const IndParam& slow_n) {
  IndicatorImpPtr p = make_shared<IAma>();
  p->setIndParam("n", n);
  p->setIndParam("fast_n", fast_n);
  p->setIndParam("slow_n", slow_n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IDma)
#endif

namespace hayaku {

IDma::IDma() : Indicator2InImp("DMA") {}

IDma::IDma(const Indicator& ref_ind, bool fill_null)
    : Indicator2InImp("DMA", ref_ind, fill_null, 1) {}

IDma::~IDma() {}

void IDma::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  discard_ = std::max(ind.discard(), ref.discard());
  auto* y = this->data();
  const auto* a = ref.data();
  const auto* x = ind.data();
  y[discard_] = x[discard_];
  for (size_t i = discard_ + 1; i < total; i++) {
    if (std::isnan(y[i - 1])) {
      y[i] = x[i];
    } else {
      y[i] = a[i] * x[i] + (1 - a[i]) * y[i - 1];
    }
  }
}

size_t IDma::min_increment_start() const { return 1; }

void IDma::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  Indicator ref = prepare(data);
  auto* y = this->data();
  const auto* a = ref.data();
  const auto* x = data.data();
  for (size_t i = start_pos + 1; i < total; i++) {
    if (std::isnan(y[i - 1])) {
      y[i] = x[i];
    } else {
      y[i] = a[i] * x[i] + (1 - a[i]) * y[i - 1];
    }
  }
}

Indicator HAYAKU_API DMA(const Indicator& x, const Indicator& a,
                         bool fill_null) {
  auto p = make_shared<IDma>(a, fill_null);
  Indicator result(p);
  return result(x);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-03-02
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IKalman)
#endif

namespace hayaku {

IKalman::IKalman() : IndicatorImp("KALMAN", 1) {
  setParam<double>("q", 0.01);  // Process noise covariance
  setParam<double>("r", 0.1);   // Measurement noise covariance
}

IKalman::~IKalman() {}

void IKalman::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  value_t q = getParam<double>("q");
  value_t r = getParam<double>("r");

  auto const* src = data.data();
  auto* dst = this->data();

  value_t x = src[discard_];  // State estimate
  value_t p = 1.0;             // Estimation error covariance

  dst[discard_] = x;
  for (size_t i = discard_ + 1; i < total; ++i) {
    p = p + q;
    value_t k = p / (p + r);
    x = x + k * (src[i] - x);
    p = (1 - k) * p;
    dst[i] = x;
  }
}

Indicator HAYAKU_API KALMAN(double q, double r) {
  auto p = make_shared<IKalman>();
  p->setParam<double>("q", q);
  p->setParam<double>("r", r);
  return Indicator(p);
}

} /* namespace hayaku */
