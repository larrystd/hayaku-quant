/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include <boost/algorithm/string.hpp>
#include "extensions/ingest/QLBlockInfoDriver.h"
#include "DoNothingKDataDriver.h"
#include "extensions/ingest/KDataTempCsvDriver.h"
#include "DataDriverFactory.h"
#include "KDataDriver.h"

#if HAYAKU_ENABLE_SQLITE_KDATA || HAYAKU_ENABLE_HDF5_KDATA
#include "SQLiteBaseInfoDriver.h"
#include "SQLiteBlockInfoDriver.h"
#endif

#if HAYAKU_ENABLE_HDF5_KDATA
#include "extensions/hdf5/H5KDataDriver.h"
#endif

#if HAYAKU_ENABLE_MYSQL_KDATA
#include "extensions/mysql/MySQLBaseInfoDriver.h"
#include "extensions/mysql/MySQLBlockInfoDriver.h"
#include "extensions/mysql/MySQLKDataDriver.h"
#endif

#if HAYAKU_ENABLE_TDX_KDATA
#include "extensions/ingest/TdxKDataDriver.h"
#endif

#if HAYAKU_ENABLE_SQLITE_KDATA
#include "SQLiteKDataDriver.h"
#endif

namespace hayaku {

map<string, BaseInfoDriverPtr>* DataDriverFactory::m_baseInfoDrivers{nullptr};
map<string, BlockInfoDriverPtr>* DataDriverFactory::m_blockDrivers{nullptr};
map<string, KDataDriverPtr>* DataDriverFactory::m_kdataPrototypeDrivers{nullptr};

map<string, KDataDriverConnectPoolPtr>* DataDriverFactory::m_kdataDriverPools{nullptr};

void DataDriverFactory::init() {
    m_baseInfoDrivers = new map<string, BaseInfoDriverPtr>();
    m_blockDrivers = new map<string, BlockInfoDriverPtr>();
    DataDriverFactory::regBlockDriver(make_shared<QLBlockInfoDriver>());

#if HAYAKU_ENABLE_SQLITE_KDATA || HAYAKU_ENABLE_HDF5_KDATA
    DataDriverFactory::regBaseInfoDriver(make_shared<SQLiteBaseInfoDriver>());
    DataDriverFactory::regBlockDriver(make_shared<SQLiteBlockInfoDriver>());
#endif

#if HAYAKU_ENABLE_MYSQL_KDATA
    DataDriverFactory::regBaseInfoDriver(make_shared<MySQLBaseInfoDriver>());
    DataDriverFactory::regBlockDriver(make_shared<MySQLBlockInfoDriver>());
#endif

    m_kdataPrototypeDrivers = new map<string, KDataDriverPtr>();
    m_kdataDriverPools = new map<string, KDataDriverConnectPoolPtr>();

    DataDriverFactory::regKDataDriver(make_shared<DoNothingKDataDriver>());
    DataDriverFactory::regKDataDriver(make_shared<KDataTempCsvDriver>());

#if HAYAKU_ENABLE_TDX_KDATA
    DataDriverFactory::regKDataDriver(make_shared<TdxKDataDriver>());
#endif

#if HAYAKU_ENABLE_HDF5_KDATA
    DataDriverFactory::regKDataDriver(make_shared<H5KDataDriver>());
#endif

#if HAYAKU_ENABLE_MYSQL_KDATA
    DataDriverFactory::regKDataDriver(make_shared<MySQLKDataDriver>());
#endif

#if HAYAKU_ENABLE_SQLITE_KDATA
    DataDriverFactory::regKDataDriver(make_shared<SQLiteKDataDriver>());
#endif
}

void DataDriverFactory::release() {
    if (m_baseInfoDrivers) {
        m_baseInfoDrivers->clear();
        delete m_baseInfoDrivers;
        m_baseInfoDrivers = nullptr;
    }

    if (m_blockDrivers) {
        m_blockDrivers->clear();
        delete m_blockDrivers;
        m_blockDrivers = nullptr;
    }

    if (m_kdataPrototypeDrivers) {
        m_kdataPrototypeDrivers->clear();
        delete m_kdataPrototypeDrivers;
        m_kdataPrototypeDrivers = nullptr;
    }

    if (m_kdataDriverPools) {
        m_kdataDriverPools->clear();
        delete m_kdataDriverPools;
        m_kdataDriverPools = nullptr;
    }
}

void DataDriverFactory::regBaseInfoDriver(const BaseInfoDriverPtr& driver) {
    HAYAKU_CHECK(driver, "driver is nullptr!");
    string new_type(driver->name());
    to_upper(new_type);
    (*m_baseInfoDrivers)[new_type] = driver;
}

void DataDriverFactory::removeBaseInfoDriver(const string& name) {
    string new_type(name);
    to_upper(new_type);
    m_baseInfoDrivers->erase(new_type);
}

BaseInfoDriverPtr DataDriverFactory ::getBaseInfoDriver(const Parameter& params) {
    map<string, BaseInfoDriverPtr>::const_iterator iter;
    string type = params.get<string>("type");
    to_upper(type);
    iter = m_baseInfoDrivers->find(type);
    BaseInfoDriverPtr result;
    if (iter != m_baseInfoDrivers->end()) {
        result = iter->second;
        result->init(params);
    }
    return result;
}

void DataDriverFactory::regBlockDriver(const BlockInfoDriverPtr& driver) {
    HAYAKU_CHECK(driver, "driver is nullptr!");
    string name(driver->name());
    to_upper(name);
    (*m_blockDrivers)[name] = driver;
}

void DataDriverFactory::removeBlockDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_blockDrivers->erase(new_name);
}

