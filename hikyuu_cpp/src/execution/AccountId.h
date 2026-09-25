/*
 * Copyright (c) 2026 hikyuu.org
 */

#pragma once
#ifndef HIKYUU_TRADE_ACCOUNTID_H
#define HIKYUU_TRADE_ACCOUNTID_H

#include <cstdint>
#include <functional>

#include "../config.h"

namespace hku {

/** Stable, strongly typed identity for one execution account. */
class HKU_API AccountId {
public:
    constexpr AccountId() noexcept = default;
    explicit constexpr AccountId(std::uint64_t value) noexcept : m_value(value) {}

    [[nodiscard]] constexpr std::uint64_t value() const noexcept {
        return m_value;
    }

    [[nodiscard]] constexpr bool valid() const noexcept {
        return m_value != 0;
    }

    friend constexpr bool operator==(AccountId, AccountId) noexcept = default;

private:
    std::uint64_t m_value{0};
};

}  // namespace hku

template <>
struct std::hash<hku::AccountId> {
    size_t operator()(hku::AccountId id) const noexcept {
        return std::hash<std::uint64_t>{}(id.value());
    }
};

#endif /* HIKYUU_TRADE_ACCOUNTID_H */
