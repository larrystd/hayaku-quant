#include "RiskOperators.h"

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#include "Indicator.h"

namespace hayaku {

class IMdd : public IndicatorImp {
  INDICATOR_IMP(IMdd)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IMdd();
  virtual ~IMdd() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-12-24
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IMdd)
#endif

namespace hayaku {

IMdd::IMdd() : IndicatorImp("MDD", 1) { setParam<int>("n", 0); }

IMdd::~IMdd() {}

void IMdd::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void IMdd::_calculate(const Indicator& ind) {
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
    value_t pre_max = src[discard_];
    value_t min_dd = 0.0;
    for (size_t i = discard_; i < total; i++) {
      if (src[i] > pre_max) {
        pre_max = src[i];
      }
      value_t dd = (src[i] >= pre_max || pre_max == 0.)
                       ? 0.0
                       : (src[i] - pre_max) / pre_max;
      if (dd < min_dd) {
        min_dd = dd;
      }
      dst[i] = std::abs(min_dd * 100.0);
    }
    return;
  }

  value_t pre_max = src[discard_];
  value_t min_dd = 0.0;
  for (size_t i = discard_; i < discard_ + n; ++i) {
    if (src[i] > pre_max) {
      pre_max = src[i];
    }
    value_t dd = (src[i] >= pre_max || pre_max == 0.)
                     ? 0.0
                     : (src[i] - pre_max) / pre_max;
    if (dd < min_dd) {
      min_dd = dd;
    }
    dst[i] = std::abs(min_dd * 100.0);
  }

  if (discard_ + n < total) {
    _increment_calculate(ind, discard_ + n);
  }
}

bool IMdd::supportIncrementCalculate() const { return getParam<int>("n") > 1; }

size_t IMdd::min_increment_start() const { return getParam<int>("n"); }

void IMdd::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  size_t n = static_cast<size_t>(getParam<int>("n"));
  auto const* src = ind.data();
  auto* dst = this->data();

  // The standard maximum drawdown semantics: dd_j = (run_max_j - src[j]) /
  // run_max_j, where run_max_j = max(src[window_left..j]) is the accumulated
  // maximum up to j. The original implementation wrongly used the global
  // maximum of the window as the drawdown base of all the points; when the
  // highest point of the window appears after the lowest point it introduces
  // the look-ahead bias and overestimates the drawdown. Here the original O(1)
  // fast path state machine is abandoned (its judgment of current_dd >
  // window_max_dd does not hold in principle under the standard MDD semantics,
  // because the dd of different j uses a different run_max_j base), and it
  // degenerates to an O(n) brute force scan per point, using the run_max base
  // to guarantee the correctness.
  for (size_t i = start_pos; i < total; ++i) {
    Indicator::value_t current_nav = src[i];
    if (std::isnan(current_nav) || current_nav <= 0.0) {
      // An invalid point does not write dst[i] and keeps the original value,
      // the same as the original semantics
      continue;
    }

    size_t window_left = i + 1 - n;
    Indicator::value_t run_max =
        0.0;  // Not initialized with src[window_left], avoiding NaN pollution
    Indicator::value_t window_max_dd = 0.0;
    bool has_valid = false;
    for (size_t j = window_left; j <= i; ++j) {
      Indicator::value_t v = src[j];
      if (std::isnan(v) || v <= 0.0) {
        continue;
      }
      has_valid = true;
      if (v > run_max) {
        run_max = v;
      }
      // run_max is always > 0 (the data is constrained to be positive), the
      // division is safe
      Indicator::value_t dd = (run_max - v) / run_max;
      if (dd > window_max_dd) {
        window_max_dd = dd;
      }
    }

    if (has_valid) {
      dst[i] = window_max_dd * 100.0;
    }
  }
}

