/*
 * SignalBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "SignalBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os, const SignalBase& sg) {
  os << "Signal(" << sg.name() << ", " << sg.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os, const SignalPtr& sg) {
  if (sg) {
    os << *sg;
  } else {
    os << "Signal(NULL)";
  }

  return os;
}

SignalBase::SignalBase()
    : name_("SignalBase"), hold_long_(false), hold_short_(false) {
  initParam();
}

SignalBase::SignalBase(const string& name)
    : name_(name), hold_long_(false), hold_short_(false) {
  initParam();
}

SignalBase::~SignalBase() {}

void SignalBase::initParam() {
  setParam<bool>("cycle",
                 false);  // Calculate within the given cycle range only
  setParam<bool>("alternate",
                 true);  // The buy and sell signals appear alternately
  setParam<bool>("support_borrow_stock",
                 false);  // Support issuing a short signal
}

void SignalBase::baseCheckParam(const string& name) const {}
void SignalBase::paramChanged() { calculated_ = false; }

SignalPtr SignalBase::clone() {
  SignalPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = SignalPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->name_ = name_;
  p->params_ = params_;
  p->is_python_object_ = is_python_object_;
  p->kdata_ = kdata_;
  p->calculated_ = calculated_;
  p->hold_long_ = hold_long_;
  p->hold_short_ = hold_short_;
  p->buy_sig_ = buy_sig_;
  p->sell_sig_ = sell_sig_;
  p->cycle_start_ = cycle_start_;
  p->cycle_end_ = cycle_end_;
  return p;
}

void SignalBase::setTO(const KData& kdata) {
  HAYAKU_IF_RETURN(calculated_ && kdata_ == kdata, void());
  kdata_ = kdata;
  calculated_ = false;
  HAYAKU_IF_RETURN(kdata.empty(), void());

  bool cycle = getParam<bool>("cycle");
  cycle_start_ = kdata[0].datetime;

  if (!cycle) {
    _calculate(kdata);
  }

  calculated_ = true;
}

void SignalBase::reset() {
  kdata_ = Null<KData>();
  buy_sig_.clear();
  sell_sig_.clear();
  hold_long_ = false;
  hold_short_ = false;
  cycle_start_ = Null<Datetime>();
  cycle_end_ = Null<Datetime>();
  _reset();
}

void SignalBase::startCycle(const Datetime& start, const Datetime& close) {
  HAYAKU_IF_RETURN(!getParam<bool>("cycle"), void());
  HAYAKU_CHECK(
      start != Null<Datetime>() && close != Null<Datetime>() && start < close,
      "{}", name_);
  HAYAKU_CHECK(start >= cycle_end_ || cycle_end_ == Null<Datetime>(),
               "curretn start: {}, pre cycle end: {}", start, cycle_end_);
  cycle_start_ = start;
  cycle_end_ = close;
  KData kdata = kdata_.getKData(start, close);
  if (!kdata.empty()) {
    _calculate(kdata);
  }
}

DatetimeList SignalBase::getBuySignal() const {
  DatetimeList result;
  result.reserve(buy_sig_.size());
  for (auto iter = buy_sig_.begin(); iter != buy_sig_.end(); ++iter) {
    result.emplace_back(iter->first);
  }
  return result;
}

DatetimeList SignalBase::getSellSignal() const {
  DatetimeList result;
  result.reserve(sell_sig_.size());
  for (auto iter = sell_sig_.begin(); iter != sell_sig_.end(); ++iter) {
    result.emplace_back(iter->first);
  }
  return result;
}

double SignalBase::getBuyValue(const Datetime& datetime) const {
  auto iter = buy_sig_.find(datetime);
  return iter != buy_sig_.end() ? iter->second : 0.0;
}
double SignalBase::getSellValue(const Datetime& datetime) const {
  auto iter = sell_sig_.find(datetime);
  return iter != sell_sig_.end() ? iter->second : 0.0;
}

void SignalBase::_addSignal(const Datetime& datetime, double value) {
  HAYAKU_IF_RETURN(iszero(value) || std::isnan(value), void());

  double new_value = value + getBuyValue(datetime) + getSellValue(datetime);
  HAYAKU_IF_RETURN(iszero(new_value), void());

  if (new_value > 0.0) {
    auto iter = buy_sig_.find(datetime);
    if (!getParam<bool>("alternate")) {
      if (iter != buy_sig_.end()) {
        iter->second += new_value;
      } else {
        buy_sig_.insert({datetime, new_value});
      }
      return;
    }

    if (!hold_long_) {
      if (iter != buy_sig_.end()) {
        iter->second += new_value;
      } else {
        buy_sig_.insert({datetime, new_value});
      }
      if (getParam<bool>("support_borrow_stock") && hold_short_) {
        hold_short_ = false;
      } else {
        hold_long_ = true;
      }
    }

  } else {
    auto iter = sell_sig_.find(datetime);
    if (!getParam<bool>("alternate")) {
      if (iter != sell_sig_.end()) {
        iter->second += new_value;
      } else {
        sell_sig_.insert({datetime, new_value});
      }
      return;
    }

    if (!hold_short_) {
      if (hold_long_) {
        if (iter != sell_sig_.end()) {
          iter->second += new_value;
        } else {
          sell_sig_.insert({datetime, new_value});
        }
        hold_long_ = false;
      } else if (getParam<bool>("support_borrow_stock")) {
        if (iter != sell_sig_.end()) {
          iter->second += new_value;
        } else {
          sell_sig_.insert({datetime, new_value});
        }
        hold_short_ = true;
      }
    }
  }
}

bool SignalBase::nextTimeShouldBuy() const {
  size_t total = kdata_.size();
  HAYAKU_IF_RETURN(total == 0, false);
  return shouldBuy(kdata_[total - 1].datetime);
}

bool SignalBase::nextTimeShouldSell() const {
  size_t total = kdata_.size();
  HAYAKU_IF_RETURN(total == 0, false);
  return shouldSell(kdata_[total - 1].datetime);
}

} /* namespace hayaku */
