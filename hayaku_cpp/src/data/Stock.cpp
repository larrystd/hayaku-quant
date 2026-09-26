/*
 * Stock.cpp
 *
 *  Created on: 2011-11-9
 *      Author: fasiondog
 */

#include <algorithm>
#include <cstring>
#include <set>

#include "KData.h"
#include "KDataExtension.h"
#include "data/DataRuntime.h"
#include "data/storage/KDataDriver.h"
#include "extensions/realtime/ShmClientHook.h"
#include "extensions/realtime/ShmMirrorSink.h"

namespace hayaku {

const string Stock::default_market;
const string Stock::default_code;
const string Stock::default_market_code;
const string Stock::default_name;
const uint32_t Stock::default_type = Null<uint32_t>();
const bool Stock::default_valid = false;
const Datetime Stock::default_startDate;  // = Null<Datetime>();
const Datetime Stock::default_lastDate;   // = Null<Datetime>();
const price_t Stock::default_tick = 0.01;
const price_t Stock::default_tickValue = 0.01;
const price_t Stock::default_unit = 1.0;
const int Stock::default_precision = 2;
const size_t Stock::default_minTradeNumber = 100;
const size_t Stock::default_maxTradeNumber = 1000000;

HAYAKU_API std::ostream& operator<<(std::ostream& os, const Stock& stock) {
  string strip(", ");
  const DataRuntime& sm = getDataRuntime();
  StockTypeInfo typeInfo(sm.getStockTypeInfo(stock.type()));
  os << "Stock(" << stock.market() << strip << stock.code() << strip
     << stock.name() << strip << typeInfo.description() << strip
     << stock.valid() << strip << stock.startDatetime() << strip
     << stock.lastDatetime() << ")";
  return os;
}

string Stock::toString() const {
  std::stringstream os;
  string strip(", ");
  const DataRuntime& sm = getDataRuntime();
  StockTypeInfo typeInfo(sm.getStockTypeInfo(type()));
  os << "Stock(" << market() << strip << code() << strip << name() << strip
     << typeInfo.description() << strip << valid() << strip << startDatetime()
     << strip << lastDatetime() << ")";
  return os.str();
}

Stock::Data::Data()
    : market_(default_market),
      code_(default_code),
      name_(default_name),
      type_(default_type),
      valid_(default_valid),
      start_date_(default_startDate),
      last_date_(default_lastDate),
      tick_(default_tick),
      tick_value_(default_tickValue),
      unit_(default_unit),
      precision_(default_precision),
      min_trade_number_(default_minTradeNumber),
      max_trade_number_(default_maxTradeNumber) {
  auto ktype_list = KQuery::getBaseKTypeList();
  for (const auto& ktype : ktype_list) {
    pKData[ktype] = nullptr;
    pMutex[ktype] = nullptr;
    last_update_[ktype] = Datetime::min();
  }
  ktype_list = KQuery::getExtraKTypeList();
  for (const auto& ktype : ktype_list) {
    last_update_[ktype] = Datetime::min();
  }
}

Stock::Data::Data(const string& market, const string& code, const string& name,
                  uint32_t type, bool valid, const Datetime& startDate,
                  const Datetime& lastDate, price_t tick, price_t tickValue,
                  int precision, double minTradeNumber, double maxTradeNumber)
    : market_(market),
      code_(code),
      name_(name),
      type_(type),
      valid_(valid),
      start_date_(startDate),
      last_date_(lastDate),
      tick_(tick),
      tick_value_(tickValue),
      precision_(precision),
      min_trade_number_(minTradeNumber),
      max_trade_number_(maxTradeNumber) {
  if (0.0 == tick_) {
    HAYAKU_WARN("tick should not be zero! now use as 1.0");
    unit_ = 1.0;
  } else {
    unit_ = tick_value_ / tick_;
  }

  to_upper(market_);
  market_code_ = marketCode();

  auto ktype_list = KQuery::getBaseKTypeList();
  for (const auto& ktype : ktype_list) {
    pMutex[ktype] = new std::shared_mutex();
    pKData[ktype] = nullptr;
    last_update_[ktype] = Datetime::min();
  }
  ktype_list = KQuery::getExtraKTypeList();
  for (const auto& ktype : ktype_list) {
    last_update_[ktype] = Datetime::min();
  }
}

string Stock::Data::marketCode() const {
  if (type_ == STOCKTYPE_CRYPTO) return market_ + "/" + code_;
  return market_ + code_;
}

Stock::Data::~Data() {
  for (auto iter = pKData.begin(); iter != pKData.end(); ++iter) {
    if (iter->second) {
      delete iter->second;
    }
  }

  for (auto iter = pMutex.begin(); iter != pMutex.end(); ++iter) {
    if (iter->second) {
      delete iter->second;
    }
  }
}

Stock::Stock() {}

Stock::~Stock() {}

Stock::Stock(const Stock& x) noexcept
    : data_(x.data_), kdata_driver_(x.kdata_driver_) {}

Stock::Stock(Stock&& x) noexcept
    : data_(std::move(x.data_)), kdata_driver_(std::move(x.kdata_driver_)) {}

Stock& Stock::operator=(const Stock& x) {
  HAYAKU_IF_RETURN(this == &x, *this);
  data_ = x.data_;
  kdata_driver_ = x.kdata_driver_;
  return *this;
}

Stock& Stock::operator=(Stock&& x) noexcept {
  HAYAKU_IF_RETURN(this == &x, *this);
  data_ = std::move(x.data_);
  kdata_driver_ = std::move(x.kdata_driver_);
  return *this;
}

Stock::Stock(const string& market, const string& code, const string& name) {
  data_ = make_shared<Data>(market, code, name, default_type, default_valid,
                             default_startDate, default_lastDate, default_tick,
                             default_tickValue, default_precision,
                             default_minTradeNumber, default_maxTradeNumber);
}

Stock::Stock(const string& market, const string& code, const string& name,
             uint32_t type, bool valid, const Datetime& startDate,
             const Datetime& lastDate) {
  data_ =
      make_shared<Data>(market, code, name, type, valid, startDate, lastDate,
                        default_tick, default_tickValue, default_precision,
                        default_minTradeNumber, default_maxTradeNumber);
}

Stock::Stock(const string& market, const string& code, const string& name,
             uint32_t type, bool valid, const Datetime& startDate,
             const Datetime& lastDate, price_t tick, price_t tickValue,
             int precision, size_t minTradeNumber, size_t maxTradeNumber)
    : data_(make_shared<Data>(market, code, name, type, valid, startDate,
                               lastDate, tick, tickValue, precision,
                               minTradeNumber, maxTradeNumber)) {}

bool Stock::operator==(const Stock& stock) const {
  return this == &stock || data_ == stock.data_ ||
         (data_ && stock.data_ && (data_->code_ == stock.data_->code_) &&
          (data_->market_ == stock.data_->market_));
}

const string& Stock::market() const noexcept {
  return data_ ? data_->market_ : default_market;
}

const string& Stock::code() const noexcept {
  return data_ ? data_->code_ : default_code;
}

const string& Stock::market_code() const noexcept {
  return data_ ? data_->market_code_ : default_market_code;
}

const string& Stock::name() const noexcept {
  return data_ ? data_->name_ : default_name;
}

uint32_t Stock::type() const noexcept {
  return data_ ? data_->type_ : default_type;
}

bool Stock::valid() const noexcept {
  return data_ ? data_->valid_ : default_valid;
}

const Datetime& Stock::startDatetime() const noexcept {
  return data_ ? data_->start_date_ : default_startDate;
}

const Datetime& Stock::lastDatetime() const noexcept {
  return data_ ? data_->last_date_ : default_lastDate;
}

price_t Stock::tick() const noexcept {
  return data_ ? data_->tick_ : default_tick;
}

price_t Stock::tickValue() const noexcept {
  return data_ ? data_->tick_value_ : default_tickValue;
}

price_t Stock::unit() const noexcept {
  return data_ ? data_->unit_ : default_unit;
}

int Stock::precision() const noexcept {
  return data_ ? data_->precision_ : default_precision;
}

double Stock::atom() const noexcept {
  return data_ ? data_->min_trade_number_ : default_minTradeNumber;
}

double Stock::minTradeNumber() const noexcept {
  return data_ ? data_->min_trade_number_ : default_minTradeNumber;
}

double Stock::maxTradeNumber() const noexcept {
  return data_ ? data_->max_trade_number_ : default_maxTradeNumber;
}

void Stock::market(const string& market_) {
  string n_market(market_);
  to_upper(n_market);
  if (!data_) {
    data_ = make_shared<Data>(
        n_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->market_ = n_market;
    data_->market_code_ = data_->marketCode();
  }
}

void Stock::code(const string& code_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, code_, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->code_ = code_;
    data_->market_code_ = data_->marketCode();
  }
}

void Stock::name(const string& name_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, name_, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->name_ = name_;
  }
}

