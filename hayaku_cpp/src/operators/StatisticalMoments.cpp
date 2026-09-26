#include "StatisticsOperators.h"

/*
 * IKurtosis.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IKURTOSIS_H_
#define INDICATOR_IMP_IKURTOSIS_H_

#include "Indicator.h"

namespace hayaku {

/*
 * Calculate the unadjusted population kurtosis (excess kurtosis = kurtosis - 3)
 */
class IKurtosis : public hayaku::IndicatorImp {
  INDICATOR_IMP(IKurtosis)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IKurtosis();
  virtual ~IKurtosis() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IKURTOSIS_H_ */

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

class IMrr : public IndicatorImp {
  INDICATOR_IMP(IMrr)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IMrr();
  virtual ~IMrr() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 * ISkewness.h
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISKEWNESS_H_
#define INDICATOR_IMP_ISKEWNESS_H_

namespace hayaku {

/*
 * Calculate the unadjusted population skewness
 */
class ISkewness : public hayaku::IndicatorImp {
  INDICATOR_IMP(ISkewness)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISkewness();
  virtual ~ISkewness() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ISKEWNESS_H_ */

/*
 * IStdev.h
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

/*
 * Calculate the sample standard deviation within N periods
 * Parameters: n: N-day time window
 * TODO      ma : the function prototype for calculating the average
 *       link: the linkage flag of the average parameters, true by default
 *
 */
class IStdev : public hayaku::IndicatorImp {
  INDICATOR_IMP(IStdev)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IStdev();
  virtual ~IStdev() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */

/*
 * IStdp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_ISTDP_H_
#define INDICATOR_IMP_ISTDP_H_

namespace hayaku {

/*
 * Calculate the population standard deviation within N periods
 * Parameters: n: N-day time window
 */
class IStdp : public hayaku::IndicatorImp {
  INDICATOR_IMP(IStdp)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IStdp();
  virtual ~IStdp() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_ISTDP_H_ */

/*
 * IVar.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IVAR_H_
#define INDICATOR_IMP_IVAR_H_

namespace hayaku {

/*
 * Estimate the sample variance
 */
class IVar : public hayaku::IndicatorImp {
  INDICATOR_IMP(IVar)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IVar();
  virtual ~IVar() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IVAR_H_ */

/*
 * IVarp.h
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-5-2
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_IVARP_H_
#define INDICATOR_IMP_IVARP_H_

namespace hayaku {

/*
 * Estimate the population sample variance
 */
class IVarp : public hayaku::IndicatorImp {
  INDICATOR_IMP(IVarp)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IVarp();
  virtual ~IVarp() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_IVARP_H_ */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

class IZScore : public IndicatorImp {
  INDICATOR_IMP(IZScore)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IZScore();
  IZScore(bool outExtreme, double nsigma, bool recursive);
  virtual ~IZScore() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 * IKurtosis.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IKurtosis)
#endif

namespace hayaku {

IKurtosis::IKurtosis() : IndicatorImp("KURT", 1) { setParam<int>("n", 10); }

IKurtosis::~IKurtosis() {}

void IKurtosis::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>(name);
    HAYAKU_ASSERT(n >= 4 || n == 0);
  }
}

void IKurtosis::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow2_buf(data.size());
  vector<price_t> pow3_buf(data.size());
  vector<price_t> pow4_buf(data.size());

  size_t start_pos = data.discard();
  size_t first_end = start_pos + n >= total ? total : start_pos + n;
  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;

  value_t k = src[start_pos];
  for (size_t i = start_pos; i < first_end; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
    value_t d4 = d3 * d;
    pow4_buf[i] = d4;
    ex4 += d4;
  }

  value_t var = ex2 / n - ex * ex / (n * n);
  value_t std_dev = sqrt(var);
  value_t std_dev_pow4 = var * var;

  if (std_dev == 0) {
    dst[first_end - 1] = -3.0;
  } else {
    value_t ex_over_n = ex / n;
    value_t ex2_over_n = ex2 / n;
    value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                            6 * ex2_over_n * ex_over_n * ex_over_n -
                            3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
    value_t kurtosis = fourth_moment / std_dev_pow4;
    dst[first_end - 1] = kurtosis - 3.0;
  }

