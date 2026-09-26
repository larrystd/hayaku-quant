#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-05-27
 *      Author: fasiondog
 */

#include "strategy/selection/SelectorBase.h"

namespace hayaku {

class HAYAKU_API OperatorSelector : public SelectorBase {
 public:
  OperatorSelector();
  explicit OperatorSelector(const string& name);
  OperatorSelector(const string& name, const SelectorPtr& se1,
                   const SelectorPtr& se2);
  virtual ~OperatorSelector();

  virtual void _reset() override;
  virtual SelectorPtr _clone() override;
  virtual bool isMatchAF(const AFPtr& af) override;
  virtual void _calculate() override;
  virtual StrategyWeightList _getSelected(Datetime date) override {
    return StrategyWeightList();
  }

  virtual void _addSystem(const internal::StrategyRuntimePtr& sys) override;
  virtual void _removeAll() override;

 protected:
  virtual void bindRealToProto(
      const internal::StrategyRuntimePtr& real,
      const internal::StrategyRuntimePtr& proto) override {
    m_real_to_proto[real] = proto;
  }

  StrategyWeightList getUnionSelected(
      Datetime date, const std::function<double(double, double)>&& func);

  StrategyWeightList getIntersectionSelected(
      Datetime date, const std::function<double(double, double)>&& func);

 protected:
  static void sortStrategyWeightList(StrategyWeightList& swlist);

  void build();
  void cloneRebuild(const SelectorPtr& se1, const SelectorPtr& se2);

 protected:
  SelectorPtr m_se1;
  SelectorPtr m_se2;
  std::unordered_set<internal::StrategyRuntimePtr>
      m_se1_set;  // The prototype system instance set of se1
  std::unordered_set<internal::StrategyRuntimePtr>
      m_se2_set;  // The prototype system instance set of se2
  std::unordered_map<internal::StrategyRuntimePtr, internal::StrategyRuntimePtr>
      m_real_to_proto;

 private:
  static std::unordered_set<internal::StrategyRuntime*> findIntersection(
      const SelectorPtr& se1, const SelectorPtr& se2);

 private:
  //============================================
  // Serialization support
  //============================================
#if HAYAKU_SUPPORT_SERIALIZATION
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    ar& BOOST_SERIALIZATION_NVP(m_se1);
    ar& BOOST_SERIALIZATION_NVP(m_se2);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(SelectorBase);
    ar& BOOST_SERIALIZATION_NVP(m_se1);
    ar& BOOST_SERIALIZATION_NVP(m_se2);
    build();
  }
#endif
};

#define OPERATOR_SELECTOR_IMP(classname, name)                     \
 public:                                                           \
  classname() : OperatorSelector(name) {}                          \
  classname(const SelectorPtr& se1, const SelectorPtr& se2)        \
      : OperatorSelector(name, se1, se2) {}                        \
  virtual ~classname() override {}                                 \
                                                                   \
  virtual StrategyWeightList _getSelected(Datetime date) override; \
                                                                   \
  virtual SelectorPtr _clone() override {                          \
    HAYAKU_THROW("OperatorSelector Could't support clone!");       \
  }

#if HAYAKU_SUPPORT_SERIALIZATION
#define OPERATOR_SELECTOR_SERIALIZATION                        \
 private:                                                      \
  friend class boost::serialization::access;                   \
  template <class Archive>                                     \
  void serialize(Archive& ar, const unsigned int version) {    \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(OperatorSelector); \
  }
#else
#define OPERATOR_SELECTOR_SERIALIZATION
#endif

}  // namespace hayaku
