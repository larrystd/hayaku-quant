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

void setFactorStoreResolver(FactorStoreResolver resolver);
[[nodiscard]] FactorStore* getFactorStore() noexcept;
[[nodiscard]] bool hasFactorStore() noexcept;

bool hasFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);
Factor getFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);
void saveFactor(const Factor& factor, bool updateBefore = true);
void saveSpecialFactorValues(const Factor& factor, const Stock& stock,
                             const DatetimeList& dates, const PriceList& values,
                             bool replace);
void removeFactor(const string& name, const KQuery::KType& ktype = KQuery::DAY);
FactorList getAllFactors();
FactorSetList getAllFactorSets();
void updateAllFactorsValues(const KQuery::KType& ktype = KQuery::DAY);
void saveFactorSet(const FactorSet& set);
void removeFactorSet(const string& name, const KQuery::KType& ktype);
FactorSet getFactorSet(const string& name,
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
