/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "PerformanceOptimalSelector.h"

#include "data/DataRuntime.h"
#include "metrics/Performance.h"
#include "strategy/StrategyRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::PerformanceOptimalSelector)
#endif

namespace hayaku {

PerformanceOptimalSelector::PerformanceOptimalSelector()
    : OptimalSelectorBase("SE_PerformanceOptimal") {
  setParam<string>("key", "Account Avg Annual Return %");
  setParam<int>("mode",
                0);  // 0 takes the highest value, 1 takes the lowest value
}

void PerformanceOptimalSelector::_checkParam(const string& name) const {
  OptimalSelectorBase::_checkParam(name);
  if ("mode" == name) {
    int mode = getParam<int>(name);
    HAYAKU_ASSERT(0 == mode || 1 == mode);
  }
}

StrategyWeightList PerformanceOptimalSelector::_getSelected(Datetime date) {
  StrategyWeightList ret;
  auto iter = this->sys_dict_.find(date);
  if (iter != this->sys_dict_.end()) {
    ret.emplace_back(StrategyWeight(iter->second, 1.0));
  }
  return ret;
}

SelectorPtr PerformanceOptimalSelector::_clone() {
  return std::make_shared<PerformanceOptimalSelector>();
}

void PerformanceOptimalSelector::_reset() {
  OptimalSelectorBase::_reset();
  sys_dict_.clear();
}

void PerformanceOptimalSelector::calculate(
    const internal::StrategyRuntimeList& pf_realSysList, const KQuery& query) {
  // SPEND_TIME(OptimalSelector_calculate);
  HAYAKU_IF_RETURN(calculated_ && query_ == query, void());

  query_ = query;
  real_sys_list_ = pf_realSysList;

  bool trace = getParam<bool>("trace");
  CLS_INFO_IF(trace, "candidate sys list size: {}", pro_sys_list_.size());
  CLS_WARN_IF_RETURN(pro_sys_list_.empty(), void(),
                     "candidate sys list is empty!");

  // The check is done at runtime rather than in addSystem, so that
  // WalkForwardSystem can add the system list without a given security directly
  for (const auto& sys : pro_sys_list_) {
    CLS_ERROR_IF_RETURN(sys->getStock().isNull(), void(),
                        "The candidate sys ({}) was specified stock!",
                        sys->name());
  }

  size_t train_len = static_cast<size_t>(getParam<int>("train_len"));
  size_t test_len = static_cast<size_t>(getParam<int>("test_len"));

  auto dates =
      getDataRuntime().getTradingCalendar(query, getParam<string>("market"));
  size_t dates_len = dates.size();

  vector<std::pair<size_t, size_t>> train_ranges;
  size_t start = 0, end = train_len;
  if (end < dates_len) {
    train_ranges.emplace_back(std::make_pair(start, end));
  }
  start += test_len;
  end += test_len;
  while (end < dates_len) {
    train_ranges.emplace_back(std::make_pair(start, end));
    start += test_len;
    end += test_len;
  }

  string key = getParam<string>("key");
  int mode = getParam<int>("mode");
  CLS_INFO_IF(trace, "statistic key: {}, mode: {}", getParam<string>("key"),
              getParam<int>("mode"));

  _calculate_parallel(train_ranges, dates, key, mode, test_len, trace);

  calculated_ = true;
}

void PerformanceOptimalSelector::_calculate_parallel(
    const vector<std::pair<size_t, size_t>>& train_ranges,
    const DatetimeList& dates, const string& key, int mode, size_t test_len,
    bool trace) {
  // SPEND_TIME(OptimalSelector_calculate_parallel);
  auto sys_list = global_parallel_for_index(
      0, train_ranges.size(),
      [this, &train_ranges, &dates, query = query_, trace, key,
       mode](size_t i) {
        Datetime start_date = dates[train_ranges[i].first];
        Datetime end_date = dates[train_ranges[i].second];
        KQuery q = KQueryByDate(start_date, end_date, query.kType(),
                                query.recoverType());
        CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1,
                    train_ranges.size(), q);

        Performance per;
        internal::StrategyRuntimePtr selected_sys;
        if (pro_sys_list_.size() == 1) {
          selected_sys = pro_sys_list_.back()->clone();
        } else if (0 == mode) {
          double max_value = std::numeric_limits<double>::lowest();
          for (const auto& sys : pro_sys_list_) {
            // Cut off all the shared parts to avoid a parallel conflict
            auto new_sys = sys->clone();
            new_sys->run(q, true);
            per.statistics(new_sys->getAccount(), end_date);
            double value = per.get(key);
            CLS_TRACE_IF(trace, "value: {}, sys: {}", value, new_sys->name());
            if (value > max_value) {
              max_value = value;
              selected_sys = new_sys;
            }
          }
        } else if (1 == mode) {
          double min_value = std::numeric_limits<double>::max();
          for (const auto& sys : pro_sys_list_) {
            auto new_sys = sys->clone();
            new_sys->run(q, true);
            per.statistics(new_sys->getAccount(), end_date);
            double value = per.get(key);
            CLS_TRACE_IF(trace, "value: {}, sys: {}", value, sys->name());
            if (value < min_value) {
              min_value = value;
              selected_sys = new_sys;
            }
          }
        }

        return selected_sys;
      });

  size_t dates_len = dates.size();
  for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
    auto& selected_sys = sys_list[i];
    if (selected_sys) {
      selected_sys->reset();

      size_t train_start = train_ranges[i].first;
      size_t test_start = train_ranges[i].second;
      size_t test_end = test_start + test_len;
      if (test_end > dates_len) {
        test_end = dates_len;
      }

      for (size_t pos = test_start; pos < test_end; pos++) {
        sys_dict_[dates[pos]] = selected_sys;
      }

      if (test_end < dates_len) {
        run_ranges_.emplace_back(
            RunRanges(dates[train_start], dates[test_start], dates[test_end]));
      } else {
        run_ranges_.emplace_back(RunRanges(dates[train_start],
                                           dates[test_start],
                                           dates[test_end - 1] + Minutes(1)));
      }

      CLS_INFO_IF(trace, "iteration: {}, selected_sys: {}", i + 1,
                  selected_sys->name());
    }
  }
}

SEPtr SE_PerformanceOptimal(const string& key, int mode) {
  PerformanceOptimalSelector* p = new PerformanceOptimalSelector();
  p->setParam<string>("key", key);
  p->setParam<int>("mode", mode);
  return SEPtr(p);
}

}  // namespace hayaku
