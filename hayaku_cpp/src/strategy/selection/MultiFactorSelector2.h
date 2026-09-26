#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-xx-xx
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hayaku {

class MultiFactorSelector2 : public SelectorBase {
 public:
  MultiFactorSelector2();
  MultiFactorSelector2(const MFPtr& mf);
  virtual ~MultiFactorSelector2();

  virtual void _checkParam(const string& name) const override;
  virtual void _reset() override;
  virtual SelectorPtr _clone() override;
  virtual StrategyWeightList _getSelected(Datetime date) override;
  virtual bool isMatchAF(const AFPtr& af) override;
  virtual void _calculate() override;

  void setFactorSet(const FactorSet& factorset) {
    HAYAKU_ASSERT(!factorset.empty());
    factorset_ = factorset;
    calculated_ = false;
  }

 private:
  FactorSet factorset_;
  unordered_map<Stock, internal::StrategyRuntimePtr> stk_sys_dict_;

  //============================================
  // Serialization support
  //============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void serialize(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    ar& boost::serialization::make_nvp("m_factorset", factorset_);
  }
#endif
};

}  // namespace hayaku
