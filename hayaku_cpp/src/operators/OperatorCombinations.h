#pragma once

/*
 * Copyright (c) 2023 hikyuu.org
 */


#include "operators/Indicator.h"
#include "common/Log.h"

namespace hayaku {

/** Return all non-empty combinations of indexes in the supplied sequence. */
template <class T>
std::vector<std::vector<size_t>> combinateIndex(const std::vector<T>& inputs) {
    const size_t total = inputs.size();
    HAYAKU_CHECK(total <= 15, "The length of inputs exceeds the maximum limit!");

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
std::vector<Indicator> HAYAKU_API combinateIndicator(const std::vector<Indicator>& inputs, int n);

}  // namespace hayaku
