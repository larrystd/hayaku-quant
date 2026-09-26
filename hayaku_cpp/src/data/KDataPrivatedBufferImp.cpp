/*
 * KDataPrivatedBufferImp.cpp
 *
 *  Created on: 2013-2-4
 *      Author: fasiondog
 */

#include "KDataPrivatedBufferImp.h"
#include "KData.h"

#include <functional>

namespace hayaku {

KDataPrivatedBufferImp::KDataPrivatedBufferImp() : KDataImp() {}

KDataPrivatedBufferImp::KDataPrivatedBufferImp(const Stock& stock,
                                               const KQuery& query)
    : KDataImp(stock, query), buffer_(stock_.getKRecordList(query)) {
  _recover();
}

KDataPrivatedBufferImp::KDataPrivatedBufferImp(const Stock& stock,
                                               const KQuery& query,
                                               const KRecordList& krecords)
    : KDataImp(stock, query), buffer_(krecords) {}

KDataPrivatedBufferImp::~KDataPrivatedBufferImp() {}

DatetimeList KDataPrivatedBufferImp::getDatetimeList() const {
  DatetimeList result;
  result.reserve(buffer_.size());
  for (const auto& record : buffer_) {
    result.emplace_back(record.datetime);
  }
  return result;
}

size_t KDataPrivatedBufferImp::startPos() const {
  if (!have_pos_in_stock_) {
    _getPosInStock();
  }
  return start_;
}

size_t KDataPrivatedBufferImp::endPos() const {
  if (!have_pos_in_stock_) {
    _getPosInStock();
  }
  return end_;
}

size_t KDataPrivatedBufferImp::lastPos() const {
  if (!have_pos_in_stock_) {
    _getPosInStock();
  }
  return end_ == 0 ? 0 : end_ - 1;
}

void KDataPrivatedBufferImp::_getPosInStock() const {
  bool sucess = stock_.getIndexRange(query_, start_, end_);
  if (!sucess) {
    start_ = 0;
    end_ = 0;
  }
  have_pos_in_stock_ = true;
}

size_t KDataPrivatedBufferImp::getPos(const Datetime& datetime) const noexcept {
  KRecordList::const_iterator iter;
  KRecord comp_record;
  comp_record.datetime = datetime;
  iter = lower_bound(
      buffer_.cbegin(), buffer_.cend(), comp_record,
      std::bind(std::less<Datetime>(),
                std::bind(&KRecord::datetime, std::placeholders::_1),
                std::bind(&KRecord::datetime, std::placeholders::_2)));
  if (iter == buffer_.cend() || iter->datetime != datetime) {
    return Null<size_t>();
  }

  return (iter - buffer_.cbegin());
}

void KDataPrivatedBufferImp::_recover() {
  // Return directly when the adjustment is not supported
  if (buffer_.empty() || query_.recoverType() == KQuery::NO_RECOVER) return;

  // The adjustment handling for the daily line and above
  int64_t secs = KQuery::getKTypeInSeconds(query_.kType());
  if (secs > KQuery::getKTypeInSeconds(KQuery::DAY)) {
    _recoverForUpDay();
    return;
  }

  switch (query_.recoverType()) {
    case KQuery::NO_RECOVER:
      // do nothing
      break;

    case KQuery::FORWARD:
      _recoverForward();
      break;

    case KQuery::BACKWARD:
      _recoverBackward();
      break;

    case KQuery::EQUAL_FORWARD:
      _recoverEqualForward();
      break;

    case KQuery::EQUAL_BACKWARD:
      _recoverEqualBackward();
      break;

    default:
      HAYAKU_ERROR("Invalid RecvoerType!");
      return;
  }
}

void KDataPrivatedBufferImp::_recoverForUpDay() {
  HAYAKU_IF_RETURN(buffer_.empty(), void());
  std::function<Datetime(const Datetime&)> startOfPhase;
  if (query_.kType() == KQuery::WEEK) {
    startOfPhase = &Datetime::startOfWeek;
  } else if (query_.kType() == KQuery::MONTH) {
    startOfPhase = &Datetime::startOfMonth;
  } else if (query_.kType() == KQuery::QUARTER) {
    startOfPhase = &Datetime::startOfQuarter;
  } else if (query_.kType() == KQuery::HALFYEAR) {
    startOfPhase = &Datetime::startOfHalfyear;
  } else if (query_.kType() == KQuery::YEAR) {
    startOfPhase = &Datetime::startOfYear;
  }

  Datetime startDate = startOfPhase(buffer_.front().datetime);
  Datetime endDate = buffer_.back().datetime.nextDay();
  KQuery query =
      KQueryByDate(startDate, endDate, KQuery::DAY, query_.recoverType());
  KData day_list = stock_.getKData(query);
  if (day_list.empty()) return;

  size_t day_pos = 0;
  size_t day_total = day_list.size();
  size_t length = buffer_.size();
  for (size_t i = 0; i < length; i++) {
    Datetime phase_start_date = startOfPhase(buffer_[i].datetime);
    Datetime phase_end_date = buffer_[i].datetime;
    if (day_pos >= day_total) break;

    while (day_list[day_pos].datetime < phase_start_date) {
      day_pos++;
    }
    KRecord record = day_list[day_pos];
    int pre_day_pos = day_pos;
    while (day_pos < day_total &&
           day_list[day_pos].datetime <= phase_end_date) {
      if (day_list[day_pos].lowPrice < record.lowPrice) {
        record.lowPrice = day_list[day_pos].lowPrice;
      } else if (day_list[day_pos].highPrice > record.highPrice) {
        record.highPrice = day_list[day_pos].highPrice;
      }
      record.closePrice = day_list[day_pos].closePrice;
      record.transCount += day_list[day_pos].transCount;
      record.transAmount += day_list[day_pos].transAmount;
      day_pos++;
    }
    if (pre_day_pos != day_pos) {
      buffer_[i].openPrice = record.openPrice;
      buffer_[i].highPrice = record.highPrice;
      buffer_[i].lowPrice = record.lowPrice;
      buffer_[i].closePrice = record.closePrice;
      buffer_[i].transCount = record.transCount;
      buffer_[i].transAmount = record.transAmount;
    }
  }

  return;
}

/******************************************************************************
 * The forward adjustment formula: the adjusted price = [(the pre-adjustment
 *price - the cash dividend) + the rights (new) share price x the change ratio
 *of the outstanding shares] / (1 + the change ratio of the outstanding shares)
 *The forward adjustment takes the price after the ex-rights as the base (i.e.
 *the price after the ex-rights stays unchanged) and lowers the prices before
 *the ex-rights. In the adjustment calculation it starts from the listing date
 *and goes forward day by day; when an ex-rights date is met, all the prices
 *between the listing date and the ex-rights date (excluding the ex-rights date)
 *are lowered by the adjustment calculation; then it continues forward, and when
 *the next ex-rights date is met, the prices between the listing date and that
 *ex-rights date (excluding the ex-rights date) are lowered by the adjustment
 * calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverForward() {
  size_t total = buffer_.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Datetime start_date(buffer_.front().datetime.startOfDay());
  Datetime end_date(buffer_.back().datetime + query_.kTypeInSeconds());
  StockWeightList weightList = stock_.getWeight(start_date, end_date);
  StockWeightList::const_iterator weightIter = weightList.begin();

  size_t pre_pos = 0;
  for (; weightIter != weightList.end(); ++weightIter) {
    // Calculate the change ratio of the outstanding shares; the case where only
    // the outstanding share capital changes is not handled
    if ((weightIter->countAsGift() == 0.0 &&
         weightIter->countForSell() == 0.0 &&
         weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
         weightIter->increasement() == 0.0 && weightIter->suogu() == 0.0))
      continue;

    size_t i = pre_pos;
    while (i < total && buffer_[i].datetime < weightIter->datetime()) {
      i++;
    }
    pre_pos = i;  // The ex-rights date

    price_t denominator = 0.0, temp = 0.0;
    if (weightIter->suogu() != 0.0) {
      denominator = weightIter->suogu();
    } else {
      price_t change =
          0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                 weightIter->increasement());
      // A change less than 0 means a share contraction
      denominator = 1.0 + change;  // The denominator = (1 + the change ratio of
                                   // the outstanding shares)
      temp = weightIter->priceForSell() * change - 0.1 * weightIter->bonus();
    }

    if (denominator == 1.0 && temp == 0.0) continue;

    price_t volume_k = 1.0 / denominator;

    for (i = 0; i < pre_pos; ++i) {
      buffer_[i].openPrice = (buffer_[i].openPrice + temp) / denominator;
      buffer_[i].highPrice = (buffer_[i].highPrice + temp) / denominator;
      buffer_[i].lowPrice = (buffer_[i].lowPrice + temp) / denominator;
      buffer_[i].closePrice = (buffer_[i].closePrice + temp) / denominator;
      buffer_[i].transCount = buffer_[i].transCount * volume_k;
      buffer_[i].transAmount = buffer_[i].closePrice * buffer_[i].transCount;
    }
  }
}

/******************************************************************************
 * The backward adjustment formula: the adjusted price = the pre-adjustment
 *price x (1 + the change ratio of the outstanding shares) - the rights (new)
 *share price x the change ratio of the outstanding shares + the cash dividend
 *The backward adjustment takes the price before the ex-rights as the base (i.e.
 *the price before the ex-rights stays unchanged) and raises the prices after
 *the ex-rights. In the adjustment calculation it starts from the latest date
 *and goes backward day by day; when an ex-rights date is met, all the prices
 *between the ex-rights date and the latest date (including the ex-rights date)
 *are raised by the adjustment calculation; then it continues backward, and when
 *the next ex-rights date is met, the prices between that ex-rights date and the
 *latest date (including the ex-rights date) are raised by the adjustment
 *calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverBackward() {
  size_t total = buffer_.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Datetime start_date(buffer_.front().datetime.startOfDay());
  Datetime end_date(buffer_.back().datetime + query_.kTypeInSeconds());
  StockWeightList weightList = stock_.getWeight(start_date, end_date);
  StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();

  size_t pre_pos = total - 1;
  for (; weightIter != weightList.rend(); ++weightIter) {
    // Calculate the change ratio of the outstanding shares; the case where only
    // the outstanding share capital changes is not handled
    if ((weightIter->countAsGift() == 0.0 &&
         weightIter->countForSell() == 0.0 &&
         weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
         weightIter->increasement() == 0.0 && weightIter->suogu() == 0.0))
      continue;

    size_t i = pre_pos;
    while (i > 0 && buffer_[i].datetime > weightIter->datetime()) {
      i--;
    }

    // For the minute data the first time point needs to be skipped
    if (i != pre_pos &&
        buffer_[i].datetime != buffer_[i].datetime.startOfDay()) {
      i++;
    }

    pre_pos = i;

    price_t denominator = 1.0, temp = 0.0;
    if (weightIter->suogu() != 0.0) {
      denominator = weightIter->suogu();
    } else {
      // The change ratio of the outstanding shares
      price_t change =
          0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                 weightIter->increasement());
      // A change less than 0 means a share contraction
      denominator =
          1.0 + change;  // (1 + the change ratio of the outstanding shares)
      temp = 0.1 * weightIter->bonus() - weightIter->priceForSell() * change;
    }

    if (denominator == 1.0 && temp == 0.0) continue;

    price_t volume_multiplier =
        1.0 / denominator;  // The volume adjustment multiplier

    for (i = pre_pos; i < total; ++i) {
      buffer_[i].openPrice = buffer_[i].openPrice * denominator + temp;
      buffer_[i].highPrice = buffer_[i].highPrice * denominator + temp;
      buffer_[i].lowPrice = buffer_[i].lowPrice * denominator + temp;
      buffer_[i].closePrice = buffer_[i].closePrice * denominator + temp;
      buffer_[i].transCount = buffer_[i].transCount * volume_multiplier;
      buffer_[i].transAmount = buffer_[i].closePrice * buffer_[i].transCount;
    }
  }
}

/******************************************************************************
 * The proportional forward adjustment formula: the adjusted price = the
 *pre-adjustment price * the adjustment ratio the adjustment ratio = {[(the
 *close price of the record date - the cash dividend) + the rights (new) share
 *price x the change ratio of the outstanding shares] / (1 + the change ratio of
 *the outstanding shares)} / the close price of the record date The forward
 * adjustment takes the price after the ex-rights as the base (i.e. the price
 *after the ex-rights stays unchanged) and lowers the prices before the
 *ex-rights. In the adjustment calculation it starts from the listing date and
 *goes forward day by day; when an ex-rights date is met, all the prices between
 *the listing date and the ex-rights date (excluding the ex-rights date) are
 *lowered by the adjustment calculation; then it continues forward, and when the
 *next ex-rights date is met, the prices between the listing date and that
 *ex-rights date (excluding the ex-rights date) are lowered by the adjustment
 *calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverEqualForward() {
  size_t total = buffer_.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Datetime start_date(buffer_.front().datetime.startOfDay());
  Datetime end_date(buffer_.back().datetime + query_.kTypeInSeconds());
  StockWeightList weightList = stock_.getWeight(start_date, end_date);
  if (weightList.empty()) {
    return;
  }

  KRecordList kdata =
      buffer_;  // Prevent two ex-rights/ex-dividend records on the same day
  StockWeightList::const_iterator weightIter = weightList.begin();
  size_t pre_pos = 0;
  for (; weightIter != weightList.end(); ++weightIter) {
    // Calculate the change ratio of the outstanding shares; the case where only
    // the outstanding share capital changes is not handled
    if ((weightIter->countAsGift() == 0.0 &&
         weightIter->countForSell() == 0.0 &&
         weightIter->priceForSell() == 0.0 && weightIter->bonus() == 0.0 &&
         weightIter->increasement() == 0.0 && weightIter->suogu() == 0.0))
      continue;

    size_t i = pre_pos;
    while (i < total && buffer_[i].datetime < weightIter->datetime()) {
      i++;
    }
    pre_pos = i;  // The ex-rights date

    // The close price of the record date (i.e. the data of the day before the
    // ex-rights date)
    if (pre_pos == 0) {
      continue;
    }
    price_t closePrice = kdata[pre_pos - 1].closePrice;
    if (closePrice == 0.0) {
      continue;  // Protection against a division by zero
    }

    price_t denominator = 0.0, temp = 0.0;
    if (weightIter->suogu() != 0.0) {
      denominator = weightIter->suogu();
    } else {
      // The change ratio of the outstanding shares
      price_t change =
          0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                 weightIter->increasement());
      // A change less than 0 means a share contraction
      denominator =
          1.0 + change;  // (1 + the change ratio of the outstanding shares)
      temp = weightIter->priceForSell() * change - 0.1 * weightIter->bonus();
    }

    if (denominator == 0.0 || (denominator == 1.0 && temp == 0.0)) continue;

    price_t k = (closePrice + temp) / (denominator * closePrice);
    price_t volume_k = 1.0 / denominator;  // The volume correction factor (the
                                           // reciprocal of the change)

    for (i = 0; i < pre_pos; ++i) {
      buffer_[i].openPrice = k * buffer_[i].openPrice;
      buffer_[i].highPrice = k * buffer_[i].highPrice;
      buffer_[i].lowPrice = k * buffer_[i].lowPrice;
      buffer_[i].closePrice = k * buffer_[i].closePrice;
      buffer_[i].transCount = buffer_[i].transCount * volume_k;
      buffer_[i].transAmount = buffer_[i].closePrice * buffer_[i].transCount;
    }
  }
}

/******************************************************************************
 * The proportional backward adjustment formula: the adjusted price = the
 *pre-adjustment price / the adjustment ratio the adjustment ratio = {[(the
 *close price of the record date - the cash dividend) + the rights (new) share
 *price x the change ratio of the outstanding shares] / (1 + the change ratio of
 *the outstanding shares)} / the close price of the record date The backward
 * adjustment takes the price before the ex-rights as the base (i.e. the price
 *before the ex-rights stays unchanged) and raises the prices after the
 *ex-rights. In the adjustment calculation it starts from the latest date and
 *goes backward day by day; when an ex-rights date is met, all the prices
 *between the ex-rights date and the latest date (including the ex-rights date)
 *are raised by the adjustment calculation; then it continues backward, and when
 *the next ex-rights date is met, the prices between that ex-rights date and the
 *latest date (including the ex-rights date) are raised by the adjustment
 *calculation again.
 *****************************************************************************/
void KDataPrivatedBufferImp::_recoverEqualBackward() {
  size_t total = buffer_.size();
  HAYAKU_IF_RETURN(total == 0, void());

  Datetime start_date(buffer_.front().datetime.startOfDay());
  Datetime end_date(buffer_.back().datetime + query_.kTypeInSeconds());
  StockWeightList weightList = stock_.getWeight(start_date, end_date);
  StockWeightList::const_reverse_iterator weightIter = weightList.rbegin();

  size_t pre_pos = total - 1;
  for (; weightIter != weightList.rend(); ++weightIter) {
    size_t i = pre_pos;
    while (i > 0 && buffer_[i].datetime > weightIter->datetime()) {
      i--;
    }

    // For the minute data the first time point needs to be skipped
    if (i != pre_pos &&
        buffer_[i].datetime != buffer_[i].datetime.startOfDay()) {
      i++;
    }

    pre_pos = i;  // The ex-rights date

    // The close price of the record date (i.e. the data of the day before the
    // ex-rights date)
    if (pre_pos == 0) {
      continue;
    }

    price_t closePrice = buffer_[pre_pos - 1].closePrice;

    price_t denominator = 0.0, temp = closePrice;
    if (weightIter->suogu() != 0.0) {
      denominator = weightIter->suogu();
    } else {
      // The change ratio of the outstanding shares
      price_t change =
          0.1 * (weightIter->countAsGift() + weightIter->countForSell() +
                 weightIter->increasement());
      // A change less than 0 means a share contraction
      denominator =
          1.0 + change;  // (1 + the change ratio of the outstanding shares)
      temp = closePrice + weightIter->priceForSell() * change -
             0.1 * weightIter->bonus();
    }

    if (temp == 0.0 || denominator == 0.0) {
      continue;
    }
    price_t k = (denominator * closePrice) / temp;
    price_t volume_k = denominator;

    for (i = pre_pos; i < total; ++i) {
      buffer_[i].openPrice = k * buffer_[i].openPrice;
      buffer_[i].highPrice = k * buffer_[i].highPrice;
      buffer_[i].lowPrice = k * buffer_[i].lowPrice;
      buffer_[i].closePrice = k * buffer_[i].closePrice;
      buffer_[i].transCount = buffer_[i].transCount * volume_k;
      buffer_[i].transAmount = buffer_[i].closePrice * buffer_[i].transCount;
    }
  }
}

KDataImpPtr KDataPrivatedBufferImp::getOtherFromSelf(
    const KQuery& query) const {
  KDataImpPtr ret;
  // The other restrictions are guarded by the upper layer
  if (query.queryType() == KQuery::INDEX &&
      query_.queryType() == KQuery::INDEX) {
    ret = _getOtherFromSelfByIndex(query);
  } else if (query.queryType() == KQuery::DATE) {
    ret = _getOtherFromSelfByDate(query);
  } else {
    ret = std::make_shared<KDataPrivatedBufferImp>(stock_, query);
  }
  return ret;
}

KDataImpPtr KDataPrivatedBufferImp::_getOtherFromSelfByIndex(
    const KQuery& query) const {
  size_t new_start_pos = 0, new_end_pos = 0;
  bool success = stock_.getIndexRange(query, new_start_pos, new_end_pos);
  if (!success || new_end_pos == 0) {
    auto* p = new KDataPrivatedBufferImp;
    p->stock_ = stock_;
    p->query_ = query;
    if (query.recoverType() != KQuery::NO_RECOVER) {
      p->_recover();
    }
    return KDataImpPtr(p);
  }

  size_t new_last_pos = new_end_pos - 1;

  size_t old_start_pos = startPos();
  size_t old_last_pos = lastPos();
  if (new_start_pos < old_start_pos || new_start_pos > old_last_pos) {
    return std::make_shared<KDataPrivatedBufferImp>(stock_, query);
  }

  if (new_last_pos <= old_last_pos) {
    auto* p = new KDataPrivatedBufferImp;
    p->stock_ = stock_;
    p->query_ = query;
    size_t new_len = new_last_pos + 1 - new_start_pos;
    p->buffer_.resize(new_len);
    std::copy(buffer_.begin() + new_start_pos - old_start_pos,
              buffer_.begin() + new_last_pos + 1 - old_start_pos,
              p->buffer_.begin());
    if (query.recoverType() != KQuery::NO_RECOVER) {
      p->_recover();
    }
    return KDataImpPtr(p);
  }

  auto* p = new KDataPrivatedBufferImp;
  p->stock_ = stock_;
  p->query_ = query;
  size_t new_len = new_last_pos + 1 - new_start_pos;
  p->buffer_.resize(new_len);
  std::copy(buffer_.begin() + new_start_pos - old_start_pos, buffer_.end(),
            p->buffer_.begin());
  KRecordList klist = stock_.getKRecordList(
      KQuery(old_last_pos + 1, new_end_pos, query.kType()));
  size_t remain_len = new_last_pos - old_last_pos;
  HAYAKU_ASSERT(klist.size() == remain_len);
  std::copy(klist.begin(), klist.end(), p->buffer_.begin() + remain_len);
  if (query.recoverType() != KQuery::NO_RECOVER) {
    p->_recover();
  }
  return KDataImpPtr(p);
}

KDataImpPtr KDataPrivatedBufferImp::_getOtherFromSelfByDate(
    const KQuery& query) const {
  Datetime new_start_date = query.startDatetime();
  Datetime new_end_date = query.endDatetime();
  const auto& old_start_date = buffer_.front().datetime;
  const auto& old_last_date = buffer_.back().datetime;
  if (new_start_date >= new_end_date || new_start_date < old_start_date ||
      new_start_date > old_last_date ||
      (new_end_date != Null<Datetime>() && new_end_date <= old_start_date)) {
    return std::make_shared<KDataPrivatedBufferImp>(stock_, query);
  }

  auto iter = std::lower_bound(buffer_.begin(), buffer_.end(),
                               KRecord{new_start_date},
                               [](const KRecord& a, const KRecord& b) {
                                 return a.datetime < b.datetime;
                               });
  if (iter == buffer_.end()) {
    return std::make_shared<KDataPrivatedBufferImp>(stock_, query);
  }
  size_t new_start_pos_in_old = std::distance(buffer_.begin(), iter);

  size_t new_end_pos_in_old = buffer_.size();
  if (new_end_date != Null<Datetime>()) {
    iter = std::lower_bound(buffer_.begin(), buffer_.end(),
                            KRecord{new_end_date},
                            [](const KRecord& a, const KRecord& b) {
                              return a.datetime < b.datetime;
                            });
    if (iter != buffer_.end()) {
      new_end_pos_in_old = std::distance(buffer_.begin(), iter);
      auto* p = new KDataPrivatedBufferImp;
      p->stock_ = stock_;
      p->query_ = query;
      size_t copy_len = new_end_pos_in_old - new_start_pos_in_old;
      p->buffer_.resize(copy_len);
      std::copy(buffer_.begin() + new_start_pos_in_old,
                buffer_.begin() + new_end_pos_in_old, p->buffer_.begin());
      if (query.recoverType() != KQuery::NO_RECOVER) {
        p->_recover();
      }
      return KDataImpPtr(p);
    }
  }

  KRecordList klist = stock_.getKRecordList(KQueryByDate(
      old_last_date + Seconds(KQuery::getKTypeInSeconds(query.kType())),
      new_end_date, query.kType()));
  auto* p = new KDataPrivatedBufferImp;
  p->stock_ = stock_;
  p->query_ = query;
  size_t copy_len = new_end_pos_in_old - new_start_pos_in_old;
  size_t new_len = copy_len + klist.size();
  p->buffer_.resize(new_len);
  std::copy(buffer_.begin() + new_start_pos_in_old,
            buffer_.begin() + new_end_pos_in_old, p->buffer_.begin());
  std::copy(klist.begin(), klist.end(),
            p->buffer_.begin() + buffer_.size() - new_start_pos_in_old);
  if (query.recoverType() != KQuery::NO_RECOVER) {
    p->_recover();
  }
  return KDataImpPtr(p);
}

} /* namespace hayaku */
