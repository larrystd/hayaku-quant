#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <string>

#include "common/Config.h"

#ifndef HAYAKU_API
#define HAYAKU_API
#endif

namespace hayaku {

void sysinfo_init();
void sysinfo_clean();

/** Get the current version number of Hayaku. */
std::string HAYAKU_API getVersion();

/** Get the detailed version number, including the build time. */
std::string HAYAKU_API getVersionWithBuild();

/** Get the version number including the Git commit information. */
std::string HAYAKU_API getVersionWithGit();

/** Whether it is currently running in the Python environment. */
bool HAYAKU_API runningInPython();

/** Whether it is currently running in the Jupyter environment. */
bool HAYAKU_API pythonInJupyter();

/** Whether Python is running in interactive mode. */
bool HAYAKU_API pythonInInteractive();

/** Set whether it is running under Python. */
void HAYAKU_API setRunningInPython(bool inpython);

/** Set whether Python runs in interactive mode. */
void HAYAKU_API setPythonInInteractive(bool interactive);

/** Set whether it is running in the Jupyter environment. */
void HAYAKU_API setPythonInJupyter(bool injupyter);

}  // namespace hayaku
