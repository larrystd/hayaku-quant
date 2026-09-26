#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-04-12
 *      Author: fasiondog
 */


#include "data/MarketTypes.h"

namespace hayaku {

/**
 * @brief Bind the license code
 * @param email e-mail
 * @param active_code license code
 */
void HAYAKU_API bindEmail(const std::string& email, const std::string& active_code);

/**
 * @brief Activate the device
 * @param active_code license code
 * @param replace when the device limit is exceeded, forcefully replace the earliest activated
 * device
 */
void HAYAKU_API activeDevice(const std::string& active_code, bool replace = false);

/** View the license information */
std::string HAYAKU_API viewLicense();

/** Remove the license */
void HAYAKU_API removeLicense();

/** Get the trial license */
std::string HAYAKU_API fetchTrialLicense(const std::string& email);

/** Check whether the license is valid */
bool HAYAKU_API isValidLicense();

/** Get the license expiration time */
Datetime HAYAKU_API getExpireDate();

}  // namespace hayaku
