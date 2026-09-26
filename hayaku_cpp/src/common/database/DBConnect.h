#pragma once

/*
 * DBConnect.h
 *
 *  Copyright (c) 2019, hikyuu.org
 *
 *  Created on: 2019-7-11
 *      Author: fasiondog
 */

#include "SQLResultSet.h"
#include "AsyncSQLResultSet.h"
#include "DBConnectBase.h"
#include "SQLStatementBase.h"
#include "AsyncSQLStatementBase.h"
#include "AutoTransAction.h"
#include "AsyncTransAction.h"
#include "TableMacro.h"
#include "DBUpgrade.h"

#include "common/Config.h"
#if HAYAKU_ENABLE_MYSQL
#include "extensions/mysql/MySQLConnect.h"
#include "extensions/mysql/AsyncMySQLConnect.h"
#endif

#if HAYAKU_ENABLE_SQLITE
#include "SQLiteConnect.h"
#include "AsyncSQLiteConnect.h"
#include "SQLiteUtil.h"
#endif
