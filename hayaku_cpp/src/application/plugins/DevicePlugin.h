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
void bindEmail(const std::string& email, const std::string& active_code);

/**
 * @brief Activate the device
 * @param active_code license code
 * @param replace when the device limit is exceeded, forcefully replace the
 * earliest activated device
 */
void activeDevice(const std::string& active_code, bool replace = false);

/** View the license information */
std::string viewLicense();

/** Remove the license */
void removeLicense();

/** Get the trial license */
std::string fetchTrialLicense(const std::string& email);

/** Check whether the license is valid */
bool isValidLicense();

/** Get the license expiration time */
Datetime getExpireDate();

}  // namespace hayaku
