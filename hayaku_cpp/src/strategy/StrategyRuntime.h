#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * System-independent per-strategy state machine.
 */

#include <atomic>
#include <nlohmann/json.hpp>

#include "PendingOrderState.h"
#include "StrategyExecutionPort.h"
#include "execution/ExecutionAccountPort.h"
#include "execution/PortfolioAccountPort.h"
#include "strategy/BacktestRequest.h"
#include "strategy/StrategyDefinition.h"

namespace hayaku::internal {

class ComponentContext;

/** Mutable state for one running StrategyDefinition. */
class StrategyRuntime {
 public:
  StrategyRuntime(const StrategyDefinition& definition,
                  ExecutionAccountPortPtr account);

  void prepare();
  void bind(const KData& kdata);
  void run(const BacktestRequest& request);
  void run(const KQuery& query, bool reset = true, bool resetAll = false);
  void run(const Stock& stock, const KQuery& query, bool reset = true,
           bool resetAll = false);
  [[nodiscard]] TradeRecord runMoment(const Datetime& datetime);
  [[nodiscard]] TradeRecord runMomentOnOpen(const Datetime& datetime);
  [[nodiscard]] TradeRecord runMomentOnClose(const Datetime& datetime);
  [[nodiscard]] TradeRecord sellForceOnOpen(const Datetime& datetime,
                                            double number, OrderOrigin origin);
  [[nodiscard]] TradeRecord sellForceOnClose(const Datetime& datetime,
                                             double number, OrderOrigin origin);
  void clearPendingBuy();
  [[nodiscard]] TradeRecord processPendingBuy(const Datetime& datetime);
  [[nodiscard]] TradeRecord processPendingSell(const Datetime& datetime);
  [[nodiscard]] const PendingOrderState& pendingOrders() const noexcept;
  [[nodiscard]] AccountId accountId() const noexcept;
  [[nodiscard]] const string& name() const noexcept;
  void name(string value);
  [[nodiscard]] Stock getStock() const;
  void setStock(const Stock& stock);
  [[nodiscard]] KData getTO() const;
  [[nodiscard]] MoneyManagerPtr getMM() const;
  [[nodiscard]] SignalPtr getSG() const;
  [[nodiscard]] SlippagePtr getSP() const;
  void setSP(SlippagePtr slippage);
  [[nodiscard]] PortfolioAccountPortPtr getAccount() const;
  void setAccount(PortfolioAccountPortPtr account);
  [[nodiscard]] std::shared_ptr<StrategyRuntime> clone() const;
  void reset();
  void forceResetAll();
  [[nodiscard]] nlohmann::json lastSuggestion() const;

  template <typename T>
  [[nodiscard]] T getParam(const string& name) const {
    return m_parameters.get<T>(name);
  }

  template <typename T>
  void setParam(const string& name, const T& value) {
    m_parameters.set<T>(name, value);
    m_calculated = false;
  }

  [[nodiscard]] const TradeRecordList& trades() const noexcept;
  void setStopToken(const std::atomic_bool* stopToken) noexcept;

 private:
  friend class ComponentContext;

  void initParameters(const Parameter& overrides);
  void resetState(bool all);
  [[nodiscard]] bool stopRequested() const noexcept;
  [[nodiscard]] bool environmentIsValid(const Datetime& datetime);
  [[nodiscard]] bool conditionIsValid(const Datetime& datetime);
  void buyNotifyAll(const TradeRecord& record);
  void sellNotifyAll(const TradeRecord& record);
  [[nodiscard]] double getBuyNumber(const Datetime&, price_t, price_t,
                                    OrderOrigin);
  [[nodiscard]] double getSellNumber(const Datetime&, price_t, price_t,
                                     OrderOrigin);
  [[nodiscard]] double getSellShortNumber(const Datetime&, price_t, price_t,
                                          OrderOrigin);
  [[nodiscard]] double getBuyShortNumber(const Datetime&, price_t, price_t,
                                         OrderOrigin);
  [[nodiscard]] price_t getStoplossPrice(const KRecord&, const KRecord&,
                                         price_t);
  [[nodiscard]] price_t getShortStoplossPrice(const KRecord&, const KRecord&,
                                              price_t);
  [[nodiscard]] price_t getTakeProfitPrice(const Datetime&, price_t);
  [[nodiscard]] price_t getGoalPrice(const Datetime&, price_t);
  [[nodiscard]] price_t getShortGoalPrice(const Datetime&, price_t);
  [[nodiscard]] price_t getRealBuyPrice(const Datetime&, price_t);
  [[nodiscard]] price_t getRealSellPrice(const Datetime&, price_t);

  [[nodiscard]] TradeRecord runMomentNative(const KRecord&, const KRecord&);
  [[nodiscard]] TradeRecord runMomentOnOpenNative(const KRecord&,
                                                  const KRecord&);
  [[nodiscard]] TradeRecord runMomentOnCloseNative(const KRecord&,
                                                   const KRecord&);
  [[nodiscard]] TradeRecord buy(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord buyNow(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord buyDelay(const KRecord&, const KRecord&);
  void submitBuyRequest(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord sell(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord sellNow(const KRecord&, const KRecord&,
                                    OrderOrigin);
  [[nodiscard]] TradeRecord sellDelay(const KRecord&, const KRecord&);
  void submitSellRequest(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord sellShort(const KRecord&, const KRecord&,
                                      OrderOrigin);
  [[nodiscard]] TradeRecord sellShortNow(const KRecord&, const KRecord&,
                                         OrderOrigin);
  [[nodiscard]] TradeRecord sellShortDelay(const KRecord&, const KRecord&);
  void submitSellShortRequest(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord buyShort(const KRecord&, const KRecord&,
                                     OrderOrigin);
  [[nodiscard]] TradeRecord buyShortNow(const KRecord&, const KRecord&,
                                        OrderOrigin);
  [[nodiscard]] TradeRecord buyShortDelay(const KRecord&, const KRecord&);
  void submitBuyShortRequest(const KRecord&, const KRecord&, OrderOrigin);
  [[nodiscard]] TradeRecord processRequest(const KRecord&, const KRecord&);
  [[nodiscard]] TradeRecord sellForce(const Datetime&, double, OrderOrigin,
                                      bool onOpen);

 private:
  ExecutionAccountPortPtr m_account;
  StrategyExecutionPort m_execution;
  MoneyManagerPtr m_mm;
  EnvironmentPtr m_ev;
  ConditionPtr m_cn;
  SignalPtr m_sg;
  StoplossPtr m_st;
  StoplossPtr m_tp;
  ProfitGoalPtr m_pg;
  SlippagePtr m_sp;
  string m_name;
  Parameter m_parameters;
  Stock m_stock;
  KData m_kdata;
  KData m_rawKData;
  bool m_calculated{false};
  bool m_preEnvironmentValid{true};
  bool m_preConditionValid{true};
  int m_buyDays{0};
  int m_sellShortDays{0};
  TradeRecordList m_trades;
  price_t m_lastTakeProfit{0.0};
  price_t m_lastShortTakeProfit{0.0};
  PendingOrderState m_pendingOrders;
  const std::atomic_bool* m_stopToken{nullptr};
};

}  // namespace hayaku::internal

namespace hayaku::internal {
using StrategyRuntimePtr = std::shared_ptr<StrategyRuntime>;
using StrategyRuntimeList = std::vector<StrategyRuntimePtr>;
}  // namespace hayaku::internal
