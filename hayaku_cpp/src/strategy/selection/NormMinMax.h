#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-03
 *      Author: fasiondog
 */


#include "NormalizeBase.h"

namespace hayaku {

class NormMinMax : public NormalizeBase {
    NORMALIZE_IMP(NormMinMax)
    NORMALIZE_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    NormMinMax();
    virtual ~NormMinMax() override;
};

}  // namespace hayaku
