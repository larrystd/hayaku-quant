#pragma once

/*
 * FixedRiskMM.h
 *
 *  Created on: 2016-5-1
 *      Author: Administrator
 */


#include "MoneyManagerBase.h"

namespace hayaku {

class FixedRiskMoneyManager : public MoneyManagerBase {
    MONEY_MANAGER_IMP(FixedRiskMoneyManager)
    MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedRiskMoneyManager();
    virtual ~FixedRiskMoneyManager();
    virtual void _checkParam(const string& name) const override;
};

} /* namespace hayaku */
