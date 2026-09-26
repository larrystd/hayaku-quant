#pragma once

/*
 * StrategyWeight.h
 *
 *  Created on: 2018-1-29
 *      Author: fasiondog
 */

#include "strategy/StrategyRuntime.h"

namespace hayaku {

/**
 * Strategy weight, the valid range of the weight is [0.0, 1.0]
 * @details It is used to give the corresponding weight of a system to the asset
 * allocation algorithm
 * @ingroup Selector
 */
struct HAYAKU_API StrategyWeight {
  internal::StrategyRuntimePtr strategy;
  double weight{1.0};

  StrategyWeight() = default;
  StrategyWeight(internal::StrategyRuntimePtr strategy_, double weight_)
      : strategy(std::move(strategy_)), weight(weight_) {}
  StrategyWeight(const StrategyWeight&) = default;
  StrategyWeight(StrategyWeight&& item)
      : strategy(std::move(item.strategy)), weight(item.weight) {}
  StrategyWeight& operator=(const StrategyWeight& other) = default;
  StrategyWeight& operator=(StrategyWeight&& other);

 private:
//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& BOOST_SERIALIZATION_NVP(weight);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_NVP(weight);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

using StrategyWeightList = vector<StrategyWeight>;

HAYAKU_API std::ostream& operator<<(std::ostream&, const StrategyWeight&);

inline bool operator==(const StrategyWeight& lhs, const StrategyWeight& rhs) {
  return lhs.strategy == rhs.strategy &&
         std::abs(lhs.weight - rhs.weight) < 0.0001;
}

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::StrategyWeight> : ostream_formatter {};
#endif