Indicator MDD(int n) {
  IndicatorImpPtr p = make_shared<IMdd>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */

namespace hayaku {

class IMddCurrent : public IndicatorImp {
  INDICATOR_IMP(IMddCurrent)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IMddCurrent();
  virtual ~IMddCurrent() override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-02
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IMddCurrent)
#endif

namespace hayaku {

IMddCurrent::IMddCurrent() : IndicatorImp("MDD_CURRENT", 1) {}

IMddCurrent::~IMddCurrent() {}

void IMddCurrent::_calculate(const Indicator& data) {
  discard_ = data.discard();
  size_t total = data.size();
  HAYAKU_IF_RETURN(discard_ >= total, void());

  _increment_calculate(data, discard_);
}

void IMddCurrent::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  auto const* src = ind.data();
  auto* dst = this->data();

  value_t run_max = std::numeric_limits<value_t>::lowest();
  for (size_t i = discard_; i < start_pos; ++i) {
    if (src[i] > run_max) {
      run_max = src[i];
    }
  }

  for (size_t i = start_pos; i < total; ++i) {
    value_t current = src[i];
    if (std::isnan(current)) {
      dst[i] = 0.0;
      continue;
    }
    if (current > run_max) {
      run_max = current;
    }
    dst[i] = (run_max - current) / run_max * 100.0;
  }
}

Indicator MDD_CURRENT() { return Indicator(make_shared<IMddCurrent>()); }

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#include "IndicatorImp.h"

namespace hayaku {

class IRSRSBeta : public IndicatorImp {
  INDICATOR_IMP(IRSRSBeta)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRSRSBeta();
  virtual ~IRSRSBeta() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRSRSBeta)
#endif

namespace hayaku {

IRSRSBeta::IRSRSBeta() : IndicatorImp("RSRS_BETA", 1) {
  need_context_ = true;
  setParam<int>("n", 20);
}

IRSRSBeta::~IRSRSBeta() {}

void IRSRSBeta::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 2);
  }
}

void IRSRSBeta::_calculate(const Indicator&) {
  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, 1);

  int n = getParam<int>("n");
  if (total < static_cast<size_t>(n)) {
    discard_ = total;
    return;
  }

  auto* dst = this->data();
  discard_ = n - 1;

  value_t sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0;
  for (size_t i = 0; i < static_cast<size_t>(n); i++) {
    value_t x = k[i].lowPrice;
    value_t y = k[i].highPrice;
    sum_x += x;
    sum_y += y;
    sum_xy += x * y;
    sum_x2 += x * x;
  }

  value_t null_price = Null<price_t>();
  value_t denominator = n * sum_x2 - sum_x * sum_x;
  dst[n - 1] = denominator == 0.0 ? null_price
                                  : (n * sum_xy - sum_x * sum_y) / denominator;

  for (size_t i = n; i < total; i++) {
    value_t old_x = k[i - n].lowPrice;
    value_t old_y = k[i - n].highPrice;
    value_t new_x = k[i].lowPrice;
    value_t new_y = k[i].highPrice;

    sum_x += new_x - old_x;
    sum_y += new_y - old_y;
    sum_xy += new_x * new_y - old_x * old_y;
    sum_x2 += new_x * new_x - old_x * old_x;

    denominator = n * sum_x2 - sum_x * sum_x;
    dst[i] = denominator == 0.0 ? null_price
                                : (n * sum_xy - sum_x * sum_y) / denominator;
  }
}

