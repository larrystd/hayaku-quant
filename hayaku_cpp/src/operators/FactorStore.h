#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Data-owned port for optional factor persistence and backend-side factor
 * evaluation.
 */

#include <functional>

#include "FactorSet.h"

namespace hayaku {

class FactorStore {
 public:
  virtual ~FactorStore() = default;

  [[nodiscard]] virtual bool isValidFactorName(const string& name) noexcept = 0;
  [[nodiscard]] virtual FactorList getAllFactors() = 0;
  [[nodiscard]] virtual FactorSetList getAllFactorSets() = 0;
  virtual void updateAllFactorsValues(const KQuery::KType& ktype) = 0;

  [[nodiscard]] virtual bool hasFactor(const string& name,
                                       const KQuery::KType& ktype) = 0;
  [[nodiscard]] virtual Factor getFactor(const string& name,
                                         const KQuery::KType& ktype) = 0;
  virtual void saveFactor(const Factor& factor, bool updateBefore) = 0;
  virtual void removeFactor(const string& name, const KQuery::KType& ktype) = 0;
  virtual void saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                                       const DatetimeList& dates,
                                       const PriceList& values,
                                       bool replace) = 0;

  [[nodiscard]] virtual FactorSet getFactorSet(const string& name,
                                               const KQuery::KType& ktype) = 0;
  virtual void saveFactorSet(const FactorSet& set) = 0;
  virtual void removeFactorSet(const string& name,
                               const KQuery::KType& ktype) = 0;

  [[nodiscard]] virtual IndicatorList getValues(
      const Factor& factor, const StockList& stocks, const KQuery& query,
      bool align, bool fillNull, bool toValue,
      const DatetimeList& alignDates) = 0;
  [[nodiscard]] virtual vector<IndicatorList> getValues(
      const FactorSet& factorSet, const StockList& stocks, const KQuery& query,
      bool align, bool fillNull, bool toValue,
      const DatetimeList& alignDates) = 0;
};

using FactorStoreResolver = std::function<FactorStore*()>;

HAYAKU_API void setFactorStoreResolver(FactorStoreResolver resolver);
[[nodiscard]] HAYAKU_API FactorStore* getFactorStore() noexcept;
[[nodiscard]] HAYAKU_API bool hasFactorStore() noexcept;

HAYAKU_API bool hasFactor(const string& name,
                          const KQuery::KType& ktype = KQuery::DAY);
HAYAKU_API Factor getFactor(const string& name,
                            const KQuery::KType& ktype = KQuery::DAY);
HAYAKU_API void saveFactor(const Factor& factor, bool updateBefore = true);
HAYAKU_API void saveSpecialFactorValues(const Factor& factor,
                                        const Stock& stock,
                                        const DatetimeList& dates,
                                        const PriceList& values, bool replace);
HAYAKU_API void removeFactor(const string& name,
                             const KQuery::KType& ktype = KQuery::DAY);
HAYAKU_API FactorList getAllFactors();
HAYAKU_API FactorSetList getAllFactorSets();
HAYAKU_API void updateAllFactorsValues(
    const KQuery::KType& ktype = KQuery::DAY);
HAYAKU_API void saveFactorSet(const FactorSet& set);
HAYAKU_API void removeFactorSet(const string& name, const KQuery::KType& ktype);
HAYAKU_API FactorSet getFactorSet(const string& name,
                                  const KQuery::KType& ktype = KQuery::DAY);

[[nodiscard]] bool isValidFactorName(const string& name);
[[nodiscard]] IndicatorList getValues(const Factor& factor,
                                      const StockList& stocks,
                                      const KQuery& query, bool align,
                                      bool fillNull, bool toValue,
                                      const DatetimeList& alignDates);
[[nodiscard]] vector<IndicatorList> getValues(const FactorSet& factorSet,
                                              const StockList& stocks,
                                              const KQuery& query, bool align,
                                              bool fillNull, bool toValue,
                                              const DatetimeList& alignDates);

}  // namespace hayaku
