/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include "FactorSet.h"

#include <algorithm>

#include "CompiledFactorPlan.h"
#include "FactorStore.h"
#include "data/DataRuntime.h"
#include "operators/SeriesOperators.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const FactorSet& set) {
  os << set.str();
  return os;
}

string FactorSet::str() const {
  return fmt::format("FactorSet({}, {}, {}, {})", name(), ktype(), size(),
                     block());
}

FactorSet::FactorSet() : data_(make_shared<Data>()) {}

FactorSet::FactorSet(const IndicatorList& inds, const KQuery::KType& ktype)
    : data_(make_shared<Data>()) {
  data_->name = fmt::format("FSET_{}", Datetime::now().ticks());
  data_->ktype = ktype;
  for (const auto& factor : inds) {
    add(factor);
  }
}

FactorSet::FactorSet(const std::unordered_map<string, Indicator>& inds,
                     const KQuery::KType& ktype)
    : data_(make_shared<Data>()) {
  data_->name = fmt::format("FSET_{}", Datetime::now().ticks());
  data_->ktype = ktype;
  for (const auto& item : inds) {
    add(item.first, item.second);
  }
}

FactorSet::FactorSet(const string& name, const KQuery::KType& ktype,
                     const Block& block)
    : data_(make_shared<Data>()) {
  data_->name = utf8_to_upper(name);
  data_->ktype = ktype;
  data_->block = block;
}

FactorSet::FactorSet(const FactorList& factors, const KQuery::KType& ktype,
                     const Block& block, const string& name)
    : data_(make_shared<Data>()) {
  data_->name = utf8_to_upper(name);
  data_->ktype = ktype;
  data_->block = block;
  add(factors);
}

FactorSet::FactorSet(const FactorSet& other) : data_(other.data_) {}

FactorSet::FactorSet(FactorSet&& other) : data_(std::move(other.data_)) {}

FactorSet& FactorSet::operator=(const FactorSet& other) {
  HAYAKU_IF_RETURN(this == &other, *this);
  data_ = other.data_;
  return *this;
}

FactorSet& FactorSet::operator=(FactorSet&& other) {
  HAYAKU_IF_RETURN(this == &other, *this);
  data_ = std::move(other.data_);
  return *this;
}

void FactorSet::add(const Factor& factor) {
  HAYAKU_CHECK(!factor.isNull(), "Factor is null!");
  HAYAKU_CHECK(factor.ktype() == data_->ktype, "ktype not match!");
  HAYAKU_CHECK(factor.block() == data_->block, "block not match!");

  const string& factor_name = factor.name();

  // Check whether a factor with the same name exists already
  auto it = data_->nameIndexMap.find(factor_name);
  if (it != data_->nameIndexMap.end()) {
    // A factor with the same name exists, overwrite it
    size_t index = it->second;
    data_->factors[index] = factor;
    HAYAKU_WARN("Factor '{}' already exists, it will be overwritten!",
                factor_name);

  } else {
    // Append the new factor to the end of the vector
    size_t index = data_->factors.size();
    data_->factors.push_back(factor);
    // Record the name to index mapping in the map
    data_->nameIndexMap[factor_name] = index;
  }
}

void FactorSet::add(const string& name, const Indicator& ind) {
  add(Factor(name, ind, data_->ktype, "", "", false, Datetime::min(),
             data_->block));
}

void FactorSet::add(const Indicator& ind) {
  auto it = data_->nameIndexMap.find(ind.name());
  if (it != data_->nameIndexMap.end()) {
    add(fmt::format("{}_{}", ind.name(), Datetime::now().ticks()), ind);
  } else {
    add(ind.name(), ind);
  }
}

void FactorSet::add(const FactorList& factors) {
  for (const auto& factor : factors) {
    add(factor);
  }
}

void FactorSet::add(const IndicatorList& inds) {
  for (const auto& ind : inds) {
    add(ind);
  }
}

void FactorSet::add(const std::map<string, Indicator>& inds) {
  for (const auto& ind : inds) {
    add(ind.first, ind.second);
  }
}

void FactorSet::remove(const string& name) {
  auto it = data_->nameIndexMap.find(name);
  if (it == data_->nameIndexMap.end()) {
    return;  // The factor does not exist
  }

  size_t index_to_remove = it->second;
  size_t last_index = data_->factors.size() - 1;

  // When the element to delete is not the last one, the index of the following
  // element must be adjusted
  if (index_to_remove != last_index) {
    // Move the last element to the position to be deleted
    data_->factors[index_to_remove] = std::move(data_->factors[last_index]);
    // Update the index of the moved element in the map
    const string& moved_factor_name = data_->factors[index_to_remove].name();
    data_->nameIndexMap[moved_factor_name] = index_to_remove;
  }

  // Delete the last element and the map entry
  data_->factors.pop_back();
  data_->nameIndexMap.erase(it);
}

bool FactorSet::have(const string& name) const noexcept {
  return data_->nameIndexMap.find(name) != data_->nameIndexMap.end();
}

