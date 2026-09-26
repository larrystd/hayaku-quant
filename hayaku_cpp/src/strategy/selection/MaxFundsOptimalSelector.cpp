/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-09-22
 *      Author: fasiondog
 */

#include "MaxFundsOptimalSelector.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::MaxFundsOptimalSelector)
#endif

namespace hayaku {

MaxFundsOptimalSelector::MaxFundsOptimalSelector() : OptimalSelectorBase("SE_MaxFundsOptimal") {}

MaxFundsOptimalSelector::~MaxFundsOptimalSelector() {}

double MaxFundsOptimalSelector::evaluate(const internal::StrategyRuntimePtr& sys, const Datetime& endDate) noexcept {
    double ret = Null<double>();
    try {
        auto funds = sys->getAccount()->getFunds(endDate);
        ret = funds.total_assets();
    } catch (const std::exception& e) {
        HAYAKU_ERROR("Get funds failed! {}! {}", e.what(), name());
    } catch (...) {
        HAYAKU_ERROR("Get funds failed! Unknown exception! {}", name());
    }
    return ret;
}

SEPtr HAYAKU_API SE_MaxFundsOptimal() {
    return make_shared<MaxFundsOptimalSelector>();
}

}  // namespace hayaku
