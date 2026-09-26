/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include "KDataSharedBufferImp.h"

#include <functional>

namespace hayaku {

KDataSharedBufferImp::KDataSharedBufferImp(const Stock& stock,
                                           const KQuery& query)
    : KDataImp(stock, query) {
  bool sucess = stock_.getIndexRange(query, start_, end_);
  if (!sucess) {
    start_ = 0;
    end_ = 0;
    return;
  }

  size_ = end_ - start_;
  std::shared_lock<std::shared_mutex> lock(
      *(stock_.data_->pMutex[query_.kType()]));
  data_ = stock_.data_->pKData[query_.kType()]->data() + start_;
}

KDataSharedBufferImp::~KDataSharedBufferImp() {}

size_t KDataSharedBufferImp::getPos(const Datetime& datetime) const noexcept {
  KRecord null_record;
  if (empty()) {
    return Null<size_t>();
  }

  size_t mid, low = 0, high = size() - 1;
  while (low <= high) {
    if (datetime > data_[high].datetime) {
      mid = high + 1;
      break;
    }

    if (data_[low].datetime >= datetime) {
      mid = low;
      break;
    }

    mid = (low + high) / 2;
    if (datetime > data_[mid].datetime) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  if (mid >= size()) {
    return Null<size_t>();
  }

  return data_[mid].datetime == datetime ? mid : Null<size_t>();
}

const KRecord& KDataSharedBufferImp::getKRecord(size_t pos) const noexcept {
  return pos < size_ ? data_[pos] : KRecord::NullKRecord;
}

DatetimeList KDataSharedBufferImp::getDatetimeList() const {
  DatetimeList result(size_);
  for (size_t i = 0; i < size_; ++i) {
    result[i] = data_[i].datetime;
  }
  return result;
}

KDataImpPtr KDataSharedBufferImp::getOtherFromSelf(const KQuery& query) const {
  return std::make_shared<KDataSharedBufferImp>(stock_, query);
}

} /* namespace hayaku */
