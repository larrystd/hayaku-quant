#pragma once

/*
 * KDataPrivatedBufferImp.h
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include "KDataImp.h"

namespace hayaku {

class KDataPrivatedBufferImp : public KDataImp {
 public:
  KDataPrivatedBufferImp();
  KDataPrivatedBufferImp(const Stock& stock, const KQuery& query);
  KDataPrivatedBufferImp(const Stock& stock, const KQuery& query,
                         const KRecordList& krecords);
  virtual ~KDataPrivatedBufferImp() override;

  virtual bool empty() const noexcept override { return buffer_.empty(); }

  virtual size_t size() const noexcept override { return buffer_.size(); }

  virtual size_t startPos() const override;
  virtual size_t endPos() const override;
  virtual size_t lastPos() const override;

  virtual size_t getPos(const Datetime& datetime) const noexcept override;

  virtual const KRecord& getKRecord(size_t pos) const noexcept override {
    return buffer_[pos];
  }

  virtual const KRecord& front() const override { return buffer_.front(); }

  virtual const KRecord& back() const override { return buffer_.back(); }

  virtual const KRecord* data() const noexcept override {
    return buffer_.data();
  }

  virtual KRecord* data() noexcept override { return buffer_.data(); }

  virtual DatetimeList getDatetimeList() const override;

  virtual KDataImpPtr getOtherFromSelf(const KQuery& query) const override;

 private:
  void _getPosInStock() const;
  void _recover();
  void _recoverForward();
  void _recoverBackward();
  void _recoverEqualForward();
  void _recoverEqualBackward();
  void _recoverForUpDay();

  KDataImpPtr _getOtherFromSelfByIndex(const KQuery& query) const;
  KDataImpPtr _getOtherFromSelfByDate(const KQuery& query) const;

 private:
  KRecordList buffer_;
  mutable size_t start_{0};
  mutable size_t end_{0};
  mutable bool have_pos_in_stock_{false};
};

// typedef shared_ptr<KDataPrivatedBufferImp> KDataPrivatedBufferImpPtr;

} /* namespace hayaku */
