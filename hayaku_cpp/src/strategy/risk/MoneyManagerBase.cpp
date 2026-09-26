/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const MoneyManagerBase& mm) {
  os << "MoneyManager(" << mm.name() << ", " << mm.getParameter() << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const MoneyManagerPtr& mm) {
  if (mm) {
    os << *mm;
  } else {
    os << "MoneyManager(NULL)";
  }

  return os;
}

MoneyManagerBase::MoneyManagerBase() : name_("MoneyManagerBase") {
  setParam<bool>("auto-checkin", false);
  setParam<int>("max-stock", 20000);
  setParam<bool>("disable_ev_force_clean_position", false);
  setParam<bool>("disable_cn_force_clean_position", false);
}

MoneyManagerBase::MoneyManagerBase(const string& name) : name_(name) {
  setParam<bool>("auto-checkin", false);
  setParam<int>("max-stock", 20000);
  setParam<bool>("disable_ev_force_clean_position", false);
  setParam<bool>("disable_cn_force_clean_position", false);
}

MoneyManagerBase::~MoneyManagerBase() {}

void MoneyManagerBase::baseCheckParam(const string& name) const {
  if ("max-stock" == name) {
    HAYAKU_ASSERT(getParam<int>("max-stock") >= 1);
  }
}

void MoneyManagerBase::paramChanged() {}

void MoneyManagerBase::reset() {
  query_ = Null<KQuery>();
  account_.reset();
  buy_sell_counts_.clear();
  _reset();
}

MoneyManagerPtr MoneyManagerBase::clone() {
  MoneyManagerPtr p;
  try {
    p = _clone();
  } catch (...) {
    HAYAKU_ERROR("Subclass _clone failed!");
    p = MoneyManagerPtr();
  }

  if (!p || p.get() == this) {
    HAYAKU_ERROR("Failed clone! Will use self-ptr!");
    return shared_from_this();
  }

  p->params_ = params_;
  p->name_ = name_;
  p->is_python_object_ = is_python_object_;
  p->account_ = account_;
  p->query_ = query_;
  p->buy_sell_counts_ = buy_sell_counts_;
  return p;
}