const Factor& FactorSet::get(const string& name) const {
  auto it = data_->nameIndexMap.find(name);
  HAYAKU_CHECK(it != data_->nameIndexMap.end(), "Factor '{}' not found!", name);
  return data_->factors[it->second];
}

void FactorSet::save_to_db() const { saveFactorSet(*this); }

void FactorSet::remove_from_db() const { removeFactorSet(name(), ktype()); }

void FactorSet::load_from_db() {
  FactorSet loaded_set = getFactorSet(name(), ktype());
  // The object returned by getFactorSet is Null, which is global
  if (!loaded_set.isNull()) {
    data_ = std::move(loaded_set.data_);
  }
}

vector<IndicatorList> FactorSet::getValues(
    const StockList& stocks, const KQuery& query, bool align, bool fill_null,
    bool tovalue, bool check, const DatetimeList& align_dates) const {
  // SPEND_TIME(FactorSet_getValues);
  if (check) {
    if (!block().empty()) {
      for (auto& stock : stocks) {
        HAYAKU_CHECK(block().have(stock), "Stock not belong to block! {}",
                     stock);
      }
    }
  }

  vector<IndicatorList> result;
  if (hasFactorStore()) {
    result = hayaku::getValues(*this, stocks, query, align, fill_null, tovalue,
                               align_dates);
    return result;
  }

  // Formula-bearing results must keep independent graphs. The compiled plan is
  // therefore an internal value-only fast path and is not observable through
  // the existing public API.
  if (tovalue) {
    const size_t stk_total = stocks.size();
    const size_t factor_total = data_->factors.size();
    result.resize(stk_total, IndicatorList(factor_total));
    HAYAKU_IF_RETURN(stk_total == 0 || factor_total == 0, result);

    DatetimeList dates;
    Indicator null_ind;
    if (align) {
      dates = align_dates.empty() ? getDataRuntime().getTradingCalendar(query)
                                  : align_dates;
      HAYAKU_IF_RETURN(dates.empty(), result);
      null_ind = PRICELIST(PriceList(dates.size(), Null<price_t>()), dates);
    }

    IndicatorList formulas;
    formulas.reserve(factor_total);
    for (const auto& factor : data_->factors) {
      formulas.emplace_back(align ? ALIGN(factor.formula(), dates, fill_null)
                                  : factor.formula());
    }
    const detail::CompiledFactorPlan plan(formulas);
    if (plan.isReusable()) {
      auto calculate_one = [&](detail::FactorPlanExecutor& executor, size_t i) {
        IndicatorList one_result(factor_total);
        KData kdata = stocks[i].getKData(query);
        if (kdata.empty()) {
          if (align) {
            std::fill(one_result.begin(), one_result.end(), null_ind);
          }
          return one_result;
        }

        return executor.executeValues(kdata);
      };

      // A range task is submitted to the global thread pool directly here, and
      // an outer caller may itself submit this function from a work thread (a
      // nested call). wait_for_all_non_blocking must support work-stealing the
      // submitted subtasks during the waiting (otherwise, when the pool is
      // filled by the outer tasks, it would deadlock: the outer waits for this
      // task, this task waits for the subtasks and no idle worker executes
      // them). Before changing this part,
      auto* task_group = get_global_task_group();
      HAYAKU_ASSERT(task_group);
      auto ranges = parallelIndexRange(0, stk_total, task_group->worker_num());
      vector<std::future<void>> tasks;
      tasks.reserve(ranges.size());
      for (const auto& range : ranges) {
        tasks.emplace_back(task_group->submit([&, range]() {
          auto executor = plan.createExecutor();
          for (size_t i = range.first; i < range.second; ++i) {
            result[i] = calculate_one(executor, i);
          }
        }));
      }
      wait_for_all_non_blocking(*task_group, tasks);
      for (auto& task : tasks) {
        task.get();
      }
      return result;
    }
  }

  // Create the result container, one IndicatorList per stock
  size_t stk_total = stocks.size();
  size_t factor_total = data_->factors.size();
  result.resize(stk_total);
  for (size_t i = 0; i < stk_total; ++i) {
    result[i].resize(factor_total);
  }

  const auto& factors = data_->factors;
  global_parallel_for_index_void(0, factor_total, [&](size_t i) {
    IndicatorList factor_values = factors[i].getValues(
        stocks, query, align, fill_null, tovalue, false, align_dates);
    for (size_t j = 0; j < stk_total; ++j) {
      result[j][i] = std::move(factor_values[j]);
    }
  });

  return result;
}

vector<IndicatorList> FactorSet::getAllValues(
    const KQuery& query, bool align, bool fill_null, bool tovalue,
    const DatetimeList& align_dates) const {
  StockList stocks = block().empty() ? getDataRuntime().getStockList()
                                     : block().getStockList();
  return getValues(stocks, query, align, fill_null, tovalue, false,
                   align_dates);
}

}  // namespace hayaku
