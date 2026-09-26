#pragma once

/*
 * build_in.h
 *
 *  Created on: 2016-3-28
 *      Author: fasiondog
 */

/*
 * SE_Fixed.h
 *
 *  Created on: 2018-1-13
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hayaku {

/**
 * @brief Fixed selector, it selects the given trading system every day
 * @param weight fixed weight
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_Fixed(double weight = 1.0);

/**
 * @brief Fixed selector, it selects the given trading system every day
 * @details It creates the corresponding trading system with the prototype
 * system for every given stock
 * @param stock_list the given stock list
 * @param sys prototype system
 * @param weight fixed weight
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_Fixed(const StockList& stock_list,
                     const internal::StrategyRuntimePtr& sys,
                     double weight = 1.0);

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

namespace hayaku {

SelectorPtr operator+(const SelectorPtr& se1, const SelectorPtr& se2);
SelectorPtr operator-(const SelectorPtr& se1, const SelectorPtr& se2);
SelectorPtr operator*(const SelectorPtr& se1, const SelectorPtr& se2);
SelectorPtr operator/(const SelectorPtr& se1, const SelectorPtr& se2);

inline SelectorPtr operator&(const SelectorPtr& se1, const SelectorPtr& se2) {
  return se1 * se2;
}

inline SelectorPtr operator|(const SelectorPtr& se1, const SelectorPtr& se2) {
  return se1 + se2;
}

SelectorPtr operator+(const SelectorPtr& se, double value);
inline SelectorPtr operator+(double value, const SelectorPtr& se) {
  return se + value;
}

SelectorPtr operator-(const SelectorPtr& se, double value);
SelectorPtr operator-(double value, const SelectorPtr& se);

SelectorPtr operator*(const SelectorPtr& se, double value);
inline SelectorPtr operator*(double value, const SelectorPtr& se) {
  return se * value;
}

SelectorPtr operator/(const SelectorPtr& se, double value);
SelectorPtr operator/(double value, const SelectorPtr& se);

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "strategy/selection/MultiFactorBase.h"

namespace hayaku {

/**
 * @brief Stock selection algorithm based on MultiFactor
 * @param mf the MultiFactor instance
 * @param topn select the first topn systems in the time cross-section only
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_MultiFactor(const MFPtr& mf, int topn = 10);

/**
 * @brief Stock selection algorithm based on MultiFactor, it supports multiple
 * factor input ways
 *
 * Three creation ways are supported:
 * 1. Use a pre-created MultiFactor object directly
 * 2. Create it directly with a FactorSet object
 * 3. Use an IndicatorList (it is converted into a FactorSet internally and
 * automatically)
 *
 * @param src_inds the factor input, it can be a FactorSet object or an
 * IndicatorList
 * @param topn select the first topn systems in the time cross-section only;
 * less than or equal to 0 means no limit
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day
 * moving average of the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001
 * when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by
 * default, otherwise pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of
 * the factor synthesis algorithm
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_MultiFactor(const FactorSet& src_inds, int topn = 10,
                           int ic_n = 5, int ic_rolling_n = 120,
                           const Stock& ref_stk = Stock(), bool spearman = true,
                           const string& mode = "MF_ICIRWeight");

/**
 * @brief Convenience interface for creating the MultiFactor stock selection
 * algorithm based on an IndicatorList
 * @details It converts the IndicatorList into a FactorSet internally and then
 * calls the main function
 * @param src_inds the original factor list
 * @param topn select the first topn systems in the time cross-section only;
 * less than or equal to 0 means no limit
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day
 * moving average of the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001
 * when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by
 * default, otherwise pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of
 * the factor synthesis algorithm
 * @return SelectorPtr
 * @ingroup Selector
 */