Indicator RSRS_BETA(int n) {
  auto p = make_shared<IRSRSBeta>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator RSRS_BETA(const KData& kdata, int n) {
  auto p = make_shared<IRSRSBeta>();
  p->setParam<int>("n", n);
  p->setContext(kdata);
  return Indicator(p);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

namespace hayaku {

class IRSRSBull : public IndicatorImp {
  INDICATOR_IMP(IRSRSBull)
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IRSRSBull();
  virtual ~IRSRSBull() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-07-12
 *  Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IRSRSBull)
#endif

namespace hayaku {

IRSRSBull::IRSRSBull() : IndicatorImp("RSRS_BULL", 4) {
  need_context_ = true;
  setParam<int>("n", 20);  // Regression window
  setParam<int>("m", 60);  // Z-score window
}

IRSRSBull::~IRSRSBull() {}

void IRSRSBull::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 2);
  } else if ("m" == name) {
    HAYAKU_ASSERT(getParam<int>("m") >= 2);
  }
}

void IRSRSBull::_calculate(const Indicator&) {
  const KData& k = getContext();
  size_t total = k.size();
  HAYAKU_IF_RETURN(total == 0, void());

  int n = getParam<int>("n");
  int m = getParam<int>("m");

  _readyBuffer(total, 4);

  size_t start_idx = n - 1;
  size_t z_start = start_idx + m - 1;

  if (total <= z_start) {
    discard_ = total;
    return;
  }

  auto* bull = this->data(
      0);  // The level 4 correction value (the right skew correction)
  auto* beta = this->data(1);  // The beta value
  auto* r2 = this->data(2);    // The R2 value
  auto* z = this->data(3);     // The Z-score value

  // Step 1: calculate the beta and the R2 of every point (starting from
  // start_idx)
  value_t sum_x = 0.0, sum_y = 0.0, sum_xy = 0.0, sum_x2 = 0.0, sum_y2 = 0.0;
  for (size_t i = 0; i < static_cast<size_t>(n); i++) {
    value_t x = k[i].lowPrice;
    value_t y = k[i].highPrice;
    sum_x += x;
    sum_y += y;
    sum_xy += x * y;
    sum_x2 += x * x;
    sum_y2 += y * y;
  }

  // Calculate the beta and the R2 of the (n-1)-th point
  value_t denom_beta = n * sum_x2 - sum_x * sum_x;
  value_t denom_r2 = n * sum_y2 - sum_y * sum_y;

  if (denom_beta != 0.0 && denom_r2 != 0.0) {
    beta[start_idx] = (n * sum_xy - sum_x * sum_y) / denom_beta;
    value_t ssr = beta[start_idx] * beta[start_idx] * denom_beta / n;
    value_t sst = denom_r2 / n;
    r2[start_idx] = ssr / sst;
  }

  // Calculate the beta and the R2 of the following points in a rolling way
  for (size_t i = n; i < total; i++) {
    value_t old_x = k[i - n].lowPrice;
    value_t old_y = k[i - n].highPrice;
    value_t new_x = k[i].lowPrice;
    value_t new_y = k[i].highPrice;

    sum_x += new_x - old_x;
    sum_y += new_y - old_y;
    sum_xy += new_x * new_y - old_x * old_y;
    sum_x2 += new_x * new_x - old_x * old_x;
    sum_y2 += new_y * new_y - old_y * old_y;

    denom_beta = n * sum_x2 - sum_x * sum_x;
    denom_r2 = n * sum_y2 - sum_y * sum_y;

    if (denom_beta != 0.0 && denom_r2 != 0.0) {
      beta[i] = (n * sum_xy - sum_x * sum_y) / denom_beta;
      value_t ssr = beta[i] * beta[i] * denom_beta / n;
      value_t sst = denom_r2 / n;
      r2[i] = ssr / sst;
    }
  }

  // Step 2: calculate the Z-score of the rolling M days
  // Calculate the mean and the standard deviation of the first M betas (from
  // start_idx to z_start)
  value_t sum_beta = 0.0, sum_beta2 = 0.0;
  int valid_count = 0;
  for (size_t i = start_idx; i <= z_start; i++) {
    if (!std::isnan(beta[i])) {
      sum_beta += beta[i];
      sum_beta2 += beta[i] * beta[i];
      valid_count++;
    }
  }

  if (valid_count >= 2) {
    value_t mean_beta = sum_beta / valid_count;
    value_t var_beta =
        (sum_beta2 - sum_beta * sum_beta / valid_count) / (valid_count - 1);
    value_t std_beta = std::sqrt(var_beta);

    if (std_beta > 0.0 && !std::isnan(beta[z_start])) {
      z[z_start] = (beta[z_start] - mean_beta) / std_beta;
      bull[z_start] = z[z_start] * r2[z_start] * beta[z_start];
    }

    // Calculate the following Z-scores and the level 4 correction values in a
    // rolling way
    for (size_t i = z_start + 1; i < total; i++) {
      // Remove the oldest beta
      size_t oldest_idx = i - m;
      if (oldest_idx >= start_idx && !std::isnan(beta[oldest_idx])) {
        sum_beta -= beta[oldest_idx];
        sum_beta2 -= beta[oldest_idx] * beta[oldest_idx];
        valid_count--;
      }

      // Add the new beta
      if (!std::isnan(beta[i])) {
        sum_beta += beta[i];
        sum_beta2 += beta[i] * beta[i];
        valid_count++;
      }

      if (valid_count >= 2) {
        mean_beta = sum_beta / valid_count;
        var_beta =
            (sum_beta2 - sum_beta * sum_beta / valid_count) / (valid_count - 1);
        std_beta = std::sqrt(var_beta);

        if (std_beta > 0.0 && !std::isnan(beta[i])) {
          z[i] = (beta[i] - mean_beta) / std_beta;
          bull[i] = z[i] * r2[i] * beta[i];
        }
      }
    }
  }

  // Set the discard and set the positions before beta and r2 to Null (refer to
  // the ADX way)
  discard_ = z_start;
  for (size_t i = 0; i < z_start; i++) {
    beta[i] = Null<value_t>();
    r2[i] = Null<value_t>();
  }
}

Indicator RSRS_BULL(int n, int m) {
  auto p = make_shared<IRSRSBull>();
  p->setParam<int>("n", n);
  p->setParam<int>("m", m);
  return Indicator(p);
}

Indicator RSRS_BULL(const KData& kdata, int n, int m) {
  auto p = make_shared<IRSRSBull>();
  p->setParam<int>("n", n);
  p->setParam<int>("m", m);
  p->setContext(kdata);
  return Indicator(p);
}

}  // namespace hayaku

