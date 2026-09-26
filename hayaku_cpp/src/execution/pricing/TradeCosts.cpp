#include "TradeCosts.h"

/*
 * FixedATradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include "common/Log.h"
#include "data/StockTypeInfo.h"

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedATradeCost)
#endif

namespace hayaku {

FixedATradeCost::FixedATradeCost() : TradeCostBase("TC_FixedA") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.001);
    setParam<price_t>("lowest_transferfee", 1.0);
}

FixedATradeCost::FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                                 price_t transferfee, price_t lowestTransferfee)
: TradeCostBase("FixedATradeCost") {
    setParam<price_t>("commission", commission);
    setParam<price_t>("lowest_commission", lowestCommission);
    setParam<price_t>("stamptax", stamptax);
    setParam<price_t>("transferfee", transferfee);
    setParam<price_t>("lowest_transferfee", lowestTransferfee);
}

FixedATradeCost::~FixedATradeCost() {}

void FixedATradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HAYAKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("transferfee" == name) {
        HAYAKU_ASSERT(getParam<price_t>("transferfee") >= 0.0);
    } else if ("lowest_transferfee" == name) {
        HAYAKU_ASSERT(getParam<price_t>("lowest_transferfee") >= 0.0);
    }
}

CostRecord FixedATradeCost::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // The Shanghai Stock Exchange charges a transfer fee on a buy
    if (stock.market() == "SH") {
        result.transferfee = num > 1000 ? roundEx(getParam<price_t>("transferfee") * num, precision)
                                        : getParam<price_t>("lowest_transferfee");
    }

    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedATradeCost::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                        double num) const {
    CostRecord result;
    if (stock.isNull()) {
        HAYAKU_WARN("Stock is NULL!");
        return result;
    }

    int precision = stock.precision();
    result.commission = roundEx(price * num * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // The A-shares and the ChiNext have the stamp duty, the others do not
    if (stock.type() == STOCKTYPE_A || stock.type() == STOCKTYPE_GEM) {
        result.stamptax = roundEx(price * num * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = 0.0;
    if (stock.market() == "SH") {
        result.transferfee = num > 1000 ? roundEx(getParam<price_t>("transferfee") * num, precision)
                                        : getParam<price_t>("lowest_transferfee");
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

TradeCostPtr FixedATradeCost::_clone() {
    return make_shared<FixedATradeCost>();
}

TradeCostPtr HAYAKU_API TC_FixedA(price_t commission, price_t lowestCommission, price_t stamptax,
                               price_t transferfee, price_t lowestTransferfee) {
    return make_shared<FixedATradeCost>(commission, lowestCommission, stamptax, transferfee,
                                        lowestTransferfee);
}

} /* namespace hayaku */

/*
 * AShareTradeCost.cpp
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedA2015TradeCost)
#endif

namespace hayaku {

FixedA2015TradeCost::FixedA2015TradeCost() : TradeCostBase("TC_FixedA2015") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2015TradeCost::~FixedA2015TradeCost() {}

void FixedA2015TradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HAYAKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("transferfee" == name) {
        HAYAKU_ASSERT(getParam<price_t>("transferfee") >= 0.0);
    }
}

CostRecord FixedA2015TradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                           price_t price, double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");

    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    if (stock.market() == "SH") {
        result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    }

    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedA2015TradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                            price_t price, double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");

    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // The A-shares and the ChiNext have the stamp duty, the others do not
    if (stock.type() == STOCKTYPE_A || stock.type() == STOCKTYPE_GEM) {
        result.stamptax = roundEx(value * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = 0.0;
    if (stock.market() == "SH") {
        result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

TradeCostPtr FixedA2015TradeCost::_clone() {
    return make_shared<FixedA2015TradeCost>();
}

TradeCostPtr HAYAKU_API TC_FixedA2015(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    TradeCostPtr p = make_shared<FixedA2015TradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return p;
}

} /* namespace hayaku */

/*
 * AShareTradeCost.cpp
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedA2017TradeCost)
#endif

namespace hayaku {

FixedA2017TradeCost::FixedA2017TradeCost() : TradeCostBase("TC_FixedA2017") {
    setParam<price_t>("commission", 0.0018);
    setParam<price_t>("lowest_commission", 5.0);
    setParam<price_t>("stamptax", 0.001);
    setParam<price_t>("transferfee", 0.00002);
}

FixedA2017TradeCost::~FixedA2017TradeCost() {}

void FixedA2017TradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    } else if ("stamptax" == name) {
        HAYAKU_ASSERT(getParam<price_t>("stamptax") >= 0.0);
    } else if ("transferfee" == name) {
        HAYAKU_ASSERT(getParam<price_t>("transferfee") >= 0.0);
    }
}

CostRecord FixedA2017TradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                           price_t price, double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    result.total = result.commission + result.transferfee;

    return result;
}

CostRecord FixedA2017TradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                            price_t price, double num) const {
    CostRecord result;
    if (stock.isNull()) {
        HAYAKU_WARN("Stock is NULL!");
        return result;
    }

    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }

    // The A-shares and the ChiNext have the stamp duty, the others do not
    if (stock.type() == STOCKTYPE_A || stock.type() == STOCKTYPE_GEM) {
        result.stamptax = roundEx(value * getParam<price_t>("stamptax"), precision);
    } else {
        result.stamptax = 0.0;
    }
    result.transferfee = 0.0;
    if (stock.market() == "SH") {
        result.transferfee = roundEx(value * getParam<price_t>("transferfee"), precision);
    }
    result.others = 0.0;
    result.total = result.commission + result.stamptax + result.transferfee;
    return result;
}

TradeCostPtr FixedA2017TradeCost::_clone() {
    return make_shared<FixedA2017TradeCost>();
}

TradeCostPtr HAYAKU_API TC_FixedA2017(price_t commission, price_t lowestCommission, price_t stamptax,
                                   price_t transferfee) {
    TradeCostPtr p = make_shared<FixedA2017TradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    p->setParam<price_t>("stamptax", stamptax);
    p->setParam<price_t>("transferfee", transferfee);
    return p;
}

} /* namespace hayaku */

