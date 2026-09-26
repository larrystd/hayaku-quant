/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "RealtimeDataSource.h"

#include <mutex>

namespace hayaku {

namespace {

struct RealtimeDataSourceState {
  std::mutex mutex;
  RealtimeDataSourceResolver resolver;
};

RealtimeDataSourceState& sourceState() {
  static auto* state = new RealtimeDataSourceState;
  return *state;
}

}  // namespace

void setRealtimeDataSourceResolver(RealtimeDataSourceResolver resolver) {
  auto& state = sourceState();
  std::lock_guard<std::mutex> lock(state.mutex);
  state.resolver = std::move(resolver);
}

RealtimeDataSource* getRealtimeDataSource() noexcept {
  RealtimeDataSourceResolver resolver;
  {
    auto& state = sourceState();
    std::lock_guard<std::mutex> lock(state.mutex);
    resolver = state.resolver;
  }
  if (!resolver) {
    return nullptr;
  }
  try {
    return resolver();
  } catch (...) {
    return nullptr;
  }
}

}  // namespace hayaku
