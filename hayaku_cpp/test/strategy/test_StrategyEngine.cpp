/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-09-25
 *      Author: hayaku
 */

#include "data/DataRuntime.h"
#include "doctest/doctest.h"
#include "execution/AccountConfig.h"
#include "execution/ExecutionEngine.h"
#include "execution/pricing/TradeCosts.h"
#include "operators/SeriesOperators.h"
#include "operators/WindowOperators.h"
#include "strategy/PendingOrderState.h"
#include "strategy/StrategyEngine.h"
#include "strategy/StrategyRuntime.h"
#include "strategy/decision/Signals.h"
#include "strategy/risk/MoneyManagers.h"

using namespace hayaku;

/**
 * @defgroup test_strategy_engine_suite test_strategy_engine_suite
 * @ingroup test_hayaku_trade_sys_suite
 * @{
 */

namespace {

StrategyDefinition makeTestDefinition() {
  auto sg = SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10));
  Parameter parameters;
  parameters.set<bool>("buy_delay", false);
  parameters.set<bool>("sell_delay", false);
  return StrategyDefinition(MM_FixedCount(100), sg, "StrategyEngineTest",
                            EnvironmentPtr(), ConditionPtr(), StoplossPtr(),
                            StoplossPtr(), ProfitGoalPtr(), SlippagePtr(),
                            std::move(parameters));
}

}  // namespace

TEST_CASE("test_BacktestRequest") {
  auto stock = getDataRuntime()["sh600000"];
  auto kdata = stock.getKData(KQueryByIndex(-20));

  BacktestRequest config(kdata, false, true);

  /** @arg Keep a lightweight KData view and the explicit reset options. */
  CHECK_EQ(config.kdata(), kdata);
  CHECK_UNARY_FALSE(config.reset());
  CHECK_UNARY(config.resetAll());
}

TEST_CASE("test_StrategyDefinition") {
  auto definition = makeTestDefinition();

  /** @arg The definition owns only strategy components and no account/runtime
   * object. */
  CHECK_EQ(definition.name(), "StrategyEngineTest");
  CHECK_UNARY(definition.moneyManager());
  CHECK_UNARY(definition.signal());
  CHECK_UNARY_FALSE(definition.environment());
  CHECK_EQ(definition.parameters().get<bool>("buy_delay"), false);

  CHECK_THROWS(StrategyDefinition(MoneyManagerPtr(),
                                  SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10))));
  CHECK_THROWS(StrategyDefinition(MM_FixedCount(100), SignalPtr()));
}

TEST_CASE("test_PendingOrderState") {
  internal::PendingOrderState pending;
  pending.buy().valid = true;
  pending.sell().valid = true;
  pending.sellShort().valid = true;
  pending.buyShort().valid = true;

  CHECK_UNARY(pending.buy().valid);
  CHECK_UNARY(pending.sell().valid);
  CHECK_UNARY(pending.sellShort().valid);
  CHECK_UNARY(pending.buyShort().valid);

  pending.clear();
  CHECK_UNARY_FALSE(pending.buy().valid);
  CHECK_UNARY_FALSE(pending.sell().valid);
  CHECK_UNARY_FALSE(pending.sellShort().valid);
  CHECK_UNARY_FALSE(pending.buyShort().valid);
}

TEST_CASE("test_StrategyEngine_constructor") {
  /** @arg An explicit component definition starts in the idle state. */
  ExecutionEngine execution(AccountConfig(Datetime(199001010000LL), 100000.0,
                                          TC_Zero(), "StrategyEngineTest"));
  StrategyEngine engine(makeTestDefinition(), execution);
  CHECK_UNARY_FALSE(engine.running());
  engine.stop();
  CHECK_UNARY_FALSE(engine.running());
}

TEST_CASE("test_StrategyEngine_run") {
  Stock stock = getDataRuntime()["sh600000"];
  KQuery query =
      KQueryByDate(Datetime(199911100000LL), Datetime(200002250000LL));
  KData kdata = stock.getKData(query);

  auto definition = makeTestDefinition();
  ExecutionEngine execution(AccountConfig(Datetime(199001010000LL), 100000.0,
                                          TC_Zero(), "StrategyEngineTest"));
  StrategyEngine engine(definition, execution);
  BacktestResult result = engine.run(BacktestRequest(kdata));

  /** @arg The result is the immutable snapshot of the bound execution account's
   * strategy trades. */
  CHECK_EQ(result.trades(), execution.history());
  CHECK_EQ(result.tradeCount(), execution.history().size());
  CHECK_EQ(result.empty(), execution.history().empty());
  CHECK_EQ(result.stock(), stock);
  CHECK_EQ(result.query(), query);
  CHECK_UNARY_FALSE(engine.running());

  /** @arg A completed result remains stable after a later run mutates account
   * state. */
  const auto tradeCount = result.tradeCount();
  static_cast<void>(engine.run(BacktestRequest(kdata)));
  CHECK_EQ(result.tradeCount(), tradeCount);
}

/** @} */
