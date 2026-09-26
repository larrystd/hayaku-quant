#pragma once

/*
 *  Copyright (c) 2023 hikyuu.org
 *
 *  Created on: 2023-09-26
 *      Author: fasiondog
 */

#include <string>

#include "common/Config.h"

namespace hayaku {

void sysinfo_init();
void sysinfo_clean();

/** Get the current version number of Hayaku. */
std::string getVersion();

/** Get the detailed version number, including the build time. */
std::string getVersionWithBuild();

/** Get the version number including the Git commit information. */
std::string getVersionWithGit();

/** Whether it is currently running in the Python environment. */
bool runningInPython();

/** Whether it is currently running in the Jupyter environment. */
bool pythonInJupyter();

/** Whether Python is running in interactive mode. */
bool pythonInInteractive();

/** Set whether it is running under Python. */
void setRunningInPython(bool inpython);

/** Set whether Python runs in interactive mode. */
void setPythonInInteractive(bool interactive);

/** Set whether it is running in the Jupyter environment. */
void setPythonInJupyter(bool injupyter);

}  // namespace hayaku
