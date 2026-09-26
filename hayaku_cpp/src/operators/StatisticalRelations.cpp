#include "StatisticsOperators.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#pragma once

#include "Indicator2InImp.h"

namespace hayaku {

class IBeta : public Indicator2InImp {
  INDICATOR2IN_IMP(IBeta)
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IBeta();
  IBeta(const Indicator& ref_ind, int n, bool fill_null);
  virtual ~IBeta() override;

  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

class ICorr : public Indicator2InImp {
  INDICATOR2IN_IMP(ICorr)
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICorr();
  ICorr(const Indicator& ref_ind, int n, bool fill_null);
  virtual ~ICorr() override;

  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

class ICov : public Indicator2InImp {
  INDICATOR2IN_IMP(ICov)
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ICov();
  ICov(const Indicator& ref_ind, int n, bool fill_null);
  virtual ~ICov() override;

  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

class ISpearman : public Indicator2InImp {
  INDICATOR2IN_IMP(ISpearman)
  INDICATOR2IN_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISpearman();
  ISpearman(const Indicator& ref_ind, int n, bool fill_null);
  virtual ~ISpearman() override;

  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hayaku {

/* Quantile truncation */
class IQuantileTrunc : public IndicatorImp {
  INDICATOR_IMP(IQuantileTrunc)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  IQuantileTrunc();
  virtual ~IQuantileTrunc() override;
  virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-XX-XX
 *  Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IBeta)
#endif

namespace hayaku {

IBeta::IBeta() : Indicator2InImp("BETA", 1) { setParam<int>("n", 10); }

IBeta::IBeta(const Indicator& ref_ind, int n, bool fill_null)
    : Indicator2InImp("BETA", ref_ind, fill_null, 1) {
  setParam<int>("n", n);
}

IBeta::~IBeta() {}

void IBeta::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

void IBeta::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  size_t startPos = std::max(ind.discard(), ref.discard());
  discard_ = startPos + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, vary = 0.0, cov = 0.0;
  value_t ey2 = 0.0;
  value_t ix, iy;

  auto* dst = this->data();
  size_t first_end = startPos + n >= total ? total : startPos + n;

  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    ey2 += iy * iy;
    exy += ix * iy;
  }

  value_t null_price = Null<price_t>();
  vary = ey2 - ey * ey / n;
  cov = exy - ex * ey / n;
  dst[first_end - 1] = vary == 0.0 ? null_price : cov / vary;

  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preiy = datay[i - n] - ky;
    ey2 += iy * iy - preiy * preiy;
    exy += ix * iy - (datax[i - n] - kx) * preiy;
    vary = (ey2 - ey * ey / n);
    cov = (exy - ex * ey / n);
    dst[i] = vary == 0.0 ? null_price : cov / vary;
  }
}

bool IBeta::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

size_t IBeta::min_increment_start() const { return getParam<int>("n"); }

void IBeta::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  size_t startPos = start_pos - n;

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, vary = 0.0, cov = 0.0;
  value_t ey2 = 0.0;
  value_t ix, iy;

  auto* dst = this->data();
  size_t first_end = start_pos;

  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    ey2 += iy * iy;
    exy += ix * iy;
  }

  vary = ey2 - ey * ey / n;
  cov = exy - ex * ey / n;

  value_t null_price = Null<price_t>();
  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preiy = datay[i - n] - ky;
    ey2 += iy * iy - preiy * preiy;
    exy += ix * iy - (datax[i - n] - kx) * preiy;
    vary = (ey2 - ey * ey / n);
    cov = (exy - ex * ey / n);
    dst[i] = vary == 0.0 ? null_price : cov / vary;
  }
}

Indicator HAYAKU_API BETA(const Indicator& ref_ind, int n, bool fill_null) {
  return Indicator(make_shared<IBeta>(ref_ind, n, fill_null));
}

Indicator HAYAKU_API BETA(const Indicator& ind1, const Indicator& ind2, int n,
                          bool fill_null) {
  auto p = make_shared<IBeta>(ind2, n, fill_null);
  Indicator result(p);
  return result(ind1);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICorr)
#endif

namespace hayaku {

ICorr::ICorr() : Indicator2InImp("CORR") { setParam<int>("n", 10); }

ICorr::ICorr(const Indicator& ref_ind, int n, bool fill_null)
    : Indicator2InImp("CORR", ref_ind, fill_null, 2) {
  setParam<int>("n", n);
}

ICorr::~ICorr() {}

void ICorr::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

void ICorr::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  size_t startPos = std::max(ind.discard(), ref.discard());
  discard_ = startPos + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  size_t first_end = startPos + n >= total ? total : startPos + n;

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, varx = 0.0, vary = 0.0, cov = 0.0;
  value_t ex2 = 0.0, ey2 = 0.0;
  value_t ix, iy;

  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    ex2 += ix * ix;
    ey2 += iy * iy;
    exy += ix * iy;
  }

