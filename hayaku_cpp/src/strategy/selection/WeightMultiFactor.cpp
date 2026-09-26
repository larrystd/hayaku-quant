/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include "WeightMultiFactor.h"

#include "common/concurrency/ParallelAlgorithms.h"
#include "operators/SeriesOperators.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::WeightMultiFactor)
#endif

namespace hayaku {

WeightMultiFactor::WeightMultiFactor() : MultiFactorBase("MF_Weight") {}

WeightMultiFactor::WeightMultiFactor(const PriceList& weights,
                                     const StockList& stks, const KQuery& query,
                                     const Stock& ref_stk, int ic_n,
                                     bool spearman, int mode,
                                     bool save_all_factors)
    : MultiFactorBase(stks, query, ref_stk, "MF_Weight", ic_n, spearman, mode,
                      save_all_factors),
      weights_(weights) {}

vector<Indicator> WeightMultiFactor::_calculate(
    const vector<IndicatorList>& all_stk_inds) {
  size_t days_total = ref_dates_.size();
  size_t stk_count = stks_.size();
  size_t ind_count = factorset_.size();

  return global_parallel_for_index(0, stk_count, [&](size_t si) {
    vector<price_t> sumByDate(days_total, 0.0);
    const auto& curStkInds = all_stk_inds[si];
    for (size_t ii = 0; ii < ind_count; ii++) {
      const auto& curInd = curStkInds[ii];
      const auto* ind_data = curInd.data();
      for (size_t di = 0; di < days_total; di++) {
        auto value = ind_data[di];
        if (!std::isnan(value)) {
          sumByDate[di] += value * weights_[ii];
        }
      }
    }

    Indicator ret = PRICELIST(sumByDate);
    ret.name("IC");

    // Update the discard
    size_t discard = days_total;
    for (size_t di = 0; di < days_total; di++) {
      if (!std::isnan(ret[di])) {
        discard = di;
        break;
      }
    }
    ret.setDiscard(discard);

    return ret;
  });
}

MultiFactorPtr HAYAKU_API MF_Weight() {
  return make_shared<WeightMultiFactor>();
}

MultiFactorPtr HAYAKU_API MF_Weight(const PriceList& weights,
                                    const StockList& stks, const KQuery& query,
                                    const Stock& ref_stk, int ic_n,
                                    bool spearman, int mode,
                                    bool save_all_factors) {
  return make_shared<WeightMultiFactor>(weights, stks, query, ref_stk, ic_n,
                                        spearman, mode, save_all_factors);
}

}  // namespace hayaku