void Stock::type(uint32_t type_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, type_, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->type_ = type_;
  }
}

void Stock::valid(bool valid_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, valid_,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->valid_ = valid_;
  }
}

void Stock::precision(int precision_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        precision_, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->precision_ = precision_;
  }
}

void Stock::startDatetime(const Datetime& date) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        date, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->start_date_ = date;
  }
}

void Stock::lastDatetime(const Datetime& date) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, date, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  } else {
    data_->last_date_ = date;
  }
}

void Stock::tick(price_t tick_) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  }
  data_->tick_ = tick_;
  if (0.0 == data_->tick_) {
    HAYAKU_WARN("tick should not be zero! now use as 1.0");
    data_->unit_ = 1.0;
  } else {
    data_->unit_ = data_->tick_value_ / data_->tick_;
  }
}

void Stock::tickValue(price_t val) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, default_maxTradeNumber);
  }
  data_->tick_value_ = val;
  if (0.0 == data_->tick_) {
    HAYAKU_WARN("tick should not be zero! now use as 1.0");
    data_->unit_ = 1.0;
  } else {
    data_->unit_ = data_->tick_value_ / data_->tick_;
  }
}

void Stock::minTradeNumber(double num) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, num, default_maxTradeNumber);
  } else {
    data_->min_trade_number_ = num;
  }
}

void Stock::maxTradeNumber(double num) {
  if (!data_) {
    data_ = make_shared<Data>(
        default_market, default_code, default_name, default_type, default_valid,
        default_startDate, default_lastDate, default_tick, default_tickValue,
        default_precision, default_minTradeNumber, num);
  } else {
    data_->max_trade_number_ = num;
  }
}

void Stock::setKDataDriver(const KDataDriverConnectPoolPtr& kdataDriver) {
  HAYAKU_CHECK(kdataDriver, "kdataDriver is nullptr!");
  kdata_driver_ = kdataDriver;
  if (data_) {
    auto ktype_list = KQuery::getBaseKTypeList();
    for (auto& ktype : ktype_list) {
      std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
      delete data_->pKData[ktype];
      data_->pKData[ktype] = nullptr;
    }
  }
}

KDataDriverConnectPoolPtr Stock::getKDataDirver() const {
  return kdata_driver_;
}

bool Stock::isBuffer(KQuery::KType ktype) const noexcept {
  HAYAKU_IF_RETURN(!data_, false);
  string nktype(ktype);
  to_upper(nktype);
  HAYAKU_IF_RETURN(data_->pMutex.find(nktype) == data_->pMutex.end(), false);
  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  return data_->pKData.find(nktype) != data_->pKData.end() &&
         data_->pKData[nktype];
}

void Stock::setPreload(const vector<KQuery::KType>& preload_ktypes) {
  if (data_) {
    data_->ktype_preload_.clear();
    for (const auto& ktype : preload_ktypes) {
      data_->ktype_preload_.insert(ktype);
    }
  }
}

bool Stock::isPreload(KQuery::KType ktype) const noexcept {
  HAYAKU_IF_RETURN(!data_, false);
  to_upper(ktype);
  return data_->ktype_preload_.find(ktype) != data_->ktype_preload_.end();
}

void Stock::releaseKDataBuffer(KQuery::KType ktype) const {
  HAYAKU_IF_RETURN(!data_, void());

  to_upper(ktype);
  HAYAKU_IF_RETURN(data_->pMutex.find(ktype) == data_->pMutex.end(), void());

  {
    std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
    auto iter = data_->pKData.find(ktype);
    if (iter->second) {
      delete iter->second;
      iter->second = nullptr;
    }
  }

  // Release the historical financial information at the same time, so that the
  // latest data is fetched on a reload
  {
    std::unique_lock<std::shared_mutex> lock(data_->history_finance_mutex_);
    data_->history_finance_ready_ = false;
    data_->history_finance_.clear();
  }
}