  varx = ex2 - ex * ex / n;
  vary = ey2 - ey * ey / n;
  cov = exy - ex * ey / n;
  dst0[first_end - 1] = cov / std::sqrt(varx * vary);
  dst1[first_end - 1] = cov / (n - 1);

  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preix = datax[i - n] - kx;
    value_t preiy = datay[i - n] - ky;
    ex2 += ix * ix - preix * preix;
    ey2 += iy * iy - preiy * preiy;
    exy += ix * iy - preix * preiy;
    varx = (ex2 - ex * ex / n);
    vary = (ey2 - ey * ey / n);
    cov = (exy - ex * ey / n);
    dst0[i] = cov / std::sqrt(varx * vary);
    dst1[i] = cov / (n - 1);
  }
}

bool ICorr::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

size_t ICorr::min_increment_start() const { return getParam<int>("n"); }

void ICorr::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  size_t startPos = start_pos - n;

  size_t first_end =
      start_pos;  // startPos + n >= total ? total : startPos + n;

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, varx = 0.0, vary = 0.0, cov = 0.0;
  value_t ex2 = 0.0, ey2 = 0.0;
  value_t ix, iy;

  auto* dst0 = this->data(0);
  auto* dst1 = this->data(1);
  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    ex2 += ix * ix;
    ey2 += iy * iy;
    exy += ix * iy;
  }

  varx = ex2 - ex * ex / n;
  vary = ey2 - ey * ey / n;
  cov = exy - ex * ey / n;
  // dst0[first_end - 1] = cov / std::sqrt(varx * vary);
  // dst1[first_end - 1] = cov / (n - 1);

  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preix = datax[i - n] - kx;
    value_t preiy = datay[i - n] - ky;
    ex2 += ix * ix - preix * preix;
    ey2 += iy * iy - preiy * preiy;
    exy += ix * iy - preix * preiy;
    varx = (ex2 - ex * ex / n);
    vary = (ey2 - ey * ey / n);
    cov = (exy - ex * ey / n);
    dst0[i] = cov / std::sqrt(varx * vary);
    dst1[i] = cov / (n - 1);
  }
}

Indicator HAYAKU_API CORR(const Indicator& ref_ind, int n, bool fill_null) {
  return Indicator(make_shared<ICorr>(ref_ind, n, fill_null));
}

Indicator HAYAKU_API CORR(const Indicator& ind1, const Indicator& ind2, int n,
                          bool fill_null) {
  auto p = make_shared<ICorr>(ind2, n, fill_null);
  Indicator result(p);
  return result(ind1);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ICov)
#endif

namespace hayaku {

ICov::ICov() : Indicator2InImp("COV") { setParam<int>("n", 10); }

ICov::ICov(const Indicator& ref_ind, int n, bool fill_null)
    : Indicator2InImp("COV", ref_ind, fill_null, 1) {
  setParam<int>("n", n);
}

ICov::~ICov() {}

void ICov::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

void ICov::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  size_t startPos = std::max(ind.discard(), ref.discard());
  discard_ = startPos + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  size_t first_end = startPos + n >= total ? total : startPos + n;

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, cov = 0.0;
  value_t ix, iy;

  auto* dst0 = this->data(0);
  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    exy += ix * iy;
  }

  cov = exy - ex * ey / n;
  dst0[first_end - 1] = cov / (n - 1);

  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preix = datax[i - n] - kx;
    value_t preiy = datay[i - n] - ky;
    exy += ix * iy - preix * preiy;
    cov = (exy - ex * ey / n);
    dst0[i] = cov / (n - 1);
  }
}

bool ICov::supportIncrementCalculate() const { return getParam<int>("n") > 0; }

size_t ICov::min_increment_start() const { return getParam<int>("n"); }

void ICov::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  size_t startPos = start_pos - n;

  size_t first_end = start_pos;

  auto const* datax = ind.data();
  auto const* datay = ref.data();
  value_t kx = datax[startPos];
  value_t ky = datay[startPos];
  value_t ex = 0.0, ey = 0.0, exy = 0.0, cov = 0.0;
  value_t ix, iy;

  auto* dst0 = this->data(0);
  for (size_t i = startPos + 1; i < first_end; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += ix;
    ey += iy;
    exy += ix * iy;
  }

  cov = exy - ex * ey / n;

  for (size_t i = first_end; i < total; i++) {
    ix = datax[i] - kx;
    iy = datay[i] - ky;
    ex += datax[i] - datax[i - n];
    ey += datay[i] - datay[i - n];
    value_t preix = datax[i - n] - kx;
    value_t preiy = datay[i - n] - ky;
    exy += ix * iy - preix * preiy;
    cov = (exy - ex * ey / n);
    dst0[i] = cov / (n - 1);
  }
}

