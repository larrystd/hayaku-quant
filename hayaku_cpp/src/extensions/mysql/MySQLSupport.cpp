
#include "common/Config.h"

#include "AsyncMySQLConnect.cpp"
#include "AsyncMySQLStatement.cpp"

#if HAYAKU_DISABLE_LIBMYSQLCLIENT
#include "MySQLConnectBoost.cpp"
#include "MySQLStatementBoost.cpp"
#else
#include "MySQLConnectNative.cpp"
#include "MySQLStatementNative.cpp"
#endif