  for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow2_buf[pre_ix];
    ex3 -= pow3_buf[pre_ix];
    ex4 -= pow4_buf[pre_ix];

    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
    value_t d4 = d3 * d;
    pow4_buf[i] = d4;
    ex4 += d4;

    var = ex2 / n - ex * ex / (n * n);
    std_dev = sqrt(var);
    std_dev_pow4 = var * var;

    if (std_dev == 0) {
      dst[i] = -3.0;
    } else {
      value_t ex_over_n = ex / n;
      value_t ex2_over_n = ex2 / n;
      value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                              6 * ex2_over_n * ex_over_n * ex_over_n -
                              3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
      value_t kurtosis = fourth_moment / std_dev_pow4;
      dst[i] = kurtosis - 3.0;
    }
  }
}

bool IKurtosis::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IKurtosis::min_increment_start() const { return getParam<int>("n"); }

void IKurtosis::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow2_buf(data.size());
  vector<price_t> pow3_buf(data.size());
  vector<price_t> pow4_buf(data.size());

  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
  value_t k = src[start_pos];
  for (size_t i = start_pos - n; i < start_pos; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
    value_t d4 = d3 * d;
    pow4_buf[i] = d4;
    ex4 += d4;
  }

  for (size_t i = start_pos, pre_ix = start_pos - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow2_buf[pre_ix];
    ex3 -= pow3_buf[pre_ix];
    ex4 -= pow4_buf[pre_ix];

    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
    value_t d4 = d3 * d;
    pow4_buf[i] = d4;
    ex4 += d4;

    value_t var = ex2 / n - ex * ex / (n * n);
    value_t std_dev = sqrt(var);
    value_t std_dev_pow4 = var * var;

    if (std_dev == 0) {
      dst[i] = -3.0;
    } else {
      value_t ex_over_n = ex / n;
      value_t ex2_over_n = ex2 / n;
      value_t fourth_moment = ex4 / n - 4 * (ex3 / n) * ex_over_n +
                              6 * ex2_over_n * ex_over_n * ex_over_n -
                              3 * ex_over_n * ex_over_n * ex_over_n * ex_over_n;
      value_t kurtosis = fourth_moment / std_dev_pow4;
      dst[i] = kurtosis - 3.0;
    }
  }
}

void IKurtosis::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                  size_t step) {
  HAYAKU_IF_RETURN(step < 4, void());

  size_t start = _get_step_start(curPos, step, ind.discard());
  HAYAKU_IF_RETURN(start != curPos + 1 - step, void());

  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0, ex4 = 0.0;
  value_t k = ind[start];
  for (size_t i = start; i <= curPos; i++) {
    value_t d = ind[i] - k;
    ex += d;
    value_t d2 = d * d;
    ex2 += d2;
    ex3 += d2 * d;
    ex4 += d2 * d2;
  }

  value_t var = ex2 / step - ex * ex / (step * step);
  value_t std_dev = sqrt(var);
  value_t std_dev_pow4 = var * var;

  if (std_dev == 0) {
    _set(-3.0, curPos);
  } else {
    value_t ex_over_step = ex / step;
    value_t ex2_over_step = ex2 / step;
    value_t fourth_moment =
        ex4 / step - 4 * (ex3 / step) * ex_over_step +
        6 * ex2_over_step * ex_over_step * ex_over_step -
        3 * ex_over_step * ex_over_step * ex_over_step * ex_over_step;
    value_t kurtosis = fourth_moment / std_dev_pow4;
    _set(kurtosis - 3.0, curPos);
  }
}

Indicator HAYAKU_API KURT(int n) {
  IndicatorImpPtr p = make_shared<IKurtosis>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API KURT(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IKurtosis>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IMrr)
#endif

namespace hayaku {

IMrr::IMrr() : IndicatorImp("MRR", 1) { setParam<int>("n", 0); }

IMrr::~IMrr() {}

void IMrr::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IMrr::_calculate(const Indicator& ind) {
  discard_ = ind.discard();
  size_t total = ind.size();
  HAYAKU_IF_RETURN(discard_ >= total, void());

  auto const* src = ind.data();
  auto* dst = this->data();

  size_t n = static_cast<size_t>(getParam<int>("n"));
  if (n == 0 || n > total - discard_) {
    n = total - discard_;
  }

  if (n == 1) {
    for (size_t i = discard_; i < total; ++i) {
      dst[i] = 0.0;
    }
    return;
  }

  if (n == total - discard_) {
    value_t pre_min = src[discard_];
    value_t max_rr = 0.0;
    for (size_t i = discard_; i < total; i++) {
      if (src[i] < pre_min || pre_min == 0.) {
        pre_min = src[i];
      }
      value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
      if (rr > max_rr) {
        max_rr = rr;
      }
      dst[i] = max_rr * 100.0;
    }
    return;
  }

  value_t pre_min = src[discard_];
  value_t max_rr = 0.0;
  for (size_t i = discard_; i < discard_ + n; ++i) {
    if (src[i] < pre_min || pre_min == 0.) {
      pre_min = src[i];
    }
    value_t rr = (src[i] <= pre_min) ? 0.0 : (src[i] / pre_min - 1.0);
    if (rr > max_rr) {
      max_rr = rr;
    }
    dst[i] = max_rr * 100.0;
  }

  if (discard_ + n < total) {
    _increment_calculate(ind, discard_ + n);
  }
}

bool IMrr::supportIncrementCalculate() const { return getParam<int>("n") > 1; }

size_t IMrr::min_increment_start() const { return getParam<int>("n"); }

void IMrr::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  size_t n = static_cast<size_t>(getParam<int>("n"));
  auto const* src = ind.data();
  auto* dst = this->data();

  // The standard maximum rise rate semantics: rr_j = src[j] / run_min_j - 1,
  // where run_min_j = min(src[window_left..j]) is the accumulated minimum up to
  // j. The original implementation wrongly used the global minimum of the
  // window as the rise base of all the points; when the lowest point of the
  // window appears after the highest point it introduces the look-ahead bias
  // and overestimates the rise rate. Here the original O(1) fast path state
  // machine is abandoned (its judgment of current_rr > window_max_rr does not
  // hold in principle under the standard MRR semantics) and it degenerates to
  // an O(n) brute force scan per point, using the run_min base to guarantee the
  // correctness. It is symmetric to the IMdd fix.
  for (size_t i = start_pos; i < total; ++i) {
    Indicator::value_t current_price = src[i];
    if (std::isnan(current_price) || current_price <= 0.0) {
      // An invalid point does not write dst[i] and keeps the original value,
      // the same as the original semantics
      continue;
    }

    size_t window_left = i + 1 - n;
    Indicator::value_t run_min =
        0.0;  // Assigned at the first valid point, avoiding NaN pollution
    Indicator::value_t window_max_rr = 0.0;
    bool has_valid = false;
    for (size_t j = window_left; j <= i; ++j) {
      Indicator::value_t v = src[j];
      if (std::isnan(v) || v <= 0.0) {
        continue;
      }
      if (!has_valid) {
        run_min = v;  // Initialize run_min at the first valid point
        has_valid = true;
        continue;  // For the first point rr = v/v - 1 = 0, skip it
      }
      if (v < run_min) {
        run_min = v;
      }
      // run_min is always > 0 (the data is constrained to be positive), the
      // division is safe
      Indicator::value_t rr = v / run_min - 1.0;
      if (rr > window_max_rr) {
        window_max_rr = rr;
      }
    }

    if (has_valid) {
      dst[i] = window_max_rr * 100.0;
    }
  }
}

Indicator HAYAKU_API MRR(int n) {
  IndicatorImpPtr p = make_shared<IMrr>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

}  // namespace hayaku

/*
 * ISkewness.cpp
 *
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISkewness)
#endif

namespace hayaku {

ISkewness::ISkewness() : IndicatorImp("SKEW", 1) { setParam<int>("n", 10); }

ISkewness::~ISkewness() {}

void ISkewness::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>(name);
    HAYAKU_ASSERT(n >= 3 || n == 0);
  }
}

void ISkewness::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow2_buf(data.size());
  vector<price_t> pow3_buf(data.size());

  size_t start_pos = data.discard();
  size_t first_end = start_pos + n >= total ? total : start_pos + n;
  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;

  value_t k = src[start_pos];
  for (size_t i = start_pos; i < first_end; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
  }

  value_t var = ex2 / n - ex * ex / (n * n);
  value_t std_dev = sqrt(var);

  if (std_dev == 0) {
    dst[first_end - 1] = 0.0;
  } else {
    value_t ex_over_n = ex / n;
    value_t third_moment = ex3 / n - 3 * (ex2 / n) * ex_over_n +
                           2 * ex_over_n * ex_over_n * ex_over_n;
    value_t skewness = third_moment / (std_dev * std_dev * std_dev);
    dst[first_end - 1] = skewness;
  }

  for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow2_buf[pre_ix];
    ex3 -= pow3_buf[pre_ix];

    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;

    var = ex2 / n - ex * ex / (n * n);
    std_dev = sqrt(var);

    if (std_dev == 0) {
      dst[i] = 0.0;
    } else {
      value_t ex_over_n = ex / n;
      value_t third_moment = ex3 / n - 3 * (ex2 / n) * ex_over_n +
                             2 * ex_over_n * ex_over_n * ex_over_n;
      value_t skewness = third_moment / (std_dev * std_dev * std_dev);
      dst[i] = skewness;
    }
  }
}

bool ISkewness::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t ISkewness::min_increment_start() const { return getParam<int>("n"); }

void ISkewness::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow2_buf(data.size());
  vector<price_t> pow3_buf(data.size());

  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
  value_t k = src[start_pos];
  for (size_t i = start_pos - n; i < start_pos; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;
  }

  for (size_t i = start_pos, pre_ix = start_pos - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow2_buf[pre_ix];
    ex3 -= pow3_buf[pre_ix];

    value_t d = src[i] - k;
    ex += d;
    value_t d2 = d * d;
    pow2_buf[i] = d2;
    ex2 += d2;
    value_t d3 = d2 * d;
    pow3_buf[i] = d3;
    ex3 += d3;

    value_t var = ex2 / n - ex * ex / (n * n);
    value_t std_dev = sqrt(var);

    if (std_dev == 0) {
      dst[i] = 0.0;
    } else {
      value_t ex_over_n = ex / n;
      value_t third_moment = ex3 / n - 3 * (ex2 / n) * ex_over_n +
                             2 * ex_over_n * ex_over_n * ex_over_n;
      value_t skewness = third_moment / (std_dev * std_dev * std_dev);
      dst[i] = skewness;
    }
  }
}

void ISkewness::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                  size_t step) {
  HAYAKU_IF_RETURN(step < 3, void());

  size_t start = _get_step_start(curPos, step, ind.discard());
  HAYAKU_IF_RETURN(start != curPos + 1 - step, void());

  value_t ex = 0.0, ex2 = 0.0, ex3 = 0.0;
  value_t k = ind[start];
  for (size_t i = start; i <= curPos; i++) {
    value_t d = ind[i] - k;
    ex += d;
    value_t d2 = d * d;
    ex2 += d2;
    ex3 += d2 * d;
  }

  value_t var = ex2 / step - ex * ex / (step * step);
  value_t std_dev = sqrt(var);

  if (std_dev == 0) {
    _set(0.0, curPos);
  } else {
    value_t ex_over_step = ex / step;
    value_t third_moment = ex3 / step - 3 * (ex2 / step) * ex_over_step +
                           2 * ex_over_step * ex_over_step * ex_over_step;
    value_t skewness = third_moment / (std_dev * std_dev * std_dev);
    _set(skewness, curPos);
  }
}

Indicator HAYAKU_API SKEW(int n) {
  IndicatorImpPtr p = make_shared<ISkewness>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API SKEW(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ISkewness>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IStd.cpp
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#include "WindowOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IStdev)
#endif

namespace hayaku {

IStdev::IStdev() : IndicatorImp("STDEV", 1) { setParam<int>("n", 10); }

IStdev::~IStdev() {}

void IStdev::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

void IStdev::_calculate(const Indicator& data) {
  size_t total = data.size();
  int n = getParam<int>("n");

  auto const* src = data.data();
  auto* dst = this->data();

  // n == 0: the full accumulated standard deviation (the expand-all semantics;
  // every position outputs the accumulated std up to the current position)
  if (0 == n) {
    discard_ = data.discard();
    if (discard_ >= total) {
      discard_ = total;
      return;
    }
    size_t valid_count = 0;
    price_t mean = 0.0;
    price_t M2 = 0.0;
    for (size_t i = discard_; i < total; ++i) {
      if (!std::isnan(src[i])) {
        valid_count++;
        if (valid_count == 1) {
          mean = src[i];
          M2 = 0.0;
        } else {
          price_t delta = src[i] - mean;
          mean += delta / valid_count;
          M2 += delta * (src[i] - mean);
        }
      }
      if (valid_count > 1) {
        dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
      }
    }
    return;
  }

  // n > 0: the rolling window Welford variance, the state is (valid_count,
  // mean, M2), the enqueue / dequeue is O(1). It shares the same
  // valid_count/mean update logic with IMa, guaranteeing that MA/STDEV are
  // based on a consistent sample set. When an outlier leaves the window, M2 may
  // become negative or lose the low order precision due to the catastrophic
  // cancellation; in that case an O(k) single pass recalculation (k = the
  // window length n) is triggered to rebuild the exact state.
  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  size_t startPos = data.discard();
  size_t valid_count = 0;
  price_t mean = 0.0;
  price_t M2 = 0.0;
  for (size_t i = startPos; i < total; ++i) {
    // Remove the leaving value
    if (i >= static_cast<size_t>(startPos) + static_cast<size_t>(n)) {
      price_t leaving = src[i - n];
      if (!std::isnan(leaving)) {
        if (valid_count > 1) {
          price_t old_M2 = M2;
          price_t delta = leaving - mean;
          mean -= delta / (valid_count - 1);
          M2 -= delta * (leaving - mean);
          valid_count--;
          // Catastrophic cancellation detection: recalculate when M2 becomes
          // negative or shrinks by a factor of 10^9 compared with the value
          // before remove (the float64 significant digits collapse)
          if (M2 < 0.0 || M2 < 1e-9 * old_M2) {
            // An O(n) single pass Welford recalculation of [i-n+1, i-1]
            // (leaving has been removed and entering has not been added)
            size_t vc_r = 0;
            price_t mean_r = 0.0, M2_r = 0.0;
            size_t lo = i - static_cast<size_t>(n) + 1;
            for (size_t j = lo; j < i; ++j) {
              if (!std::isnan(src[j])) {
                vc_r++;
                if (vc_r == 1) {
                  mean_r = src[j];
                } else {
                  price_t d = src[j] - mean_r;
                  mean_r += d / vc_r;
                  M2_r += d * (src[j] - mean_r);
                }
              }
            }
            valid_count = vc_r;
            mean = mean_r;
            M2 = M2_r;
          }
        } else {
          // valid_count == 1, the window becomes empty after the removal
          mean = 0.0;
          M2 = 0.0;
          valid_count = 0;
        }
      }
    }
    // Add the entering value
    price_t entering = src[i];
    if (!std::isnan(entering)) {
      valid_count++;
      if (valid_count == 1) {
        mean = entering;
        M2 = 0.0;
      } else {
        price_t delta = entering - mean;
        mean += delta / valid_count;
        M2 += delta * (entering - mean);
      }
    }
    // Write no output when the window is not full or the valid values are not
    // enough (the buffer is already NaN)
    if (i >= discard_ && valid_count > 1) {
      dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
    }
  }
}

bool IStdev::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IStdev::min_increment_start() const { return getParam<int>("n"); }

void IStdev::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();
  HAYAKU_ASSERT(start_pos + 1 >= (size_t)n);

  // Rebuild the Welford state with a single pass from the window start
  // start_pos+1-n, then roll to total
  size_t start = start_pos + 1 - n;
  size_t valid_count = 0;
  price_t mean = 0.0;
  price_t M2 = 0.0;
  for (size_t i = start; i < total; ++i) {
    if (i > start_pos) {
      price_t leaving = src[i - n];
      if (!std::isnan(leaving)) {
        if (valid_count > 1) {
          price_t old_M2 = M2;
          price_t delta = leaving - mean;
          mean -= delta / (valid_count - 1);
          M2 -= delta * (leaving - mean);
          valid_count--;
          if (M2 < 0.0 || M2 < 1e-9 * old_M2) {
            size_t vc_r = 0;
            price_t mean_r = 0.0, M2_r = 0.0;
            size_t lo = i - static_cast<size_t>(n) + 1;
            for (size_t j = lo; j < i; ++j) {
              if (!std::isnan(src[j])) {
                vc_r++;
                if (vc_r == 1) {
                  mean_r = src[j];
                } else {
                  price_t d = src[j] - mean_r;
                  mean_r += d / vc_r;
                  M2_r += d * (src[j] - mean_r);
                }
              }
            }
            valid_count = vc_r;
            mean = mean_r;
            M2 = M2_r;
          }
        } else {
          mean = 0.0;
          M2 = 0.0;
          valid_count = 0;
        }
      }
    }
    price_t entering = src[i];
    if (!std::isnan(entering)) {
      valid_count++;
      if (valid_count == 1) {
        mean = entering;
        M2 = 0.0;
      } else {
        price_t delta = entering - mean;
        mean += delta / valid_count;
        M2 += delta * (entering - mean);
      }
    }
    if (i >= start_pos && valid_count > 1) {
      dst[i] = std::sqrt(std::max(0.0, M2 / (valid_count - 1)));
    }
  }
}

void IStdev::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                               size_t step) {
  if (curPos + 1 < ind.discard() + step) {
    return;
  }
  size_t start = _get_step_start(curPos, step, ind.discard());
  // A single pass Welford (the left boundary of the dynamic window jumps,
  // remove is not used and no recalculation is needed)
  size_t valid_count = 0;
  price_t mean = 0.0;
  price_t M2 = 0.0;
  for (size_t i = start; i <= curPos; i++) {
    if (!std::isnan(ind[i])) {
      valid_count++;
      if (valid_count == 1) {
        mean = ind[i];
        M2 = 0.0;
      } else {
        price_t delta = ind[i] - mean;
        mean += delta / valid_count;
        M2 += delta * (ind[i] - mean);
      }
    }
  }
  if (valid_count > 1) {
    _set(std::sqrt(std::max(0.0, M2 / (valid_count - 1))), curPos);
  }
}

Indicator HAYAKU_API STDEV(int n) {
  IndicatorImpPtr p = make_shared<IStdev>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API STDEV(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IStdev>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IStdp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2019-4-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IStdp)
#endif

namespace hayaku {

IStdp::IStdp() : IndicatorImp("STDP", 1) { setParam<int>("n", 10); }

IStdp::~IStdp() {}

void IStdp::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

void IStdp::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  int n = getParam<int>("n");
  if (0 == n) {
    n = total;
  }

  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size());
  price_t ex = 0.0, ex2 = 0.0;
  size_t num = 0;
  size_t start_pos = discard_;
  size_t first_end = start_pos + n >= total ? total : start_pos + n;
  price_t k = src[start_pos];
  for (size_t i = start_pos; i < first_end; i++) {
    num++;
    price_t d = src[i] - k;
    ex += d;
    price_t d_pow = std::pow(d, 2);
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = std::sqrt((ex2 - std::pow(ex, 2) / num) / num);
  }

  for (size_t i = first_end; i < total; i++) {
    ex -= src[i - n] - k;
    ex2 -= pow_buf[i - n];
    price_t d = src[i] - k;
    ex += d;
    price_t d_pow = std::pow(d, 2);
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = std::sqrt((ex2 - std::pow(ex, 2) / n) / n);
  }
}
bool IStdp::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IStdp::min_increment_start() const { return getParam<int>("n"); }

void IStdp::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size() + n - start_pos);
  price_t ex = 0.0, ex2 = 0.0;
  price_t k = src[start_pos - n];
  for (size_t i = start_pos - n; i < start_pos; i++) {
    price_t d = src[i] - k;
    ex += d;
    price_t d_pow = std::pow(d, 2);
    pow_buf[i + n - start_pos] = d_pow;
    ex2 += d_pow;
  }

  for (size_t i = start_pos; i < total; i++) {
    ex -= src[i - n] - k;
    ex2 -= pow_buf[i - start_pos];
    price_t d = src[i] - k;
    ex += d;
    price_t d_pow = std::pow(d, 2);
    pow_buf[i + n - start_pos] = d_pow;
    ex2 += d_pow;
    dst[i] = std::sqrt((ex2 - std::pow(ex, 2) / n) / n);
  }
}

void IStdp::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                              size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  size_t num = 0;
  price_t ex = 0.0, ex2 = 0.0;
  price_t k = ind[start];
  for (size_t i = start; i <= curPos; i++) {
    num++;
    price_t d = ind[i] - k;
    ex += d;
    ex2 += std::pow(d, 2);
  }
  _set(num == 0 ? 0.0 : std::sqrt((ex2 - std::pow(ex, 2) / num) / num), curPos);
}

Indicator HAYAKU_API STDP(int n) {
  IndicatorImpPtr p = make_shared<IStdp>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API STDP(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IStdp>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IVar.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IVar)
#endif

namespace hayaku {

IVar::IVar() : IndicatorImp("VAR", 1) { setParam<int>("n", 10); }

IVar::~IVar() {}

void IVar::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>(name);
    HAYAKU_ASSERT(n >= 2 || n == 0);
  }
}

void IVar::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size());

  size_t start_pos = data.discard();
  size_t first_end = start_pos + n >= total ? total : start_pos + n;
  value_t ex = 0.0, ex2 = 0.0;

  value_t k = src[start_pos];
  for (size_t i = start_pos; i < first_end; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
  }

  value_t p1 = 1. / (n - 1);
  value_t p2 = 1. / (n * (n - 1.));
  dst[first_end - 1] = ex2 * p1 - ex * ex * p2;

  for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow_buf[pre_ix];
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = ex2 * p1 - ex * ex * p2;
  }
}

bool IVar::supportIncrementCalculate() const { return getParam<int>("n") != 0; }

size_t IVar::min_increment_start() const { return getParam<int>("n"); }

void IVar::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size());

  value_t ex = 0.0, ex2 = 0.0;
  value_t k = src[start_pos];
  for (size_t i = start_pos - n; i < start_pos; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
  }

  value_t p1 = 1. / (n - 1);
  value_t p2 = 1. / (n * (n - 1.));
  // dst[first_end - 1] = ex2 * p1 - ex * ex * p2;

  for (size_t i = start_pos, pre_ix = start_pos - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow_buf[pre_ix];
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = ex2 * p1 - ex * ex * p2;
  }
}

void IVar::_dyn_run_one_step(const Indicator& ind, size_t curPos, size_t step) {
  HAYAKU_IF_RETURN(step < 2, void());

  size_t start = _get_step_start(curPos, step, ind.discard());
  HAYAKU_IF_RETURN(start != curPos + 1 - step, void());

  price_t ex = 0.0, ex2 = 0.0;
  price_t k = ind[start];
  for (size_t i = start; i <= curPos; i++) {
    price_t d = ind[i] - k;
    ex += d;
    ex2 += d * d;
  }
  _set((ex2 - ex * ex / step) / (step - 1), curPos);
}

Indicator HAYAKU_API VAR(int n) {
  IndicatorImpPtr p = make_shared<IVar>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API VAR(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IVar>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */

/*
 * IVarp.cpp
 *
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2013-4-18
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IVarp)
#endif

namespace hayaku {

IVarp::IVarp() : IndicatorImp("VARP", 1) { setParam<int>("n", 10); }

IVarp::~IVarp() {}

void IVarp::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>(name);
    HAYAKU_ASSERT(n >= 2 || n == 0);
  }
}

void IVarp::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size());

  size_t start_pos = data.discard();
  size_t first_end = start_pos + n >= total ? total : start_pos + n;
  value_t ex = 0.0, ex2 = 0.0;

  value_t k = src[start_pos];
  for (size_t i = start_pos; i < first_end; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
  }

  value_t p1 = 1. / n;
  value_t p2 = 1. / (n * n);
  dst[first_end - 1] = ex2 * p1 - ex * ex * p2;

  for (size_t i = first_end, pre_ix = first_end - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow_buf[pre_ix];
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = ex2 * p1 - ex * ex * p2;
  }
}

bool IVarp::supportIncrementCalculate() const {
  return getParam<int>("n") != 0;
}

size_t IVarp::min_increment_start() const { return getParam<int>("n"); }

void IVarp::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  auto const* src = data.data();
  auto* dst = this->data();

  vector<price_t> pow_buf(data.size());

  value_t ex = 0.0, ex2 = 0.0;
  value_t k = src[start_pos];
  for (size_t i = start_pos - n; i < start_pos; i++) {
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
  }

  value_t p1 = 1. / n;
  value_t p2 = 1. / (n * n);
  // dst[first_end - 1] = ex2 * p1 - ex * ex * p2;

  for (size_t i = start_pos, pre_ix = start_pos - n; i < total; i++, pre_ix++) {
    ex -= src[pre_ix] - k;
    ex2 -= pow_buf[pre_ix];
    value_t d = src[i] - k;
    ex += d;
    value_t d_pow = d * d;
    pow_buf[i] = d_pow;
    ex2 += d_pow;
    dst[i] = ex2 * p1 - ex * ex * p2;
  }
}

void IVarp::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                              size_t step) {
  HAYAKU_IF_RETURN(step < 2, void());

  size_t start = _get_step_start(curPos, step, ind.discard());
  HAYAKU_IF_RETURN(start != curPos + 1 - step, void());

  price_t ex = 0.0, ex2 = 0.0;
  price_t k = ind[start];
  for (size_t i = start; i <= curPos; i++) {
    price_t d = ind[i] - k;
    ex += d;
    ex2 += d * d;
  }
  _set((ex2 - ex * ex / step) / step, curPos);
}

Indicator HAYAKU_API VARP(int n) {
  IndicatorImpPtr p = make_shared<IVarp>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator HAYAKU_API VARP(const IndParam& n) {
  IndicatorImpPtr p = make_shared<IVarp>();
  p->setIndParam("n", n);
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
BOOST_CLASS_EXPORT(hayaku::IZScore)
#endif

namespace hayaku {

IZScore::IZScore() : IndicatorImp("ZSCORE", 1) {
  setParam<double>("nsigma", 3);
  setParam<bool>("out-extreme", false);
  setParam<bool>("recursive", false);
}

IZScore::IZScore(bool outExtreme, double nsigma, bool recursive)
    : IndicatorImp("ZSCORE", 1) {
  setParam<double>("nsigma", nsigma);
  setParam<bool>("out-extreme", outExtreme);
  setParam<bool>("recursive", recursive);
}

IZScore::~IZScore() {}

void IZScore::_checkParam(const string& name) const {
  if ("nsigma" == name) {
    HAYAKU_ASSERT(getParam<double>("nsigma") > 0.);
  }
}

static void normalize(IndicatorImp::value_t* dst, Indicator::value_t const* src,
                      size_t total, bool outExtreme, double nsigma,
                      bool recursive) {
  IndicatorImp::value_t sum = 0.0;
  size_t count = 0;
  for (size_t i = 0; i < total; i++) {
    if (!std::isnan(src[i])) {
      sum += src[i];
      count++;
    }
  }

  HAYAKU_IF_RETURN(count <= 1, void());

  IndicatorImp::value_t mean = sum / count;

  vector<IndicatorImp::value_t> tmp(total, Null<IndicatorImp::value_t>());
  sum = 0.0;
  for (size_t i = 0; i < total; i++) {
    if (!std::isnan(src[i])) {
      tmp[i] = src[i] - mean;
      sum += tmp[i] * tmp[i];
    }
  }

  IndicatorImp::value_t sigma = std::sqrt(sum / (count - 1));
  for (size_t i = 0; i < total; i++) {
    if (!std::isnan(src[i])) {
      dst[i] = (src[i] - mean) / sigma;
    }
  }

  if (outExtreme) {
    IndicatorImp::value_t ulimit = nsigma;
    IndicatorImp::value_t llimit = -nsigma;

    bool found = false;
    for (size_t i = 0; i < total; i++) {
      if (!std::isnan(dst[i])) {
        if (dst[i] > ulimit) {
          dst[i] = ulimit;
          found = true;
        } else if (dst[i] < llimit) {
          dst[i] = llimit;
          found = true;
        }
      }
    }

    if (found && recursive) {
      normalize(dst, dst, total, outExtreme, nsigma, recursive);
    }
  }
}

void IZScore::_calculate(const Indicator& data) {
  size_t total = data.size();
  discard_ = data.discard();
  if (discard_ + 1 >= total) {
    discard_ = total;
    return;
  }

  double nsigma = getParam<double>("nsigma");
  bool outExtreme = getParam<bool>("out-extreme");
  bool recursive = getParam<bool>("recursive");
  auto const* src = data.data() + discard_;
  auto* dst = this->data() + discard_;
  normalize(dst, src, total - discard_, outExtreme, nsigma, recursive);

  for (size_t i = discard_; i < total; i++) {
    if (!std::isnan(dst[i])) {
      discard_ = i;
      break;
    }
  }
}

Indicator HAYAKU_API ZSCORE(bool outExtreme, double nsigma, bool recursive) {
  return Indicator(make_shared<IZScore>(outExtreme, nsigma, recursive));
}

}  // namespace hayaku
