#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Data-owned port used by the optional realtime/IPC component.
 */

#include <functional>

#include "data/storage/BaseInfoDriver.h"
#include "data/storage/BlockInfoDriver.h"
#include "data/storage/KDataDriver.h"

namespace hayaku {

enum class LoadEvent {
  BASE_DATA_READY,
  BLOCKS_LOADED,
  KDATA_PRELOAD_FINISHED,
  HISTORY_FINANCE_LOADED,
};

class RealtimeDataSource {
 public:
  virtual ~RealtimeDataSource() = default;

  virtual bool connect(const std::string& datadir,
                       uint64_t waitTimeoutSec) noexcept = 0;
  [[nodiscard]] virtual std::string serverAddr() const noexcept = 0;
  [[nodiscard]] virtual KDataDriverPtr createKDataDriver(
      const KDataDriverConnectPoolPtr& localPool) noexcept = 0;
  [[nodiscard]] virtual BaseInfoDriverPtr createBaseInfoDriver(
      const BaseInfoDriverPtr& local) noexcept = 0;
  [[nodiscard]] virtual BlockInfoDriverPtr createBlockDriver(
      const BlockInfoDriverPtr& local) noexcept = 0;
};

using RealtimeDataSourceResolver = std::function<RealtimeDataSource*()>;

HAYAKU_API void setRealtimeDataSourceResolver(
    RealtimeDataSourceResolver resolver);
[[nodiscard]] HAYAKU_API RealtimeDataSource* getRealtimeDataSource() noexcept;

}  // namespace hayaku