/*
 * SaftyLoss.h
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

namespace hayaku {

/*
 * Alexander Elder's safe zone stop-loss
 * See "Come Into My Trading Room" (2007, Earthquake Press) by Alexander Elder,
 * P202 Calculation description: within the lookback period (generally 10 to 20
 * days), add up the lengths of all the downward crossings and divide by the
 * number of the downward crossings to get the average noise, and subtract (the
 * previous day's average noise multiplied by a multiple) from today's low price
 * to get the stop-loss line. To offset the fluctuation and guarantee that the
 * stop-loss line moves upward, the highest value within N days (generally 3
 * days) is taken based on the above result Note: the first (lookback period
 * width + the width for taking the highest value) points in the returned result
 * are invalid Parameters: n1: the lookback time window for calculating the
 * average noise, 10 days by default n2: take the highest value within n2 days
 * for the preliminary stop-loss line, 3 by default p: the noise coefficient, 2
 * by default
 */
class ISaftyLoss : public hayaku::IndicatorImp {
  INDICATOR_IMP(ISaftyLoss)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISaftyLoss();
  virtual ~ISaftyLoss() override;

  virtual void _checkParam(const string& name) const override;
  virtual void _dyn_calculate(const Indicator&) override;

 private:
  void _dyn_one_circle(const Indicator& ind, size_t curPos, int n1, int n2,
                       double p);
};

} /* namespace hayaku */

/*
 * ISaftyLoss.cpp
 *
 *  Created on: 2013-4-12
 *      Author: fasiondog
 */

#include "SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISaftyLoss)
#endif

