#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Core-owned port for optional extended performance reporting.
 */

#include <functional>

#include "common/time/Datetime.h"
#include "data/KQuery.h"
#include "execution/ExecutionAccountPort.h"
#include "metrics/Performance.h"

namespace hayaku {

class ReportExtension {
 public:
  virtual ~ReportExtension() = default;

  virtual Performance getExtPerformance(
      const internal::ExecutionAccountPortPtr& account,
      const Datetime& datetime, const KQuery::KType& ktype) = 0;
};

using ReportExtensionResolver = std::function<ReportExtension*()>;

HAYAKU_API void setReportExtensionResolver(ReportExtensionResolver resolver);
[[nodiscard]] HAYAKU_API ReportExtension* getReportExtension() noexcept;

}  // namespace hayaku
