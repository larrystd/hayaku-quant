#include "StatisticsOperators.h"

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#pragma once

#include "Indicator.h"

namespace hayaku {

class IIc : public IndicatorImp {
 public:
  IIc();
  IIc(const StockList& stks, int n, bool spearman, bool strict);
  virtual ~IIc() override;

  virtual void _checkParam(const string& name) const override;
  virtual void _calculate(const Indicator& data) override;
  virtual IndicatorImpPtr _clone() override;

  virtual bool selfAlike(const IndicatorImp& other) const noexcept override;

 private:
  StockList stks_;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(IndicatorImp);
    ar& boost::serialization::make_nvp("m_stks", stks_);
  }
#endif
};

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#pragma once

namespace hayaku {

// Calculate the linear regression slope, the goodness of fit R² and the
// relative maximum residual; N supports a variable Result set:
//   result(0): slope
//   result(1): goodness of fit R²
//   result(2): relative maximum residual RelMaxRes = max|yi - ŷi| / ȳ
class ISlope : public IndicatorImp {
  INDICATOR_IMP(ISlope)
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ISlope();
  virtual ~ISlope() override;
  virtual void _checkParam(const string& name) const override;
  virtual bool supportIncrementCalculate() const override;
  virtual size_t min_increment_start() const override;
  virtual void _increment_calculate(const Indicator& ind,
                                    size_t start_pos) override;
};

}  // namespace hayaku

/*
 * ITsRank.h
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

#pragma once
#ifndef INDICATOR_IMP_HAYAKU_ITSRANK_H_
#define INDICATOR_IMP_HAYAKU_ITSRANK_H_

namespace hayaku {

class ITsRank : public IndicatorImp {
  INDICATOR_IMP(ITsRank)
  INDICATOR_IMP_SUPPORT_INCREMENT
  INDICATOR_IMP_SUPPORT_DYNAMIC_CYCLE
  INDICATOR_IMP_NO_PRIVATE_MEMBER_SERIALIZATION

 public:
  ITsRank();
  virtual ~ITsRank() override;
  virtual void _checkParam(const string& name) const override;
  virtual size_t min_increment_start() const override;
};

} /* namespace hayaku */
#endif /* INDICATOR_IMP_HAYAKU_ITSRANK_H_ */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-09
 *      Author: fasiondog
 */

#include "data/Block.h"
#include "operators/MomentumOperators.h"
#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IIc)
#endif

namespace hayaku {

IIc::IIc() : IndicatorImp("IC", 1) {
  need_self_alike_compare_ = true;
  setParam<int>("n", 1);  // Position adjustment cycle
  // Whether to fill with nan during the alignment, otherwise the last value
  // earlier than the current date is used as the fill
  setParam<bool>("fill_null", true);
  setParam<bool>(
      "use_spearman",
      true);  // Use SPEARMAN by default, otherwise the pearson correlation

  // Strict IC calculation, i.e. fill NA when the future return is unknown; it
  // is equivalent to shifting left by n in the non-strict case
  setParam<bool>("strict", false);
}

IIc::IIc(const StockList& stks, int n, bool spearman, bool strict)
    : IndicatorImp("IC", 1), stks_(stks) {
  need_self_alike_compare_ = true;
  setParam<int>("n", n);
  setParam<bool>("fill_null", true);
  setParam<bool>("use_spearman", spearman);
  setParam<bool>("strict", strict);
}

IIc::~IIc() {}

void IIc::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 1);
  }
}

IndicatorImpPtr IIc::_clone() {
  auto p = make_shared<IIc>();
  p->stks_ = stks_;
  return p;
}

bool IIc::selfAlike(const IndicatorImp& other) const noexcept {
  const auto* other_ind = dynamic_cast<const IIc*>(&other);
  HAYAKU_IF_RETURN(other_ind == nullptr, false);
  HAYAKU_IF_RETURN(other_ind->stks_.size() != stks_.size(), false);
  std::unordered_set<string> names;
  names.reserve(stks_.size());
  for (const auto& stk : stks_) {
    names.insert(stk.market_code());
  }
  for (const auto& stk : other_ind->stks_) {
    if (names.find(stk.market_code()) == names.end()) {
      return false;
    }
  }
  return true;
}