double MoneyManagerBase::getSellNumber(const Datetime& datetime,
                                       const Stock& stock, price_t price,
                                       price_t risk, OrderOrigin origin) {
  HAYAKU_ERROR_IF_RETURN(
      !account_, 0.0,
      "m_account is null! Datetime({}) Stock({}) price({:<.4f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);

  if (OrderOrigin::ENVIRONMENT == origin) {
    // Force selling everything
    HAYAKU_IF_RETURN(!getParam<bool>("disable_ev_force_clean_position"),
                     MAX_DOUBLE);
  }

  if (OrderOrigin::CONDITION == origin) {
    HAYAKU_IF_RETURN(!getParam<bool>("disable_cn_force_clean_position"),
                     MAX_DOUBLE);
  }

  // Ignore it when the risk is not greater than 0
  HAYAKU_IF_RETURN(risk <= 0.0, 0.0);

  return _getSellNumber(datetime, stock, price, risk, origin);
}

double MoneyManagerBase::getBuyNumber(const Datetime& datetime,
                                      const Stock& stock, price_t price,
                                      price_t risk, OrderOrigin origin) {
  HAYAKU_ERROR_IF_RETURN(
      !account_, 0.0,
      "m_account is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), 0.0, "stock is Null!");

  HAYAKU_INFO_IF_RETURN(
      risk <= 0.0, 0.0,
      "risk less zero (Mayby single-line price board, can ignored)! "
      "Datetime({}) Stock({} {}) price({:<.3f}) risk({:<.2f}) Part({})",
      datetime, stock.market_code(), stock.name(), price, risk,
      getOrderOriginName(origin));

  HAYAKU_TRACE_IF_RETURN(
      account_->getStockNumber() >= getParam<int>("max-stock"), 0.0,
      "Ignore! execution account reached max-stock number!");

  double n = _getBuyNumber(datetime, stock, price, risk, origin);
  double min_trade = stock.minTradeNumber();
  HAYAKU_TRACE_IF_RETURN(
      n < min_trade, 0.0,
      "Ignore! Is less than the minimum number of transactions({:<.4f}<{}) {}",
      n, min_trade, stock.market_code());

  // Convert it into an integer multiple of the minimum trade quantity
  n = int64_t(n / min_trade) * min_trade;

  double max_trade = stock.maxTradeNumber();
  HAYAKU_WARN_IF_RETURN(
      n > max_trade, max_trade,
      "Over stock.maxTradeNumber({}), will use maxTradeNumber", max_trade);

  // Automatically deposit more cash when the cash is not enough
  if (getParam<bool>("auto-checkin")) {
    price_t cash = account_->cash(datetime, query_.kType());
    CostRecord cost = account_->getBuyCost(datetime, stock, price, n);
    int precision = account_->precision();
    price_t money = roundUp(price * n * stock.unit() + cost.total, precision);
    if (money > cash) {
      account_->checkin(datetime, roundUp(money - cash, precision));
    }
  } else {
    CostRecord cost = account_->getBuyCost(datetime, stock, price, n);
    price_t need_cash = n * price + cost.total;
    price_t current_cash = account_->cash(datetime, query_.kType());
    while (n > min_trade && need_cash > current_cash) {
      n = n - min_trade;
      cost = account_->getBuyCost(datetime, stock, price, n);
      need_cash = n * price + cost.total;
    }
    if (need_cash > current_cash) {
      n = 0.0;
    }
  }

  return n;
}

double MoneyManagerBase::getSellShortNumber(const Datetime& datetime,
                                            const Stock& stock, price_t price,
                                            price_t risk, OrderOrigin origin) {
  HAYAKU_ERROR_IF_RETURN(
      !account_, 0.0,
      "m_account is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);
  HAYAKU_ERROR_IF_RETURN(
      risk >= 0.0, 0.0,
      "risk is positive! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);
  return _getSellShortNumber(datetime, stock, price, risk, origin);
}

double MoneyManagerBase ::getBuyShortNumber(const Datetime& datetime,
                                            const Stock& stock, price_t price,
                                            price_t risk, OrderOrigin origin) {
  HAYAKU_ERROR_IF_RETURN(
      !account_, 0.0,
      "m_account is null! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);
  HAYAKU_ERROR_IF_RETURN(
      risk >= 0.0, 0.0,
      "risk is positive! Datetime({}) Stock({}) price({:<.3f}) risk({:<.2f})",
      datetime, stock.market_code(), price, risk);
  return _getBuyShortNumber(datetime, stock, price, risk, origin);
}

double MoneyManagerBase::_getSellNumber(const Datetime& datetime,
                                        const Stock& stock, price_t price,
                                        price_t risk, OrderOrigin origin) {
  // Sell everything by default
  return MAX_DOUBLE;
}

double MoneyManagerBase::_getSellShortNumber(const Datetime& datetime,
                                             const Stock& stock, price_t price,
                                             price_t risk, OrderOrigin origin) {
  return 0;
}

double MoneyManagerBase::_getBuyShortNumber(const Datetime& datetime,
                                            const Stock& stock, price_t price,
                                            price_t risk, OrderOrigin origin) {
  // Liquidate everything by default
  return MAX_DOUBLE;
}

size_t MoneyManagerBase::currentBuyCount(const Stock& stk) const {
  const auto iter = buy_sell_counts_.find(stk);
  return iter == buy_sell_counts_.cend() ? 0 : iter->second.first;
}

size_t MoneyManagerBase::currentSellCount(const Stock& stk) const {
  const auto iter = buy_sell_counts_.find(stk);
  return iter == buy_sell_counts_.cend() ? 0 : iter->second.second;
}

void MoneyManagerBase::buyNotify(const TradeRecord& tr) {
  auto iter = buy_sell_counts_.find(tr.stock);
  if (iter == buy_sell_counts_.end()) {
    buy_sell_counts_[tr.stock] = std::make_pair<size_t, size_t>(1, 0);
  } else {
    iter->second.first++;
    iter->second.second = 0;
  }
  _buyNotify(tr);
}

void MoneyManagerBase::sellNotify(const TradeRecord& tr) {
  auto iter = buy_sell_counts_.find(tr.stock);
  if (iter == buy_sell_counts_.end()) {
    buy_sell_counts_[tr.stock] = std::make_pair<size_t, size_t>(0, 1);
  } else {
    iter->second.first = 0;
    iter->second.second++;
  }
  _sellNotify(tr);
}

} /* namespace hayaku */
