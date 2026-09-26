#pragma once

/*
 * StockMapIterator.h
 *
 *  Created on: 2015-2-8
 *      Author: fasiondog
 */

#include <iterator>

#include "Stock.h"

namespace hayaku {

class StockMapIterator {
 public:
  typedef unordered_map<string, Stock> stock_map_t;
  typedef Stock value_type;
  typedef Stock* pointer;
  typedef const Stock& reference;
  typedef stock_map_t::const_iterator::difference_type difference_type;
  typedef std::input_iterator_tag iterator_category;

  StockMapIterator() {}

  // cppcheck-suppress noExplicitConstructor
  StockMapIterator(const stock_map_t::const_iterator& iter) : iter_(iter) {}

  StockMapIterator(const StockMapIterator& iter) : iter_(iter.iter_) {}

  StockMapIterator& operator=(const StockMapIterator& iter) {
    HAYAKU_IF_RETURN(this == &iter, *this);
    iter_ = iter.iter_;
    return *this;
  }

  StockMapIterator& operator++() {
    ++iter_;
    return *this;
  }

  const StockMapIterator operator++(int) {
    auto old_iter = iter_;
    ++iter_;
    return StockMapIterator(old_iter);
  }

  bool operator==(const StockMapIterator& iter) const {
    return iter_ == iter.iter_;
  }

  bool operator!=(const StockMapIterator& iter) const {
    return iter_ != iter.iter_;
  }

  const Stock& operator*() const { return iter_->second; }

  const Stock* const operator->() const { return &(iter_->second); }

 private:
  stock_map_t::const_iterator iter_;
};

} /* namespace hayaku */
