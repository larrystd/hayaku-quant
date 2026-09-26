#pragma once

/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#include <mutex>

#include "data/storage/BlockInfoDriver.h"

namespace hayaku {

class MySQLBlockInfoDriver : public BlockInfoDriver {
 public:
  MySQLBlockInfoDriver() : BlockInfoDriver("mysql") {};
  virtual ~MySQLBlockInfoDriver() override;

  virtual void load() override;
  virtual bool _init() override;
  virtual StringList getAllCategory() override;
  virtual Block getBlock(const string&, const string&) override;
  virtual BlockList getBlockList(const string& category) override;
  virtual BlockList getBlockList() override;
  virtual void save(const Block& block) override;
  virtual void remove(const string& category, const string& name) override;

 private:
  DBConnectPtr getConnect();

 private:
  unordered_map<string, unordered_map<string, Block>> buffer_;
  std::shared_mutex buffer_mutex_;
};

}  // namespace hayaku