Indicator HAYAKU_API COV(const Indicator& ref_ind, int n, bool fill_null) {
  return Indicator(make_shared<ICov>(ref_ind, n, fill_null));
}

Indicator HAYAKU_API COV(const Indicator& ind1, const Indicator& ind2, int n,
                         bool fill_null) {
  auto p = make_shared<ICov>(ind2, n, fill_null);
  Indicator result(p);
  return result(ind1);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-10
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISpearman)
#endif

namespace hayaku {

ISpearman::ISpearman() : Indicator2InImp("SPEARMAN") { setParam<int>("n", 0); }

ISpearman::ISpearman(const Indicator& ref_ind, int n, bool fill_null)
    : Indicator2InImp("SPEARMAN", ref_ind, fill_null, 1) {
  setParam<int>("n", n);
}

ISpearman::~ISpearman() {}

void ISpearman::_checkParam(const string& name) const {
  if ("n" == name) {
    int n = getParam<int>("n");
    HAYAKU_ASSERT(n == 0 || n >= 2);
  }
}

static void spearmanLevel(const IndicatorImp::value_t* data,
                          IndicatorImp::value_t* level, size_t total) {
  std::vector<std::pair<IndicatorImp::value_t, size_t>> data_index(total);
  for (size_t i = 0; i < total; i++) {
    data_index[i].first = data[i];
    data_index[i].second = i;
  }

  std::sort(
      data_index.begin(), data_index.end(),
      std::bind(std::less<IndicatorImp::value_t>(),
                std::bind(&std::pair<IndicatorImp::value_t, size_t>::first,
                          std::placeholders::_1),
                std::bind(&std::pair<IndicatorImp::value_t, size_t>::first,
                          std::placeholders::_2)));

  size_t i = 0;
  while (i < total) {
    size_t count = 1;
    IndicatorImp::value_t score = i + 1.0;
    for (size_t j = i + 1; j < total; j++) {
      if (data_index[i].first != data_index[j].first) {
        break;
      }
      count++;
      score += j + 1;
    }
    score = score / count;
    for (size_t j = 0; j < count; j++) {
      level[data_index[i + j].second] = score;
    }
    i += count;
  }
}

void ISpearman::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  if (n == 0) {
    n = total;
  }

  discard_ = std::max(ind.discard(), ref.discard());
  discard_ += n - 1;
  if (discard_ > total) {
    discard_ = total;
    return;
  }

  _increment_calculate(ind, discard_);
}

bool ISpearman::supportIncrementCalculate() const {
  return getParam<int>("n") > 0;
}

size_t ISpearman::min_increment_start() const { return getParam<int>("n"); }

void ISpearman::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  Indicator ref = prepare(ind);

  int n = getParam<int>("n");
  // Consistent with _calculate: n=0 means the whole window. _calculate passes
  // only the normalized n to m_discard and then delegates to this function;
  // this function calls getParam again and would read the original 0, so it
  // must be converted once more.
  if (n == 0) {
    n = total;
  }
  auto* dst = this->data();
  auto const* srca = ind.data() + 1 - n;
  auto const* srcb = ref.data() + 1 - n;
  global_parallel_for_index_void(
      start_pos, total,
      [=](size_t i) {
        const auto* a = srca + i;
        const auto* b = srcb + i;
        vector<IndicatorImp::value_t> tmpa;
        vector<IndicatorImp::value_t> tmpb;
        tmpa.reserve(n);
        tmpa.reserve(n);
        for (int j = 0; j < n; j++) {
          if (!std::isnan(a[j]) && !std::isnan(b[j])) {
            tmpa.push_back(a[j]);
            tmpb.push_back(b[j]);
          }
        }
        int act_count = tmpa.size();
        if (act_count < 2) {
          return;
        }

        auto levela = std::vector<value_t>(n);
        auto levelb = std::vector<value_t>(n);
        auto* ptra = levela.data();
        auto* ptrb = levelb.data();

        spearmanLevel(tmpa.data(), ptra, act_count);
        spearmanLevel(tmpb.data(), ptrb, act_count);
        // Calculate the Pearson correlation coefficient on the ranks directly
        // (tie-safe). Key property: the average rank does not change the sum of
        // the ranks, so the mean of the ranks is always the prior constant
        // (act_count+1)/2, which needs no traversal estimation and eliminates
        // the catastrophic cancellation of subtracting large numbers.
        double mean_rank = (act_count + 1) / 2.0;
        double var_r = 0.0, var_s = 0.0, cov = 0.0;
        for (int j = 0; j < act_count; j++) {
          double dev_r = ptra[j] - mean_rank;
          double dev_s = ptrb[j] - mean_rank;
          var_r += dev_r * dev_r;
          var_s += dev_s * dev_s;
          cov += dev_r * dev_s;
        }
        // On a zero variance (the factors or the returns are all equal in the
        // cross section) it returns 0.0: the semantics is "no discrimination
        // means no predictive power". NaN is not returned, because a downstream
        // MA indicator spreads NaN like "one NaN makes all NaN" (and a NaN in
        // the first window silently outputs a low wrong value), the weighted
        // combination has no NaN guard, and NaN would spread and pollute the
        // weights of about ic_rolling_n days.
        if (var_r == 0.0 || var_s == 0.0) {
          dst[i] = 0.0;
        } else {
          double rho = cov / std::sqrt(var_r * var_s);
          // Clamp the out of range caused by the floating point error
          dst[i] = std::max(-1.0, std::min(1.0, rho));
        }
      },
      100);
}