// Called during the initialization only
void Stock::loadKDataToBuffer(KQuery::KType kType) const {
  HAYAKU_IF_RETURN(!data_ || !kdata_driver_, void());

  to_upper(kType);
  HAYAKU_IF_RETURN(data_->pMutex.find(kType) == data_->pMutex.end(), void());

  auto driver = kdata_driver_->getConnect();
  size_t total = driver->getCount(data_->market_, data_->code_, kType);

  // CSV is loaded into the memory entirely, the other types are loaded
  // according to the configured preload parameters
  KQuery query = KQuery(0, Null<int64_t>(), kType);

  if (driver->name() != "TMPCSV") {
    const auto& param = getDataRuntime().getPreloadParameter();
    string preload_type = fmt::format("{}_max", kType);
    to_lower(preload_type);
    int64_t max_num = param.tryGet<int64_t>(preload_type, 4096);
    HAYAKU_ERROR_IF_RETURN(max_num < 0, void(), "Invalid preload {} param: {}",
                           preload_type, max_num);
    int64_t start = total <= (size_t)max_num ? 0 : total - max_num;
    query = KQuery(start, Null<int64_t>(), kType);
    if (driver->isColumnFirst() && market_code() != "SH000001") {
      Stock sh000001 = getDataRuntime().getStock("SH000001");
      if (!sh000001.isNull()) {
        if (!sh000001.isBuffer(kType)) {
          sh000001.loadKDataToBuffer(kType);
        }

        auto k = sh000001.getKRecord(0, kType);
        if (k.isValid()) {
          query = KQueryByDate(k.datetime, Null<Datetime>(), kType);
        }
      }
    }
  }

  {
    std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[kType]));
    // A second check of the caching is needed, to prevent it from having been
    // cached before the lock
    if (data_->pKData.find(kType) != data_->pKData.end() &&
        data_->pKData[kType]) {
      return;
    }
    KRecordList* ptr_klist = new KRecordList;
    data_->pKData[kType] = ptr_klist;
    if (total != 0) {
      (*ptr_klist) =
          driver->getKRecordList(data_->market_, data_->code_, query);
      ptr_klist->shrink_to_fit();
      if ((kType == KQuery::TIMELINE || kType == KQuery::TRANS) &&
          (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
           type() == STOCKTYPE_B)) {
        for (auto& k : *ptr_klist) {
          k.closePrice *= 0.1;
        }
      }
      data_->last_update_[kType] = Datetime::now();
    }
  }
}

KRecordList Stock::getKRecordListFromBuffer(KQuery::KType inkType) const {
  KRecordList result;
  HAYAKU_IF_RETURN(!data_, result);
  string kType(inkType);
  to_upper(kType);
  auto mutex_iter = data_->pMutex.find(kType);
  HAYAKU_IF_RETURN(mutex_iter == data_->pMutex.end(), result);
  std::shared_lock<std::shared_mutex> lock(*(mutex_iter->second));
  auto iter = data_->pKData.find(kType);
  HAYAKU_IF_RETURN(iter == data_->pKData.end() || !iter->second, result);
  result = *(iter->second);
  return result;
}

void Stock::loadKDataToBufferFromKRecordList(const KQuery::KType& inkType,
                                             KRecordList&& ks) const {
  HAYAKU_IF_RETURN(!data_ || !kdata_driver_, void());

  string kType(inkType);
  to_upper(kType);

  HAYAKU_IF_RETURN(data_->pMutex.find(kType) == data_->pMutex.end(), void());

  {
    std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[kType]));
    if (data_->pKData.find(kType) != data_->pKData.end() &&
        data_->pKData[kType]) {
      return;
    }
    KRecordList* ptr_klist = new KRecordList;
    (*ptr_klist) = std::move(ks);
    ptr_klist->shrink_to_fit();
    data_->pKData[kType] = ptr_klist;
    data_->last_update_[kType] = Datetime::now();
  }
}

StockWeightList Stock::getWeight(const Datetime& start,
                                 const Datetime& end) const {
  StockWeightList result;
  HAYAKU_IF_RETURN(!data_ || start >= end, result);
  // The readiness policy of the ex-rights/ex-dividend cache (materialization /
  // lazy loading fallback / re-reading after a release) differs by the process
  // role:
  // - The client mode: all the ex-rights/ex-dividend data has been materialized
  // at the startup according to the load_stock_weight config (see
  //   DataRuntime::loadAllStockWeights); here only the not materialized
  //   securities are handled on demand, covering the config being off
  //   (load_stock_weight=false, no preload at the startup) and the securities
  //   added by addStock or newly constructed that are out of the
  //   materialization scope, which are read as a whole through the driver
  //   (IpcBaseInfoDriver, shm first, falling back to IPC / local when not
  //   covered) and cached;
  // - The shm server role: after the basic snapshot with the finance is
  // published, DataRuntime::releaseShmServerBaseInfoCache()
  //   has released the local ex-rights/ex-dividend cache of every security (see
  //   the description of that method; the clients all read through the shared
  //   memory), and here the released securities are re-read lazily through the
  //   driver for self healing on demand (the IPC fallback reply and the API
  //   access in the same process), guaranteeing a correct result;
  // - The ordinary standalone mode of the main process: it reads the local
  // m_weightList directly with no lazy loading fallback (everything is
  // materialized at the startup). The concurrent lazy loading of a not ready
  // security may be duplicated, the results are the same and it is tolerable
  // (the same as the getHistoryFinance lazy loading). An empty result also sets
  // m_weight_ready, avoiding the repeated driver access of the securities
  // without the ex-rights/ex-dividend data on every query.
  DataRuntime& sm = getDataRuntime();
  auto lazy_load_weight = [this, &sm]() {
    // The caller must already hold the unique lock of m_weight_mutex
    StockWeightList full_list =
        sm.getStockWeightList(*this, Datetime::min(), Null<Datetime>());
    full_list.shrink_to_fit();
    data_->weight_list_.swap(full_list);
    data_->weight_ready_.store(true, std::memory_order_release);
  };
  if ((sm.isIpcClientMode() || sm.isBaseInfoCacheEvictionEnabled()) &&
      !data_->weight_ready_.load(std::memory_order_acquire)) {
    std::unique_lock<std::shared_mutex> unique_lock(data_->weight_mutex_);
    if (!data_->weight_ready_.load(std::memory_order_relaxed)) {
      lazy_load_weight();
    }
  }
  std::shared_lock<std::shared_mutex> lock(data_->weight_mutex_);
  // The release window concurrent with releaseShmServerBaseInfoCache(): a read
  // thread that passed the ready check above may find inside the read lock that
  // the cache has just been cleared and ready is set to false (a security
  // really without the data appears as a ready=true empty table and does not
  // enter this branch); it gives up the read lock, performs one more lazy
  // loading and queries again, avoiding an IPC fallback reply returning empty
  // ex-rights/ex-dividend data. The supplementary query is limited to the roles
  // with the lazy loading fallback (client / server); in the ordinary
  // standalone mode the existing semantics of "reading the materialized cache
  // only" are kept even if the cache is cleared (a release normally does not
  // happen in that mode)
  if (data_->weight_list_.empty() &&
      !data_->weight_ready_.load(std::memory_order_acquire) &&
      (sm.isIpcClientMode() || sm.isBaseInfoCacheEvictionEnabled())) {
    lock.unlock();
    std::unique_lock<std::shared_mutex> unique_lock(data_->weight_mutex_);
    if (!data_->weight_ready_.load(std::memory_order_relaxed)) {
      lazy_load_weight();
    }
    unique_lock.unlock();
    lock.lock();
  }
  StockWeightList::const_iterator start_iter, end_iter;
  start_iter =
      lower_bound(data_->weight_list_.begin(), data_->weight_list_.end(),
                  StockWeight(start), std::less<StockWeight>());
  if (start_iter == data_->weight_list_.end()) {
    return result;
  }

  end_iter = lower_bound(
      start_iter, (StockWeightList::const_iterator)data_->weight_list_.end(),
      StockWeight(end), std::less<StockWeight>());
  for (; start_iter != end_iter; ++start_iter) {
    result.push_back(*start_iter);
  }

  return result;
}

