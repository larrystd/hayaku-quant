#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Data-owned port for optional extended K-line implementations.
 */

#include <functional>

#include "KDataImp.h"

namespace hayaku {

class KDataExtension {
 public:
  virtual ~KDataExtension() = default;

  virtual void registerKTypeExtra(
      const string& ktype, const string& basetype, int32_t minutes,
      std::function<Datetime(const Datetime&)> getPhaseEnd) = 0;
  virtual void releaseKExtra() = 0;
  [[nodiscard]] virtual bool isExtraKType(const string& ktype) = 0;
  [[nodiscard]] virtual std::vector<string> getExtraKTypeList() = 0;
  [[nodiscard]] virtual int32_t getKTypeExtraMinutes(const string& ktype) = 0;
  [[nodiscard]] virtual KRecordList getExtraKRecordList(
      const Stock& stock, const KQuery& query) = 0;
  [[nodiscard]] virtual size_t getStockExtraCount(const Stock& stock,
                                                  const string& ktype) = 0;
  [[nodiscard]] virtual bool getStockExtraIndexRange(const Stock& stock,
                                                     const KQuery& query,
                                                     size_t& outStart,
                                                     size_t& outEnd) = 0;
  [[nodiscard]] virtual KDataImpPtr getKDataImp(const Stock& stock,
                                                const KQuery& query) = 0;
  [[nodiscard]] virtual bool canLazyLoad(const KQuery::KType& ktype) = 0;
  virtual void enableKDataCache(bool enable) = 0;
};

using KDataExtensionResolver = std::function<KDataExtension*()>;

/** Install/remove the application-provided extension resolver. No plugin is
 * loaded here. */
void setKDataExtensionResolver(KDataExtensionResolver resolver);
[[nodiscard]] KDataExtension* getKDataExtension() noexcept;

// Internal data-domain helpers. They always have a safe no-extension fallback.
[[nodiscard]] bool isExtraKType(const string& ktype);
[[nodiscard]] int32_t getKTypeExtraMinutes(const string& ktype);
[[nodiscard]] std::vector<string> getExtraKTypeList();
[[nodiscard]] KRecordList getExtraKRecordList(const Stock& stock,
                                              const KQuery& query);
[[nodiscard]] size_t getStockExtraCount(const Stock& stock,
                                        const string& ktype);
[[nodiscard]] bool getStockExtraIndexRange(const Stock& stock,
                                           const KQuery& query,
                                           size_t& outStart, size_t& outEnd);
[[nodiscard]] KDataImpPtr getKDataImp(const Stock& stock, const KQuery& query);
[[nodiscard]] bool canLazyLoad(const KQuery::KType& ktype);

}  // namespace hayaku
