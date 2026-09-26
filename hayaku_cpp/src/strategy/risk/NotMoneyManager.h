#pragma once

/*
 * NotMoneyManager.h
 *
 *  Created on: 2017-5-22
 *      Author: Administrator
 */


#include "MoneyManagerBase.h"

namespace hayaku {

/*
 * No money management strategy, i.e. it buys as much as the available money allows
 */
class NotMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(NotMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NotMoneyManager();
    virtual ~NotMoneyManager();
};

} /* namespace hayaku */
