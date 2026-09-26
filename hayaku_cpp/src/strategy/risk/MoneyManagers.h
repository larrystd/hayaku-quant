#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */




/*
 * MM_FixedCapital.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */


#include "MoneyManagerBase.h"

namespace hayaku {

/**
 * Fixed capital money management strategy
 * Formula: buy quantity = current cash / capital
 * @param capital
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_FixedCapital(double capital = 10000.00);

} /* namespace hayaku */


/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-19
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Fixed capital funds management strategy
 * Formula: buy quantity = current total assets / capital
 * @param capital
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_FixedCapitalFunds(double capital = 10000.00);

} /* namespace hayaku */


/*
 * MM_FixedCount.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Fixed trade quantity money management strategy
 * @details A fixed quantity is bought every time.
 * @param n the quantity bought every time (it should be an integer multiple of the minimum trade
 *          quantity of the trading object, the program does not check this here)
 * @note 1) This strategy is mainly used to test and compare the results with the other strategies,
 * it does not conform to the reality itself. \n 2) This strategy does not judge the existing
 * positions; if a trade cannot be made with the existing positions, that judgment should be the
 * responsibility of the System itself
 * @ingroup MoneyManager
 */
MoneyManagerPtr HAYAKU_API MM_FixedCount(double n = 100);

}  // namespace hayaku


/*
 * MM_FixedCapital.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */



namespace hayaku {

/**
 * @brief Money management strategy of buying / selling a fixed quantity consecutively.
 * @param buy_counts the buy quantities in turn
 * @param sell_counts the sell quantities in turn
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_FixedCountTps(const vector<double>& buy_counts,
                                         const vector<double>& sell_counts);

} /* namespace hayaku */


/*
 * MM_FixedPercent.h
 *
 *  Created on: 2015-4-5
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Percentage risk model
 * @details See "Financial Freedom Through Electronic Day Trading" (June 2008, China Machine Press)
 *          by Van K. Tharp, P312
 *          Formula: P (position size) = C (total risk) / R (risk per share) [here C, the cash, is
 * the total risk]
 * @param p the percentage of the total risk of every trade in the total assets, e.g. 0.02 means 2%
 *          of the total assets
 * @ingroup MoneyManager
 */
MoneyManagerPtr HAYAKU_API MM_FixedPercent(double p);

}  // namespace hayaku


/*
 * MM_FixedRisk.h
 *
 *  Created on: 2016-5-1
 *      Author: Administrator
 */



namespace hayaku {

/**
 * The fixed risk money management strategy limits a predetermined or fixed fund risk for every
 * trade, such as a fixed risk of 1000 yuan for every trade.
 * Formula: trade quantity = fixed risk / trade risk.
 * @param risk
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_FixedRisk(double risk = 1000.00);

} /* namespace hayaku */


/*
 * MM_FixedUnits.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */



namespace hayaku {

/**
 * Fixed units money management strategy
 * Formula: buy quantity = current cash / n / current risk
 * @param n
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_FixedUnits(int n = 33);

}  // namespace hayaku


/*
 * MM_FixedCount.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * No money management is done, it buys as much as the available money allows
 * @ingroup MoneyManager
 */
MoneyManagerPtr HAYAKU_API MM_Nothing();

}  // namespace hayaku


/*
 * MM_WilliamsFixedRisk.h
 *
 *  Created on: 2016-5-3
 *      Author: Administrator
 */



namespace hayaku {

/**
 * @brief Williams fixed risk money management strategy
 * Buy quantity = (account balance × risk percentage p) ÷ maximum loss (max_loss)
 * @param p risk percentage
 * @param max_loss maximum loss
 * @return MoneyManagerPtr
 */
MoneyManagerPtr HAYAKU_API MM_WilliamsFixedRisk(double p = 0.1, price_t max_loss = 1000.0);

}  // namespace hayaku