KData Stock::getKData(const KQuery& query) const { return KData(*this, query); }

size_t Stock::getCount(KQuery::KType ktype) const {
  HAYAKU_IF_RETURN(isNull(), 0);
  to_upper(ktype);

  if (KQuery::isBaseKType(ktype)) {
    if (isPreload(ktype) && !isBuffer(ktype)) {
      loadKDataToBuffer(ktype);
    }

    if (isBuffer(ktype)) {
      std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
      return data_->pKData[ktype]->size();
    }

    return kdata_driver_->getConnect()->getCount(market(), code(), ktype);
  }

  if (isExtraKType(ktype)) {
    return getStockExtraCount(*this, ktype);
  }

  HAYAKU_ERROR("Invalid ktype: {}", ktype);
  return 0;
}

price_t Stock::getMarketValue(const Datetime& datetime,
                              KQuery::KType ktype) const {
  HAYAKU_IF_RETURN(isNull(), 0.0);
  HAYAKU_IF_RETURN(!valid() && datetime > lastDatetime(), 0.0);

  to_upper(ktype);

  if (KQuery::isBaseKType(ktype)) {
    if (isPreload(ktype) && !isBuffer(ktype)) {
      loadKDataToBuffer(ktype);
    }

    // If it is an in-memory cache or the data driver prefers the index, acquire
    // it by index
    if (isBuffer(ktype)) {
      KQuery query = KQueryByDate(datetime, Null<Datetime>(), ktype);
      size_t out_start, out_end;

      if (_getIndexRangeFromBuffer(query, out_start, out_end)) {
        // The record found is >= datetime
        KRecord k = _getKRecordFromBuffer(out_start, ktype);
        if (k.datetime == datetime) {
          return k.closePrice;
        }
        if (out_start != 0) {
          k = _getKRecordFromBuffer(out_start - 1, ktype);
          return k.closePrice;
        }
      }

    } else if (kdata_driver_->getConnect()->isIndexFirst()) {
      KQuery query = KQueryByDate(datetime, Null<Datetime>(), ktype);
      size_t out_start, out_end;
      if (getIndexRange(query, out_start, out_end)) {
        // The record found is >= datetime
        KRecord k = getKRecord(out_start, ktype);
        if (k.datetime == datetime) {
          return k.closePrice;
        }
        if (out_start != 0) {
          k = getKRecord(out_start - 1, ktype);
          return k.closePrice;
        }
      }

    } else {
      // It is not in the cache and the date is preferred
      // Try to get the K-line data equal to that date first
      // If it is not found, get the last record earlier than that date
      KQuery query = KQueryByDate(datetime, datetime + Minutes(1), ktype);
      auto k_list = getKRecordList(query);
      if (k_list.size() > 0 && k_list[0].datetime == datetime) {
        return k_list[0].closePrice;
      }

      query = KQueryByDate(startDatetime(), datetime, ktype);
      k_list = getKRecordList(query);
      if (k_list.size() > 0) {
        return k_list[k_list.size() - 1].closePrice;
      }
    }

    // If it is not found, take the last record
    price_t price = 0.0;
    size_t total = getCount(ktype);
    if (total > 0) {
      price = getKRecord(total - 1, ktype).closePrice;
    }

    return price;
  }

  if (KQuery::isExtraKType(ktype)) {
    KQuery query = KQueryByDate(datetime, Null<Datetime>(), ktype);
    size_t out_start, out_end;
    if (getIndexRange(query, out_start, out_end)) {
      // The record found is >= datetime
      KRecord k = getKRecord(out_start, ktype);
      if (k.datetime == datetime) {
        return k.closePrice;
      }
      if (out_start != 0) {
        k = getKRecord(out_start - 1, ktype);
        return k.closePrice;
      }
    }

    // If it is not found, take the last record
    price_t price = 0.0;
    size_t total = getCount(ktype);
    if (total > 0) {
      price = getKRecord(total - 1, ktype).closePrice;
    }

    return price;
  }

  HAYAKU_ERROR("Invalid ktype: {}", ktype);
  return 0.0;
}

bool Stock::getIndexRange(const KQuery& query, size_t& out_start,
                          size_t& out_end) const {
  out_start = 0;
  out_end = 0;
  HAYAKU_IF_RETURN(!data_ || !kdata_driver_, false);

  if (KQuery::isBaseKType(query.kType())) {
    if (isPreload(query.kType()) && !isBuffer(query.kType())) {
      loadKDataToBuffer(query.kType());
    }

    if (isBuffer(query.kType())) {
      return _getIndexRangeFromBuffer(query, out_start, out_end);
    }

    if (KQuery::INDEX == query.queryType())
      return _getIndexRangeByIndex(query, out_start, out_end);

    if ((KQuery::DATE != query.queryType()) ||
        query.startDatetime() >= query.endDatetime())
      return false;

    if (!kdata_driver_->getConnect()->getIndexRangeByDate(
            data_->market_, data_->code_, query, out_start, out_end)) {
      out_start = 0;
      out_end = 0;
      return false;
    }

    return true;
  }

  if (KQuery::isExtraKType(query.kType())) {
    return getStockExtraIndexRange(*this, query, out_start, out_end);
  }

  HAYAKU_ERROR("Invalid query type: {}", query.kType());
  return false;
}

