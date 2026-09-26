/*
 * Block.cpp
 *
 *  Copyright (c) hikyuu.org
 *
 *  Created on: 2015-2-8
 *      Author: fasiondog
 */

#include "data/DataRuntime.h"
#include "xxhash.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const Block& blk) {
  string strip(", ");
  os << "Block(" << blk.category() << strip << blk.name() << strip << blk.size()
     << ")";
  return os;
}

Block::Block() noexcept {}

Block::~Block() {}

Block::Block(const string& category, const string& name)
    : data_(make_shared<Data>()) {
  data_->category_ = category;
  data_->name_ = name;
}

Block::Block(const string& category, const string& name,
             const string& indexCode)
    : Block(category, name) {
  if (!indexCode.empty()) {
    auto stock = getDataRuntime().getStock(indexCode);
    if (!stock.isNull()) {
      data_->index_stock_ = stock;
    } else {
      // Ignore it directly, no more printing
      HAYAKU_TRACE("Can't find index stock: {}, will ignore!", indexCode);
    }
  }
}

Block::Block(const Block& block) noexcept {
  if (!block.data_) return;
  data_ = block.data_;
}

Block::Block(Block&& block) noexcept {
  if (!block.data_) return;
  data_ = std::move(block.data_);
}

Block::Block(const StockList& stocks) : data_(make_shared<Data>()) {
  for (const auto& stock : stocks) {
    add(stock);
  }
}

Block::Block(const StringList& market_codes) : data_(make_shared<Data>()) {
  for (const auto& market_code : market_codes) {
    add(market_code);
  }
}

Block& Block::operator=(const Block& block) noexcept {
  HAYAKU_IF_RETURN(this == &block || data_ == block.data_, *this);
  data_ = block.data_;
  return *this;
}

Block& Block::operator=(Block&& block) noexcept {
  HAYAKU_IF_RETURN(this == &block || data_ == block.data_, *this);
  data_ = std::move(block.data_);
  return *this;
}

bool Block::have(const string& market_code) const {
  HAYAKU_IF_RETURN(!data_, false);
  string query_str = market_code;
  to_upper(query_str);
  return data_->stock_dict_.count(query_str) ? true : false;
}

bool Block::have(const Stock& stock) const {
  HAYAKU_IF_RETURN(!data_, false);
  return data_->stock_dict_.count(stock.market_code()) ? true : false;
}

Stock Block::get(const string& market_code) const {
  Stock result;
  HAYAKU_IF_RETURN(!data_, result);
  string query_str = market_code;
  to_upper(query_str);
  auto iter = data_->stock_dict_.find(query_str);
  if (iter != data_->stock_dict_.end()) {
    result = iter->second;
  }
  return result;
}

StockList Block::getStockList(
    std::function<bool(const Stock&)>&& filter) const {
  StockList ret;
  ret.reserve(size());
  auto iter = data_->stock_dict_.begin();
  if (filter) {
    for (; iter != data_->stock_dict_.end(); ++iter) {
      if (filter(iter->second)) {
        ret.emplace_back(iter->second);
      }
    }
  } else {
    for (; iter != data_->stock_dict_.end(); ++iter) {
      ret.emplace_back(iter->second);
    }
  }
  return ret;
}

bool Block::add(const Stock& stock) {
  HAYAKU_IF_RETURN(stock.isNull() || have(stock), false);
  if (!data_) data_ = make_shared<Data>();

  data_->stock_dict_[stock.market_code()] = stock;
  return true;
}

bool Block::add(const string& market_code) {
  const DataRuntime& sm = getDataRuntime();
  Stock stock = sm.getStock(market_code);
  // No log is printed when the stock is empty, to prevent too much printing,
  // especially because some accumulating and unused blocks print a lot of logs
  // during the initialization
  HAYAKU_IF_RETURN(stock.isNull() || have(stock), false);
  if (!data_) [[unlikely]]
    data_ = make_shared<Data>();

  data_->stock_dict_[stock.market_code()] = stock;
  return true;
}

bool Block::add(const StockList& stocks) {
  bool success = true;
  for (const auto& stk : stocks) {
    success = add(stk);
  }
  return success;
}

bool Block::add(const StringList& market_codes) {
  bool success = true;
  for (const auto& code : market_codes) {
    success = add(code);
  }
  return success;
}

bool Block::remove(const string& market_code) {
  HAYAKU_IF_RETURN(!have(market_code), false);
  string query_str = market_code;
  to_upper(query_str);
  data_->stock_dict_.erase(query_str);
  return true;
}

bool Block::remove(const Stock& stock) {
  HAYAKU_IF_RETURN(!have(stock), false);
  data_->stock_dict_.erase(stock.market_code());
  return true;
}

void Block::setIndexStock(const Stock& stk) {
  if (!data_) data_ = make_shared<Data>();
  data_->index_stock_ = stk;
}

uint64_t Block::strongHash() const {
  HAYAKU_IF_RETURN(!data_, 0);

  XXH64_state_t* state = XXH64_createState();
  HAYAKU_IF_RETURN(!state, 0);

  uint64_t seed = 0;
  XXH64_reset(state, seed);

  XXH64_update(state, data_->category_.data(), data_->category_.size());
  XXH64_update(state, data_->name_.data(), data_->name_.size());

  StockList stocks = getStockList();
  std::sort(stocks.begin(), stocks.end(), [](const Stock& a, const Stock& b) {
    return a.market_code() < b.market_code();
  });
  for (const auto& stk : stocks) {
    auto stkid = stk.id();
    XXH64_update(state, &stkid, sizeof(stkid));
  }

  // Get the final hash value
  uint64_t result = XXH64_digest(state);
  XXH64_freeState(state);
  return result;
}

bool Block::operator==(const Block& blk) const noexcept {
  HAYAKU_IF_RETURN(this == &blk || data_ == blk.data_, true);
  HAYAKU_IF_RETURN(category() != blk.category() || name() != blk.name() ||
                       size() != blk.size() ||
                       getIndexStock() != blk.getIndexStock(),
                   false);

  auto self_stocks = getStockList();
  std::sort(self_stocks.begin(), self_stocks.end(),
            [](const Stock& a, const Stock& b) {
              return a.market_code() < b.market_code();
            });

  auto blk_stocks = blk.getStockList();
  std::sort(blk_stocks.begin(), blk_stocks.end(),
            [](const Stock& a, const Stock& b) {
              return a.market_code() < b.market_code();
            });

  for (size_t i = 0; i < self_stocks.size(); i++) {
    HAYAKU_IF_RETURN(
        self_stocks[i].market_code() != blk_stocks[i].market_code(), false);
  }

  return true;
}

Block getBlock(const string& category, const string& name) {
  auto& sm = getDataRuntime();
  return sm.getBlock(category, name);
}

} /* namespace hayaku */
