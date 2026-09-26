#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <string>
#include <utility>

#include "common/Config.h"
#include "common/time/Datetime.h"

namespace hayaku {

/** Judge whether there is a newer version to upgrade to. */
bool HAYAKU_API CanUpgrade();

struct HAYAKU_API LatestVersionInfo {
  int version{1003001};
  Datetime release_date;
  std::string remark;
  LatestVersionInfo() = default;
  LatestVersionInfo(const LatestVersionInfo&) = default;
  LatestVersionInfo(LatestVersionInfo&& rhs)
      : version(rhs.version),
        release_date(rhs.release_date),
        remark(std::move(rhs.remark)) {}
  LatestVersionInfo& operator=(LatestVersionInfo&& rhs) {
    if (this == &rhs) {
      return *this;
    }
    version = rhs.version;
    release_date = rhs.release_date;
    remark = std::move(rhs.remark);
    return *this;
  }
};

LatestVersionInfo HAYAKU_API getLatestVersionInfo();

void updateSysInfoExpiredTime(Datetime time);

/** License expiration reminder. */
void HAYAKU_API reminderLicenseExpiration();

/** Send the feedback information. */
void HAYAKU_API sendFeedback();

/** Send the Python version information. */
void HAYAKU_API sendPythonVersionFeedBack(int major, int minor, int micro);

}  // namespace hayaku
