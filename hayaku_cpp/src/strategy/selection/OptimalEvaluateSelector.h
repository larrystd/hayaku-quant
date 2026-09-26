#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */


#include "OptimalSelectorBase.h"

namespace hayaku {

class OptimalEvaluateSelector : public OptimalSelectorBase {
    OPTIMAL_SELECTOR_IMP(OptimalEvaluateSelector)
    OPTIMAL_SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    OptimalEvaluateSelector();
    OptimalEvaluateSelector(std::function<double(const internal::StrategyRuntimePtr&, const Datetime&)>&& evaluate);

    virtual ~OptimalEvaluateSelector();

private:
    std::function<double(const internal::StrategyRuntimePtr&, const Datetime&)> m_evaluate;
};

}  // namespace hayaku
