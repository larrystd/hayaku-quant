#pragma once

/*
 * FixedSelector.h
 *
 *  Created on: 2018-1-12
 *      Author: fasiondog
 */


#include "SelectorBase.h"

namespace hayaku {

class FixedSelector : public SelectorBase {
    SELECTOR_IMP(FixedSelector)
    SELECTOR_NO_PRIVATE_MEMBER_SERIALIZATION

public:
    FixedSelector();
    explicit FixedSelector(double weight);
    virtual ~FixedSelector();
};

} /* namespace hayaku */