namespace hayaku {

ISaftyLoss::ISaftyLoss() : IndicatorImp("SAFTYLOSS", 1) {
  setParam<int>("n1", 10);
  setParam<int>("n2", 3);
  setParam<double>("p", 2.0);
}

ISaftyLoss::~ISaftyLoss() {}

void ISaftyLoss::_checkParam(const string& name) const {
  if ("n1" == name) {
    HAYAKU_ASSERT(getParam<int>("n1") >= 2);
  } else if ("n2" == name) {
    HAYAKU_ASSERT(getParam<int>("n2") >= 1);
  }
}

void ISaftyLoss::_calculate(const Indicator& data) {
  size_t total = data.size();
  HAYAKU_IF_RETURN(total == 0, void());
  _readyBuffer(total, 1);

  int n1 = getParam<int>("n1");
  int n2 = getParam<int>("n2");

  discard_ = data.discard() + n1 + n2 - 2;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void ISaftyLoss::_increment_calculate(const Indicator& data, size_t start_pos) {
  size_t total = data.size();

  int n1 = getParam<int>("n1");
  int n2 = getParam<int>("n2");
  double p = getParam<double>("p");

  auto const* src = data.data();
  auto* dst = this->data();

  size_t start = start_pos;
  for (size_t i = start; i < total; ++i) {
    price_t result = 0.0;
    for (size_t j = i + 1 - n2; j <= i; ++j) {
      price_t sum = 0.0;
      size_t num = 0;
      for (size_t k = j + 2 - n1; k <= j; ++k) {
        price_t pre = src[k - 1];
        price_t cur = src[k];
        if (pre > cur) {
          sum += pre - cur;
          ++num;
        }
      }

      price_t temp = src[j];
      if (num != 0) {
        temp = temp - (p * sum / num);
      }

      if (temp > result) {
        result = temp;
      }
    }

    dst[i] = result;
  }
}

void ISaftyLoss::_dyn_one_circle(const Indicator& ind, size_t curPos, int n1,
                                 int n2, double p) {
  HAYAKU_IF_RETURN(n1 < 2 || n2 < 2, void());
  Indicator slice = SLICE(ind, 0, curPos + 1);
  Indicator st = SAFTYLOSS(slice, n1, n2, p);
  if (st.size() > 0) {
    _set(st[st.size() - 1], curPos);
  }
}

void ISaftyLoss::_dyn_calculate(const Indicator& ind) {
  auto iter = ind_params_.find("n1");
  Indicator n1 = iter != ind_params_.end() ? Indicator(iter->second)
                                           : CVAL(ind, getParam<int>("n1"));
  iter = ind_params_.find("n2");
  Indicator n2 = iter != ind_params_.end() ? Indicator(iter->second)
                                           : CVAL(ind, getParam<int>("n2"));
  iter = ind_params_.find("p");
  Indicator p = iter != ind_params_.end() ? Indicator(iter->second)
                                          : CVAL(ind, getParam<int>("p"));

  HAYAKU_CHECK(n1.size() == ind.size(),
               "ind_param(n1).size()={}, ind.size()={}!", n1.size(),
               ind.size());
  HAYAKU_CHECK(n2.size() == ind.size(),
               "ind_param(n2).size()={}, ind.size()={}!", n2.size(),
               ind.size());
  HAYAKU_CHECK(p.size() == ind.size(), "ind_param(p).size()={}, ind.size()={}!",
               p.size(), ind.size());

  discard_ = std::max(ind.discard(), n1.discard());
  discard_ = std::max(discard_, n2.discard());
  discard_ = std::max(discard_, p.discard());
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total || discard_ >= total, void());

  global_parallel_for_index_void(
      ind.discard(), total,
      [&](size_t i) { _dyn_one_circle(ind, i, n1[i], n2[i], p[i]); }, 400);

  updateDiscard();
}

Indicator SAFTYLOSS(int n1, int n2, double p) {
  IndicatorImpPtr result = make_shared<ISaftyLoss>();
  result->setParam<int>("n1", n1);
  result->setParam<int>("n2", n2);
  result->setParam<double>("p", p);
  return Indicator(result);
}

Indicator SAFTYLOSS(const IndParam& n1, const IndParam& n2, double p) {
  IndicatorImpPtr result = make_shared<ISaftyLoss>();
  result->setIndParam("n1", n1);
  result->setIndParam("n2", n2);
  result->setParam<double>("p", p);
  return Indicator(result);
}

Indicator SAFTYLOSS(const IndParam& n1, const IndParam& n2, const IndParam& p) {
  IndicatorImpPtr result = make_shared<ISaftyLoss>();
  result->setIndParam("n1", n1);
  result->setIndParam("n2", n2);
  result->setIndParam("p", p);
  return Indicator(result);
}

} /* namespace hayaku */