void IIc::_calculate(const Indicator& inputInd) {
  // Allocate the memory first, keeping the same length as the reference dates
  auto ref_dates = getContext().getDatetimeList();
  size_t days_total = ref_dates.size();
  _readyBuffer(days_total, 1);

  // Detect the abnormal input data
  discard_ = days_total;
  HAYAKU_IF_RETURN(days_total < 2, void());

  size_t stk_count = stks_.size();
  HAYAKU_ERROR_IF_RETURN(
      stk_count < 2, void(),
      "The number(>=2) of stock is insufficient! current stock number: {}",
      stk_count);
  for (size_t i = 0; i < stk_count; i++) {
    HAYAKU_ERROR_IF_RETURN(stks_[i].isNull(), void(), "The [{}] stock is null!",
                           i);
  }

  int n = getParam<int>("n");
  HAYAKU_IF_RETURN(n >= days_total, void());

  bool fill_null = getParam<bool>("fill_null");

  // Calculate the aligned factor value and the n-day return of every security
  vector<Indicator> all_inds(
      stk_count);  // Save the aligned factor value of every security
  vector<Indicator> all_returns(
      stk_count);  // Save the aligned n-day return of every security

  KQuery query = getContext().getQuery();

  // Note: using the excess return and the absolute return gives the same final
  // result (they are identical in theory; an extra excess return calculation
  // only introduces a negligible calculation error)
  global_parallel_for_index_void(
      0, stk_count, [&, n, fill_null, ind = inputInd.clone()](size_t i) {
        auto k = stks_[i].getKData(query);
        // Suppose IC originally needs "the factor value at t -> the return at
        // t+1"; it is changed to calculate "the factor value at t -> the return
        // of the N days before t" (such as the return of the past 5 days),
        // which is called the "current IC". (Otherwise the current values would
        // all be missing NA.)
        all_inds[i] =
            ALIGN(REF(ind, n), ref_dates, fill_null)(k).getResult(0)();

        // Calculate the absolute return
        all_returns[i] =
            ALIGN(ROCP(CLOSE(), n), ref_dates, fill_null)(k).getResult(0)();
      });

  discard_ = n;

  Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) =
      hayaku::SPEARMAN;
  if (!getParam<bool>("use_spearman")) {
    spearman = hayaku::CORR;
  }

  auto* dst = this->data();
  global_parallel_for_index_void(
      discard_, days_total, [&, stk_count, dst](size_t i) {
        // Calculate the daily cross-sectional spearman correlation coefficient,
        // i.e. the IC value
        PriceList tmp(stk_count, Null<price_t>());
        PriceList tmp_return(stk_count, Null<price_t>());
        for (size_t j = 0; j < stk_count; j++) {
          tmp[j] = all_inds[j][i];
          tmp_return[j] = all_returns[j][i];
        }
        auto a = PRICELIST(tmp);
        auto b = PRICELIST(tmp_return);
        auto ic = spearman(a, b, stk_count, true);
        if (ic.size() > 0) {
          dst[i] = ic[ic.size() - 1];
        }
      });

  if (getParam<bool>("strict")) {
    // The strict mode, i.e. the calculation result of the current moment
    // corresponding to the future return
    for (size_t i = discard_; i < days_total; i++) {
      dst[i - n] = dst[i];
    }
    if (days_total > n) {
      for (size_t i = days_total - n; i < days_total; i++) {
        dst[i] = Null<price_t>();
      }
    }
    discard_ = 0;
  }

  updateDiscard();
}

Indicator IC(const StockList& stks, int n, bool spearman, bool strict) {
  return Indicator(make_shared<IIc>(stks, n, spearman, strict));
}

Indicator IC(const Block& blk, int n, bool spearman, bool strict) {
  StockList stks = blk.getStockList();
  return IC(stks, n, spearman, strict);
}

