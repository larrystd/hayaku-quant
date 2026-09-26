#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal live-broker orchestration boundary.
 */

#include "broker/OrderBrokerBase.h"

namespace hayaku::internal {

/** Broker synchronization capabilities kept outside the strategy-facing account
 * contract. */
class ExecutionBrokerPort {
 public:
  virtual ~ExecutionBrokerPort() = default;

  virtual void regBroker(const OrderBrokerPtr& broker) = 0;
  virtual void clearBroker() = 0;
  [[nodiscard]] virtual Datetime getBrokerLastDatetime() const noexcept = 0;
  virtual void setBrokerLastDatetime(const Datetime& datetime) noexcept = 0;
  virtual void fetchAssetInfoFromBroker(
      const OrderBrokerPtr& broker,
      const Datetime& datetime = Null<Datetime>()) = 0;
};

using ExecutionBrokerPortPtr = std::shared_ptr<ExecutionBrokerPort>;

}  // namespace hayaku::internal
