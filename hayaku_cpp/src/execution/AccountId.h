#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <cstdint>
#include <functional>

#include "../config.h"

namespace hayaku {

/** Stable, strongly typed identity for one execution account. */
class HAYAKU_API AccountId {
 public:
  constexpr AccountId() noexcept = default;
  explicit constexpr AccountId(std::uint64_t value) noexcept : m_value(value) {}

  [[nodiscard]] constexpr std::uint64_t value() const noexcept {
    return m_value;
  }

  [[nodiscard]] constexpr bool valid() const noexcept { return m_value != 0; }

  friend constexpr bool operator==(AccountId, AccountId) noexcept = default;

 private:
  std::uint64_t m_value{0};
};

}  // namespace hayaku

template <>
struct std::hash<hayaku::AccountId> {
  size_t operator()(hayaku::AccountId id) const noexcept {
    return std::hash<std::uint64_t>{}(id.value());
  }
};
