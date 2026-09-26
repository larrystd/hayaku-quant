/*
 * MoneyManagerBase.cpp
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "MoneyManagerBase.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const MoneyManagerBase& mm) {
  os << "MoneyManager(" << mm.name() << ", " << mm.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const MoneyManagerPtr& mm) {
  if (mm) {
    os << *mm;
  } else {
    os << "MoneyManager(NULL)";
  }

  return os;
}

MoneyManagerBase::MoneyManagerBase() : m_name("MoneyManagerBase") {
  setParam<bool>("auto-checkin", false);
  setParam<int>("max-stock", 20000);
  setParam<bool>("disable_ev_force_clean_position", false);
  setParam<bool>("disable_cn_force_clean_position", false);
}

MoneyManagerBase::MoneyManagerBase(const string& name) : m_name(name) {
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
  m_query = Null<KQuery>();
  m_account.reset();
  m_buy_sell_counts.clear();
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

  p->m_params = m_params;
  p->m_name = m_name;
  p->m_is_python_object = m_is_python_object;
  p->m_account = m_account;
  p->m_query = m_query;
  p->m_buy_sell_counts = m_buy_sell_counts;
  return p;
}

double MoneyManagerBase::getSellNumber(const Datetime& datetime,
                                       const Stock& stock, price_t price,
                                       price_t risk, OrderOrigin origin) {
  HAYAKU_ERROR_IF_RETURN(
      !m_account, 0.0,
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
      !m_account, 0.0,
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
      m_account->getStockNumber() >= getParam<int>("max-stock"), 0.0,
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
    price_t cash = m_account->cash(datetime, m_query.kType());
    CostRecord cost = m_account->getBuyCost(datetime, stock, price, n);
    int precision = m_account->precision();
    price_t money = roundUp(price * n * stock.unit() + cost.total, precision);
    if (money > cash) {
      m_account->checkin(datetime, roundUp(money - cash, precision));
    }
  } else {
    CostRecord cost = m_account->getBuyCost(datetime, stock, price, n);
    price_t need_cash = n * price + cost.total;
    price_t current_cash = m_account->cash(datetime, m_query.kType());
    while (n > min_trade && need_cash > current_cash) {
      n = n - min_trade;
      cost = m_account->getBuyCost(datetime, stock, price, n);
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
      !m_account, 0.0,
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
      !m_account, 0.0,
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
  const auto iter = m_buy_sell_counts.find(stk);
  return iter == m_buy_sell_counts.cend() ? 0 : iter->second.first;
}

size_t MoneyManagerBase::currentSellCount(const Stock& stk) const {
  const auto iter = m_buy_sell_counts.find(stk);
  return iter == m_buy_sell_counts.cend() ? 0 : iter->second.second;
}

void MoneyManagerBase::buyNotify(const TradeRecord& tr) {
  auto iter = m_buy_sell_counts.find(tr.stock);
  if (iter == m_buy_sell_counts.end()) {
    m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(1, 0);
  } else {
    iter->second.first++;
    iter->second.second = 0;
  }
  _buyNotify(tr);
}

void MoneyManagerBase::sellNotify(const TradeRecord& tr) {
  auto iter = m_buy_sell_counts.find(tr.stock);
  if (iter == m_buy_sell_counts.end()) {
    m_buy_sell_counts[tr.stock] = std::make_pair<size_t, size_t>(0, 1);
  } else {
    iter->second.first = 0;
    iter->second.second++;
  }
  _sellNotify(tr);
}

} /* namespace hayaku */
