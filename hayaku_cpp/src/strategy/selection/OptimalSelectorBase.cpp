/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-13
 *      Author: fasiondog
 */

#include "OptimalSelectorBase.h"

#include "data/DataRuntime.h"
#include "strategy/StrategyRuntime.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OptimalSelectorBase)
#endif

namespace hayaku {

string OptimalSelectorBase::str() const {
  std::ostringstream buf;
  buf << "Selector(" << name() << ", " << getParameter()
      << ", candidate systems count: " << pro_sys_list_.size() << ")";
  return buf.str();
}

OptimalSelectorBase::OptimalSelectorBase()
    : SelectorBase("OptimalSelectorBase") {
  _initParams();
}

OptimalSelectorBase::OptimalSelectorBase(const string& name)
    : SelectorBase(name) {
  _initParams();
}

void OptimalSelectorBase::_initParams() {
  setParam<bool>("depend_on_proto_sys", true);
  setParam<string>("market", "SH");
  setParam<int>("index", 0);  // Take the index-th result after the sorting
  setParam<int>("train_len", 100);
  setParam<int>("test_len", 20);
  setParam<bool>("trace", false);
}

void OptimalSelectorBase::_checkParam(const string& name) const {
  if ("train_len" == name) {
    HAYAKU_ASSERT(getParam<int>("train_len") > 0);
  } else if ("test_len" == name) {
    HAYAKU_ASSERT(getParam<int>("test_len") > 0);
  } else if ("index" == name) {
    HAYAKU_ASSERT(getParam<int>("index") >= 0);
  } else if ("depend_on_proto_sys" == name) {
    HAYAKU_ASSERT(getParam<bool>("depend_on_proto_sys"));
  } else if ("market" == name) {
    string market = getParam<string>(name);
    auto market_info = getDataRuntime().getMarketInfo(market);
    HAYAKU_CHECK(market_info != Null<MarketInfo>(), "Invalid market: {}",
                 market);
  }
}

StrategyWeightList OptimalSelectorBase::_getSelected(Datetime date) {
  StrategyWeightList ret;
  auto iter = sys_dict_.find(date);
  if (iter != sys_dict_.end()) {
    int index = getParam<int>("index");
    if (index < iter->second->size()) {
      ret.emplace_back(iter->second->at(index));
    } else {
      ret.emplace_back(iter->second->at(iter->second->size() - 1));
    }
  }
  return ret;
}

bool OptimalSelectorBase::isMatchAF(const AFPtr& af) { return true; }

void OptimalSelectorBase::_reset() {
  sys_dict_.clear();
  run_ranges_.clear();
}

void OptimalSelectorBase::_calculate() {}

void OptimalSelectorBase::calculate(
    const internal::StrategyRuntimeList& pf_realSysList, const KQuery& query) {
  SPEND_TIME(OptimalSelectorBase_calculate);
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

  _calculate_parallel(train_ranges, dates, test_len, trace);

  calculated_ = true;
}

void OptimalSelectorBase::_calculate_parallel(
    const vector<std::pair<size_t, size_t>>& train_ranges,
    const DatetimeList& dates, size_t test_len, bool trace) {
  // SPEND_TIME(OptimalSelectorBase_calculate_parallel);
  auto sys_list = global_parallel_for_index(
      0, train_ranges.size(),
      [this, &train_ranges, &dates, query = query_, trace](size_t i) {
        Datetime start_date = dates[train_ranges[i].first];
        Datetime end_date = dates[train_ranges[i].second];
        KQuery q = KQueryByDate(start_date, end_date, query.kType(),
                                query.recoverType());
        CLS_INFO_IF(trace, "iteration: {}|{}, range: {}", i + 1,
                    train_ranges.size(), q);

        auto selected_sys_list = std::make_shared<StrategyWeightList>();
        for (const auto& sys : pro_sys_list_) {
          try {
            auto nsys = sys->clone();
            nsys->run(q, true);
            double value = evaluate(nsys, end_date);
            nsys->reset();
            if (!std::isnan(value)) {
              selected_sys_list->emplace_back(
                  StrategyWeight(nsys->clone(), value));
            }
          } catch (const std::exception& e) {
            CLS_ERROR("{}! {}", e.what(), sys->name());
          } catch (...) {
            CLS_ERROR("Unknown error! {}", sys->name());
          }
        }

        if (!selected_sys_list->empty()) {
          // Sort in the descending order; on equality the one earlier in the
          // candidates is taken
          std::stable_sort(
              selected_sys_list->begin(), selected_sys_list->end(),
              [](const StrategyWeight& a, const StrategyWeight& b) {
                return a.weight > b.weight;
              });
        }
        return selected_sys_list;
      });

  size_t dates_len = dates.size();
  for (size_t i = 0, total = train_ranges.size(); i < total; i++) {
    auto& selected_sys_list = sys_list[i];
    if (!selected_sys_list->empty()) {
      size_t train_start = train_ranges[i].first;
      size_t test_start = train_ranges[i].second;
      size_t test_end = test_start + test_len;
      if (test_end > dates_len) {
        test_end = dates_len;
      }

      for (size_t pos = test_start; pos < test_end; pos++) {
        sys_dict_[dates[pos]] = selected_sys_list;
      }

      if (test_end < dates_len) {
        run_ranges_.emplace_back(
            RunRanges(dates[train_start], dates[test_start], dates[test_end]));
      } else {
        run_ranges_.emplace_back(RunRanges(dates[train_start],
                                            dates[test_start],
                                            dates[test_end - 1] + Minutes(1)));
      }
    }
  }
}

}  // namespace hayaku
