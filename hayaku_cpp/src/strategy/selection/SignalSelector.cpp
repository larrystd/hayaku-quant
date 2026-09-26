/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-19
 *      Author: fasiondog
 */

#include "SignalSelector.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::SignalSelector)
#endif

namespace hayaku {

SignalSelector::SignalSelector() : SelectorBase("SE_Sigal") {}

SignalSelector::~SignalSelector() {}

bool SignalSelector::isMatchAF(const AFPtr& af) {
  // HAYAKU_WARN_IF_RETURN(
  //   af->getParam<bool>("adjust_running_sys"), false,
  //   "AF will adjust running system funds, but this se is not suitable the
  //   case!");
  return true;
}

StrategyWeightList SignalSelector::_getSelected(Datetime date) {
  auto iter = sys_dict_.find(date);
  return iter != sys_dict_.end() ? iter->second : StrategyWeightList();
}

void SignalSelector::_calculate() {
  size_t total = real_sys_list_.size();
  for (size_t i = 0; i < total; i++) {
    auto& sys = real_sys_list_[i];
    auto sg = sys->getSG();
    auto dates = sg->getBuySignal();
    for (auto& date : dates) {
      auto iter = sys_dict_.find(date);
      if (iter != sys_dict_.end()) {
        iter->second.emplace_back(sys, 1.0);
      } else {
        sys_dict_[date] = {StrategyWeight(sys, 1.0)};
      }
    }
  }
}

SEPtr SE_Signal() { return make_shared<SignalSelector>(); }

SEPtr SE_Signal(const StockList& stock_list,
                const internal::StrategyRuntimePtr& sys) {
  SelectorPtr p = make_shared<SignalSelector>();
  p->addStockList(stock_list, sys);
  return p;
}

}  // namespace hayaku
