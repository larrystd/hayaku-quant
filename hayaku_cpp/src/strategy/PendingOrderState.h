#pragma once

/*
 * Copyright (c) 2026 hikyuu.org
 *
 * Internal delayed-order state owned by StrategyRuntime.
 */


#include "data/KRecord.h"
#include "execution/TradeRecord.h"

namespace hayaku::internal {

struct PendingOrder {
    void clear() noexcept {
        valid = false;
        business = BUSINESS_INVALID;
        datetime = Datetime();
        stoploss = 0.0;
        goal = 0.0;
        number = 0.0;
        origin = OrderOrigin::UNSPECIFIED;
        remark.clear();
        count = 0;
        krecord = KRecord();
    }

    bool valid{false};
    BUSINESS business{BUSINESS_INVALID};
    Datetime datetime;
    price_t stoploss{0.0};
    price_t goal{0.0};
    double number{0.0};
    OrderOrigin origin{OrderOrigin::UNSPECIFIED};
    string remark;
    int count{0};
    KRecord krecord;
};

class PendingOrderState {
public:
    [[nodiscard]] PendingOrder& buy() noexcept {
        return m_buy;
    }

    [[nodiscard]] const PendingOrder& buy() const noexcept {
        return m_buy;
    }

    [[nodiscard]] PendingOrder& sell() noexcept {
        return m_sell;
    }

    [[nodiscard]] const PendingOrder& sell() const noexcept {
        return m_sell;
    }

    [[nodiscard]] PendingOrder& sellShort() noexcept {
        return m_sellShort;
    }

    [[nodiscard]] const PendingOrder& sellShort() const noexcept {
        return m_sellShort;
    }

    [[nodiscard]] PendingOrder& buyShort() noexcept {
        return m_buyShort;
    }

    [[nodiscard]] const PendingOrder& buyShort() const noexcept {
        return m_buyShort;
    }

    void clear() noexcept {
        m_buy.clear();
        m_sell.clear();
        m_sellShort.clear();
        m_buyShort.clear();
    }

private:
    PendingOrder m_buy;
    PendingOrder m_sell;
    PendingOrder m_sellShort;
    PendingOrder m_buyShort;
};

}  // namespace hayaku::internal