/*
 * FixedETFTradeCost.cpp
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::FixedETFTradeCost)
#endif

namespace hayaku {

FixedETFTradeCost::FixedETFTradeCost() : TradeCostBase("TC_FixedETF") {
    setParam<price_t>("commission", 0.0001);
    setParam<price_t>("lowest_commission", 5.0);
}

FixedETFTradeCost::~FixedETFTradeCost() {}

void FixedETFTradeCost::_checkParam(const string& name) const {
    if ("commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("commission") >= 0.0);
    } else if ("lowest_commission" == name) {
        HAYAKU_ASSERT(getParam<price_t>("lowest_commission") >= 0.0);
    }
}

CostRecord FixedETFTradeCost::getBuyCost(const Datetime& datetime, const Stock& stock,
                                         price_t price, double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }
    result.transferfee = 0.0;
    result.total = result.commission;
    return result;
}

CostRecord FixedETFTradeCost::getSellCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const {
    CostRecord result;
    HAYAKU_WARN_IF_RETURN(stock.isNull(), result, "Stock is Null!");
    int precision = stock.precision();
    price_t value = price * num;
    result.commission = roundEx(value * getParam<price_t>("commission"), precision);
    price_t lowestCommission = getParam<price_t>("lowest_commission");
    if (result.commission < lowestCommission) {
        result.commission = lowestCommission;
    }
    result.stamptax = 0.0;
    result.transferfee = 0.0;
    result.others = 0.0;
    result.total = result.commission;
    return result;
}

TradeCostPtr FixedETFTradeCost::_clone() {
    return make_shared<FixedETFTradeCost>();
}

TradeCostPtr HAYAKU_API TC_FixedETF(price_t commission, price_t lowestCommission) {
    TradeCostPtr p = make_shared<FixedETFTradeCost>();
    p->setParam<price_t>("commission", commission);
    p->setParam<price_t>("lowest_commission", lowestCommission);
    return p;
}

} /* namespace hayaku */

/*
 * ZeroTradeCost.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::ZeroTradeCost)
#endif

namespace hayaku {

ZeroTradeCost::ZeroTradeCost() : TradeCostBase("TC_Zero") {}

ZeroTradeCost::~ZeroTradeCost() {}

CostRecord ZeroTradeCost ::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num) const {
    return CostRecord();
}

CostRecord ZeroTradeCost ::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                       double num) const {
    return CostRecord();
}

TradeCostPtr ZeroTradeCost::_clone() {
    return make_shared<ZeroTradeCost>();
}

TradeCostPtr HAYAKU_API TC_Zero() {
    return make_shared<ZeroTradeCost>();
}

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-01-24
 *      Author: fasiondog
 */


#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::TradeCostStub)
#endif

namespace hayaku {

TradeCostStub::TradeCostStub() : TradeCostBase("TestStub") {}

TradeCostStub::~TradeCostStub() {}

CostRecord TradeCostStub::getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                     double num) const {
    return CostRecord(0, 0, 0, 10, 10);
}

CostRecord TradeCostStub::getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                      double num) const {
    return CostRecord(0, 0, 0, 20, 20);
}

CostRecord TradeCostStub::getBorrowCashCost(const Datetime& datetime, price_t cash) const {
    return CostRecord(0, 0, 0, 30, 30);
}

CostRecord TradeCostStub::getReturnCashCost(const Datetime& borrow_datetime,
                                            const Datetime& return_datetime, price_t cash) const {
    return CostRecord(0, 0, 0, 40, 40);
}

CostRecord TradeCostStub::getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                             price_t price, double num) const {
    return CostRecord(0, 0, 0, 50, 50);
}

CostRecord TradeCostStub::getReturnStockCost(const Datetime& borrow_datetime,
                                             const Datetime& return_datetime, const Stock& stock,
                                             price_t price, double num) const {
    return CostRecord(0, 0, 0, 60, 60);
}

TradeCostPtr TradeCostStub::_clone() {
    return make_shared<TradeCostStub>();
}

}  // namespace hayaku

/*
 * TC_TestStub.cpp
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */


namespace hayaku {

HAYAKU_API TradeCostPtr TC_TestStub() {
    return make_shared<TradeCostStub>();
}

}  // namespace hayaku
