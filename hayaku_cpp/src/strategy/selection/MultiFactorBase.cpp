/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include "MultiFactorBase.h"

#include <cmath>

#include "IndustryNeutralize.h"
#include "StyleRegression.h"
#include "common/concurrency/ParallelAlgorithms.h"
#include "data/DataRuntime.h"
#include "operators/MomentumOperators.h"
#include "operators/SeriesOperators.h"
#include "operators/StatisticsOperators.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& out,
                                    const MultiFactorBase& mf) {
  out << "MultiFactor{" << "\n  name: " << mf.name()
      << "\n  params: " << mf.getParameter() << "\n  query: " << mf.getQuery()
      << "\n  ref stock: " << mf.ref_stk_;

  out << "\n  src inds count: " << mf.factorset_.size() << " [";
  if (mf.factorset_.size() <= 5) {
    for (const auto& ind : mf.factorset_) {
      out << ind.name() << ", ";
    }
  } else {
    for (size_t i = 0; i < 5; i++) {
      out << mf.factorset_[i].name() << ", ";
    }
    out << "......";
  }
  out << "]";

  out << "\n  normalize: " << (mf.norm_ ? mf.norm_->name() : "NULL");

  if (!mf.special_norms_.empty()) {
    out << "\n  special norms: " << mf.special_norms_.size();
    for (const auto& [name, norm] : mf.special_norms_) {
      out << "\n    " << name << " -> " << norm->name();
    }
  }

  if (!mf.special_category_.empty()) {
    out << "\n  special catogory handle: " << mf.special_category_.size();
    for (const auto& [name, category] : mf.special_category_) {
      out << "\n    " << name << " -> " << category;
    }
  }

  if (!mf.special_style_inds_.empty()) {
    out << "\n  special style inds: " << mf.special_style_inds_.size();
    for (const auto& [name, style_inds] : mf.special_style_inds_) {
      out << "\n    " << name << " -> [";
      for (const auto& ind : style_inds) {
        out << ind.name() << ", ";
      }
      out << "]";
    }
  }

  out << "\n  stocks count: " << mf.stks_.size() << " [";
  size_t print_stk_len = std::min<size_t>(5, mf.stks_.size());
  for (size_t i = 0; i < print_stk_len; i++) {
    out << mf.stks_[i].market_code() << ", ";
  }
  if (mf.stks_.size() > 5) {
    out << "......";
  }
  out << "]";

  out << "\n}";
  return out;
}

HAYAKU_API std::ostream& operator<<(std::ostream& out,
                                    const MultiFactorPtr& mf) {
  if (mf) {
    out << *mf;
  } else {
    out << "MultiFactor(NULL)";
  }
  return out;
}

MultiFactorBase::MultiFactorBase() : name_("MultiFactorBase") { initParam(); }

MultiFactorBase::MultiFactorBase(const string& name) : name_(name) {
  initParam();
}

MultiFactorBase::MultiFactorBase(const MultiFactorBase& base)
    : params_(base.params_),
      name_(base.name_),
      factorset_(base.factorset_),
      stks_(base.stks_),
      ref_stk_(base.ref_stk_),
      query_(base.query_) {}

MultiFactorBase::MultiFactorBase(const StockList& stks, const KQuery& query,
                                 const Stock& ref_stk, const string& name,
                                 int ic_n, bool spearman, int mode,
                                 bool save_all_factors)
    : name_(name), stks_(stks), ref_stk_(ref_stk), query_(query) {
  initParam();
  setParam<bool>("use_spearman", spearman);
  setParam<int>("ic_n", ic_n);
  setParam<int>("mode", mode);
  setParam<bool>("save_all_factors", save_all_factors);
  checkParam("ic_n");
  checkParam("mode");
  // _checkData();
}

void MultiFactorBase::initParam() {
  setParam<bool>("fill_null", true);
  setParam<int>("ic_n", 1);
  setParam<bool>(
      "use_spearman",
      true);  // SPEARMAN is used by default, otherwise the pearson one
  setParam<int>("mode",
                0);  // Sorting mode: 0-descending, 1-ascending, 2-no sorting
  setParam<bool>("save_all_factors",
                 false);  // Keep all the factor data after the calculation
                          // the getAllFactors/getFactor methods
}

