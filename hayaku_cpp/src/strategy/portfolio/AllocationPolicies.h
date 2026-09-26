#pragma once

/*
 * build_in.h
 *
 *  Created on: 2018-2-1
 *      Author: fasiondog
 */




/*
 * AF_EqualWeight.h
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */


#include "AllocateFundsBase.h"

namespace hayaku {

/**
 * @brief Equal weight asset allocation, it allocates the selected assets in equal proportions
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HAYAKU_API AF_EqualWeight();

} /* namespace hayaku */


/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2025-12-9
 *      Author: stone
 */



namespace hayaku {

/**
 * @brief Fixed amount allocation, the trade of every selected asset cannot exceed this amount
 * @param amount the given fixed trade amount
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HAYAKU_API AF_FixedAmount(double amount = 20000.0);

} /* namespace hayaku */


/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * @brief Fixed proportion asset allocation, every selected asset accounts for a fixed proportion of
 * the total assets only
 * @param weight the given asset proportion (0, 1]
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HAYAKU_API AF_FixedWeight(double weight = 0.1);

} /* namespace hayaku */


/*
 * Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2018-2-8
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * @brief Fixed proportion asset allocation
 * @param weights the given asset proportion list
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HAYAKU_API AF_FixedWeightList(const vector<double>& weights);

} /* namespace hayaku */

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-30
 *      Author: fasiondog
 */



namespace hayaku {

/**
 * Create an asset allocation algorithm instance with the MultiFactor score weights, i.e. the scores
 * returned by SE are used as the weights directly.
 * @return AFPtr
 * @ingroup AllocateFunds
 */
AFPtr HAYAKU_API AF_MultiFactor();

}  // namespace hayaku
