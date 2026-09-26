#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "SelectorBase.h"

namespace hayaku {

class MultiFactorSelector : public SelectorBase {
 public:
  MultiFactorSelector();
  MultiFactorSelector(const MFPtr& mf, int topn);
  virtual ~MultiFactorSelector();

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
  ScoreRecordList filterOnlyShouldBuy(Datetime date,
                                      const ScoreRecordList& scores,
                                      size_t topn);
  ScoreRecordList filterTopN(Datetime date, const ScoreRecordList& raw_scores,
                             size_t topn, bool only_should_buy);
  ScoreRecordList filterTopNReverse(Datetime date,
                                    const ScoreRecordList& raw_scores,
                                    size_t topn, bool only_should_buy,
                                    bool ignore_null);

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
