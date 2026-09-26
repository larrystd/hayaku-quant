#pragma once

/*
 * build_in.h
 *
 *  Created on: 2013-4-19
 *      Author: fasiondog
 */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */

#include "SignalBase.h"

namespace hayaku {

/**
 * A system that always issues a buy signal, it is mainly used for the
 * MultiFactor stock selection matching in PF
 * @return SignalPtr
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_AllwaysBuy();

}  // namespace hayaku

/*
 * SG_Band.h
 *
 *   Created on: 2023-09-23
 *       Author: yangrq1018
 */

#include "operators/Indicator.h"

namespace hayaku {

/**
 * Indicator band indicator: buy when the indicator exceeds the upper band, and
 * sell when the indicator is lower than the lower band.
 * @note It is suitable for the RSI-like indicators with an absolute value range
 * @param ind indicator
 * @param lower lower band
 * @param upper upper band
 * @return SignalPtr
 */
SignalPtr HAYAKU_API SG_Band(const Indicator& ind, price_t lower,
                             price_t upper);

/**
 * Indicator band indicator: buy when the indicator exceeds the upper band
 * indicator, and sell when the indicator is lower than the lower band
 * indicator.
 * @param ind indicator
 * @param lower lower band indicator
 * @param upper upper band indicator
 * @return SignalPtr
 */
SignalPtr HAYAKU_API SG_Band(const Indicator& ind, const Indicator& lower,
                             const Indicator& upper);

} /* namespace hayaku */

/*
 * SG_Bool.h
 *
 *  Created on: 2017-7-2
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Boolean signal generator
 * @param buy the buy indication (a value > 0 at the corresponding position of
 * the result Indicator means a buy)
 * @param sell the sell indication (a value > 0 at the corresponding position of
 * the result Indicator means a sell)
 * @param alternate whether the buy and sell signals appear alternately, true by
 * default
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_Bool(const Indicator& buy, const Indicator& sell,
                             bool alternate = true);

} /* namespace hayaku */

/*
 * CROSS_SG.h
 *
 *  Created on: 2015-2-20
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Two line crossing indicator: it buys when the fast line crosses the slow line
 * upward from below, and sells when the fast line crosses the slow line
 * downward from above.
 * @param fast fast line
 * @param slow slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_Cross(const Indicator& fast, const Indicator& slow);

} /* namespace hayaku */

/*
 * CROSS_SG.h
 *
 *  Created on: 2015-2-20
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Golden cross indicator: it is a golden cross and a buy when the fast line
 * crosses the slow line upward from below and both the fast line and the slow
 * line are directed upward; it is a death cross and a sell when the fast line
 * crosses the slow line downward from above and both the fast line and the slow
 * line are directed downward.
 * @param fast fast line
 * @param slow slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_CrossGold(const Indicator& fast, const Indicator& slow);

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-04-01
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * It uses the PF rebalancing cycle as the buy signal
 * @return SignalPtr
 */
SignalPtr HAYAKU_API SG_Cycle();

}  // namespace hayaku

/*
 * FLEX._SG.h
 *
 *  Created on: 2015-3-21
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Self-crossing single line inflection point indicator.
 * It uses its own EMA(slow_n) as the slow line and itself as the fast line: it
 * buys when the fast line crosses the slow line upward and sells when the fast
 * line crosses the slow line downward.
 * @param op
 * @param slow_n the EMA period of the slow line
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_Flex(const Indicator& op, int slow_n);

} /* namespace hayaku */

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-08
 *      Author: fasiondog
 */