Indicator HAYAKU_API SPEARMAN(const Indicator& ref_ind, int n, bool fill_null) {
  return Indicator(make_shared<ISpearman>(ref_ind, n, fill_null));
}

Indicator HAYAKU_API SPEARMAN(const Indicator& ind, const Indicator& ref_ind,
                              int n, bool fill_null) {
  auto p = make_shared<ISpearman>(ref_ind, n, fill_null);
  Indicator result(p);
  return result(ind);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IQuantileTrunc)
#endif

namespace hayaku {

IQuantileTrunc::IQuantileTrunc() : IndicatorImp("QUANTILE_TRUNC", 1) {
  setParam<int>("n", 60);
  setParam<double>("quantile_min", 0.01);
  setParam<double>("quantile_max", 0.99);
}

IQuantileTrunc::~IQuantileTrunc() {}

void IQuantileTrunc::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") > 0);
  } else if ("quantile_min" == name) {
    double quantile_min = getParam<double>("quantile_min");
    HAYAKU_ASSERT(quantile_min >= 0.0 && quantile_min <= 1.0);
    if (haveParam("quantile_max")) {
      double quantile_max = getParam<double>("quantile_max");
      HAYAKU_ASSERT(quantile_min < quantile_max);
    }
  } else if ("quantile_max" == name) {
    double quantile_max = getParam<double>("quantile_max");
    HAYAKU_ASSERT(quantile_max >= 0.0 && quantile_max <= 1.0);
    if (haveParam("quantile_min")) {
      double quantile_min = getParam<double>("quantile_min");
      HAYAKU_ASSERT(quantile_min < quantile_max);
    }
  }
}

// Replace the values outside the quantile range
static Indicator::value_t quantile_trunc(Indicator::value_t const* src,
                                         size_t total, double quantile_min,
                                         double quantile_max) {
  Indicator::value_t result = src[total - 1];
  HAYAKU_IF_RETURN(quantile_min == 0.0 && quantile_max == 1.0, result);

  std::vector<IndicatorImp::value_t> tmp;
  tmp.reserve(total);
  for (size_t i = 0; i < total; i++) {
    if (!std::isnan(src[i])) {
      tmp.push_back(src[i]);
    }
  }
  HAYAKU_IF_RETURN(tmp.empty(), result);

  std::sort(tmp.begin(), tmp.end());

  auto down_limit = get_quantile(tmp, quantile_min);
  auto up_limit = get_quantile(tmp, quantile_max);
  if (result > up_limit) {
    result = up_limit;
  } else if (result < down_limit) {
    result = down_limit;
  }
  return result;
}

void IQuantileTrunc::_calculate(const Indicator& data) {
  size_t total = data.size();
  int n = getParam<int>("n");
  discard_ = data.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  _increment_calculate(data, discard_);
}

void IQuantileTrunc::_increment_calculate(const Indicator& data,
                                          size_t start_pos) {
  size_t total = data.size();
  int n = getParam<int>("n");
  double quantile_min = getParam<double>("quantile_min");
  double quantile_max = getParam<double>("quantile_max");
  auto* dst = this->data();
  // for (size_t i = m_discard; i < total; i++) {
  //     auto const *src = data.data() + 1 + i - n;
  //     dst[i] = quantile_trunc(src, n, quantile_min, quantile_max);
  // }
  const auto* data_ptr = data.data();
  global_parallel_for_index_void(
      start_pos, total,
      [n, quantile_min, quantile_max, dst, data_ptr](size_t i) {
        auto const* src = data_ptr + 1 + i - n;
        dst[i] = quantile_trunc(src, n, quantile_min, quantile_max);
      });
}

Indicator HAYAKU_API QUANTILE_TRUNC(int n, double quantile_min,
                                    double quantile_max) {
  auto p = make_shared<IQuantileTrunc>();
  p->setParam<int>("n", n);
  p->setParam<double>("quantile_min", quantile_min);
  p->setParam<double>("quantile_max", quantile_max);
  return Indicator(p);
}

}  // namespace hayaku
