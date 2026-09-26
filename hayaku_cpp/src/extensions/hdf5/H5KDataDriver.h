#pragma once

/*
 * H5KDataDriver.h
 *
 *  Created on: 2017-10-11
 *      Author: fasiondog
 */

#include "H5Record.h"
#include "data/storage/KDataDriver.h"

namespace hayaku {

class H5KDataDriver : public KDataDriver {
 public:
  H5KDataDriver();
  virtual ~H5KDataDriver() override;

  virtual KDataDriverPtr _clone() override {
    return std::make_shared<H5KDataDriver>();
  }

  virtual bool _init() override;

  virtual bool isIndexFirst() override { return true; }

  virtual bool canParallelLoad() override {
#if defined(H5_HAVE_THREADSAFE)
    return true;
#else
    HAYAKU_WARN("Current hdf5 library is not thread-safe!");
    return false;
#endif
  }

  virtual size_t getCount(const string& market, const string& code,
                          const KQuery::KType& kType) override;
  virtual bool getIndexRangeByDate(const string& market, const string& code,
                                   const KQuery& query, size_t& out_start,
                                   size_t& out_end) override;
  virtual KRecordList getKRecordList(const string& market, const string& code,
                                     const KQuery& query) override;
  virtual TimeLineList getTimeLineList(const string& market, const string& code,
                                       const KQuery& query) override;
  virtual TransList getTransList(const string& market, const string& code,
                                 const KQuery& query) override;

 private:
  void H5ReadRecords(H5::DataSet&, hsize_t, hsize_t, void*);
  void H5ReadIndexRecords(H5::DataSet&, hsize_t, hsize_t, void*);
  void H5ReadTimeLineRecords(H5::DataSet&, hsize_t, hsize_t, void*);
  void H5ReadTransRecords(H5::DataSet&, hsize_t, hsize_t, void*);

  bool _getH5FileAndGroup(const string& market, const string& code,
                          KQuery::KType kType, H5FilePtr& out_file,
                          H5::Group& out_group);

  bool _getBaseIndexRangeByDate(const string&, const string&, const KQuery&,
                                size_t& out_start, size_t& out_end);
  bool _getOtherIndexRangeByDate(const string&, const string&, const KQuery&,
                                 size_t& out_start, size_t& out_end);

  KRecordList _getBaseKRecordList(const string& market, const string& code,
                                  const KQuery::KType& kType, size_t start_ix,
                                  size_t end_ix);
  KRecordList _getIndexKRecordList(const string& market, const string& code,
                                   const KQuery::KType& kType, size_t start_ix,
                                   size_t end_ix);

  TimeLineList _getTimeLine(const string& market, const string& code,
                            int64_t start, int64_t end);
  TimeLineList _getTimeLine(const string& market, const string& code,
                            const Datetime& start, const Datetime& end);

  TransList _getTransList(const string& market, const string& code,
                          int64_t start, int64_t end);
  TransList _getTransList(const string& market, const string& code,
                          const Datetime& start, const Datetime& end);

 private:
  H5::CompType h5_data_type_;
  H5::CompType h5_index_type_;
  H5::CompType h5_time_line_type_;
  H5::CompType h5_trans_type_;
  unordered_map<string, H5FilePtr> h5file_map_;  // key: market+code
};

} /* namespace hayaku */