void MultiFactorBase::baseCheckParam(const string& name) const {
  if ("ic_n" == name) {
    HAYAKU_ASSERT(getParam<int>("ic_n") >= 1);
  } else if ("zscore_nsigma" == name) {
    HAYAKU_ASSERT(getParam<double>("zscore_nsigma") > 0.0);
  } else if ("mode" == name) {
    int mode = getParam<int>("mode");
    HAYAKU_ASSERT(mode == 0 || mode == 1 || mode == 2);
  }
}

void MultiFactorBase::paramChanged() {
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::_checkData() {
  HAYAKU_CHECK(!factorset_.empty(), "Input factor set is empty!");

  // The subsequent calculation needs to stay aligned and handling a mixed Null
  // stock is troublesome, so an exception is thrown to block it
  for (const auto& stk : stks_) {
    HAYAKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
  }

  // Get the reference dates used for the alignment
  if (ref_stk_.isNull()) {
    ref_stk_ = getDataRuntime().getMarketStock("SH");
  }
  ref_dates_ = ref_stk_.getDatetimeList(query_);
  HAYAKU_CHECK(ref_dates_.size() >= 2,
               "The dates len is insufficient! current len: {}",
               ref_dates_.size());

  HAYAKU_CHECK(stks_.size() >= 2,
               "The number of stock is insufficient! current stock number: {}",
               stks_.size());
}

void MultiFactorBase::clearCalculatedData() {
  ref_dates_ = {};
  stk_map_ = {};
  all_factors_ = {};
  date_index_ = {};
  stk_factor_by_date_ = {};
  ic_ = {};
}

void MultiFactorBase::reset() {
  // The lock is held throughout: to avoid a write-write interleaving with an
  // ongoing calculate. Note: _reset() is a virtual function and a custom
  // implementation must not re-enter, inside the lock, a method of the same
  // instance that needs m_mutex.
  std::lock_guard<std::mutex> lock(mutex_);
  _reset();
  clearCalculatedData();
  calculated_.store(false, std::memory_order_release);
}

MultiFactorPtr MultiFactorBase::clone() {
  MultiFactorPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = MultiFactorPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->name_ = name_;
  p->params_ = params_;
  p->is_python_object_ = is_python_object_;
  p->stks_ = stks_;
  p->ref_stk_ = ref_stk_;
  // p->m_ref_dates = m_ref_dates;
  p->query_ = query_;

  p->factorset_ = factorset_;

  if (norm_) {
    p->norm_ = norm_->clone();
  }

  for (const auto& [norm_name, norm] : special_norms_) {
    p->special_norms_[norm_name] = norm->clone();
  }

  p->special_category_ = special_category_;

  p->calculated_.store(false, std::memory_order_relaxed);
  // Force a recalculation without cloning the following caches, to avoid the
  // thread unsafety p->m_stk_map = m_stk_map; p->m_date_index = m_date_index;
  // p->m_stk_factor_by_date = m_stk_factor_by_date;
  // p->m_ic = m_ic.clone();
  // p->m_all_factors.reserve(m_all_factors.size());
  // for (const auto& ind : m_all_factors) {
  //     p->m_all_factors.emplace_back(ind.clone());
  // }
  return p;
}

void MultiFactorBase::setQuery(const KQuery& query) {
  query_ = query;
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setRefStock(const Stock& stk) {
  Stock tmp_stk = stk.isNull() ? getDataRuntime().getMarketStock("SH") : stk;
  DatetimeList ref_dates = tmp_stk.getDatetimeList(query_);
  HAYAKU_CHECK(ref_dates.size() >= 2,
               "The dates len is insufficient! current len: {}",
               ref_dates.size());
  ref_stk_ = tmp_stk;
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setStockList(const StockList& stks) {
  // The subsequent calculation needs to stay aligned and handling a mixed Null
  // stock is troublesome, so an exception is thrown to block it
  for (const auto& stk : stks) {
    HAYAKU_CHECK(!stk.isNull(), "Exist null stock in stks!");
  }

  stks_ = stks;
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setRefFactorSet(const FactorSet& factorset) {
  HAYAKU_CHECK(!factorset.isNull() && !factorset.empty(),
               "Input factor set is null or empty!");
  factorset_ = factorset;
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::setNormalize(NormPtr norm) {
  norm_ = norm;
  calculated_.store(false, std::memory_order_relaxed);
}

void MultiFactorBase::addSpecialNormalize(const string& name, NormalizePtr norm,
                                          const string& category,
                                          const IndicatorList& style_inds) {
  // No special handling is given
  HAYAKU_WARN_IF(!norm && category.empty() && style_inds.empty(),
                 "No special handling is specified!");

  bool found = false;
  string found_name;
  for (const auto& ind : factorset_) {
    // An indicator may use a Chinese name, but a Factor may not
    if (utf8_fold_equal(ind.name(), name)) {
      found = true;
      found_name = ind.name();
      break;
    }
  }
  HAYAKU_CHECK(found, "Can't find factor ({}) in MF!", name);

  if (!category.empty()) {
    auto blks = getDataRuntime().getBlockList(category);
    HAYAKU_CHECK(!blks.empty(), "Can't find category block list: {}", category);
  }

  if (norm) {
    special_norms_[found_name] = norm;
  }

  if (!category.empty()) {
    special_category_[found_name] = category;
  }

  if (!style_inds.empty()) {
    special_style_inds_[found_name] = style_inds;
  }

  calculated_.store(false, std::memory_order_relaxed);
}

const DatetimeList& MultiFactorBase::getDatetimeList() {
  calculate();
  return ref_dates_;
}

const Indicator& MultiFactorBase::getFactor(const Stock& stk) {
  HAYAKU_CHECK(getParam<bool>("save_all_factors"),
               "param \"save_all_factors\" is false, can't get all factors!");
  calculate();
  const auto iter = stk_map_.find(stk);
  HAYAKU_CHECK(iter != stk_map_.cend(), "Could not find this stock: {}", stk);
  return all_factors_[iter->second];
}

const IndicatorList& MultiFactorBase::getAllFactors() {
  if (getParam<bool>("save_all_factors")) {
    calculate();
  } else {
    HAYAKU_WARN("param \"save_all_factors\" is false, can't get all factors!");
  }
  return all_factors_;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& d) {
  calculate();
  ScoreRecordList ret;
  const auto iter = date_index_.find(d);
  HAYAKU_IF_RETURN(iter == date_index_.cend(), ret);
  ret = stk_factor_by_date_[iter->second];
  return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start,
                                           size_t end) {
  ScoreRecordList ret;
  HAYAKU_IF_RETURN(start >= end, ret);

  auto cross = getScores(date);
  if (end == Null<size_t>() || end > cross.size()) {
    end = cross.size();
  }

  ret.reserve(end - start);
  for (size_t i = start; i < end; i++) {
    ret.emplace_back(cross[i]);
  }

  return ret;
}

ScoreRecordList MultiFactorBase::getScores(
    const Datetime& date, size_t start, size_t end,
    std::function<bool(const ScoreRecord&)>&& filter) {
  ScoreRecordList ret;
  HAYAKU_IF_RETURN(start >= end, ret);

  auto cross = getScores(date);
  HAYAKU_IF_RETURN(cross.empty(), ret);

  if (end == Null<size_t>() || end > cross.size()) {
    end = cross.size();
  }

  ret.reserve(end - start);
  if (filter) {
    for (size_t i = start; i < end; i++) {
      if (filter(cross[i])) {
        ret.emplace_back(cross[i]);
      }
    }
  } else {
    for (size_t i = start; i < end; i++) {
      ret.emplace_back(cross[i]);
    }
  }

  return ret;
}

ScoreRecordList MultiFactorBase::getScores(
    const Datetime& date, size_t start, size_t end,
    std::function<bool(const Datetime&, const ScoreRecord&)>&& filter) {
  ScoreRecordList ret;
  HAYAKU_IF_RETURN(start >= end, ret);

  auto cross = getScores(date);
  HAYAKU_IF_RETURN(cross.empty(), ret);

  if (end == Null<size_t>() || end > cross.size()) {
    end = cross.size();
  }

  ret.reserve(end - start);
  if (filter) {
    for (size_t i = start; i < end; i++) {
      if (filter(date, cross[i])) {
        ret.emplace_back(cross[i]);
      }
    }
  } else {
    for (size_t i = start; i < end; i++) {
      ret.emplace_back(cross[i]);
    }
  }

  return ret;
}

ScoreRecordList MultiFactorBase::getScores(const Datetime& date, size_t start,
                                           size_t end,
                                           const ScoresFilterPtr& filter) {
  ScoreRecordList ret;
  HAYAKU_IF_RETURN(start >= end, ret);

  auto cross = getScores(date);
  HAYAKU_IF_RETURN(cross.empty(), ret);

  if (end == Null<size_t>() || end > cross.size()) {
    end = cross.size();
  }

  ret.reserve(end - start);
  for (size_t i = start; i < end; i++) {
    ret.emplace_back(cross[i]);
  }

  if (filter) {
    ret = filter->filter(ret, date, query_);
  }

  return ret;
}

const vector<ScoreRecordList>& MultiFactorBase::getAllScores() {
  calculate();
  return stk_factor_by_date_;
}

Indicator MultiFactorBase::getIC(int ndays) {
  HAYAKU_WARN_IF_RETURN(!getParam<bool>("save_all_factors"), Indicator(),
                        htr("mf param \"save_all_factors\" is false, can't get "
                            "all factors!, please "
                            "set it to true if you want to get IC/ICIR!"));

  calculate();

  std::lock_guard<std::mutex> lock(mutex_);

  // When ndays equals the ic_n parameter, the cached IC result is taken first
  // The IC of a new factor essentially needs no cache (such as the equal
  // weight), but for a new factor whose weight is calculated through IC or
  // ICIR, although the IC of the new factor can be calculated with a different
  // N-day return, it is better to use the same value The weight is often
  // calculated through IC/ICIR, so a copy is cached here directly to reduce the
  // repeated calculation In the actual usage it is better to keep ndays in
  // getIC(ndays) consistent with ic_n
  int ic_n = getParam<int>("ic_n");
  if (ndays <= 0) {
    ndays = ic_n;
  }
  HAYAKU_IF_RETURN(ic_n == ndays && !ic_.empty(), ic_.clone());

  auto all_returns = _getAllReturns(ndays);
  Indicator result =
      IC(all_factors_, all_returns, ndays, getParam<bool>("use_spearman"));
  result.setParam<DatetimeList>("align_date_list", ref_dates_);
  result.name("IC");

  // When ndays equals the ic_n parameter, cache the calculation result
  if (ic_n == ndays) {
    ic_ = result;
  }
  return result;
}

Indicator MultiFactorBase::getICIR(int ir_n, int ic_n) {
  Indicator ic = getIC(ic_n);
  Indicator x = (MA(ic, ir_n) / STDEV(ic, ir_n)).getResult(0);
  x.name("ICIR");
  x.setParam<int>("n", ic_n);
  x.setParam<int>("rolling_n", ir_n);
  return x;
}

unordered_map<string, std::pair<PriceList, size_t>>
MultiFactorBase::_buildDummyIndex() {
  // When the industry neutralization of a special indicator is given, build its
  // industry membership labels
  unordered_map<string, std::pair<PriceList, size_t>> stock_dummy_index;
  for (const auto& [ind_name, catefory] : special_category_) {
    stock_dummy_index[ind_name] = {PriceList(stks_.size(), Null<price_t>()),
                                   0};
    auto blks = getDataRuntime().getBlockList(catefory);
    if (blks.empty()) {
      HAYAKU_WARN("Block list ({}) is empty, please check your block category!",
                  catefory);
      continue;
    }

    auto iter = stock_dummy_index.find(ind_name);
    auto& dummy = iter->second.first;
    size_t blk_count = blks.size();
    iter->second.second = blk_count;

    for (size_t i = 0; i < stks_.size(); i++) {
      bool found = false;
      size_t j = 0;
      for (const auto& blk : blks) {
        if (blk.have(stks_[i])) {
          dummy[i] = j;
          found = true;
          break;
        }
        j++;
      }
      if (!found) {
        // A stock without a membership is assigned blk_count and skipped when
        // the group mean is removed downstream (the residual is set to NaN),
        // avoiding an implicit clustering into a pseudo industry group that
        // would cause a pseudo regression.
        dummy[i] = blk_count;
      }
    }
  }
  return stock_dummy_index;
}

IndicatorList MultiFactorBase::_getAllReturns(int ndays) const {
  bool fill_null = getParam<bool>("fill_null");
  return global_parallel_for_index(
      0, stks_.size(), [this, ndays, fill_null](size_t i) {
        auto k = stks_[i].getKData(query_);
        return ALIGN(ROCP(CLOSE(), ndays), ref_dates_, fill_null)(k).getResult(
            0);
      });
}

// The pure function implementation of the industry neutralization (removing the
// group mean by industry group) is in industry_neutralize.h; it is extracted as
// an internal inline header so that the white box unit tests can include and
// call it directly. The residual regression implementation of the style factor
// neutralization is in StyleRegression.cpp; it is extracted from this class as
// a serial kernel and no longer modifies the process level Eigen thread
// configuration at runtime.

vector<IndicatorList> MultiFactorBase::getAllSrcFactors() {
  vector<IndicatorList> all_stk_inds;
  size_t stk_count = stks_.size();
  HAYAKU_IF_RETURN(stk_count == 0, all_stk_inds);
  all_stk_inds.resize(stk_count);

  size_t ind_count = factorset_.size();
  HAYAKU_IF_RETURN(ind_count == 0, all_stk_inds);

  size_t days_total = ref_dates_.size();
  auto null_ind =
      PRICELIST(PriceList(days_total, Null<price_t>()), ref_dates_);

  bool fill_null = getParam<bool>("fill_null");

  all_stk_inds = factorset_.getValues(stks_, query_, true, fill_null, true,
                                       true, ref_dates_);

  // The style factors are stored in three dimensions [style factor name][style
  // factor][stock] (vector<IndicatorList>).
  //
  // The original implementation was two-dimensional [style factor name][style
  // factor] and every style factor shared the same time series across the whole
  // market, which caused two independent defects:
  //   1) during a concurrent extraction multiple threads wrote the same
  //   Indicator (the non-atomic assignment to the shared_ptr object itself is
  //      a C++ UB), which is the root cause of the segfault (no Python
  //      exception) of get_ic / get_all_src_factors after the style
  //      neutralization is enabled;
  //   2) the independent variable of the cross-sectional regression took the
  //   same value for all the stocks, degenerating into a constant column
  //   collinear with the intercept,
  //      so the market value / style neutralization failed completely in
  //      mathematics (the residual is about the original value).
  //
  // After the stock dimension is added:
  //   - Write isolation: every si thread writes per_factor[j][si] only and
  //   different si write different slots;
  //   - Read isolation: every thread first clone()s an independent copy of the
  //   template styles[j] and then calculates ALIGN on it,
  //     so calculate() writes the buffer of the thread copy only and does not
  //     touch the shared styles[j]. (IndicatorImp::clone() is strictly
  //     read-only on the root object, which was verified in the code, so
  //     concurrent
  //      styles[j].clone() is safe; here styles[j] is the root Indicator passed
  //      in by the user.)
  unordered_map<string, vector<IndicatorList>> use_style_inds;
  for (const auto& [style_ind_name, style_inds] : special_style_inds_) {
    auto& per_factor = use_style_inds[style_ind_name];
    per_factor.resize(style_inds.size());
    for (auto& v : per_factor) {
      v.resize(stk_count);
    }
  }
  if (!special_style_inds_.empty()) {
    global_parallel_for_index_void(0, stk_count, [&](size_t si) {
      const auto& stk = stks_[si];
      auto kdata = stk.getKData(query_);
      for (auto& [style_ind_name, styles] : special_style_inds_) {
        auto& per_factor = use_style_inds[style_ind_name];
        for (size_t j = 0; j < styles.size(); j++) {
          if (kdata.size() == 0) {
            per_factor[j][si] = null_ind;
          } else {
            // An independent clone per thread: calculate writes the buffer of
            // the thread copy only
            per_factor[j][si] =
                ALIGN(styles[j].clone(), ref_dates_, fill_null)(kdata)
                    .getResult(0);
          }
          per_factor[j][si].name(style_ind_name);
        }
      }
    });
  }

  // The time cross-sectional standardization / normalization
  if (norm_ || !special_category_.empty() || !special_style_inds_.empty()) {
    // The residual regression of the style factor neutralization has been
    // extracted as a serial kernel (StyleRegression.cpp), it no longer modifies
    // the process level Eigen::setNbThreads at runtime, avoiding the concurrent
    // MFs polluting the global config mutually; the outer day-by-day
    // parallelism is naturally reentrant and the regression internals are all
    // stack local objects.
    unordered_map<string, std::pair<PriceList, size_t>> ind_dummy_dict =
        _buildDummyIndex();
    global_parallel_for_index_void(
        0, days_total,
        [this, stk_count, ind_count, &all_stk_inds, &ind_dummy_dict,
         &use_style_inds](size_t di) {
          auto sub_norm = norm_ ? norm_->clone() : norm_;
          NormPtr special_norm;
          PriceList one_day(stk_count, Null<price_t>());
          PriceList new_value;
          for (size_t ii = 0; ii < ind_count; ii++) {
            auto* one_day_data = one_day.data();
            for (size_t si = 0; si < stk_count; si++) {
              one_day_data[si] = all_stk_inds[si][ii][di];
            }

            // Note: the all_stk_inds[*][ii] produced by
            // m_factorset.getValues(align=true) is renamed to "ALIGN" by the
            // ALIGN wrapper (verified at runtime). But the key stored into
            // m_special_norms/m_special_category by addSpecialNormalize is the
            // original factor name in the FactorSet (such as "MA"). If
            // all_stk_inds[0][ii].name() were taken as the find key it would
            // never hit and the industry / style / special standardization
            // would all fail silently. Instead, m_factorset[ii].name() is used
            // to trace back the original name by index: getValues produces
            // result[j][i] strictly corresponding to factors[i], and the index
            // ii corresponds one to one with m_factorset[ii], with no risk of
            // an out of order access.
            auto ind_name = factorset_[ii].name();
            auto special_norm_iter = special_norms_.find(ind_name);
            if (special_norm_iter != special_norms_.end()) {
              special_norm = special_norm_iter->second->clone();
            } else {
              special_norm.reset();
            }

            if (special_norm) {
              new_value = special_norm->normalize(one_day);
            } else if (sub_norm) {
              new_value = sub_norm->normalize(one_day);
            } else {
              new_value = one_day;
            }

            auto category_iter = ind_dummy_dict.find(ind_name);
            if (category_iter != ind_dummy_dict.end()) {
              // The industry neutralization: removing the group mean (O(n),
              // mathematically equivalent to a multiple regression without the
              // intercept on the one-hot encoding). The original implementation
              // wrongly used the univariate calculate_residuals to do a unary
              // regression on the integer block index, implying the wrong
              // assumption "the effect of industry j = b0 + b1*j", so the
              // result depended on the order of the block list.
              const auto& [labels, blk_count] = category_iter->second;
              new_value =
                  calculate_industry_residuals(new_value, labels, blk_count);
            }
            auto style_iter = use_style_inds.find(ind_name);
            if (style_iter != use_style_inds.end()) {
              auto& per_factor = style_iter->second;  // [style factor][stock]
              vector<PriceList> style_value_day(per_factor.size());
              for (size_t j = 0; j < per_factor.size(); j++) {
                auto& style_value = style_value_day[j];
                style_value.resize(stk_count);
                for (size_t si = 0; si < stk_count; si++) {
                  // Take the style factor value of the si-th stock itself (the
                  // original implementation wrongly took the shared value)
                  style_value[si] = per_factor[j][si][di];
                }
              }
              new_value = calculate_style_residuals(new_value, style_value_day);
            }

            for (size_t si = 0; si < stk_count; si++) {
              auto* dst = all_stk_inds[si][ii].data();
              dst[di] = new_value[si];
            }
          }
        });
  }

  return all_stk_inds;
}

void MultiFactorBase::_buildIndex() {
  size_t stk_count = stks_.size();
  for (size_t i = 0; i < stk_count; i++) {
    stk_map_[stks_[i]] = i;
  }

  size_t days_total = ref_dates_.size();
  stk_factor_by_date_.resize(days_total);
  for (size_t i = 0; i < days_total; i++) {
    date_index_[ref_dates_[i]] = i;
    stk_factor_by_date_[i].resize(
        stk_count);  // Pre-allocate one slot per stock for every date
  }

  // Traverse the stock j first and then the date i, no sorting by default
  global_parallel_for_index_void(0, stk_count, [this, days_total](size_t j) {
    const auto& stk = stks_[j];
    const auto* data = all_factors_[j].data();
    for (size_t i = 0; i < days_total; i++) {
      stk_factor_by_date_[i][j] = ScoreRecord(stk, data[i]);
    }
  });

  // The deterministic comparator of the cross-sectional sorting: the valid
  // values are sorted in the target direction and, when the values are equal,
  // the market_code lexicographic order is used as the secondary key to break
  // the tie, making the result stable across the input order, the process, the
  // platform and the compiler. NaN is placed at the end and the NaNs are also
  // ordered by the market_code lexicographic order to guarantee the determinism
  // within the NaN section. A lambda is used instead of an anonymous namespace
  // function to avoid the ODR risk under unity_build, and at the same time the
  // closure type is unique at compile time, letting std::sort fully inline the
  // comparison logic.
  auto scoreDescLess = [](const ScoreRecord& a,
                          const ScoreRecord& b) noexcept -> bool {
    const bool a_nan = std::isnan(a.value);
    const bool b_nan = std::isnan(b.value);
    if (a_nan != b_nan) {
      return !a_nan;  // A valid value always comes before NaN
    }
    if (!a_nan) {
      if (a.value > b.value) return true;
      if (a.value < b.value) return false;
    }
    return a.stock.market_code() < b.stock.market_code();
  };

  auto scoreAscLess = [](const ScoreRecord& a,
                         const ScoreRecord& b) noexcept -> bool {
    const bool a_nan = std::isnan(a.value);
    const bool b_nan = std::isnan(b.value);
    if (a_nan != b_nan) {
      return !a_nan;  // A valid value always comes before NaN
    }
    if (!a_nan) {
      if (a.value < b.value) return true;
      if (a.value > b.value) return false;
    }
    return a.stock.market_code() < b.stock.market_code();
  };

  int mode = getParam<int>("mode");
  if (0 == mode) {
    global_parallel_for_index_void(
        0, days_total,
        [this, scoreDescLess](size_t i) {
          std::sort(stk_factor_by_date_[i].begin(),
                    stk_factor_by_date_[i].end(), scoreDescLess);
        },
        100);

  } else if (1 == mode) {
    global_parallel_for_index_void(
        0, days_total,
        [this, scoreAscLess](size_t i) {
          std::sort(stk_factor_by_date_[i].begin(),
                    stk_factor_by_date_[i].end(), scoreAscLess);
        },
        100);
  }
}

void MultiFactorBase::calculate() {
  // Fast path: check whether it is already Ready with a lock-free acquire
  if (calculated_.load(std::memory_order_acquire)) {
    return;
  }

  std::lock_guard<std::mutex> lock(mutex_);

  // A second check inside the lock: the mutex already provides the slow path
  // synchronization, so relaxed is enough
  if (calculated_.load(std::memory_order_relaxed)) {
    return;
  }

  // Clean up the old result before the building, ensuring the retry is based on
  // a clean state
  clearCalculatedData();

  try {
    _checkData();

    {  // Get all the aligned original factors of all the securities
      vector<IndicatorList> all_stk_inds = getAllSrcFactors();

      size_t factor_count = factorset_.size();
      if (factor_count == 1) {
        // Use the original factors directly
        size_t stk_count = stks_.size();
        all_factors_.resize(stk_count);
        for (size_t i = 0; i < stk_count; i++) {
          all_factors_[i] = std::move(all_stk_inds[i][0]);
        }
      } else {
        // Calculate the adjusted composite factor of every security
        all_factors_ = _calculate(all_stk_inds);
      }
    }

    // Create the cross-sectional index after the calculation
    _buildIndex();

    if (!getParam<bool>("save_all_factors")) {
      all_factors_ = {};
      stk_map_ = {};
    }
  } catch (...) {
    // The failure cleanup: all the asynchronous subtasks have ended under the
    // wait_and_drain semantics, so the half-finished product of the base class
    // is cleared, the not calculated state is kept, the original exception
    // propagates up and the next caller may retry.
    clearCalculatedData();
    calculated_.store(false, std::memory_order_relaxed);
    throw;
  }

  // Publish: release guarantees that all the previous writes are visible to the
  // subsequent acquire reads
  calculated_.store(true, std::memory_order_release);
}

}  // namespace hayaku