namespace hayaku {

HAYAKU_API SignalPtr operator+(const SignalPtr& sg1, const SignalPtr& sg2);
HAYAKU_API SignalPtr operator-(const SignalPtr& sg1, const SignalPtr& sg2);
HAYAKU_API SignalPtr operator*(const SignalPtr& sg1, const SignalPtr& sg2);
HAYAKU_API SignalPtr operator/(const SignalPtr& sg1, const SignalPtr& sg2);

HAYAKU_API SignalPtr operator+(const SignalPtr& sg, double value);
HAYAKU_API SignalPtr operator-(const SignalPtr& sg, double value);
HAYAKU_API SignalPtr operator*(const SignalPtr& sg, double value);
HAYAKU_API SignalPtr operator/(const SignalPtr& sg, double value);

HAYAKU_API SignalPtr operator+(double value, const SignalPtr& sg);
HAYAKU_API SignalPtr operator-(double value, const SignalPtr& sg);
HAYAKU_API SignalPtr operator*(double value, const SignalPtr& sg);
HAYAKU_API SignalPtr operator/(double value, const SignalPtr& sg);

HAYAKU_API SignalPtr operator&(const SignalPtr& sg1, const SignalPtr& sg2);
HAYAKU_API SignalPtr operator|(const SignalPtr& sg1, const SignalPtr& sg2);

//------------------------------------------------------------------
// Since the alternate of SG is True by default, when using a form such as "sg1
// + sg2 + sg3" the alternate attribute of sg1 + sg2 is easily ignored, so the
// functions such as SG_Add are added It is recommended to use: SG_Add(sg1, sg2,
// False) + sg3 to avoid the alternate problem
//------------------------------------------------------------------
inline SignalPtr SG_Add(const SignalPtr& sg1, const SignalPtr& sg2,
                        bool alternate) {
  auto sg = sg1 + sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_Add(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_Add(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_Add(tmp, sg_list[i], alternate);
  }
  return tmp;
}

inline SignalPtr SG_Sub(const SignalPtr& sg1, const SignalPtr& sg2,
                        bool alternate) {
  auto sg = sg1 - sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_Sub(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_Sub(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_Sub(tmp, sg_list[i], alternate);
  }
  return tmp;
}

inline SignalPtr SG_Mul(const SignalPtr& sg1, const SignalPtr& sg2,
                        bool alternate) {
  auto sg = sg1 * sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_Mul(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_Mul(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_Mul(tmp, sg_list[i], alternate);
  }
  return tmp;
}

inline SignalPtr SG_Div(const SignalPtr& sg1, const SignalPtr& sg2,
                        bool alternate) {
  auto sg = sg1 / sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_Div(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_Div(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_Div(tmp, sg_list[i], alternate);
  }
  return tmp;
}

inline SignalPtr SG_And(const SignalPtr& sg1, const SignalPtr& sg2,
                        bool alternate) {
  auto sg = sg1 & sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_And(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_And(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_And(tmp, sg_list[i], alternate);
  }
  return tmp;
}

inline SignalPtr SG_Or(const SignalPtr& sg1, const SignalPtr& sg2,
                       bool alternate) {
  auto sg = sg1 | sg2;
  sg->setParam<bool>("alternate", alternate);
  return sg;
}

inline SignalPtr SG_Or(const vector<SignalPtr> sg_list, bool alternate) {
  HAYAKU_CHECK(sg_list.size() >= 2, "sg_list is empty!");
  SignalPtr tmp = SG_Or(sg_list[0], sg_list[1], alternate);
  for (size_t i = 2; i < sg_list.size(); ++i) {
    tmp = SG_Or(tmp, sg_list[i], alternate);
  }
  return tmp;
}

}  // namespace hayaku

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240916 added by fasiondog
 */

namespace hayaku {

/**
 * A signal generator whose signals can only be added manually, used for the
 * testing or other special purposes
 * @return SignalPtr
 */
SignalPtr HAYAKU_API SG_Manual();

}  // namespace hayaku

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-02-13
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Build a one-sided signal (containing the buy or the sell signals only) from
 * the input indicator; a signal is added if the indicator value is greater than
 * 0
 * @param ind the indicating indicator
 * @param is_buy whether the added signal is a buy or a sell signal
 * @return signal generator
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_OneSide(const Indicator& ind, bool is_buy);

/** Generate a one-sided buy signal */
inline SignalPtr SG_Buy(const Indicator& ind) { return SG_OneSide(ind, true); }

/** Generate a one-sided sell signal */
inline SignalPtr SG_Sell(const Indicator& ind) {
  return SG_OneSide(ind, false);
}

} /* namespace hayaku */

/*
 * SINGLE_SG.h
 *
 *  Created on: 2015-2-22
 *      Author: fasiondog
 */

namespace hayaku {

/**
 * Single line inflection point signal
 * @details
 * <pre>
 * The curve inflection point algorithm given in "Smarter Trading" is used to
 * judge the curve trend, the formula is as follows:
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - AMA[1] > filter
 *     or Buy When AMA - AMA[2] > filter
 *     or Buy When AMA - AMA[3] > filter
 * </pre>
 * @param ind
 * @param filter_n N-day period
 * @param filter_p filter percentage
 * @return
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_Single(const Indicator& ind, int filter_n = 20,
                               double filter_p = 0.1);

/**
 * Single line inflection point signal 2
 * @details
 * <pre>
 * The curve inflection point algorithm given in "Smarter Trading" is used to
 * judge the curve trend, the formula is as follows:
 *
 *     filter = percentage * STDEV((AMA-AMA[1], N)
 *     Buy  When AMA - lowest(AMA,n) > filter
 *     Sell When highest(AMA, n) - AMA > filter
 * </pre>
 * @param ind
 * @param filter_n N-day period
 * @param filter_p filter percentage
 * @return
 * @ingroup Signal
 */
SignalPtr HAYAKU_API SG_Single2(const Indicator& ind, int filter_n = 20,
                                double filter_p = 0.1);

} /* namespace hayaku */
