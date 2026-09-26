/*
 * Copyright (c) 2023 hikyuu.org
 */

#include "OperatorCombinations.h"

#include "operators/BooleanOperators.h"

namespace hayaku {

std::vector<Indicator> combinateIndicator(const std::vector<Indicator>& inputs,
                                          int n) {
  std::vector<Indicator> result;
  for (const auto& indexes : combinateIndex(inputs)) {
    Indicator combined = EXIST(inputs[indexes.front()], n);
    string name = inputs[indexes.front()].name();
    for (size_t i = 1; i < indexes.size(); ++i) {
      combined = combined & EXIST(inputs[indexes[i]], n);
      name = fmt::format("{} & {}", name, inputs[indexes[i]].name());
    }
    combined.name(name);
    result.emplace_back(std::move(combined));
  }
  return result;
}

}  // namespace hayaku