Indicator IC(IndicatorList inds, IndicatorList returns, int n,
             bool use_spearman, bool strict) {
  HAYAKU_CHECK(n >= 1, "The n({}) must be greater than 1!", n);
  HAYAKU_CHECK(
      inds.size() == returns.size(),
      "The number({}) of indicators is not equal to the number({}) of returns!",
      inds.size(), returns.size());

  size_t stk_count = inds.size();
  HAYAKU_CHECK(stk_count >= 2, "The number of indicators is less than 2!");

  size_t days_total = inds[0].size();
  HAYAKU_CHECK(days_total >= 2, "The size of ind is less than 2!");

  for (size_t i = 0; i < stk_count; i++) {
    HAYAKU_CHECK(inds[i].size() == days_total,
                 "{}: The number of days({}) is not equal to the number of "
                 "days({}) of indicators!",
                 i, days_total, inds[i].size());
    HAYAKU_CHECK(returns[i].size() == days_total,
                 "{} The number of days({}) is not equal to the number of "
                 "days({}) of returns!",
                 i, days_total, returns[i].size());
  }

  Indicator (*spearman)(const Indicator&, const Indicator&, int, bool) =
      hayaku::SPEARMAN;
  if (!use_spearman) {
    spearman = hayaku::CORR;
  }

  IndicatorList ref_inds(stk_count);
  global_parallel_for_index_void(
      0, stk_count, [&](size_t i) { ref_inds[i] = REF(inds[i], n); });

  PriceList ret(days_total, Null<price_t>());
  auto* dst = ret.data();
  global_parallel_for_index_void(0, days_total, [&, stk_count, dst](size_t i) {
    // Calculate the daily cross-sectional spearman correlation coefficient,
    // i.e. the IC value
    PriceList tmp(stk_count, Null<price_t>());
    PriceList tmp_return(stk_count, Null<price_t>());
    for (size_t j = 0; j < stk_count; j++) {
      tmp[j] = ref_inds[j][i];
      tmp_return[j] = returns[j][i];
    }
    auto a = PRICELIST(tmp);
    auto b = PRICELIST(tmp_return);
    auto ic = spearman(a, b, stk_count, true);
    if (ic.size() > 0) {
      dst[i] = ic[ic.size() - 1];
    }
  });

  if (strict) {
    // The strict mode, i.e. the calculation result of the current moment
    // corresponding to the future return
    for (size_t i = n; i < days_total; i++) {
      dst[i - n] = dst[i];
    }
    if (days_total > n) {
      for (size_t i = days_total - n; i < days_total; i++) {
        dst[i] = Null<price_t>();
      }
    }
  }

  size_t discard = days_total;
  for (size_t i = 0; i < days_total; i++) {
    if (!std::isnan(dst[i])) {
      discard = i;
      break;
    }
  }

  return PRICELIST(ret, discard);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-11-09
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ISlope)
#endif

namespace hayaku {

ISlope::ISlope() : IndicatorImp("SLOPE", 3) { setParam<int>("n", 22); }

ISlope::~ISlope() {}

void ISlope::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") >= 0);
  }
}