bool Stock::_getIndexRangeByIndex(const KQuery& query, size_t& out_start,
                                  size_t& out_end) const {
  assert(query.queryType() == KQuery::INDEX);
  out_start = 0;
  out_end = 0;

  size_t total = getCount(query.kType());
  HAYAKU_IF_RETURN(0 == total, false);

  int64_t startix, endix;
  startix = query.start();
  if (startix < 0) {
    startix += total;
    if (startix < 0) startix = 0;
  }

  endix = query.end();
  if (endix < 0) {
    endix += total;
    if (endix < 0) endix = 0;
  }

  size_t null_size_t = Null<size_t>();
  size_t startpos = 0;
  size_t endpos = null_size_t;

  try {
    startpos = boost::numeric_cast<size_t>(startix);
  } catch (...) {
    startpos = null_size_t;
  }

  try {
    endpos = boost::numeric_cast<size_t>(endix);
  } catch (...) {
    endpos = null_size_t;
  }

  if (endpos > total) {
    endpos = total;
  }

  if (startpos >= endpos) {
    return false;
  }

  out_start = startpos;
  out_end = endpos;
  return true;
}

bool Stock::_getIndexRangeFromBuffer(const KQuery& query, size_t& out_start,
                                     size_t& out_end) const {
  return query.queryType() == KQuery::INDEX
             ? _getIndexRangeByIndexFromBuffer(query, out_start, out_end)
             : _getIndexRangeByDateFromBuffer(query, out_start, out_end);
}

bool Stock::_getIndexRangeByIndexFromBuffer(const KQuery& query,
                                            size_t& out_start,
                                            size_t& out_end) const {
  assert(query.queryType() == KQuery::INDEX);
  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[query.kType()]));
  out_start = 0;
  out_end = 0;

  size_t total = data_->pKData[query.kType()]->size();
  HAYAKU_IF_RETURN(0 == total, false);

  int64_t startix, endix;
  startix = query.start();
  if (startix < 0) {
    startix += total;
    if (startix < 0) startix = 0;
  }

  endix = query.end();
  if (endix < 0) {
    endix += total;
    if (endix < 0) endix = 0;
  }

  size_t null_size_t = Null<size_t>();
  size_t startpos = 0;
  size_t endpos = null_size_t;

  try {
    startpos = boost::numeric_cast<size_t>(startix);
  } catch (...) {
    startpos = null_size_t;
  }

  try {
    endpos = boost::numeric_cast<size_t>(endix);
  } catch (...) {
    endpos = null_size_t;
  }

  if (endpos > total) {
    endpos = total;
  }

  if (startpos >= endpos) {
    return false;
  }

  out_start = startpos;
  out_end = endpos;
  return true;
}

bool Stock::_getIndexRangeByDateFromBuffer(const KQuery& query,
                                           size_t& out_start,
                                           size_t& out_end) const {
  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[query.kType()]));
  out_start = 0;
  out_end = 0;

  const KRecordList& kdata = *(data_->pKData[query.kType()]);
  size_t total = kdata.size();
  HAYAKU_IF_RETURN(0 == total, false);

  size_t mid = total, low = 0, high = total - 1;
  size_t startpos, endpos;
  while (low <= high) {
    if (query.startDatetime() > kdata[high].datetime) {
      mid = high + 1;
      break;
    }

    if (kdata[low].datetime >= query.startDatetime()) {
      mid = low;
      break;
    }

    mid = (low + high) / 2;
    if (query.startDatetime() > kdata[mid].datetime) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  if (mid >= total) {
    return false;
  }

  startpos = mid;

  low = mid;
  high = total - 1;
  while (low <= high) {
    if (query.endDatetime() > kdata[high].datetime) {
      mid = high + 1;
      break;
    }

    if (kdata[low].datetime >= query.endDatetime()) {
      mid = low;
      break;
    }

    mid = (low + high) / 2;
    if (query.endDatetime() > kdata[mid].datetime) {
      low = mid + 1;
    } else {
      high = mid - 1;
    }
  }

  endpos = (mid >= total) ? total : mid;
  if (startpos >= endpos) {
    return false;
  }

  out_start = startpos;
  out_end = endpos;

  return true;
}

const KRecord& Stock::_getKRecordFromBuffer(size_t pos,
                                            const KQuery::KType& ktype) const {
  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  const auto& buf = *(data_->pKData[ktype]);
  return pos >= buf.size() ? KRecord::NullKRecord : buf[pos];
}

KRecord Stock::getKRecord(size_t pos, const KQuery::KType& kType) const {
  HAYAKU_IF_RETURN(!data_, KRecord::NullKRecord);

  if (KQuery::isBaseKType(kType)) {
    if (isPreload(kType) && !isBuffer(kType)) {
      loadKDataToBuffer(kType);
    }

    if (isBuffer(kType)) {
      return _getKRecordFromBuffer(pos, kType);
    }

    HAYAKU_IF_RETURN(!kdata_driver_ || pos >= size_t(Null<int64_t>()),
                     KRecord::NullKRecord);
    auto klist = kdata_driver_->getConnect()->getKRecordList(
        market(), code(), KQuery(pos, pos + 1, kType));
    if (klist.size() > 0) {
      if ((kType == KQuery::TIMELINE || kType == KQuery::TRANS) &&
          (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
           type() == STOCKTYPE_B)) {
        klist[0].closePrice *= 0.1;
      }
      return klist[0];
    }
    return KRecord::NullKRecord;
  }

  if (KQuery::isExtraKType(kType)) {
    auto ks = getExtraKRecordList(*this, KQueryByIndex(pos, pos + 1, kType));
    return ks.empty() ? KRecord::NullKRecord : ks[0];
  }

  HAYAKU_ERROR("Invalid KType: {}", kType);
  return KRecord::NullKRecord;
}

