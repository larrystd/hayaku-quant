/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <boost/algorithm/string.hpp>

#include "OrderOrigin.h"

namespace hku {

string getOrderOriginName(OrderOrigin origin) {
    switch (origin) {
        case OrderOrigin::ENVIRONMENT:
            return "EV";
        case OrderOrigin::CONDITION:
            return "CN";
        case OrderOrigin::SIGNAL:
            return "SG";
        case OrderOrigin::STOP_LOSS:
            return "ST";
        case OrderOrigin::TAKE_PROFIT:
            return "TP";
        case OrderOrigin::MONEY_MANAGEMENT:
            return "MM";
        case OrderOrigin::PROFIT_GOAL:
            return "PG";
        case OrderOrigin::SLIPPAGE:
            return "SP";
        case OrderOrigin::ALLOCATION:
            return "AF";
        case OrderOrigin::PORTFOLIO:
            return "PF";
        default:
            return "--";
    }
}

OrderOrigin getOrderOriginEnum(const string& value) {
    string name(value);
    to_upper(name);
    HKU_IF_RETURN(name == "EV", OrderOrigin::ENVIRONMENT);
    HKU_IF_RETURN(name == "CN", OrderOrigin::CONDITION);
    HKU_IF_RETURN(name == "SG", OrderOrigin::SIGNAL);
    HKU_IF_RETURN(name == "ST", OrderOrigin::STOP_LOSS);
    HKU_IF_RETURN(name == "TP", OrderOrigin::TAKE_PROFIT);
    HKU_IF_RETURN(name == "MM", OrderOrigin::MONEY_MANAGEMENT);
    HKU_IF_RETURN(name == "PG", OrderOrigin::PROFIT_GOAL);
    HKU_IF_RETURN(name == "SP", OrderOrigin::SLIPPAGE);
    HKU_IF_RETURN(name == "AF", OrderOrigin::ALLOCATION);
    HKU_IF_RETURN(name == "PF", OrderOrigin::PORTFOLIO);
    return OrderOrigin::UNSPECIFIED;
}

}  // namespace hku
