/*
 * DatabaseDriverFactory.cpp
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include "DataDriverFactory.h"

#include <boost/algorithm/string.hpp>

#include "DoNothingKDataDriver.h"
#include "KDataDriver.h"
#include "extensions/ingest/KDataTempCsvDriver.h"
#include "extensions/ingest/QLBlockInfoDriver.h"

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

map<string, BaseInfoDriverPtr>* DataDriverFactory::base_info_drivers_{nullptr};
map<string, BlockInfoDriverPtr>* DataDriverFactory::block_drivers_{nullptr};
map<string, KDataDriverPtr>* DataDriverFactory::kdata_prototype_drivers_{
    nullptr};

map<string, KDataDriverConnectPoolPtr>* DataDriverFactory::kdata_driver_pools_{
    nullptr};

void DataDriverFactory::init() {
  base_info_drivers_ = new map<string, BaseInfoDriverPtr>();
  block_drivers_ = new map<string, BlockInfoDriverPtr>();
  DataDriverFactory::regBlockDriver(make_shared<QLBlockInfoDriver>());

#if HAYAKU_ENABLE_SQLITE_KDATA || HAYAKU_ENABLE_HDF5_KDATA
  DataDriverFactory::regBaseInfoDriver(make_shared<SQLiteBaseInfoDriver>());
  DataDriverFactory::regBlockDriver(make_shared<SQLiteBlockInfoDriver>());
#endif

#if HAYAKU_ENABLE_MYSQL_KDATA
  DataDriverFactory::regBaseInfoDriver(make_shared<MySQLBaseInfoDriver>());
  DataDriverFactory::regBlockDriver(make_shared<MySQLBlockInfoDriver>());
#endif

  kdata_prototype_drivers_ = new map<string, KDataDriverPtr>();
  kdata_driver_pools_ = new map<string, KDataDriverConnectPoolPtr>();

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
  if (base_info_drivers_) {
    base_info_drivers_->clear();
    delete base_info_drivers_;
    base_info_drivers_ = nullptr;
  }

  if (block_drivers_) {
    block_drivers_->clear();
    delete block_drivers_;
    block_drivers_ = nullptr;
  }

  if (kdata_prototype_drivers_) {
    kdata_prototype_drivers_->clear();
    delete kdata_prototype_drivers_;
    kdata_prototype_drivers_ = nullptr;
  }

  if (kdata_driver_pools_) {
    kdata_driver_pools_->clear();
    delete kdata_driver_pools_;
    kdata_driver_pools_ = nullptr;
  }
}

void DataDriverFactory::regBaseInfoDriver(const BaseInfoDriverPtr& driver) {
  HAYAKU_CHECK(driver, "driver is nullptr!");
  string new_type(driver->name());
  to_upper(new_type);
  (*base_info_drivers_)[new_type] = driver;
}

void DataDriverFactory::removeBaseInfoDriver(const string& name) {
  string new_type(name);
  to_upper(new_type);
  base_info_drivers_->erase(new_type);
}

BaseInfoDriverPtr DataDriverFactory ::getBaseInfoDriver(
    const Parameter& params) {
  map<string, BaseInfoDriverPtr>::const_iterator iter;
  string type = params.get<string>("type");
  to_upper(type);
  iter = base_info_drivers_->find(type);
  BaseInfoDriverPtr result;
  if (iter != base_info_drivers_->end()) {
    result = iter->second;
    result->init(params);
  }
  return result;
}

void DataDriverFactory::regBlockDriver(const BlockInfoDriverPtr& driver) {
  HAYAKU_CHECK(driver, "driver is nullptr!");
  string name(driver->name());
  to_upper(name);
  (*block_drivers_)[name] = driver;
}

void DataDriverFactory::removeBlockDriver(const string& name) {
  string new_name(name);
  to_upper(new_name);
  block_drivers_->erase(new_name);
}

BlockInfoDriverPtr DataDriverFactory::getBlockDriver(const Parameter& params) {
  BlockInfoDriverPtr result;
  map<string, BlockInfoDriverPtr>::const_iterator iter;
  string name = params.get<string>("type");
  to_upper(name);
  iter = block_drivers_->find(name);
  if (iter != block_drivers_->end()) {
    result = iter->second;
    result->init(params);
  }

  return result;
}

void DataDriverFactory::regKDataDriver(const KDataDriverPtr& driver) {
  string new_type(driver->name());
  to_upper(new_type);
  HAYAKU_CHECK(kdata_driver_pools_->find(new_type) == kdata_driver_pools_->end(),
               "Repeat regKDataDriver!");
  (*kdata_prototype_drivers_)[new_type] = driver;
  // The connection pool is not created here
  //(*m_kdataDriverPools)[new_type] = make_shared<KDataDriverPool>();
}

void DataDriverFactory::removeKDataDriver(const string& name) {
  string new_name(name);
  to_upper(new_name);
  kdata_prototype_drivers_->erase(new_name);
  auto iter = kdata_driver_pools_->find(new_name);
  if (iter != kdata_driver_pools_->end()) {
    kdata_driver_pools_->erase(iter);
  }
}

KDataDriverConnectPoolPtr DataDriverFactory::getKDataDriverPool(
    const Parameter& params) {
  KDataDriverConnectPoolPtr result;
  string name = params.get<string>("type");
  to_upper(name);
  auto iter = kdata_driver_pools_->find(name);
  if (iter != kdata_driver_pools_->end()) {
    result = iter->second;
  } else {
    auto prototype_iter = kdata_prototype_drivers_->find(name);
    HAYAKU_CHECK(prototype_iter != kdata_prototype_drivers_->end(),
                 "Unregistered driver: {}", name);
    HAYAKU_CHECK(prototype_iter->second->init(params), "Failed init driver: {}",
                 name);
    (*kdata_driver_pools_)[name] = make_shared<KDataDriverConnectPool>(
        prototype_iter->second,
        std::min<size_t>(std::thread::hardware_concurrency() * 3, 100));
    result = (*kdata_driver_pools_)[name];
  }
  return result;
}

} /* namespace hayaku */