KRecord Stock::getKRecord(const Datetime& datetime,
                          const KQuery::KType& ktype) const {
  KRecord result;
  HAYAKU_IF_RETURN(isNull(), result);

  if (KQuery::isBaseKType(ktype)) {
    if (isPreload(ktype) && !isBuffer(ktype)) {
      loadKDataToBuffer(ktype);
    }

    KQuery query = KQueryByDate(datetime, datetime + Minutes(1), ktype);
    auto driver = kdata_driver_->getConnect();
    if (isBuffer(query.kType()) || driver->isIndexFirst()) {
      size_t startix = 0, endix = 0;
      return getIndexRange(query, startix, endix) ? getKRecord(startix, ktype)
                                                  : KRecord::NullKRecord;
    }

    auto klist = driver->getKRecordList(market(), code(), query);
    if (klist.size() > 0) {
      if ((ktype == KQuery::TIMELINE || ktype == KQuery::TRANS) &&
          (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
           type() == STOCKTYPE_B)) {
        klist[0].closePrice *= 0.1;
      }
      return klist[0];
    }
    return KRecord::NullKRecord;
  }

  if (KQuery::isExtraKType(ktype)) {
    auto ks = getExtraKRecordList(
        *this, KQueryByDate(datetime, datetime + Minutes(1), ktype));
    return ks.empty() ? KRecord::NullKRecord : ks[0];
  }

  HAYAKU_ERROR("Invalid ktype: {}", ktype);
  return KRecord::NullKRecord;
}

KRecordList Stock::_getKRecordListFromBuffer(size_t start_ix, size_t end_ix,
                                             KQuery::KType ktype) const {
  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  KRecordList result;
  size_t total = data_->pKData[ktype]->size();
  HAYAKU_IF_RETURN(total == 0, result);
  HAYAKU_WARN_IF_RETURN(
      start_ix >= end_ix || start_ix >= total, result,
      "Invalid param (start_ix: {}, end_ix: {})! current total: {} | {} | {}",
      start_ix, end_ix, total, name(), ktype);
  size_t length = end_ix > total ? total - start_ix : end_ix - start_ix;
  result.resize(length);
  memcpy((void*)&(result.front()), &((*data_->pKData[ktype])[start_ix]),
         sizeof(KRecord) * length);
  return result;
}

KRecordList Stock::getKRecordList(const KQuery& query) const {
  KRecordList result;
  if (KQuery::isBaseKType(query.kType())) {
    result = _getKRecordList(query);
  } else if (KQuery::isExtraKType(query.kType())) {
    result = getExtraKRecordList(*this, query);
  } else {
    HAYAKU_ERROR("Invalid ktype: {}!", query.kType());
  }
  return result;
}

KRecordList Stock::_getKRecordList(const KQuery& query) const {
  KRecordList result;
  HAYAKU_IF_RETURN(isNull(), result);

  if (isPreload(query.kType()) && !isBuffer(query.kType())) {
    loadKDataToBuffer(query.kType());
  }

  // If it is in the in-memory cache
  if (isBuffer(query.kType())) {
    size_t start_ix = 0, end_ix = 0;
    if (query.queryType() == KQuery::DATE) {
      if (!_getIndexRangeByDateFromBuffer(query, start_ix, end_ix)) {
        return result;
      }
    } else {
      if (query.start() < 0 || query.end() < 0) {
        // Handle a negative index
        if (!getIndexRange(query, start_ix, end_ix)) {
          return result;
        }
      } else {
        start_ix = query.start();
        end_ix = query.end();
      }
    }
    result = _getKRecordListFromBuffer(start_ix, end_ix, query.kType());

  } else {
    if (query.queryType() == KQuery::DATE) {
      result = kdata_driver_->getConnect()->getKRecordList(
          data_->market_, data_->code_, query);
    } else {
      size_t start_ix = 0, end_ix = 0;
      if (query.queryType() == KQuery::INDEX) {
        if (query.start() < 0 || query.end() < 0) {
          // Handle a negative index
          if (!getIndexRange(query, start_ix, end_ix)) {
            return result;
          }
        } else {
          start_ix = query.start();
          end_ix = query.end();
        }
      }
      result = kdata_driver_->getConnect()->getKRecordList(
          data_->market_, data_->code_,
          KQuery(start_ix, end_ix, query.kType()));
    }
    result.shrink_to_fit();
    if ((query.kType() == KQuery::TIMELINE || query.kType() == KQuery::TRANS) &&
        (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
         type() == STOCKTYPE_B)) {
      for (auto& k : result) {
        k.closePrice *= 0.1;
      }
    }
  }

  return result;
}

DatetimeList Stock::getDatetimeList(const KQuery& query) const {
  DatetimeList result;
  KRecordList k_list = getKRecordList(query);
  result.reserve(k_list.size());
  for (const auto& k : k_list) {
    result.push_back(k.datetime);  // cppcheck-suppress useStlAlgorithm
  }
  return result;
}

TimeLineList Stock::getTimeLineList(const KQuery& query) const {
  TimeLineList result;
  HAYAKU_IF_RETURN(!kdata_driver_, result);
  result =
      kdata_driver_->getConnect()->getTimeLineList(market(), code(), query);
  if (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
      type() == STOCKTYPE_B) {
    for (auto& tl : result) {
      tl.price *= 0.1;
    }
  }
  return result;
}

TransList Stock::getTransList(const KQuery& query) const {
  TransList result;
  HAYAKU_IF_RETURN(!kdata_driver_, result);
  result = kdata_driver_->getConnect()->getTransList(market(), code(), query);
  if (type() == STOCKTYPE_ETF || type() == STOCKTYPE_FUND ||
      type() == STOCKTYPE_B) {
    for (auto& t : result) {
      t.price *= 0.1;
    }
  }
  return result;
}

Parameter Stock::getFinanceInfo() const {
  Parameter result;
  HAYAKU_IF_RETURN(type() != STOCKTYPE_A && type() != STOCKTYPE_GEM &&
                       type() != STOCKTYPE_START && type() != STOCKTYPE_A_BJ,
                   result);

  BaseInfoDriverPtr driver = getDataRuntime().getBaseInfoDriver();
  if (driver) {
    result = driver->getFinanceInfo(market(), code());
  }

  return result;
}

vector<Block> Stock::getBelongToBlockList(const string& category) const {
  return getDataRuntime().getStockBelongs(*this, category);
}

