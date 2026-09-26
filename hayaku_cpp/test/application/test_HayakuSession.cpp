/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "doctest/doctest.h"
#include <application/HayakuSession.h>
#include <application/SessionOptions.h>
#include <data/DataRuntime.h>
#include <operators/SeriesOperators.h>
#include <operators/WindowOperators.h>
#include <execution/pricing/TradeCosts.h>
#include <strategy/risk/MoneyManagers.h>
#include <strategy/decision/Signals.h>

using namespace hayaku;

/**
 * @defgroup test_hayakusession_suite test_hayakusession_suite
 * @ingroup test_hayaku_application_suite
 * @{
 */

namespace {

SessionOptions makeCurrentOptions() {
    const auto& data = getDataRuntime();
    return SessionOptions(data.getBaseInfoDriverParameter(), data.getBlockDriverParameter(),
                          data.getKDataDriverParameter(), data.getPreloadParameter(),
                          data.getHayakuParameter(), data.getStrategyContext());
}

StrategyDefinition makeSessionDefinition(const string& name = "session-strategy") {
    return StrategyDefinition(MM_FixedCount(100),
                              SG_Cross(MA(CLOSE(), 5), MA(CLOSE(), 10)), name);
}

}  // namespace

TEST_CASE("test_HayakuSession_open_close") {
    auto* dataRuntime = &getDataRuntime();
    auto options = makeCurrentOptions();
    auto session = HayakuSession::open(options);

    /** @arg An opened session exposes a ready DataEngine. */
    CHECK_UNARY(session.isOpen());
    CHECK_UNARY(session.ready());
    CHECK_EQ(session.data().size(), dataRuntime->size());

    /** @arg Closing a session is explicit and idempotent. */
    session.close();
    CHECK_UNARY_FALSE(session.isOpen());
    CHECK_NOTHROW(session.close());

    /** @arg A closed session no longer exposes its DataEngine. */
    CHECK_THROWS(static_cast<void>(session.data()));

    /** @arg A new session can be opened after another handle has closed. */
    auto reopened = HayakuSession::open(options);
    CHECK_EQ(reopened.data().getStock("sh000001"), getDataRuntime().getStock("sh000001"));
}

TEST_CASE("test_HayakuSession_multiple_handles") {
    auto first = HayakuSession::open(makeCurrentOptions());
    auto second = HayakuSession::open(makeCurrentOptions());

    /** @arg Closing one handle does not invalidate another open session. */
    first.close();
    CHECK_UNARY_FALSE(first.isOpen());
    CHECK_UNARY(second.isOpen());
    CHECK_EQ(second.data().getStock("sh000001"), getDataRuntime().getStock("sh000001"));
}

TEST_CASE("test_HayakuSession_native_execution") {
    const AccountId accountId(7001);
    const AccountConfig accountConfig(Datetime(199001010000), 100000.0, TC_Zero(), "session-native",
                                      2, false, false, accountId);
    auto session = HayakuSession::open(makeCurrentOptions(), accountConfig);

    /** @arg AccountConfig is the native Session assembly path and requires no TradeManager. */
    CHECK_UNARY(session.hasExecution());
    CHECK_EQ(session.execution().accountId(), accountId);
    CHECK_EQ(session.execution().view().funds().cash, 100000.0);
    CHECK_EQ(session.execution().history().size(), 1);

    /** @arg A strategy is bound to the session-owned execution account. */
    auto& strategy = session.bindStrategy(makeSessionDefinition());
    CHECK_EQ(&session.strategy(), &strategy);

    auto* execution = &session.execution();
    session.close();
    CHECK_THROWS(static_cast<void>(execution->snapshot()));
}

TEST_CASE("test_HayakuSession_three_engine_lifecycle") {
    const AccountConfig accountConfig(Datetime(199001010000), 100000.0, TC_Zero(),
                                      "session-engine");
    auto session = HayakuSession::open(makeCurrentOptions(), accountConfig);

    /** @arg ExecutionEngine is installed explicitly when the session is opened. */
    CHECK_UNARY(session.hasExecution());

    /** @arg Binding an explicit definition installs the StrategyEngine only. */
    auto definition = makeSessionDefinition();
    auto& strategy = session.bindStrategy(definition);
    auto& execution = session.execution();
    CHECK_UNARY(session.hasStrategy());
    CHECK_UNARY(session.hasExecution());
    CHECK_EQ(&session.strategy(), &strategy);
    CHECK_EQ(&session.execution(), &execution);
    CHECK_EQ(&session.bindStrategy(definition), &strategy);
    CHECK_THROWS(static_cast<void>(
      session.bindStrategy(makeSessionDefinition("different-strategy"))));

    auto* scopedExecution = &session.execution();
    auto* scopedStrategy = &session.strategy();
    session.close();

    /** @arg Closing the session invalidates all three scoped engine handles. */
    CHECK_UNARY_FALSE(session.hasExecution());
    CHECK_UNARY_FALSE(session.hasStrategy());
    CHECK_THROWS(static_cast<void>(scopedExecution->snapshot()));
    CHECK_THROWS(static_cast<void>(scopedStrategy->run(BacktestRequest(KData()))));
}

/** @} */
