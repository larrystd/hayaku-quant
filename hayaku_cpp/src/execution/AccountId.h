#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <cstdint>
#include <functional>

#include "config.h"

namespace hayaku {

/** Stable, strongly typed identity for one execution account. */
class AccountId {
 public:
  constexpr AccountId() noexcept = default;
  explicit constexpr AccountId(std::uint64_t value) noexcept : value_(value) {}

  [[nodiscard]] constexpr std::uint64_t value() const noexcept {
    return value_;
  }

  [[nodiscard]] constexpr bool valid() const noexcept { return value_ != 0; }

  friend constexpr bool operator==(AccountId, AccountId) noexcept = default;

 private:
  std::uint64_t value_{0};
};

}  // namespace hayaku

template <>
struct std::hash<hayaku::AccountId> {
  size_t operator()(hayaku::AccountId id) const noexcept {
    return std::hash<std::uint64_t>{}(id.value());
  }
};