// Judge whether it is within the trading time range (the date is not judged)
bool Stock::isTransactionTime(Datetime time) {
  MarketInfo market_info = getDataRuntime().getMarketInfo(market());
  HAYAKU_IF_RETURN(market_info == Null<MarketInfo>(), false);

  HAYAKU_ERROR_IF_RETURN(market_info.openTime1() > market_info.closeTime1() ||
                             market_info.openTime2() > market_info.closeTime2(),
                         false, "Error transaction time in market({})!",
                         market_info.market());

  // When the start and the end time of the trading time range are equal it is
  // regarded as unlimited and the whole day is tradable
  HAYAKU_IF_RETURN(market_info.openTime1() == market_info.closeTime1() &&
                       market_info.openTime2() == market_info.closeTime2(),
                   true);

  Datetime today = Datetime::today();
  Datetime openTime1 = today + market_info.openTime1();
  Datetime closeTime1 = today + market_info.closeTime1();
  // The tick time of the last day of some market data may be delayed by a few
  // seconds, so a margin is added
  HAYAKU_IF_RETURN(time >= openTime1 && time <= closeTime1 + Seconds(30), true);

  Datetime openTime2 = today + market_info.openTime2();
  Datetime closeTime2 = today + market_info.closeTime2();
  return time >= openTime2 && time <= closeTime2 + Seconds(30);
}

void Stock::realtimeUpdate(KRecord record, const KQuery::KType& inktype) {
  // The client mode without a local buffer (an ordinary proxy security): it is
  // forwarded to the main process to apply (updating its buffer and mirroring
  // to the shared memory segment, from which all the clients read), keeping the
  // ability of the client to update the market data actively; it falls back to
  // the original behavior (silently ignored) when no forwarding connection is
  // registered or the forwarding fails. Conversely, if there is already a local
  // buffer (such as a temporary security with external data given by
  // setKRecordList, isBuffer is true), that data is visible to this client only
  // and the main process does not have this security, so the local buffer must
  // be updated in place without sending it out; therefore the !isBuffer gate is
  // used (it falls to the local logic below; the client has no publisher and
  // shmMirror is a safe no-op).
  if (getDataRuntime().isIpcClientMode() && !isBuffer(inktype)) {
    HAYAKU_IF_RETURN(
        record.datetime.isNull() || getDataRuntime().isHoliday(record.datetime),
        void());
    ipc::forwardRealtimeUpdate(market_code(), inktype, record);
    return;
  }
  HAYAKU_IF_RETURN(!isBuffer(inktype) || record.datetime.isNull() ||
                       getDataRuntime().isHoliday(record.datetime),
                   void());

  string ktype(inktype);
  to_upper(ktype);

  // Acquire the write lock
  std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));

  // A second check of the caching is needed, to prevent the cache from being
  // released before the lock
  if (data_->pKData.find(ktype) == data_->pKData.end() ||
      !data_->pKData[ktype]) {
    return;
  }

  if (data_->pKData[ktype]->empty()) {
    data_->pKData[ktype]->push_back(record);
    // Mirrored to the shared memory segment (silently skipped when this
    // security / type is not published); called inside the write lock of the
    // security x ktype, guaranteeing the single writer serialization within the
    // segment
    ipc::shmMirrorRealtimeUpdate(market_code(), inktype, record);
    return;
  }

  KRecord& tmp = data_->pKData[ktype]->back();

  // When the date of the passed record equals the date of the last record,
  // update the last record; otherwise append it to the cache
  if (tmp.datetime == record.datetime) {
    if (tmp.highPrice < record.highPrice) {
      tmp.highPrice = record.highPrice;
    }
    if (tmp.lowPrice > record.lowPrice) {
      tmp.lowPrice = record.lowPrice;
    }
    tmp.closePrice = record.closePrice;
    tmp.transAmount = record.transAmount;
    tmp.transCount = record.transCount;
    data_->last_update_[ktype] = Datetime::now();

  } else if (tmp.datetime < record.datetime) {
    data_->pKData[ktype]->push_back(record);
    data_->last_update_[ktype] = Datetime::now();

  } else {
    HAYAKU_DEBUG(
        "Ignore record, datetime({}) < last record.datetime({})! {} {}",
        record.datetime, tmp.datetime, market_code(), inktype);
  }

  // Mirrored to the shared memory segment (silently skipped when it is not
  // published; an expired record is ignored by the mirror rule as well), so the
  // client processes read the near realtime data without an IPC round trip
  ipc::shmMirrorRealtimeUpdate(market_code(), inktype, record);
}

Datetime Stock::getLastUpdateTime(const KQuery::KType& inktype) const {
  auto ktype = inktype;
  to_upper(ktype);
  // The client mode without a local buffer (an ordinary proxy security):
  // m_lastUpdate is always Datetime::min(), so it is forwarded to the main
  // process to get its buffer refresh moment, staying consistent with the data
  // the client reads through the shared memory (when no forwarding connection
  // is registered / it fails it degrades to returning min()). If there is
  // already a local buffer (such as a temporary security given by
  // setKRecordList), m_lastUpdate is written locally and the local value should
  // be returned directly instead of forwarding, so the !isBuffer gate is used.
  if (getDataRuntime().isIpcClientMode() && !isBuffer(ktype)) {
    return ipc::forwardGetLastUpdateTime(market_code(), ktype);
  }
  if (data_->pMutex.find(ktype) == data_->pMutex.end()) {
    auto iter = data_->last_update_.find(ktype);
    if (iter == data_->last_update_.end()) {
      // The possibly newly added extended K-line type
      if (KQuery::isExtraKType(ktype)) {
        data_->last_update_[ktype] = Datetime::min();
      }
      return Datetime::min();
    }
    return iter->second;
  }

  std::shared_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  auto iter = data_->last_update_.find(ktype);
  return iter == data_->last_update_.end() ? Datetime::min() : iter->second;
}

void Stock::setKRecordList(const KRecordList& ks, const KQuery::KType& ktype) {
  HAYAKU_CHECK(
      isNull(),
      "The stock is Null, can't set kdata! Please create a stock using the "
      "format Stock(market, "
      "code, name)! Calling Stock() will create a special null instance.");

  HAYAKU_IF_RETURN(ks.empty(), void());
  string nktype(ktype);
  to_upper(nktype);

  // Write lock
  std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  HAYAKU_CHECK(data_->pKData.find(nktype) != data_->pKData.end(),
               "Invalid ktype: {}", ktype);

  if (!data_->pKData[nktype]) {
    data_->pKData[nktype] = new KRecordList();
  }

  (*(data_->pKData[nktype])) = ks;
  data_->last_update_[nktype] = Datetime::now();

  Parameter param;
  param.set<string>("type", "DoNothing");
  kdata_driver_ = DataDriverFactory::getKDataDriverPool(param);

  data_->valid_ = true;
  data_->start_date_ = ks.front().datetime;
  data_->last_date_ = ks.back().datetime;
}

