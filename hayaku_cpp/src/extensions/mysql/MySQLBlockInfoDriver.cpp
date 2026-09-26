/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240102 added by fasiondog
 */

#include "MySQLBlockInfoDriver.h"

#include "common/database/DBConnect.h"
#include "extensions/mysql/MySQLConnect.h"

namespace hayaku {

struct MySQLBlockView {
  TABLE_BIND4(MySQLBlockView, block, category, name, market_code, index_code)
  string category;
  string name;
  string market_code;
  string index_code;
};

struct MySQLBlockTable {
  TABLE_BIND3(MySQLBlockTable, block, category, name, market_code)
  string category;
  string name;
  string market_code;
};

struct MySQLBlockIndexTable {
  TABLE_BIND3(MySQLBlockIndexTable, BlockIndex, category, name, market_code)
  string category;
  string name;
  string market_code;
};

MySQLBlockInfoDriver::~MySQLBlockInfoDriver() {}

bool MySQLBlockInfoDriver::_init() { return true; }

DBConnectPtr MySQLBlockInfoDriver::getConnect() {
  Parameter connect_param;
  connect_param.set<string>(
      "host", getParamFromOther<string>(params_, "host", "127.0.0.1"));
  connect_param.set<string>("usr",
                            getParamFromOther<string>(params_, "usr", "root"));
  connect_param.set<string>("pwd",
                            getParamFromOther<string>(params_, "pwd", ""));
  connect_param.set<string>(
      "db", getParamFromOther<string>(params_, "db", "hayaku_base"));
  string port_str = getParamFromOther<string>(params_, "port", "3306");
  unsigned int port = boost::lexical_cast<unsigned int>(port_str);
  connect_param.set<int>("port", port);
  return std::make_shared<MySQLConnect>(connect_param);
}

void MySQLBlockInfoDriver::load() {
  auto connect = getConnect();
  vector<MySQLBlockView> records;
  connect->batchLoadView(
      records,
      "select a.id, a.category, a.name, a.market_code, b.market_code as "
      "index_code from `hayaku_base`.`block` a left "
      "join `hayaku_base`.`BlockIndex` b on a.category=b.category and a.name = "
      "b.name");

  std::unique_lock<std::shared_mutex> lock(buffer_mutex_);
  for (auto& record : records) {
    auto category_iter = buffer_.find(record.category);
    if (category_iter == buffer_.end()) {
      buffer_[record.category] = {};
    }
    auto& name_dict = buffer_[record.category];
    auto name_iter = name_dict.find(record.name);
    if (name_iter == name_dict.end()) {
      name_dict[record.name] = {
          Block(record.category, record.name, record.index_code)};
    }
    name_dict[record.name].add(record.market_code);
  }
}

StringList MySQLBlockInfoDriver::getAllCategory() {
  StringList ret;
  std::shared_lock<std::shared_mutex> lock(buffer_mutex_);
  ret.reserve(buffer_.size());
  for (auto& category_iter : buffer_) {
    ret.push_back(category_iter.first);
  }
  return ret;
}

Block MySQLBlockInfoDriver::getBlock(const string& category,
                                     const string& name) {
  Block ret;
  std::shared_lock<std::shared_mutex> lock(buffer_mutex_);
  auto category_iter = buffer_.find(category);
  HAYAKU_IF_RETURN(category_iter == buffer_.end(), ret);

  auto block_iter = category_iter->second.find(name);
  HAYAKU_IF_RETURN(block_iter == category_iter->second.end(), ret);

  ret = block_iter->second;
  return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList(const string& category) {
  BlockList ret;
  std::shared_lock<std::shared_mutex> lock(buffer_mutex_);
  auto category_iter = buffer_.find(category);
  HAYAKU_IF_RETURN(category_iter == buffer_.end(), ret);

  const auto& category_blocks = category_iter->second;
  for (auto iter = category_blocks.begin(); iter != category_blocks.end();
       ++iter) {
    ret.emplace_back(iter->second);
  }

  return ret;
}

BlockList MySQLBlockInfoDriver::getBlockList() {
  BlockList ret;
  std::shared_lock<std::shared_mutex> lock(buffer_mutex_);
  for (auto category_iter = buffer_.begin(); category_iter != buffer_.end();
       ++category_iter) {
    const auto& category_blocks = category_iter->second;
    for (auto iter = category_blocks.begin(); iter != category_blocks.end();
         ++iter) {
      ret.emplace_back(iter->second);
    }
  }
  return ret;
}

void MySQLBlockInfoDriver::save(const Block& block) {
  std::unique_lock<std::shared_mutex> lock(buffer_mutex_);
  auto category_iter = buffer_.find(block.category());
  if (category_iter == buffer_.end()) {
    buffer_.emplace(block.category(),
                     unordered_map<string, Block>{{block.name(), block}});
  } else {
    category_iter->second.emplace(block.name(), block);
  }

  auto connect = getConnect();
  AutoTransAction trans(connect);
  auto condition =
      (Field("category") == block.category()) & (Field("name") == block.name());
  connect->remove(MySQLBlockView::getTableName(), condition, false);
  connect->remove(MySQLBlockIndexTable::getTableName(), condition, false);

  if (!block.getIndexStock().isNull()) {
    MySQLBlockIndexTable index;
    index.category = block.category();
    index.name = block.name();
    index.market_code = block.getIndexStock().market_code();
    connect->save(index, false);
  }

  for (auto iter = block.begin(); iter != block.end(); ++iter) {
    MySQLBlockTable record;
    record.category = block.category();
    record.name = block.name();
    record.market_code = iter->market_code();
    connect->save(record, false);
  }
}

void MySQLBlockInfoDriver::remove(const string& category, const string& name) {
  {
    auto connect = getConnect();
    AutoTransAction trans(connect);
    auto condition = (Field("category") == category) & (Field("name") == name);
    connect->remove(MySQLBlockTable::getTableName(), condition, false);
    connect->remove(MySQLBlockIndexTable::getTableName(), condition, false);
  }

  std::unique_lock<std::shared_mutex> lock(buffer_mutex_);
  auto category_iter = buffer_.find(category);
  HAYAKU_IF_RETURN(category_iter == buffer_.end(), void());

  auto block_iter = category_iter->second.find(name);
  HAYAKU_IF_RETURN(block_iter == category_iter->second.end(), void());

  category_iter->second.erase(block_iter);
  buffer_.erase(category_iter);
}

}  // namespace hayaku
