#pragma once

/*
 * DatabaseDriverFactory.h
 *
 *  Created on: 2012-8-14
 *      Author: fasiondog
 */

#include "BaseInfoDriver.h"
#include "BlockInfoDriver.h"
#include "DriverConnectPool.h"
#include "KDataDriver.h"

namespace hayaku {

typedef DriverConnectPool<KDataDriverConnect> KDataDriverConnectPool;
typedef shared_ptr<KDataDriverConnectPool> KDataDriverConnectPoolPtr;

/**
 * Data driver factory class
 * @ingroup DataDriver
 */
class DataDriverFactory {
 public:
  /**
   * Initialize the supported default drivers
   */
  static void init();

  /**
   * Release the resources proactively, mainly used for memory leak detection,
   * cleaning up proactively on exit to avoid false positives
   */
  static void release();

  static void regBaseInfoDriver(const BaseInfoDriverPtr &);
  static void removeBaseInfoDriver(const string &name);
  static BaseInfoDriverPtr getBaseInfoDriver(const Parameter &);

  static void regBlockDriver(const BlockInfoDriverPtr &);
  static void removeBlockDriver(const string &name);
  static BlockInfoDriverPtr getBlockDriver(const Parameter &);

  static void regKDataDriver(const KDataDriverPtr &);
  static void removeKDataDriver(const string &name);
  static KDataDriverConnectPoolPtr getKDataDriverPool(const Parameter &);

 private:
  static map<string, BaseInfoDriverPtr> *base_info_drivers_;
  static map<string, BlockInfoDriverPtr> *block_drivers_;
  static map<string, KDataDriverPtr>
      *kdata_prototype_drivers_;  // K-line driver prototype
  static map<string, KDataDriverConnectPoolPtr>
      *kdata_driver_pools_;  // K-line driver pool
};

} /* namespace hayaku */