void Stock::setKRecordList(KRecordList&& ks, const KQuery::KType& ktype) {
  HAYAKU_CHECK(
      isNull(),
      "The stock is Null, can't set kdata! Please create a stock using the "
      "format Stock(market, "
      "code, name)! Calling Stock() will create a special null instance.");

  HAYAKU_IF_RETURN(ks.empty(), void());
  string nktype(ktype);
  to_upper(nktype);

  // Write lock
  std::unique_lock<std::shared_mutex> lock(*(data_->pMutex[ktype]));
  HAYAKU_CHECK(data_->pKData.find(nktype) != data_->pKData.end(),
               "Invalid ktype: {}", ktype);

  if (!data_->pKData[nktype]) {
    data_->pKData[nktype] = new KRecordList();
  }

  (*data_->pKData[nktype]) = std::move(ks);
  data_->last_update_[nktype] = Datetime::now();

  Parameter param;
  param.set<string>("type", "DoNothing");
  kdata_driver_ = DataDriverFactory::getKDataDriverPool(param);

  data_->valid_ = true;
  data_->start_date_ = (*data_->pKData[nktype]).front().datetime;
  data_->last_date_ = (*data_->pKData[nktype]).back().datetime;
}

vector<HistoryFinanceInfo> Stock::getHistoryFinance() const {
  HAYAKU_ASSERT(data_);
  // The historical finance reading policy:
  // - The client mode: the historical finance is not materialized locally (see
  // the early return of the client mode in DataRuntime::loadAllKData) and it is
  // read on demand
  //   through the driver from the shared memory snapshot published by the main
  //   process (IpcBaseInfoDriver prefers shm and falls back to IPC / local when
  //   not covered), avoiding a duplicated occupation of the client memory with
  //   the snapshot, and every call reflects the current cache of the main
  //   process;
  // - The shm server role: after the basic snapshot with the finance is
  // published, DataRuntime::releaseShmServerBaseInfoCache()
  //   has released the local historical finance cache (the clients all read
  //   through the shared memory and the server does not need to keep a copy); a
  //   released security is re-read lazily for self healing on the next access
  //   (the IPC fallback reply and the API access in the same process);
  // - The ordinary main process mode: the materialized cache is preloaded at
  // the startup and its copy is returned. getHistoryFinance is not a per-K-line
  // hot path (it is called once per indicator / query), so the on-demand shm
  // read and decoding cost is negligible. The return type must be by value: the
  // client mode has no local cache and cannot return a reference.
  DataRuntime& sm = getDataRuntime();
  if (sm.isIpcClientMode()) {
    return sm.getHistoryFinance(*this, Datetime::min(), Null<Datetime>());
  }
  auto lazy_load_finance = [this, &sm]() {
    // The caller must already hold the unique lock of m_history_finance_mutex
    data_->history_finance_ =
        sm.getHistoryFinance(*this, Datetime::min(), Null<Datetime>());
    data_->history_finance_.shrink_to_fit();
    data_->history_finance_ready_ = true;
  };
  if (!data_->history_finance_ready_) {
    // At present m_history_finance_ready and m_history_finance_mutex are
    // separate, so in parallel a short period may cause multiple acquisitions,
    // which is tolerable
    std::unique_lock<std::shared_mutex> lock(data_->history_finance_mutex_);
    if (!data_->history_finance_ready_) {
      lazy_load_finance();
    }
  }
  std::shared_lock<std::shared_mutex> lock(data_->history_finance_mutex_);
  // The release window concurrent with releaseShmServerBaseInfoCache(): the
  // description is the same as Stock::getWeight
  if (data_->history_finance_.empty() && !data_->history_finance_ready_) {
    lock.unlock();
    std::unique_lock<std::shared_mutex> unique_lock(
        data_->history_finance_mutex_);
    if (!data_->history_finance_ready_) {
      lazy_load_finance();
    }
    unique_lock.unlock();
    lock.lock();
  }
  return data_->history_finance_;
}

void Stock::setHistoryFinance(vector<HistoryFinanceInfo>&& history_finance) {
  HAYAKU_IF_RETURN(!data_, void());
  // The client mode: the historical finance is not materialized locally
  // (symmetric with the client branch of getHistoryFinance(); the local cache
  // is always empty and the reading side queries shm/IPC through the baseInfo
  // driver uniformly); the batch import result is discarded directly, avoiding
  // a duplicated occupation of the client memory with the snapshot. In the
  // normal flow the client preload (loadAllKData) returns early and would not
  // reach here at all; this is the foolproof invariant at the Stock layer: this
  // function is the only batch writing entry of the finance cache (the
  // column-oriented getAllHistoryFinance, see design §11.23①) and no future
  // call path (such as reload or a plugin) may recreate a local copy in the
  // client mode.
  if (getDataRuntime().isIpcClientMode()) {
    return;
  }
  // Deduplicate by (reportDate, fileDate): the batch getAllHistoryFinance of
  // the column-oriented driver may return duplicate rows for the same key (in
  // the real test ClickHouse returned 124 rows in a batch for a security versus
  // 123 rows by a direct per-security query, and the keys unique to the cache
  // were empty, i.e. a pure multiplicity duplication). The deduplication makes
  // the cache consistent with the other three paths (the direct per-security
  // query, the SHM snapshot and the IPC fallback); semantically every
  // (reportDate, fileDate) should be unique, so the first occurrence is kept
  // (remove_if is stable and keeps the original relative order; the sorting is
  // the responsibility of the publishing side)
  std::set<std::pair<uint64_t, uint64_t>> seen;
  history_finance.erase(
      std::remove_if(history_finance.begin(), history_finance.end(),
                     [&seen](const HistoryFinanceInfo& r) {
                       return !seen.emplace(r.reportDate.number(),
                                            r.fileDate.number())
                                   .second;
                     }),
      history_finance.end());
  history_finance.shrink_to_fit();
  if (!data_->history_finance_ready_) {
    std::unique_lock<std::shared_mutex> lock(data_->history_finance_mutex_);
    data_->history_finance_ = std::move(history_finance);
    data_->history_finance_ready_ = true;
  }
}

DatetimeList Stock::getTradingCalendar(const KQuery& query) const {
  return getDataRuntime().getTradingCalendar(query, market());
}

Stock HAYAKU_API getStock(const string& querystr) {
  const DataRuntime& sm = getDataRuntime();
  return sm.getStock(querystr);
}

}  // namespace hayaku
