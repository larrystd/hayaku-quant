/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#include "OptimalEvaluateSelector.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::OptimalEvaluateSelector)
#endif

namespace hayaku {

OptimalEvaluateSelector::OptimalEvaluateSelector() : OptimalSelectorBase("SE_EvaluateOptimal") {}

OptimalEvaluateSelector::OptimalEvaluateSelector(
  std::function<double(const internal::StrategyRuntimePtr&, const Datetime&)>&& evaluate)
: OptimalSelectorBase("SE_EvaluateOptimal"), m_evaluate(std::move(evaluate)) {}

OptimalEvaluateSelector::~OptimalEvaluateSelector() {}

double OptimalEvaluateSelector::evaluate(const internal::StrategyRuntimePtr& sys, const Datetime& endDate) noexcept {
    double ret = Null<double>();
    try {
        ret = m_evaluate(sys, endDate);
    } catch (const std::exception& e) {
        HAYAKU_ERROR("Failed evaluate! {}! {}", e.what(), name());
    } catch (...) {
        HAYAKU_ERROR("Failed evaluate! Unknown exception! {}", name());
    }
    return ret;
}

SEPtr HAYAKU_API
SE_EvaluateOptimal(std::function<double(const internal::StrategyRuntimePtr&, const Datetime&)>&& evaluate) {
    return make_shared<OptimalEvaluateSelector>(std::move(evaluate));
}

}  // namespace hayaku