void ISlope::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  discard_ = ind.discard() + 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst_slope = this->data(0);
  auto* dst_r2 = this->data(1);
  auto* dst_relmaxres = this->data(2);

  int n = getParam<int>("n");
  if (n <= 1) {
    for (size_t i = discard_; i < total; i++) {
      dst_slope[i] = 0.0;
      dst_r2[i] = 0.0;
      dst_relmaxres[i] = 0.0;
    }
    return;
  }

  size_t startPos = discard_ - 1;
  price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
  size_t first_end = startPos + n >= total ? total : startPos + n;
  for (size_t i = startPos; i < first_end; i++) {
    price_t x = i;
    price_t y = src[i];
    xsum += x;
    ysum += y;
    xysum += x * y;
    x2sum += x * x;
    y2sum += y * y;
    size_t cnt = i + 1;
    price_t denominator = cnt * x2sum - xsum * xsum;
    price_t slope = (cnt * xysum - xsum * ysum) / denominator;
    dst_slope[i] = slope;
    price_t numerator = std::pow(cnt * xysum - xsum * ysum, 2);
    price_t denominator_r2 = denominator * (cnt * y2sum - ysum * ysum);
    dst_r2[i] = numerator / denominator_r2;

    // Calculate the relative maximum residual
    price_t y_mean = ysum / cnt;
    price_t x_mean = xsum / cnt;
    price_t intercept = y_mean - slope * x_mean;
    price_t max_residual = 0.0;
    for (size_t j = startPos; j <= i; j++) {
      price_t y_hat = intercept + slope * j;
      price_t residual = std::abs(src[j] - y_hat);
      if (residual > max_residual) {
        max_residual = residual;
      }
    }
    dst_relmaxres[i] = max_residual / y_mean;
  }

  for (size_t i = first_end; i < total; i++) {
    xsum += n;
    ysum += src[i] - src[i - n];
    xysum += src[i] * i - src[i - n] * (i - n);
    x2sum += (2 * i - n) * n;
    y2sum += src[i] * src[i] - src[i - n] * src[i - n];
    price_t denominator = n * x2sum - xsum * xsum;
    price_t slope = (n * xysum - xsum * ysum) / denominator;
    dst_slope[i] = slope;
    price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
    price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
    dst_r2[i] = numerator / denominator_r2;

    // Calculate the relative maximum residual
    price_t y_mean = ysum / n;
    price_t x_mean = xsum / n;
    price_t intercept = y_mean - slope * x_mean;
    price_t max_residual = 0.0;
    for (size_t j = i - n + 1; j <= i; j++) {
      price_t y_hat = intercept + slope * j;
      price_t residual = std::abs(src[j] - y_hat);
      if (residual > max_residual) {
        max_residual = residual;
      }
    }
    dst_relmaxres[i] = max_residual / y_mean;
  }
}

bool ISlope::supportIncrementCalculate() const {
  return getParam<int>("n") > 1;
}

size_t ISlope::min_increment_start() const { return getParam<int>("n"); }

void ISlope::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  auto const* src = ind.data();
  auto* dst_slope = this->data(0);
  auto* dst_r2 = this->data(1);
  auto* dst_relmaxres = this->data(2);

  int n = getParam<int>("n");

  price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
  for (size_t i = start_pos - n; i < start_pos; i++) {
    price_t x = i;
    price_t y = src[i];
    xsum += x;
    ysum += y;
    xysum += x * y;
    x2sum += x * x;
    y2sum += y * y;
  }

  for (size_t i = start_pos; i < total; i++) {
    xsum += n;
    ysum += src[i] - src[i - n];
    xysum += src[i] * i - src[i - n] * (i - n);
    x2sum += (2 * i - n) * n;
    y2sum += src[i] * src[i] - src[i - n] * src[i - n];
    price_t denominator = n * x2sum - xsum * xsum;
    price_t slope = (n * xysum - xsum * ysum) / denominator;
    dst_slope[i] = slope;
    price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
    price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
    dst_r2[i] = numerator / denominator_r2;

    // Calculate the relative maximum residual
    price_t y_mean = ysum / n;
    price_t x_mean = xsum / n;
    price_t intercept = y_mean - slope * x_mean;
    price_t max_residual = 0.0;
    for (size_t j = i - n + 1; j <= i; j++) {
      price_t y_hat = intercept + slope * j;
      price_t residual = std::abs(src[j] - y_hat);
      if (residual > max_residual) {
        max_residual = residual;
      }
    }
    dst_relmaxres[i] = max_residual / y_mean;
  }
}

