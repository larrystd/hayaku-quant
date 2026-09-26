/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include "application/SystemInfo.h"

#include <fmt/format.h>

#include "common/Log.h"
#include "common/Os.h"
#include "version.h"

namespace hayaku {

namespace {

struct InnerSysInfo {
  bool runningInPython{false};
  bool pythonInInteractive{false};
  bool pythonInJupyter{false};
};

InnerSysInfo& sysInfo() {
  // Python sets its runtime flags while importing the extension module, before
  // a Session exists. Keep this small state process-lived so detached
  // user-requested work can never race with destruction.
  static auto* info = new InnerSysInfo;
  return *info;
}

}  // namespace

void sysinfo_init() { (void)sysInfo(); }

void sysinfo_clean() {
  // Process-lived by design; see sysInfo().
}

std::string getVersion() { return HAYAKU_VERSION; }

std::string getVersionWithBuild() {
  return fmt::format("{}_{}_{}_{}_{}", HAYAKU_VERSION, HAYAKU_VERSION_BUILD,
                     HAYAKU_VERSION_MODE, getPlatform(), getCpuArch());
}

std::string getVersionWithGit() { return HAYAKU_VERSION_GIT; }

bool runningInPython() { return sysInfo().runningInPython; }

void setRunningInPython(bool inpython) { sysInfo().runningInPython = inpython; }

bool pythonInInteractive() { return sysInfo().pythonInInteractive; }

void setPythonInInteractive(bool interactive) {
  sysInfo().pythonInInteractive = interactive;
}

bool pythonInJupyter() { return sysInfo().pythonInJupyter; }

void setPythonInJupyter(bool injupyter) {
  sysInfo().pythonInJupyter = injupyter;
  if (createDir(fmt::format("{}/.hayaku", getUserDir()))) {
    initLogger(injupyter, fmt::format("{}/.hayaku/hayaku.log", getUserDir()));
  } else {
    initLogger(injupyter);
  }
}

}  // namespace hayaku
