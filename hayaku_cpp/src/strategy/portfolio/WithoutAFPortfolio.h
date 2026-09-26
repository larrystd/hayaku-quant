#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-18
 *      Author: fasiondog
 */

#include "strategy/portfolio/Portfolio.h"

namespace hayaku {

/*
 * Portfolio without a fund allocation algorithm
 * @ingroup WithoutAFPortfolio
 */
class HAYAKU_API WithoutAFPortfolio : public Portfolio {
  PORTFOLIO_IMP(WithoutAFPortfolio)

 public:
  WithoutAFPortfolio();
  WithoutAFPortfolio(const internal::PortfolioAccountPortPtr& tm,
                     const SelectorPtr& se);
  virtual ~WithoutAFPortfolio();

  virtual json lastSuggestion() const override;

 private:
  void initParam();
  void _runMomentWithoutAFNotForceSell(const Datetime& date,
                                       const Datetime& nextCycle, bool adjust);
  void _runMomentWithoutAFForceSell(const Datetime& date,
                                    const Datetime& nextCycle, bool adjust);

 private:
  internal::StrategyRuntimeList
      m_force_sell_sys_list;  // System list of the forced sells
  list<internal::StrategyRuntimePtr>
      m_running_sys_list;  // List of the currently running systems, they need
                           // to be executed in turn
  internal::StrategyRuntimeList
      m_selected_list;  // System list selected in the current cycle

  // Records the mapping from the systems assigned to SE to the internal actual
  // systems
  unordered_map<internal::StrategyRuntimePtr, internal::StrategyRuntimePtr>
      m_se_sys_to_pf_sys_dict;

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(Portfolio);
  }
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

} /* namespace hayaku */
