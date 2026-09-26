#pragma once

/*
 * FixedATradeCost.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */


#include "TradeCostBase.h"

namespace hayaku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share; it calculates the cost of every buy
 * or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: both the commission and the transfer fee have a minimum value; the current commission
 *   ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is 1 per mille
 *         The transfer fee of the Shanghai Stock Exchange is 1 per mille of the traded quantity,
 * and it is counted as one yuan when it is less than 1 yuan
 * </pre>
 */
class HAYAKU_API FixedATradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    /**
     * Default constructor, it also sets the default parameter values
     * @details
     * <pre>
     * Commission ratio, 1.8 per mille by default, i.e. 0.0018
     * Minimum commission value, 5 yuan by default
     * Stamp duty, 1 per mille by default, i.e. 0.001
     * Transfer fee, 1 per mille per share by default, i.e. 0.001
     * Minimum transfer fee, 1 yuan by default
     * </pre>
     */
    FixedATradeCost();

    /**
     * @param commission commission ratio
     * @param lowestCommission minimum commission value
     * @param stamptax stamp duty
     * @param transferfee transfer fee
     * @param lowestTransferfee minimum transfer fee
     */
    FixedATradeCost(price_t commission, price_t lowestCommission, price_t stamptax,
                    price_t transferfee, price_t lowestTransferfee);
    virtual ~FixedATradeCost();

    virtual void _checkParam(const string& name) const override;

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */

/*
 * AShareTradeCost.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */



namespace hayaku {

class FixedA2015TradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedA2015TradeCost();
    virtual ~FixedA2015TradeCost();

    virtual void _checkParam(const string& name) const override;

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */


/*
 * AShareTradeCost.h
 *
 *  Created on: 2018-4-11
 *      Author: Administrator
 */



namespace hayaku {

/*
 * From January 1, 2017 the transfer fee item of the Shenzhen market is listed separately, with the
 * standard of 0.02‰ of the turnover amount charged in both directions.
 */
class FixedA2017TradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedA2017TradeCost();
    virtual ~FixedA2017TradeCost();

    virtual void _checkParam(const string& name) const override;

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */


/*
 * FixedETFTradeCost.h
 */



namespace hayaku {

class FixedETFTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedETFTradeCost();
    virtual ~FixedETFTradeCost();

    virtual void _checkParam(const string& name) const override;

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */


/*
 * ZeroTradeCost.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API ZeroTradeCost : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ZeroTradeCost();
    virtual ~ZeroTradeCost();

    /**
     * Calculate the buy cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price buy price
     * @param num buy quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    /**
     * Calculate the sell cost
     * @param datetime trade date
     * @param stock the traded security object
     * @param price sell price
     * @param num sell quantity
     * @return CostRecord the trade cost record
     */
    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */

/*
 * TradeCostStub.h
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */



namespace hayaku {

class HAYAKU_API TradeCostStub : public TradeCostBase {
    TRADE_COST_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    TradeCostStub();

    virtual ~TradeCostStub();

    virtual CostRecord getBuyCost(const Datetime& datetime, const Stock& stock, price_t price,
                                  double num) const override;

    virtual CostRecord getSellCost(const Datetime& datetime, const Stock& stock, price_t price,
                                   double num) const override;

    virtual CostRecord getBorrowCashCost(const Datetime& datetime, price_t cash) const override;

    virtual CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                                         const Datetime& return_datetime,
                                         price_t cash) const override;

    virtual CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                          price_t price, double num) const override;

    virtual CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                          const Datetime& return_datetime, const Stock& stock,
                                          price_t price, double num) const override;

    /** Clone interface of the private variables of the subclass */
    virtual TradeCostPtr _clone() override;
};

} /* namespace hayaku */

/*
 * TC_FixedA.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share; it calculates the cost of every buy
 * or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: both the commission and the transfer fee have a minimum value; the current commission
 *   ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is 1 per mille
 *         The transfer fee of the Shanghai Stock Exchange is 1 per mille of the traded quantity,
 * and it is counted as one yuan when it is less than 1 yuan
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 1 per mille by default, i.e. 0.001
 * @param transferfee transfer fee, 1 per mille per share by default, i.e. 0.001
 * @param lowestTransferfee minimum transfer fee, 1 yuan by default
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HAYAKU_API TC_FixedA(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                               price_t stamptax = 0.001, price_t transferfee = 0.001,
                               price_t lowestTransferfee = 1.0);

}  // namespace hayaku


/*
 * TC_FixedA2015.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share after August 1, 2015; it calculates
 * the cost of every buy or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: the current commission ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is
 *   1 per mille
 *        After 2015 the transfer fee of the Shanghai Stock Exchange is 0.00002 of the turnover
 * amount
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 1 per mille by default, i.e. 0.001
 * @param transferfee transfer fee, 0.2 per mille by default, i.e. 0.00002
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HAYAKU_API TC_FixedA2015(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                                   price_t stamptax = 0.001, price_t transferfee = 0.00002);

}  // namespace hayaku


/*
 * TC_FixedA2015.h
 *
 *  Created on: 2016-5-4
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Trade cost algorithm for the Shanghai and Shenzhen A-share after August 1, 2015; it calculates
 * the cost of every buy or sell
 * The Shenzhen market also started to charge the transfer fee after January 1, 2017
 * @details
 * <pre>
 * The calculation rules are:
 *   1) Shanghai Stock Exchange
 *      Buy: commission + transfer fee
 *      Sell: commission + transfer fee + stamp duty
 *   2) Shenzhen Stock Exchange:
 *      Buy: commission
 *      Sell: commission + stamp duty
 *   Where: the current commission ratio is 1.8 per mille (5 yuan minimum), and the stamp duty is
 *   1 per mille
 *        After 2015 the transfer fee of the Shanghai Stock Exchange is 0.00002 of the turnover
 * amount
 * </pre>
 *
 * @param commission commission ratio, 1.8 per mille by default, i.e. 0.0018
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @param stamptax stamp duty, 1 per mille by default, i.e. 0.001
 * @param transferfee transfer fee, 0.2 per mille by default, i.e. 0.00002
 * @see FixedATradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HAYAKU_API TC_FixedA2017(price_t commission = 0.0018, price_t lowestCommission = 5.0,
                                   price_t stamptax = 0.001, price_t transferfee = 0.00002);

}  // namespace hayaku


/*
 * TC_FixedETF.h
 */



namespace hayaku {

/**
 * Trade cost algorithm for the ETF; it calculates the cost of every buy or sell
 * @details
 * <pre>
 * The calculation rules are:
 *   Buy: commission (5 yuan minimum)
 *   Sell: commission (5 yuan minimum)
 *   Where: the commission ratio is 0.1 per ten thousand (0.0001) by default, and the minimum
 *   commission is 5 yuan
 * </pre>
 *
 * @param commission commission ratio, 0.1 per ten thousand by default, i.e. 0.0001
 * @param lowestCommission minimum commission value, 5 yuan by default
 * @see FixedETFTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HAYAKU_API TC_FixedETF(price_t commission = 0.0001, price_t lowestCommission = 5.0);

}  // namespace hayaku


/*
 * TC_Zero.h
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Create a zero cost algorithm instance
 * @see ZeroTradeCost
 * @ingroup TradeCost
 */
TradeCostPtr HAYAKU_API TC_Zero();

}  // namespace hayaku


/*
 * TC_TestStub.h
 *
 *  Created on: 2013-5-9
 *      Author: fasiondog
 */



namespace hayaku {

/* For testing only */
HAYAKU_API TradeCostPtr TC_TestStub();

}  // namespace hayaku
