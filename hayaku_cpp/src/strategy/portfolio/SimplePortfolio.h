#pragma once

/*
 * SimplePortfolio.h
 *
 *  Created on: 2016-2-21
 *      Author: fasiondog
 */

#include "strategy/portfolio/AllocateFundsBase.h"
#include "strategy/portfolio/Portfolio.h"
#include "strategy/selection/SelectorBase.h"

namespace hayaku {

/*
 * Portfolio
 * @ingroup SimplePortfolio
 */
class HAYAKU_API SimplePortfolio : public Portfolio {
  PORTFOLIO_IMP(SimplePortfolio)

 public:
  SimplePortfolio();
  SimplePortfolio(const internal::PortfolioAccountPortPtr& tm,
                  const SelectorPtr& se, const AFPtr& af);
  virtual ~SimplePortfolio();

  virtual json lastSuggestion() const override;

 private:
  internal::StrategyRuntimeList
      dlist_sys_list_;  // The systems that cannot execute a sell because the
                         // security is delisted (the whole assets are lost)
  StrategyWeightList
      delay_adjust_sys_list_;  // System list of the delayed rebalancing sells
  StrategyWeightList tmp_selected_list_;
  StrategyWeightList tmp_will_remove_sys_;

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
