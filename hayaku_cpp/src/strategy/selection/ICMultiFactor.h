#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */


#include "MultiFactorBase.h"

namespace hayaku {

class ICMultiFactor : public MultiFactorBase {
    MULTIFACTOR_IMP(ICMultiFactor)
    MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    ICMultiFactor();
    ICMultiFactor(const StockList& stks, const KQuery& query, const Stock& ref_stk, int ic_n,
                  int ic_rolling_n, bool spearman, int mode, bool save_all_factors);
    virtual ~ICMultiFactor() override = default;

    virtual void _checkParam(const string& name) const override;
};

}  // namespace hayaku
