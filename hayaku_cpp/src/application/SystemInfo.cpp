/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include "version.h"
#include "common/Os.h"
#include "application/SystemInfo.h"

namespace hayaku {

namespace {

struct InnerSysInfo {
    bool runningInPython{false};
    bool pythonInInteractive{false};
    bool pythonInJupyter{false};
};

InnerSysInfo& sysInfo() {
    // Python sets its runtime flags while importing the extension module, before a Session exists.
    // Keep this small state process-lived so detached user-requested work can never race with
    // destruction.
    static auto* info = new InnerSysInfo;
    return *info;
}

}  // namespace

void sysinfo_init() {
    (void)sysInfo();
}

void sysinfo_clean() {
    // Process-lived by design; see sysInfo().
}

std::string getVersion() {
    return HAYAKU_VERSION;
}

std::string getVersionWithBuild() {
    return fmt::format("{}_{}_{}_{}_{}", HAYAKU_VERSION, HAYAKU_VERSION_BUILD, HAYAKU_VERSION_MODE,
                       getPlatform(), getCpuArch());
}

std::string getVersionWithGit() {
    return HAYAKU_VERSION_GIT;
}

bool HAYAKU_API runningInPython() {
    return sysInfo().runningInPython;
}

void HAYAKU_API setRunningInPython(bool inpython) {
    sysInfo().runningInPython = inpython;
}

bool HAYAKU_API pythonInInteractive() {
    return sysInfo().pythonInInteractive;
}

void HAYAKU_API setPythonInInteractive(bool interactive) {
    sysInfo().pythonInInteractive = interactive;
}

bool HAYAKU_API pythonInJupyter() {
    return sysInfo().pythonInJupyter;
}

void HAYAKU_API setPythonInJupyter(bool injupyter) {
    sysInfo().pythonInJupyter = injupyter;
    if (createDir(fmt::format("{}/.hayaku", getUserDir()))) {
        initLogger(injupyter, fmt::format("{}/.hayaku/hayaku.log", getUserDir()));
    } else {
        initLogger(injupyter);
    }
}

}  // namespace hayaku