void ISlope::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                               size_t step) {
  size_t start = _get_step_start(curPos, step, ind.discard());
  if (curPos <= ind.discard()) {
    _set(Null<price_t>(), curPos);
    _set(Null<price_t>(), curPos, 1);
    _set(Null<price_t>(), curPos, 2);
    return;
  }

  if (step <= 1) {
    _set(0, curPos);
    _set(0, curPos, 1);
    _set(0, curPos, 2);
    return;
  }

  double n = curPos - start + 1;
  price_t xsum = 0.0, ysum = 0.0, xysum = 0.0, x2sum = 0.0, y2sum = 0.0;
  for (size_t i = start; i <= curPos; i++) {
    price_t x = i;
    price_t y = ind[i];
    xsum += x;
    ysum += y;
    xysum += x * y;
    x2sum += x * x;
    y2sum += y * y;
  }

  price_t denominator = n * x2sum - xsum * xsum;
  price_t slope = (n * xysum - xsum * ysum) / denominator;
  price_t numerator = std::pow(n * xysum - xsum * ysum, 2);
  price_t denominator_r2 = denominator * (n * y2sum - ysum * ysum);
  price_t r2 = numerator / denominator_r2;

  // Calculate the relative maximum residual
  price_t y_mean = ysum / n;
  price_t x_mean = xsum / n;
  price_t intercept = y_mean - slope * x_mean;
  price_t max_residual = 0.0;
  for (size_t i = start; i <= curPos; i++) {
    price_t y_hat = intercept + slope * i;
    price_t residual = std::abs(ind[i] - y_hat);
    if (residual > max_residual) {
      max_residual = residual;
    }
  }
  price_t relmaxres = max_residual / y_mean;

  _set(slope, curPos);
  _set(r2, curPos, 1);
  _set(relmaxres, curPos, 2);
}

Indicator SLOPE(int n) {
  IndicatorImpPtr p = make_shared<ISlope>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator SLOPE(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ISlope>();
  p->setIndParam("n", n);
  return Indicator(p);
}

}  // namespace hayaku

/*
 * ITsRank.cpp
 *
 *  Created on: 2026-6-9
 *      Author: fasiondog
 */

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ITsRank)
#endif

namespace hayaku {

ITsRank::ITsRank() : IndicatorImp("TS_RANK", 1) { setParam<int>("n", 20); }

ITsRank::~ITsRank() {}

void ITsRank::_checkParam(const string& name) const {
  if ("n" == name) {
    HAYAKU_ASSERT(getParam<int>("n") > 0);
  }
}

void ITsRank::_calculate(const Indicator& ind) {
  size_t total = ind.size();
  if (0 == total || ind.discard() >= total) {
    discard_ = total;
    return;
  }

  auto const* src = ind.data();
  auto* dst = this->data();

  int n = getParam<int>("n");
  discard_ = ind.discard() + n - 1;
  if (discard_ >= total) {
    discard_ = total;
    return;
  }

  global_parallel_for_index_void(discard_, total, [&, n, src, dst](size_t i) {
    int count = 0;
    size_t start = i + 1 - n;
    value_t current = src[i];
    for (size_t j = start; j <= i; ++j) {
      if (src[j] <= current) {
        count++;
      }
    }
    dst[i] = static_cast<price_t>(count) / n;
  });
}

size_t ITsRank::min_increment_start() const { return getParam<int>("n"); }

void ITsRank::_increment_calculate(const Indicator& ind, size_t start_pos) {
  size_t total = ind.size();
  auto const* src = ind.data();
  auto* dst = this->data();

  int n = getParam<int>("n");
  for (size_t i = start_pos; i < total; ++i) {
    int count = 0;
    size_t start = i + 1 - n;
    value_t current = src[i];
    for (size_t j = start; j <= i; ++j) {
      if (src[j] <= current) {
        count++;
      }
    }
    dst[i] = static_cast<price_t>(count) / n;
  }
}

void ITsRank::_dyn_run_one_step(const Indicator& ind, size_t curPos,
                                size_t step) {
  if (curPos < ind.discard() + step - 1) {
    return;
  }
  size_t start = curPos + 1 - step;
  int count = 0;
  for (size_t j = start; j <= curPos; ++j) {
    if (ind[j] <= ind[curPos]) {
      count++;
    }
  }
  _set(static_cast<price_t>(count) / step, curPos);
}

Indicator TS_RANK(int n) {
  IndicatorImpPtr p = make_shared<ITsRank>();
  p->setParam<int>("n", n);
  return Indicator(p);
}

Indicator TS_RANK(const IndParam& n) {
  IndicatorImpPtr p = make_shared<ITsRank>();
  p->setIndParam("n", n);
  return Indicator(p);
}

} /* namespace hayaku */