BlockInfoDriverPtr DataDriverFactory::getBlockDriver(const Parameter& params) {
    BlockInfoDriverPtr result;
    map<string, BlockInfoDriverPtr>::const_iterator iter;
    string name = params.get<string>("type");
    to_upper(name);
    iter = m_blockDrivers->find(name);
    if (iter != m_blockDrivers->end()) {
        result = iter->second;
        result->init(params);
    }

    return result;
}

void DataDriverFactory::regKDataDriver(const KDataDriverPtr& driver) {
    string new_type(driver->name());
    to_upper(new_type);
    HAYAKU_CHECK(m_kdataDriverPools->find(new_type) == m_kdataDriverPools->end(),
              "Repeat regKDataDriver!");
    (*m_kdataPrototypeDrivers)[new_type] = driver;
    // The connection pool is not created here
    //(*m_kdataDriverPools)[new_type] = make_shared<KDataDriverPool>();
}

void DataDriverFactory::removeKDataDriver(const string& name) {
    string new_name(name);
    to_upper(new_name);
    m_kdataPrototypeDrivers->erase(new_name);
    auto iter = m_kdataDriverPools->find(new_name);
    if (iter != m_kdataDriverPools->end()) {
        m_kdataDriverPools->erase(iter);
    }
}

KDataDriverConnectPoolPtr DataDriverFactory::getKDataDriverPool(const Parameter& params) {
    KDataDriverConnectPoolPtr result;
    string name = params.get<string>("type");
    to_upper(name);
    auto iter = m_kdataDriverPools->find(name);
    if (iter != m_kdataDriverPools->end()) {
        result = iter->second;
    } else {
        auto prototype_iter = m_kdataPrototypeDrivers->find(name);
        HAYAKU_CHECK(prototype_iter != m_kdataPrototypeDrivers->end(), "Unregistered driver: {}",
                  name);
        HAYAKU_CHECK(prototype_iter->second->init(params), "Failed init driver: {}", name);
        (*m_kdataDriverPools)[name] = make_shared<KDataDriverConnectPool>(
          prototype_iter->second, std::min<size_t>(std::thread::hardware_concurrency() * 3, 100));
        result = (*m_kdataDriverPools)[name];
    }
    return result;
}

} /* namespace hayaku */
