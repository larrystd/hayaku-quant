#pragma once

/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-08-28
 *      Author: fasiondog
 */

#include "KDataImp.h"

namespace hayaku {

class KDataSharedBufferImp : public KDataImp {
 public:
  KDataSharedBufferImp() = default;
  KDataSharedBufferImp(const Stock& stock, const KQuery& query);
  virtual ~KDataSharedBufferImp() override;

  virtual bool empty() const noexcept override { return size_ == 0; }

  virtual size_t size() const noexcept override { return size_; }

  virtual size_t startPos() const override { return start_; }

  virtual size_t endPos() const override { return end_; }

  virtual size_t lastPos() const override { return end_ == 0 ? 0 : end_ - 1; }

  virtual size_t getPos(const Datetime& datetime) const noexcept override;

  virtual const KRecord& getKRecord(size_t pos) const noexcept override;

  virtual const KRecord& front() const override { return data_[0]; }

  virtual const KRecord& back() const override { return data_[size_ - 1]; }

  virtual const KRecord* data() const noexcept override { return data_; }

  virtual KRecord* data() noexcept override { return data_; }

  virtual DatetimeList getDatetimeList() const override;

  virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

 private:
  size_t start_{0};
  size_t end_{0};
  size_t size_{0};
  KRecord* data_{nullptr};
};

} /* namespace hayaku */