inline SelectorPtr SE_MultiFactor(const IndicatorList& src_inds, int topn = 10,
                                  int ic_n = 5, int ic_rolling_n = 120,
                                  const Stock& ref_stk = Stock(),
                                  bool spearman = true,
                                  const string& mode = "MF_ICIRWeight") {
  return SE_MultiFactor(FactorSet(src_inds), topn, ic_n, ic_rolling_n, ref_stk,
                        spearman, mode);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "strategy/selection/ScoreFilters.h"

namespace hayaku {

/**
 * @brief Stock selection algorithm based on MultiFactor
 * @param mf the MultiFactor instance
 * @param filter the score filter
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_MultiFactor2(
    const MFPtr& mf, const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief Stock selection algorithm based on MultiFactor, it supports multiple
 * factor input ways
 *
 * Three creation ways are supported:
 * 1. Use a pre-created MultiFactor object directly
 * 2. Create it directly with a FactorSet object
 * 3. Use an IndicatorList (it is converted into a FactorSet internally and
 * automatically)
 *
 * @param factorset the factor input, it can be a FactorSet object or an
 * IndicatorList
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day
 * moving average of the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001
 * when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by
 * default, otherwise pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of
 * the factor synthesis algorithm
 * @param filter the score filter
 * @return SelectorPtr
 * @ingroup Selector
 */
SelectorPtr SE_MultiFactor2(
    const FactorSet& factorset, int ic_n = 5, int ic_rolling_n = 120,
    const Stock& ref_stk = Stock(), bool spearman = true,
    const string& mode = "MF_ICIRWeight",
    const ScoresFilterPtr& filter = SCFilter_IgnoreNan());

/**
 * @brief Convenience interface for creating the MultiFactor2 stock selection
 * algorithm based on an IndicatorList
 * @details It converts the IndicatorList into a FactorSet internally and then
 * calls the main function
 * @param src_inds the original factor list
 * @param ic_n the ic_n day return corresponding to the ic
 * @param ic_rolling_n the period for calculating the rolling IC (i.e. the n-day
 * moving average of the IC)
 * @param ref_stk the reference security, used for the date alignment, sh000001
 * when it is not given
 * @param spearman spearman is used to calculate the correlation coefficient by
 * default, otherwise pearson
 * @param mode "MF_ICIRWeight" | "MF_ICWeight" | "MF_EqualWeight", the name of
 * the factor synthesis algorithm
 * @param filter the score filter
 * @return SelectorPtr
 * @ingroup Selector
 */
inline SelectorPtr SE_MultiFactor2(
    const IndicatorList& src_inds, int ic_n = 5, int ic_rolling_n = 120,
    const Stock& ref_stk = Stock(), bool spearman = true,
    const string& mode = "MF_ICIRWeight",
    const ScoresFilterPtr& filter = SCFilter_IgnoreNan()) {
  return SE_MultiFactor2(FactorSet(src_inds), ic_n, ic_rolling_n, ref_stk,
                         spearman, mode, filter);
}

}  // namespace hayaku

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-14
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Selector that optimizes the maximum account assets
 * @return SEPtr
 */
SEPtr SE_MaxFundsOptimal();

/**
 * Selector that performs the optimization with the Performance statistics
 * results
 * @return SEPtr
 */
SEPtr SE_PerformanceOptimal(const string& key = "Account Avg Annual Return %",
                            int mode = 0);

/**
 * Selector that performs the optimization with a custom evaluation function
 * @param evaluate
 * @return SEPtr
 */
SEPtr SE_EvaluateOptimal(
    std::function<double(const internal::StrategyRuntimePtr&,
                         const Datetime&)>&& evaluate);

}  // namespace hayaku

/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * @brief Signal selector, it selects with the system buy signals only
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr SE_Signal();

/**
 * @brief Signal selector, it selects with the system buy signals only
 * @param stock_list stock list
 * @param sys prototype system
 * @return SEPtr
 * @ingroup Selector
 */
SEPtr SE_Signal(const StockList& stock_list,
                const internal::StrategyRuntimePtr& sys);

}  // namespace hayaku
