
#include "AsyncMySQLConnect.cpp"
#include "AsyncMySQLStatement.cpp"
#include "common/Config.h"

#if HAYAKU_DISABLE_LIBMYSQLCLIENT
#include "MySQLConnectBoost.cpp"
#include "MySQLStatementBoost.cpp"
#else
#include "MySQLConnectNative.cpp"
#include "MySQLStatementNative.cpp"
#endif
