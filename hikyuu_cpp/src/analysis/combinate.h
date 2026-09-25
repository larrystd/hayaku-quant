/*
 * Copyright (c) 2023 hikyuu.org
 */

#pragma once
#ifndef HIKYUU_ANALYSIS_COMBINATE_H
#define HIKYUU_ANALYSIS_COMBINATE_H

#include "data/indicator/Indicator.h"
#include "common/Log.h"

namespace hku {

/** Return all non-empty combinations of indexes in the supplied sequence. */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    const size_t total = inputs.size();
    HKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");

    std::vector<std::vector<size_t>> result;
    std::vector<size_t> current;
    for (size_t i = 0; i < total; ++i) {
        for (size_t j = 0, len = result.size(); j < len; ++j) {
            current.assign(result[j].cbegin(), result[j].cend());
            current.push_back(i);
            result.push_back(std::move(current));
        }
        current.clear();
        current.push_back(i);
        result.push_back(std::move(current));
    }
    return result;
}

/** Combine indicators without coupling analysis to an account or strategy runtime. */
std::vector<Indicator> HKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n);

}  // namespace hku

#endif /* HIKYUU_ANALYSIS_COMBINATE_H */
