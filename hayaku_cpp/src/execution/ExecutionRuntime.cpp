/*
 * ExecutionRuntime.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "ExecutionRuntime.h"

#include <algorithm>
#include <atomic>
#include <boost/lexical_cast.hpp>
#include <fstream>
#include <functional>
#include <nlohmann/json.hpp>
#include <sstream>

#include "data/DataRuntime.h"
#include "data/KData.h"

namespace hayaku {

using json = nlohmann::json;

namespace {

std::atomic<uint64_t> g_next_account_id{1};

AccountId nextAccountId() noexcept {
  return AccountId(g_next_account_id.fetch_add(1, std::memory_order_relaxed));
}

}  // namespace

string ExecutionRuntime::str() const {
  std::stringstream os;
  os << std::fixed;
  (void)os.precision(2);

  FundsRecord funds = getFunds();
  string strip(",\n");
  os << "ExecutionRuntime {\n"
     << "  params: " << getParameter() << strip << "  name: " << name() << strip
     << "  init_date: " << initDatetime() << strip
     << "  init_cash: " << initCash() << strip
     << "  firstDatetime: " << firstDatetime() << strip
     << "  lastDatetime: " << lastDatetime() << strip
     << "  TradeCostFunc: " << costFunc() << strip << "  current total funds: "
     << funds.cash + funds.market_value + funds.borrow_asset -
            funds.short_market_value
     << strip << "  current cash: " << currentCash() << strip
     << "  current market_value: " << funds.market_value << strip
     << "  current short_market_value: " << funds.short_market_value << strip
     << "  current base_cash: " << funds.base_cash << strip
     << "  current base_asset: " << funds.base_asset << strip
     << "  current borrow_cash: " << funds.borrow_cash << strip
     << "  current borrow_asset: " << funds.borrow_asset << strip
     << "  Position: \n";

  auto& sm = getDataRuntime();
  KQuery query(-1);
  PositionRecordList position = getPositionList();
  PositionRecordList::const_iterator iter = position.begin();
  os << "    "
     << "code name takeDatetime hold_days number invest market_value bonus "
        "return_rate "
        "initial_capital_return\n";
  for (; iter != position.end(); ++iter) {
    price_t invest = iter->buyMoney - iter->sellMoney + iter->totalCost;
    KData k = iter->stock.getKData(query);
    price_t cur_val = k[0].closePrice * iter->number;
    price_t bonus = cur_val - invest;
    DatetimeList date_list = sm.getTradingCalendar(
        KQueryByDate(Datetime(iter->takeDatetime.date())));
    os << "    " << iter->stock.market_code() << " " << iter->stock.name()
       << " " << iter->takeDatetime << " " << date_list.size() << " "
       << iter->number << " " << invest << " " << cur_val << " " << bonus << " "
       << 100 * bonus / invest << "% " << 100 * bonus / ledger_.init_cash_
       << "%\n";
  }

  os << "  Short Position: \n";
  position = getShortPositionList();
  iter = position.begin();
  for (; iter != position.end(); ++iter) {
    os << "    " << iter->number << " " << iter->stock.toString() << "\n";
  }

  os << "  Borrow Stock: \n";
  BorrowRecordList borrow = getBorrowStockList();
  BorrowRecordList::const_iterator bor_iter = borrow.begin();
  for (; bor_iter != borrow.end(); ++bor_iter) {
    os << "    " << bor_iter->number << " " << bor_iter->value << " "
       << bor_iter->stock.toString() << "\n";
  }

  os << "}";

  os.unsetf(std::ostream::floatfield);
  (void)os.precision();
  return os.str();
}

ExecutionRuntime::ExecutionRuntime(const Datetime& datetime, price_t initcash,
                                   const TradeCostPtr& costfunc,
                                   const string& name)
    : name_(name), costfunc_(costfunc), broker_last_datetime_(Datetime::now()) {
  setParam<int>("precision", 2);  // Calculation precision
  ledger_.account_id_ = nextAccountId();
  ledger_.init_datetime_ = datetime;
  ledger_.last_update_datetime_ = datetime;
  setParam<bool>("support_borrow_cash",
                 false);  // Whether to finance automatically
  setParam<bool>("support_borrow_stock",
                 false);  // Whether to borrow stocks automatically
  setParam<bool>("save_action", true);  // Whether to save the commands
  ledger_.init_cash_ = roundEx(initcash, 2);
  ledger_.cash_ = ledger_.init_cash_;
  ledger_.checkin_cash_ = ledger_.init_cash_;
  ledger_.trade_list_.push_back(
      TradeRecord(Null<Stock>(), ledger_.init_datetime_, BUSINESS_INIT,
                  ledger_.init_cash_, ledger_.init_cash_, 0.0, 0, CostRecord(),
                  0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  broker_last_datetime_ = Datetime::now();
  _saveAction(ledger_.trade_list_.back());
}

ExecutionRuntime::ExecutionRuntime(const AccountConfig& config)
    : ExecutionRuntime(config.initDatetime(), config.initialCash(),
                       config.costPolicy(), config.name()) {
  setParam<int>("precision", config.precision());
  setParam<bool>("support_borrow_cash", config.supportBorrowCash());
  setParam<bool>("support_borrow_stock", config.supportBorrowStock());
  if (config.accountId().valid()) {
    ledger_.account_id_ = config.accountId();
  }
  for (const auto& broker : config.brokers()) {
    regBroker(broker);
  }
}

ExecutionRuntime::~ExecutionRuntime() {}

void ExecutionRuntime::_reset() {
  ledger_.last_update_datetime_ = ledger_.init_datetime_;
  ledger_.cash_ = ledger_.init_cash_;
  ledger_.checkin_cash_ = ledger_.init_cash_;
  ledger_.checkout_cash_ = 0.0;
  ledger_.checkin_stock_ = 0.0;
  ledger_.checkout_stock_ = 0.0;
  ledger_.borrow_cash_ = 0.0;

  ledger_.loan_list_.clear();
  ledger_.borrow_stock_.clear();

  ledger_.trade_list_.clear();
  ledger_.trade_list_.push_back(
      TradeRecord(Null<Stock>(), ledger_.init_datetime_, BUSINESS_INIT,
                  ledger_.init_cash_, ledger_.init_cash_, 0.0, 0, CostRecord(),
                  0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));

  ledger_.position_.clear();
  ledger_.position_history_.clear();
  ledger_.actions_.clear();
  _saveAction(ledger_.trade_list_.back());
}

void ExecutionRuntime::copyRuntimeStateTo(ExecutionRuntime& target) const {
  // Keep the established account clone semantics. In particular, legacy clone()
  // did not copy active/history short positions.
  target.ledger_.init_datetime_ = ledger_.init_datetime_;
  target.ledger_.init_cash_ = ledger_.init_cash_;
  target.ledger_.last_update_datetime_ = ledger_.last_update_datetime_;
  target.ledger_.cash_ = ledger_.cash_;
  target.ledger_.checkin_cash_ = ledger_.checkin_cash_;
  target.ledger_.checkout_cash_ = ledger_.checkout_cash_;
  target.ledger_.checkin_stock_ = ledger_.checkin_stock_;
  target.ledger_.checkout_stock_ = ledger_.checkout_stock_;
  target.ledger_.borrow_cash_ = ledger_.borrow_cash_;
  target.ledger_.loan_list_ = ledger_.loan_list_;
  target.ledger_.borrow_stock_ = ledger_.borrow_stock_;
  target.ledger_.trade_list_ = ledger_.trade_list_;
  target.ledger_.position_ = ledger_.position_;
  target.ledger_.position_history_ = ledger_.position_history_;
  target.ledger_.actions_ = ledger_.actions_;
}

shared_ptr<ExecutionRuntime> ExecutionRuntime::cloneRuntime() const {
  auto result = make_shared<ExecutionRuntime>(
      ledger_.init_datetime_, ledger_.init_cash_, costfunc_, name_);
  result->params_ = params_;
  result->broker_last_datetime_ = broker_last_datetime_;
  result->broker_list_ = broker_list_;
  copyRuntimeStateTo(*result);
  return result;
}

internal::PortfolioAccountPortPtr ExecutionRuntime::cloneAccount() const {
  return cloneRuntime();
}

internal::PortfolioAccountPortPtr ExecutionRuntime::createChildAccount(
    string name, price_t initialCash) const {
  return std::make_shared<ExecutionRuntime>(
      AccountConfig(initDatetime(), initialCash, costfunc_, std::move(name),
                    precision(), supportsBorrowCash(), supportsBorrowStock()));
}

AccountView ExecutionRuntime::view() const {
  return AccountView(ledger_.account_id_, ledger_.init_datetime_,
                     lastDatetime(), getFunds(), getPositionList(),
                     getShortPositionList());
}

ExecutionReport ExecutionRuntime::submit(const OrderRequest& request) {
  TradeRecord record;
  switch (request.side()) {
    case OrderSide::BUY:
      record = buy(request.datetime(), request.stock(), request.realPrice(),
                   request.number(), request.stoploss(), request.goalPrice(),
                   request.planPrice(), request.origin(), request.remark());
      break;
    case OrderSide::SELL:
      record = sell(request.datetime(), request.stock(), request.realPrice(),
                    request.number(), request.stoploss(), request.goalPrice(),
                    request.planPrice(), request.origin(), request.remark());
      break;
    case OrderSide::SELL_SHORT:
      record =
          sellShort(request.datetime(), request.stock(), request.realPrice(),
                    request.number(), request.stoploss(), request.goalPrice(),
                    request.planPrice(), request.origin(), request.remark());
      break;
    case OrderSide::BUY_SHORT:
      record =
          buyShort(request.datetime(), request.stock(), request.realPrice(),
                   request.number(), request.stoploss(), request.goalPrice(),
                   request.planPrice(), request.origin(), request.remark());
      break;
  }
  return ExecutionReport(std::move(record));
}

void ExecutionRuntime::fetchAssetInfoFromBroker(const OrderBrokerPtr& broker,
                                                const Datetime& datetime) {
  HAYAKU_CHECK(broker, "broker is null!");
  (void)datetime;  // Preserves the legacy broker-account timestamp semantics.

  const string brokerAsset = broker->getAssetInfo();
  if (brokerAsset.empty()) {
    HAYAKU_WARN("Failed fetch asset info from broker!");
    ledger_.init_datetime_ = Datetime::now();
    ledger_.init_cash_ = 0.0;
    _reset();
    broker_last_datetime_ = ledger_.init_datetime_;
    return;
  }

  try {
    const json asset = json::parse(brokerAsset);
    Datetime brokerDatetime = asset.contains("datetime")
                                  ? Datetime(asset["datetime"].get<string>())
                                  : Datetime::now();
    const price_t brokerCash = asset["cash"].get<price_t>();

    position_map_type positions;
    const auto& brokerPositions = asset["positions"];
    for (auto iter = brokerPositions.cbegin(); iter != brokerPositions.cend();
         ++iter) {
      try {
        const auto& positionJson = *iter;
        const auto market = positionJson["market"].get<string>();
        const auto code = positionJson["code"].get<string>();
        Stock stock = getStock(fmt::format("{}{}", market, code));
        if (stock.isNull()) {
          HAYAKU_DEBUG("Not found stock: {}{}", market, code);
          continue;
        }

        PositionRecord position;
        position.stock = stock;
        position.takeDatetime = brokerDatetime;
        position.number = positionJson["number"].get<double>();
        position.stoploss = positionJson["stoploss"].get<price_t>();
        position.goalPrice = positionJson["goal_price"].get<price_t>();
        position.totalNumber = position.number;
        const price_t costPrice = positionJson["cost_price"].get<price_t>();
        position.buyMoney = position.number * costPrice;
        position.totalRisk = (position.stoploss - costPrice) * position.number;
        positions[stock.id()] = std::move(position);
      } catch (const std::exception& e) {
        HAYAKU_ERROR(e.what());
      }
    }

    ledger_.init_datetime_ = brokerDatetime;
    ledger_.init_cash_ = brokerCash;
    _reset();
    ledger_.position_ = std::move(positions);
    broker_last_datetime_ = brokerDatetime;
  } catch (const std::exception& e) {
    HAYAKU_ERROR(e.what());
  }
}

double ExecutionRuntime::getMarginRate(const Datetime& datetime,
                                       const Stock& stock) {
  // TODO get the margin ratio, it is fixed to 60% by default
  return 0.6;
}

Datetime ExecutionRuntime::firstDatetime() const {
  Datetime result;
  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    if (iter->business == BUSINESS_BUY) {
      result = iter->datetime;
      break;
    }
  }
  return result;
}

double ExecutionRuntime::getHoldNumber(const Datetime& datetime,
                                       const Stock& stock) {
  // The date is earlier than the account creation date, return 0
  HAYAKU_IF_RETURN(datetime < ledger_.init_datetime_, 0.0);

  // Adjust the position quantity according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // If the given date is later than or equal to the last trade date, take the
  // current position record directly
  if (datetime >= lastDatetime()) {
    position_map_type::const_iterator pos_iter =
        ledger_.position_.find(stock.id());
    if (pos_iter != ledger_.position_.end()) {
      return pos_iter->second.number;
    }
    return 0.0;
  }

  // In the historical trade records, recalculate the position quantity of the
  // trading object at the given query date
  double number = 0;
  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    // Break the loop when the trade date in the trade record is already later
    // than the query date
    if (iter->datetime > datetime) {
      break;
    }

    if (iter->stock == stock) {
      if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
          BUSINESS_CHECKIN_STOCK == iter->business ||
          BUSINESS_SUOGU == iter->business) {
        number += iter->number;

      } else if (BUSINESS_SELL == iter->business ||
                 BUSINESS_CHECKOUT_STOCK == iter->business) {
        number -= iter->number;

      } else {
        // The other cases are ignored
      }
    }
  }
  return number;
}

double ExecutionRuntime::getShortHoldNumber(const Datetime& datetime,
                                            const Stock& stock) {
  // The date is earlier than the account creation date, return 0
  HAYAKU_IF_RETURN(datetime < ledger_.init_datetime_, 0.0);

  // Adjust the position quantity according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // If the given date is later than or equal to the last trade date, take the
  // current position record directly
  if (datetime >= lastDatetime()) {
    position_map_type::const_iterator pos_iter =
        ledger_.short_position_.find(stock.id());
    if (pos_iter != ledger_.short_position_.end()) {
      return pos_iter->second.number;
    }
    return 0;
  }

  // In the historical trade records, recalculate the position quantity of the
  // trading object at the given query date
  double number = 0;
  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    // Break the loop when the trade date in the trade record is already later
    // than the query date
    if (iter->datetime > datetime) {
      break;
    }

    if (iter->stock == stock) {
      if (BUSINESS_SELL_SHORT == iter->business) {
        number += iter->number;

      } else if (BUSINESS_BUY_SHORT == iter->business) {
        number -= iter->number;

      } else {
        // The other cases are ignored
      }
    }
  }
  return number;
}

double ExecutionRuntime::getDebtNumber(const Datetime& datetime,
                                       const Stock& stock) {
  HAYAKU_IF_RETURN(datetime < ledger_.init_datetime_, 0.0);

  // Adjust the position quantity according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  if (datetime >= lastDatetime()) {
    borrow_stock_map_type::const_iterator bor_iter;
    bor_iter = ledger_.borrow_stock_.find(stock.id());
    if (bor_iter != ledger_.borrow_stock_.end()) {
      return bor_iter->second.number;
    }
    return 0;
  }

  double debt_n = 0;
  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    if (iter->datetime > datetime) {
      break;
    }
    if (iter->stock == stock) {
      if (iter->business == BUSINESS_BORROW_STOCK) {
        debt_n += iter->number;
      } else if (iter->business == BUSINESS_RETURN_STOCK) {
        debt_n -= iter->number;
      }
    }
  }
  return debt_n;
}

price_t ExecutionRuntime::getDebtCash(const Datetime& datetime) {
  HAYAKU_IF_RETURN(datetime < ledger_.init_datetime_, 0.0);

  // Adjust the position quantity according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  HAYAKU_IF_RETURN(datetime >= lastDatetime(), ledger_.borrow_cash_);

  price_t debt_cash = 0.0;
  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    if (iter->datetime > datetime) {
      break;
    }
    if (iter->business == BUSINESS_BORROW_CASH) {
      debt_cash += iter->realPrice;
    } else if (iter->business == BUSINESS_RETURN_CASH) {
      debt_cash -= iter->realPrice;
    }
  }
  return debt_cash;
}

TradeRecordList ExecutionRuntime::getTradeList(const Datetime& start_date,
                                               const Datetime& end_date) const {
  TradeRecordList result;
  HAYAKU_IF_RETURN(start_date >= end_date, result);

  size_t total = ledger_.trade_list_.size();
  HAYAKU_IF_RETURN(total == 0, result);

  TradeRecord temp_record;
  temp_record.datetime = start_date;
  auto low = lower_bound(
      ledger_.trade_list_.begin(), ledger_.trade_list_.end(), temp_record,
      std::bind(std::less<Datetime>(),
                std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

  temp_record.datetime = end_date;
  auto high = lower_bound(
      ledger_.trade_list_.begin(), ledger_.trade_list_.end(), temp_record,
      std::bind(std::less<Datetime>(),
                std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

  result.insert(result.end(), low, high);

  return result;
}

PositionRecordList ExecutionRuntime::getPositionList() const {
  PositionRecordList result;
  position_map_type::const_iterator iter = ledger_.position_.begin();
  for (; iter != ledger_.position_.end(); ++iter) {
    result.push_back(iter->second);
  }
  return result;
}

PositionRecordList ExecutionRuntime::getShortPositionList() const {
  PositionRecordList result;
  position_map_type::const_iterator iter = ledger_.short_position_.begin();
  for (; iter != ledger_.short_position_.end(); ++iter) {
    result.push_back(iter->second);
  }
  return result;
}

PositionRecord ExecutionRuntime::getPosition(const Datetime& datetime,
                                             const Stock& stock) {
  PositionRecord result;
  HAYAKU_IF_RETURN(stock.isNull(), result);
  HAYAKU_IF_RETURN(datetime < ledger_.init_datetime_, result);

  // Adjust the position quantity according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // If the given date is later than or equal to the last trade date, take the
  // current position record directly
  if (datetime >= lastDatetime()) {
    position_map_type::const_iterator pos_iter =
        ledger_.position_.find(stock.id());
    if (pos_iter != ledger_.position_.end()) {
      result = pos_iter->second;
    }
    return result;
  }

  // In the historical trade records, recalculate the position quantity of the
  // trading object at the given query date
  double number = 0.0;
  for (auto iter = ledger_.trade_list_.begin();
       iter != ledger_.trade_list_.end(); ++iter) {
    // Break the loop when the trade date in the trade record is already later
    // than the query date
    if (iter->datetime > datetime) {
      break;
    }

    if (iter->stock == stock) {
      if (BUSINESS_BUY == iter->business || BUSINESS_GIFT == iter->business ||
          BUSINESS_CHECKIN_STOCK == iter->business ||
          BUSINESS_SUOGU == iter->business) {
        number += iter->number;

      } else if (BUSINESS_SELL == iter->business ||
                 BUSINESS_CHECKOUT_STOCK == iter->business) {
        number -= iter->number;

      } else {
        // The other cases are ignored
      }
    }
  }

  HAYAKU_IF_RETURN(0.0 == number, result);

  // Traverse the historical positions in reverse order to find the last
  // position record
  for (auto iter = ledger_.position_history_.rbegin();
       iter != ledger_.position_history_.rend(); ++iter) {
    if (iter->stock == stock) {
      result = *iter;
      break;
    }
  }

  HAYAKU_WARN_IF(result.stock != stock,
                 "Not found in the history positions, maybe exists error! {}",
                 stock);
  result.number = number;
  return result;
}

PositionRecord ExecutionRuntime::getShortPosition(const Stock& stock) const {
  HAYAKU_IF_RETURN(stock.isNull(), PositionRecord());
  position_map_type::const_iterator iter;
  iter = ledger_.short_position_.find(stock.id());
  return iter == ledger_.short_position_.end() ? PositionRecord()
                                               : iter->second;
}

BorrowRecordList ExecutionRuntime::getBorrowStockList() const {
  BorrowRecordList result;
  borrow_stock_map_type::const_iterator iter = ledger_.borrow_stock_.begin();
  for (; iter != ledger_.borrow_stock_.end(); ++iter) {
    result.push_back(iter->second);
  }
  return result;
}

bool ExecutionRuntime::checkin(const Datetime& datetime, price_t cash) {
  HAYAKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.3f}) must be > 0! ",
                         datetime, cash);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} datetime must be >= lastDatetime({})!", datetime,
                         lastDatetime());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  int precision = getParam<int>("precision");
  price_t in_cash = roundEx(cash, precision);
  ledger_.cash_ = roundEx(ledger_.cash_ + in_cash, precision);
  ledger_.checkin_cash_ = roundEx(ledger_.checkin_cash_ + in_cash, precision);
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), datetime, BUSINESS_CHECKIN, in_cash, in_cash, 0.0, 0,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  _saveAction(ledger_.trade_list_.back());
  return true;
}

bool ExecutionRuntime::checkout(const Datetime& datetime, price_t cash) {
  HAYAKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ",
                         datetime, cash);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} datetime must be >= lastDatetime({})!", datetime,
                         lastDatetime());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  int precision = getParam<int>("precision");
  price_t out_cash = roundEx(cash, precision);

  price_t tmp_cash = roundEx(ledger_.cash_ - out_cash, precision);
  HAYAKU_ERROR_IF_RETURN(tmp_cash < 0.0, false,
                         "{} cash({:<.4f}) must be <= current cash({:<.4f})!",
                         datetime, cash, ledger_.cash_);

  ledger_.cash_ = tmp_cash;
  ledger_.checkout_cash_ =
      roundEx(ledger_.checkout_cash_ + out_cash, precision);
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), datetime, BUSINESS_CHECKOUT, out_cash, out_cash, 0.0, 0,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  _saveAction(ledger_.trade_list_.back());
  return true;
}

bool ExecutionRuntime::checkinStock(const Datetime& datetime,
                                    const Stock& stock, price_t price,
                                    double number) {
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkin Null stock!",
                         datetime);
  HAYAKU_ERROR_IF_RETURN(number == 0, false, "{} {} number is zero!", datetime,
                         stock.market_code());
  HAYAKU_ERROR_IF_RETURN(price <= 0, false, "{} {} price({:<.4f}) must be > 0!",
                         datetime, stock.market_code(), price);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // Add it to the current positions
  int precision = getParam<int>("precision");
  price_t market_value = roundEx(price * number * stock.unit(), precision);
  position_map_type::iterator pos_iter = ledger_.position_.find(stock.id());
  if (pos_iter == ledger_.position_.end()) {
    PositionRecord pos(stock, datetime, Null<Datetime>(), number, 0.0, 0.0,
                       number, market_value, 0.0, 0.0, 0.0);
    pos.buyCount = 1;
    ledger_.position_[stock.id()] = pos;
  } else {
    PositionRecord& pos = pos_iter->second;
    pos.number += number;
    // pos.stoploss stays unchanged
    pos.totalNumber += number;
    pos.buyMoney = roundEx(pos.buyMoney + market_value, precision);
    // pos.totalCost stays unchanged
    // pos.totalRisk stays unchanged
    // pos.sellMoney stays unchanged
    pos.buyCount++;
  }

  // Add it to the trade records
  ledger_.trade_list_.push_back(TradeRecord(
      stock, datetime, BUSINESS_CHECKIN_STOCK, price, price, 0.0, number,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));

  // Update the record of the accumulated deposited asset value
  ledger_.checkin_stock_ =
      roundEx(ledger_.checkin_stock_ + market_value, precision);

  return true;
}

bool ExecutionRuntime::checkoutStock(const Datetime& datetime,
                                     const Stock& stock, price_t price,
                                     double number) {
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkout Null stock!",
                         datetime);
  HAYAKU_ERROR_IF_RETURN(number == 0, false, "{} {} checkout number is zero!",
                         datetime, stock.market_code());
  HAYAKU_ERROR_IF_RETURN(price <= 0.0, false,
                         "{} {} checkout price({:<.4f}) must be > 0.0! ",
                         datetime, stock.market_code(), price);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // Whether there is a current position
  position_map_type::iterator pos_iter = ledger_.position_.find(stock.id());
  HAYAKU_ERROR_IF_RETURN(pos_iter == ledger_.position_.end(), false,
                         "Try to checkout nonexistent stock!");

  PositionRecord& pos = pos_iter->second;
  // The withdrawn quantity exceeds the current position quantity
  HAYAKU_ERROR_IF_RETURN(
      number > pos.number, false,
      "{} {} Try to checkout number({}) beyond position number({})!", datetime,
      stock.market_code(), number, pos.number);

  int precision = getParam<int>("precision");
  pos.number -= number;
  pos.sellMoney =
      roundEx(pos.sellMoney + price * number * stock.unit(), precision);
  pos.sellCount++;

  // After the withdrawal all the current position quantities become 0, clear
  // the current position and store it into the historical positions
  if (0 == pos.number) {
    ledger_.position_history_.push_back(pos);
    ledger_.position_.erase(stock.id());
  }

  // Update the trade records
  ledger_.trade_list_.push_back(TradeRecord(
      stock, datetime, BUSINESS_CHECKOUT_STOCK, price, price, 0.0, number,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));

  // Update the accumulated withdrawn stock value
  ledger_.checkout_stock_ = roundEx(
      ledger_.checkout_stock_ - price * number * stock.unit(), precision);

  return true;
}

bool ExecutionRuntime::borrowCash(const Datetime& datetime, price_t cash) {
  HAYAKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0!",
                         datetime, cash);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} datetime must be >= lastDatetime({})!", datetime,
                         lastDatetime());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  int precision = getParam<int>("precision");
  price_t in_cash = roundEx(cash, precision);
  CostRecord cost = getBorrowCashCost(datetime, cash);
  ledger_.cash_ = roundEx(ledger_.cash_ + in_cash - cost.total, precision);
  ledger_.borrow_cash_ = roundEx(ledger_.borrow_cash_ + in_cash, precision);
  ledger_.loan_list_.push_back(LoanRecord(datetime, in_cash));
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), datetime, BUSINESS_BORROW_CASH, in_cash, in_cash, 0.0, 0,
      cost, 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  return true;
}

bool ExecutionRuntime::returnCash(const Datetime& datetime, price_t cash) {
  HAYAKU_ERROR_IF_RETURN(cash <= 0.0, false, "{} cash({:<.4f}) must be > 0! ",
                         datetime, cash);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} datetime must be >= lastDatetime({})!", datetime,
                         lastDatetime());
  HAYAKU_ERROR_IF_RETURN(ledger_.loan_list_.empty(), false,
                         "{} not borrow any cash!", datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < ledger_.loan_list_.back().datetime, false,
                         "{} must be >= the datetime({}) of last loan record!",
                         datetime, ledger_.loan_list_.back().datetime);

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  int precision = getParam<int>("precision");

  CostRecord cost, cur_cost;
  price_t in_cash = roundEx(cash, precision);
  price_t return_cash = in_cash;
  list<LoanRecord>::iterator iter = ledger_.loan_list_.begin();
  for (; iter != ledger_.loan_list_.end(); ++iter) {
    if (return_cash <= iter->value) {
      cur_cost = getReturnCashCost(iter->datetime, datetime, return_cash);
      return_cash = 0.0;
    } else {  // return_cash > loan.value
      cur_cost = getReturnCashCost(iter->datetime, datetime, iter->value);
      return_cash = roundEx(return_cash - iter->value, precision);
    }

    cost.commission = roundEx(cost.commission + cur_cost.commission, precision);
    cost.stamptax = roundEx(cost.stamptax + cur_cost.stamptax, precision);
    cost.transferfee =
        roundEx(cost.transferfee + cur_cost.transferfee, precision);
    cost.others = roundEx(cost.others + cur_cost.others, precision);
    cost.total = roundEx(cost.total + cur_cost.total, precision);
    if (return_cash == 0.0) break;
  }

  // The money to be returned is more than the actual debt
  HAYAKU_ERROR_IF_RETURN(return_cash != 0.0, false,
                         "{} return cash must <= borrowed cash!", datetime);

  price_t out_cash = roundEx(in_cash + cost.total, precision);
  HAYAKU_ERROR_IF_RETURN(out_cash > ledger_.cash_, false,
                         "{} cash({:<.4f}) must be <= current cash({:<.4f})!",
                         datetime, cash, ledger_.cash_);

  return_cash = in_cash;
  do {
    iter = ledger_.loan_list_.begin();
    if (return_cash == iter->value) {
      ledger_.loan_list_.pop_front();
      break;
    } else if (return_cash < iter->value) {
      iter->value = roundEx(iter->value - return_cash, precision);
      break;
    } else {  // return_cash > iter->value
      return_cash = roundEx(return_cash - iter->value, precision);
      ledger_.loan_list_.pop_front();
    }
  } while (!ledger_.loan_list_.empty());

  ledger_.cash_ = roundEx(ledger_.cash_ - out_cash, precision);
  ledger_.borrow_cash_ = roundEx(ledger_.borrow_cash_ - in_cash, precision);
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), datetime, BUSINESS_RETURN_CASH, in_cash, in_cash, 0.0, 0,
      cost, 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  return true;
}

bool ExecutionRuntime::borrowStock(const Datetime& datetime, const Stock& stock,
                                   price_t price, double number) {
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkin Null stock!",
                         datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0, false,
                         "{} {} Try to borrow number is zero!", datetime,
                         stock.market_code());
  HAYAKU_ERROR_IF_RETURN(price <= 0.0, false,
                         "{} {} price({:<.4f}) must be > 0!", datetime,
                         stock.market_code(), price);

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // Add it to the current positions
  int precision = getParam<int>("precision");
  price_t market_value = roundEx(price * number * stock.unit(), precision);
  CostRecord cost = getBorrowStockCost(datetime, stock, price, number);

  // Update the cash, deducting the cost spent when borrowing
  ledger_.cash_ = roundEx(ledger_.cash_ - cost.total, precision);

  // Add it to the trade records
  ledger_.trade_list_.push_back(
      TradeRecord(stock, datetime, BUSINESS_BORROW_STOCK, price, price, 0.0,
                  number, cost, 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));

  // Update the current borrowed stock information
  borrow_stock_map_type::iterator iter = ledger_.borrow_stock_.find(stock.id());
  if (iter == ledger_.borrow_stock_.end()) {
    BorrowRecord record(stock, number, market_value);
    BorrowRecord::Data data(datetime, price, number);
    record.record_list.push_back(data);
    ledger_.borrow_stock_[stock.id()] = record;
  } else {
    // iter->second.stock = stock;
    iter->second.number += number;
    iter->second.value = roundEx(iter->second.value + market_value, precision);
    BorrowRecord::Data data(datetime, price, number);
    iter->second.record_list.push_back(data);
  }

  return true;
}

bool ExecutionRuntime::returnStock(const Datetime& datetime, const Stock& stock,
                                   price_t price, double number) {
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), false, "{} Try checkout Null stock!",
                         datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), false,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0, false,
                         "{} {} return stock number is zero!", datetime,
                         stock.market_code());
  HAYAKU_ERROR_IF_RETURN(price <= 0.0, false,
                         "{} {} price({:<.4f}) must be > 0!", datetime,
                         stock.market_code(), price);

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  // Query the borrowed stock information
  borrow_stock_map_type::iterator bor_iter =
      ledger_.borrow_stock_.find(stock.id());

  // No stock was borrowed
  HAYAKU_ERROR_IF_RETURN(bor_iter == ledger_.borrow_stock_.end(), false,
                         "{} {} Try to return nonborrowed stock! ", datetime,
                         stock.market_code());

  BorrowRecord& bor = bor_iter->second;

  // The quantity to be returned is greater than the borrowed quantity
  HAYAKU_ERROR_IF_RETURN(number > bor.number, false,
                         "{} {} Try to return number({}) > borrow number({})!",
                         datetime, stock.market_code(), number, bor.number);

  // Update the borrowed stock information
  int precision = getParam<int>("precision");
  CostRecord cost, cur_cost;
  price_t market_value = 0.0;
  double remain_num = number;
  list<BorrowRecord::Data>::iterator iter = bor.record_list.begin();
  for (; iter != bor.record_list.end(); ++iter) {
    if (remain_num <= iter->number) {
      cur_cost =
          getReturnStockCost(iter->datetime, datetime, stock, price, number);
      market_value = roundEx(
          market_value + iter->price * remain_num * stock.unit(), precision);
      remain_num = 0;
    } else {  // number > iter->number
      cur_cost =
          getReturnStockCost(iter->datetime, datetime, stock, price, number);
      market_value = roundEx(
          market_value + iter->price * iter->number * stock.unit(), precision);
      remain_num -= iter->number;
    }

    cost.commission = roundEx(cost.commission + cur_cost.commission, precision);
    cost.stamptax = roundEx(cost.stamptax + cur_cost.stamptax, precision);
    cost.transferfee =
        roundEx(cost.transferfee + cur_cost.transferfee, precision);
    cost.others = roundEx(cost.others + cur_cost.others, precision);
    cost.total = roundEx(cost.total + cur_cost.total, precision);
    if (remain_num == 0) break;
  }

  bor.number -= number;
  bor.value = roundEx(bor.value - market_value, precision);

  remain_num = number;
  do {
    iter = bor.record_list.begin();
    if (remain_num == iter->number) {
      bor.record_list.pop_front();
      break;
    } else if (remain_num < iter->number) {
      iter->number -= remain_num;
      break;
    } else {  // remain_num > iter->number
      remain_num -= iter->number;
      bor.record_list.pop_front();
    }
  } while (!bor.record_list.empty());

  if (bor.record_list.empty()) {
    ledger_.borrow_stock_.erase(bor_iter);
  }

  // Update the cash, deducting the cost spent when returning
  ledger_.cash_ = roundEx(ledger_.cash_ - cost.total, precision);

  // Update the trade records
  ledger_.trade_list_.push_back(
      TradeRecord(stock, datetime, BUSINESS_RETURN_STOCK, price, price, 0.0,
                  number, cost, 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));

  return true;
}

TradeRecord ExecutionRuntime::buy(const Datetime& datetime, const Stock& stock,
                                  price_t realPrice, double number,
                                  price_t stoploss, price_t goalPrice,
                                  price_t planPrice, OrderOrigin from,
                                  const string& remark) {
  TradeRecord result;
  result.business = BUSINESS_INVALID;

  HAYAKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0.0, result, "{} {} numer is zero!",
                         datetime, stock.market_code());
  HAYAKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                         "{} {} Buy number({}) must be >= minTradeNumber({})!",
                         datetime, stock.market_code(), number,
                         stock.minTradeNumber());
  HAYAKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                         "{} {} Buy number({}) must be <= maxTradeNumber({})!",
                         datetime, stock.market_code(), number,
                         stock.maxTradeNumber());

#if 0  // The check here is cancelled to relax the restriction and improve the
       // efficiency; TM is only responsible for the trade management and is not
       // allowed to check
    // Check whether the daily line data exists for the day; when it does not exist it is regarded as not tradable
    bd::date daydate = datetime.date();
    KRecord krecord = stock.getKRecord(daydate, KQuery::DAY);
    if (!krecord.isValid()) {
        HAYAKU_ERROR(datetime << " " << stock.market_code()
                <<" Non-trading day(" << daydate
                << ") [ExecutionRuntime::buy]");
        return result;
    }

    // Whether the buy price is within the highest / lowest price range of the day
    if (realPrice > krecord.highPrice || realPrice < krecord.lowPrice) {
        HAYAKU_ERROR(datetime << " " << stock.market_code()
                << " Invalid buy price(" << realPrice
                << ")! out of highPrice(" << krecord.highPrice
                << ") or lowPrice(" << krecord.lowPrice
                << "! [ExecutionRuntime::buy]");
        return result;
    }
#endif

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  CostRecord cost = getBuyCost(datetime, stock, realPrice, number);

  // The cash needed by the actual trade = the trade quantity * the actual trade
  // price + the total trade cost
  int precision = getParam<int>("precision");
  // price_t money = roundEx(realPrice * number * stock.unit() + cost.total,
  // precision);
  price_t money = roundEx(realPrice * number * stock.unit(), precision);

  if (getParam<bool>("support_borrow_cash")) {
    // Get the required principal amount
    CostRecord bor_cost = getBorrowCashCost(datetime, money);
    double rate = getMarginRate(datetime, stock);
    price_t x =
        roundEx(ledger_.cash_ / rate + cost.total + bor_cost.total, precision);
    if (x < money) {
      // The financing that can be obtained is not enough, add the principal
      // automatically
      checkin(datetime, roundUp(money - x, precision));
    }

    // Financing, borrow the funds
    borrowCash(datetime, roundUp(money, precision));
  }

  HAYAKU_WARN_IF_RETURN(
      ledger_.cash_ < roundEx(money + cost.total, precision), result,
      "{} {} Can't buy, need cash({:<.4f}) > current cash({:<.4f})!", datetime,
      stock.market_code(), roundEx(money + cost.total, precision),
      ledger_.cash_);

  // Update the cash
  ledger_.cash_ = roundEx(ledger_.cash_ - money - cost.total, precision);

  // Add it to the trade records
  result = TradeRecord(stock, datetime, BUSINESS_BUY, planPrice, realPrice,
                       goalPrice, number, cost, stoploss, ledger_.cash_, from,
                       remark);
  ledger_.trade_list_.push_back(result);

  // Update the current position record
  position_map_type::iterator pos_iter = ledger_.position_.find(stock.id());
  if (pos_iter == ledger_.position_.end()) {
    PositionRecord position(
        stock, datetime, Null<Datetime>(), number, stoploss, goalPrice, number,
        money, cost.total,
        roundEx((realPrice - stoploss) * number * stock.unit(), precision),
        0.0);
    position.buyCount = 1;
    ledger_.position_[stock.id()] = position;
  } else {
    PositionRecord& position = pos_iter->second;
    position.number += number;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    position.totalNumber += number;
    position.buyMoney = roundEx(money + position.buyMoney, precision);
    position.totalCost = roundEx(cost.total + position.totalCost, precision);
    position.totalRisk = roundEx(
        position.totalRisk + (realPrice - stoploss) * number * stock.unit(),
        precision);
    position.buyCount++;
  }

  if (datetime > broker_last_datetime_) {
    list<OrderBrokerPtr>::const_iterator broker_iter = broker_list_.begin();
    for (; broker_iter != broker_list_.end(); ++broker_iter) {
      (*broker_iter)
          ->buy(datetime, stock.market(), stock.code(), realPrice, number,
                stoploss, goalPrice, from, remark);
      if (datetime > broker_last_datetime_) {
        broker_last_datetime_ = datetime;
      }
    }
  }

  _saveAction(result);

  return result;
}

TradeRecord ExecutionRuntime::sell(const Datetime& datetime, const Stock& stock,
                                   price_t realPrice, double number,
                                   price_t stoploss, price_t goalPrice,
                                   price_t planPrice, OrderOrigin from,
                                   const string& remark) {
  HAYAKU_CHECK(!std::isnan(number), "sell number should be a valid double!");
  TradeRecord result;

  HAYAKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0.0, result, "{} {} number is zero!",
                         datetime, stock.market_code());

  // For the case where the dividend and the capital increase make the quantity
  // not an integer multiple of the minimum trade quantity, the whole position
  // can only be sold with number=MAX_DOUBLE
  HAYAKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                         "{} {} Sell number({}) must be >= minTradeNumber({})!",
                         datetime, stock.market_code(), number,
                         stock.minTradeNumber());
  HAYAKU_ERROR_IF_RETURN(
      number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
      "{} {} Sell number({}) must be <= maxTradeNumber({})!", datetime,
      stock.market_code(), number, stock.maxTradeNumber());

  // There is no position
  position_map_type::iterator pos_iter = ledger_.position_.find(stock.id());
  HAYAKU_TRACE_IF_RETURN(
      pos_iter == ledger_.position_.end(), result,
      "{} {} This stock was not bought never! ({}, {:<.4f}, {}, {})", datetime,
      stock.market_code(), datetime, realPrice, number,
      getOrderOriginName(from));

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  PositionRecord& position = pos_iter->second;

  // Adjust the quantity to be sold; a sell quantity equal to MAX_DOUBLE means
  // selling everything
  double real_number = number == MAX_DOUBLE ? position.number : number;

  // The quantity to be sold is greater than the current position quantity
  HAYAKU_ERROR_IF_RETURN(
      position.number < real_number, result,
      "{} {} Try to sell number({}) > number of position({})!", datetime,
      stock.market_code(), real_number, position.number);

  CostRecord cost = getSellCost(datetime, stock, realPrice, real_number);

  int precision = getParam<int>("precision");
  price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

  // Update the cash balance
  ledger_.cash_ = roundEx(ledger_.cash_ + money - cost.total, precision);

  // Update the trade records
  result = TradeRecord(stock, datetime, BUSINESS_SELL, planPrice, realPrice,
                       goalPrice, real_number, cost, stoploss, ledger_.cash_,
                       from, remark);
  ledger_.trade_list_.push_back(result);

  // Update the current position
  position.number -= real_number;
  position.stoploss = stoploss;
  position.goalPrice = goalPrice;
  // position.buyMoney = position.buyMoney;
  position.totalCost = roundEx(position.totalCost + cost.total, precision);
  position.sellMoney = roundEx(position.sellMoney + money, precision);
  position.sellCount++;

  if (position.number == 0) {
    position.cleanDatetime = datetime;
    ledger_.position_history_.push_back(position);
    // Delete the current position
    ledger_.position_.erase(stock.id());
  }

  // Return the loan if there is one
  if (getParam<bool>("support_borrow_cash") && ledger_.borrow_cash_ > 0.0 &&
      ledger_.cash_ > 0.0) {
    returnCash(datetime, ledger_.borrow_cash_ < ledger_.cash_
                             ? ledger_.borrow_cash_
                             : ledger_.cash_);
  }

  if (datetime > broker_last_datetime_) {
    list<OrderBrokerPtr>::const_iterator broker_iter = broker_list_.begin();
    for (; broker_iter != broker_list_.end(); ++broker_iter) {
      (*broker_iter)
          ->sell(datetime, stock.market(), stock.code(), realPrice, real_number,
                 stoploss, goalPrice, from, remark);
      if (datetime > broker_last_datetime_) {
        broker_last_datetime_ = datetime;
      }
    }
  }

  _saveAction(result);

  return result;
}

TradeRecord ExecutionRuntime::sellShort(const Datetime& datetime,
                                        const Stock& stock, price_t realPrice,
                                        double number, price_t stoploss,
                                        price_t goalPrice, price_t planPrice,
                                        OrderOrigin from,
                                        const string& remark) {
  TradeRecord result;
  result.business = BUSINESS_INVALID;

  HAYAKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0, result, "{} {} numer is zero! ", datetime,
                         stock.market_code());
  HAYAKU_ERROR_IF_RETURN(number < stock.minTradeNumber(), result,
                         "{} {} Buy number({}) must be >= minTradeNumber({})!",
                         datetime, stock.market_code(), number,
                         stock.minTradeNumber());
  HAYAKU_ERROR_IF_RETURN(number > stock.maxTradeNumber(), result,
                         "{} {} Buy number({}) must be <= maxTradeNumber({})!",
                         datetime, stock.market_code(), number,
                         stock.maxTradeNumber());
  HAYAKU_ERROR_IF_RETURN(stoploss != 0.0 && stoploss < realPrice, result,
                         "{} {} Sell short's stoploss({:<.4f}) must be > "
                         "realPrice({:<.4f}) or = 0! ",
                         datetime, stock.market_code(), stoploss, realPrice);

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  int precision = getParam<int>("precision");

  if (getParam<bool>("support_borrow_stock")) {
    CostRecord cost = getSellCost(datetime, stock, realPrice, number);
    price_t money =
        roundEx(realPrice * number * stock.unit() + cost.total, precision);
    price_t x =
        roundEx(ledger_.cash_ / getMarginRate(datetime, stock), precision);
    if (x < money) {
      checkin(datetime, roundEx(money - x, precision));
    }

    borrowStock(datetime, stock, realPrice, number);
  }

  // Judge whether there is a borrowed stock and its quantity
  borrow_stock_map_type::const_iterator bor_iter;
  bor_iter = ledger_.borrow_stock_.find(stock.id());
  HAYAKU_ERROR_IF_RETURN(bor_iter == ledger_.borrow_stock_.end(), result,
                         "{} {} Non borrowed, can't sell short! ", datetime,
                         stock.market_code());

  double total_borrow_num = bor_iter->second.number;
  double can_sell_num = 0;
  position_map_type::iterator pos_iter =
      ledger_.short_position_.find(stock.id());
  if (pos_iter == ledger_.short_position_.end()) {
    // The borrowed stock has not been sold
    can_sell_num = total_borrow_num;

  } else {
    // The borrowed stock has been sold
    HAYAKU_ERROR_IF_RETURN(pos_iter->second.number >= total_borrow_num, result,
                           "{} {} Borrowed Stock had all selled!", datetime,
                           stock.market_code());

    // The quantity that can be sold = the total borrowed - the already sold
    // quantity
    can_sell_num = total_borrow_num - pos_iter->second.number;
  }

  // If the planned sell quantity is greater than the sellable quantity, sell
  // the sellable quantity
  double sell_num = number;
  if (number > can_sell_num) {
    sell_num = can_sell_num;
  }

  CostRecord cost = getSellCost(datetime, stock, realPrice, sell_num);

  price_t money =
      roundEx(realPrice * sell_num * stock.unit() - cost.total, precision);

  // Update the cash
  ledger_.cash_ = roundEx(ledger_.cash_ + money, precision);

  // Add it to the trade records
  result = TradeRecord(stock, datetime, BUSINESS_SELL_SHORT, planPrice,
                       realPrice, goalPrice, sell_num, cost, stoploss,
                       ledger_.cash_, from, remark);
  ledger_.trade_list_.push_back(result);

  // Update the current short position record
  price_t risk =
      roundEx((stoploss - realPrice) * sell_num * stock.unit(), precision);

  if (pos_iter == ledger_.short_position_.end()) {
    PositionRecord position(stock, datetime, Null<Datetime>(), sell_num,
                            stoploss, goalPrice, sell_num, cost.total,
                            cost.total, risk, money);
    position.sellCount = 1;
    ledger_.short_position_[stock.id()] = position;
  } else {
    PositionRecord& position = pos_iter->second;
    position.number += sell_num;
    position.stoploss = stoploss;
    position.goalPrice = goalPrice;
    position.totalNumber += sell_num;
    position.buyMoney = roundEx(position.buyMoney + cost.total);
    position.totalCost = roundEx(cost.total + position.totalCost, precision);
    position.totalRisk = roundEx(position.totalRisk + risk, precision);
    position.sellMoney = roundEx(position.sellMoney + money, precision);
    position.sellCount++;
  }

  if (datetime > broker_last_datetime_) {
    list<OrderBrokerPtr>::const_iterator broker_iter = broker_list_.begin();
    for (; broker_iter != broker_list_.end(); ++broker_iter) {
      (*broker_iter)
          ->sell(datetime, stock.market(), stock.code(), realPrice, sell_num,
                 stoploss, goalPrice, from, remark);
      if (datetime > broker_last_datetime_) {
        broker_last_datetime_ = datetime;
      }
    }
  }

  _saveAction(result);

  return result;
}

TradeRecord ExecutionRuntime::buyShort(const Datetime& datetime,
                                       const Stock& stock, price_t realPrice,
                                       double number, price_t stoploss,
                                       price_t goalPrice, price_t planPrice,
                                       OrderOrigin from, const string& remark) {
  TradeRecord result;
  HAYAKU_ERROR_IF_RETURN(stock.isNull(), result, "{} Stock is Null!", datetime);
  HAYAKU_ERROR_IF_RETURN(datetime < lastDatetime(), result,
                         "{} {} datetime must be >= lastDatetime({})!",
                         datetime, stock.market_code(), lastDatetime());
  HAYAKU_ERROR_IF_RETURN(number == 0, result, "{} {} number is zero!", datetime,
                         stock.market_code());
  HAYAKU_ERROR_IF_RETURN(
      number < stock.minTradeNumber(), result,
      "{} {} buyShort number({}) must be >= minTradeNumber({})!", datetime,
      stock.market_code(), number, stock.minTradeNumber());
  HAYAKU_ERROR_IF_RETURN(
      number != MAX_DOUBLE && number > stock.maxTradeNumber(), result,
      "{} {} buyShort number({}) must be <= maxTradeNumber({})!", datetime,
      stock.market_code(), number, stock.maxTradeNumber());

  // There is no short position
  position_map_type::iterator pos_iter =
      ledger_.short_position_.find(stock.id());
  HAYAKU_WARN_IF_RETURN(pos_iter == ledger_.short_position_.end(), result,
                        "{} {} This stock was not sell never! ", datetime,
                        stock.market_code());

  // Adjust the current position according to the ex-rights/ex-dividend
  // information
  updateWithWeight(datetime);

  PositionRecord& position = pos_iter->second;

  // Adjust the quantity to be bought; a buy quantity equal to MAX_DOUBLE or
  // greater than the actual position means buying everything
  double real_number = (number == MAX_DOUBLE || number > position.number)
                           ? position.number
                           : number;

  CostRecord cost = getBuyCost(datetime, stock, realPrice, real_number);

  int precision = getParam<int>("precision");
  price_t money = roundEx(realPrice * real_number * stock.unit(), precision);

  // Update the cash balance
  ledger_.cash_ = roundEx(ledger_.cash_ - money - cost.total, precision);

  // Update the trade records
  result = TradeRecord(stock, datetime, BUSINESS_BUY_SHORT, planPrice,
                       realPrice, goalPrice, real_number, cost, stoploss,
                       ledger_.cash_, from, remark);
  ledger_.trade_list_.push_back(result);

  // Update the current short position
  position.number -= real_number;
  position.buyMoney =
      roundEx(position.buyMoney + money + cost.total, precision);
  position.totalCost = roundEx(position.totalCost + cost.total, precision);
  // position.sellMoney = roundEx(position.sellMoney, precision);
  position.buyCount++;

  if (position.number == 0) {
    position.cleanDatetime = datetime;
    ledger_.short_position_history_.push_back(position);
    // Delete the current position
    ledger_.short_position_.erase(stock.id());
  }

  if (datetime > broker_last_datetime_) {
    list<OrderBrokerPtr>::const_iterator broker_iter = broker_list_.begin();
    for (; broker_iter != broker_list_.end(); ++broker_iter) {
      (*broker_iter)
          ->buy(datetime, stock.market(), stock.code(), realPrice, real_number,
                stoploss, goalPrice, from, remark);
      if (datetime > broker_last_datetime_) {
        broker_last_datetime_ = datetime;
      }
    }
  }

  if (getParam<bool>("support_borrow_stock")) {
    returnStock(datetime, stock, realPrice, real_number);
  }

  _saveAction(result);

  return result;
}

price_t ExecutionRuntime::cash(const Datetime& datetime, KQuery::KType ktype) {
  // If the given time is later than the last ex-rights/ex-dividend update time,
  // update the ex-rights/ex-dividend data first
  if (datetime > ledger_.last_update_datetime_) {
    updateWithWeight(datetime);
    return ledger_.cash_;
  }

  // If the given time equals the last ex-rights/ex-dividend update time, return
  // the current cash directly
  HAYAKU_IF_RETURN(datetime == ledger_.last_update_datetime_, ledger_.cash_);

  // If the given time is earlier than the last ex-rights/ex-dividend update
  // time, get the funds balance by calculating the assets at the given moment
  FundsRecord funds = getFunds(datetime, ktype);
  return funds.cash;
}

FundsRecord ExecutionRuntime::getFunds(KQuery::KType inktype) const {
  FundsRecord funds;
  int precision = getParam<int>("precision");

  string ktype(inktype);
  to_upper(ktype);

  price_t value{0.0};  // Current market value
  position_map_type::const_iterator iter = ledger_.position_.begin();
  for (; iter != ledger_.position_.end(); ++iter) {
    const PositionRecord& record = iter->second;
    auto price = record.stock.getMarketValue(lastDatetime(), ktype);
    value = roundEx((value + record.number * price * record.stock.unit()),
                    precision);
  }

  price_t short_value = 0.0;  // Current market value of the short position
  iter = ledger_.short_position_.begin();
  for (; iter != ledger_.short_position_.end(); ++iter) {
    const PositionRecord& record = iter->second;
    auto price = record.stock.getMarketValue(lastDatetime(), ktype);
    short_value = roundEx(
        (short_value + record.number * price * record.stock.unit()), precision);
  }
  funds.cash = ledger_.cash_;
  funds.market_value = value;
  funds.short_market_value = short_value;
  funds.base_cash = ledger_.checkin_cash_ - ledger_.checkout_cash_;
  funds.base_asset = ledger_.checkin_stock_ - ledger_.checkout_stock_;
  funds.borrow_cash = ledger_.borrow_cash_;
  funds.borrow_asset = 0;
  borrow_stock_map_type::const_iterator bor_iter =
      ledger_.borrow_stock_.begin();
  for (; bor_iter != ledger_.borrow_stock_.end(); ++bor_iter) {
    funds.borrow_asset += bor_iter->second.value;
  }
  return funds;
}

FundsRecord ExecutionRuntime::getFunds(const Datetime& indatetime,
                                       KQuery::KType ktype) {
  FundsRecord funds;
  int precision = getParam<int>("precision");

  // When datetime is Null, return the cash in the current account and the funds
  // occupied at the buy, as well as the accumulated deposit and withdrawal
  // funds
  HAYAKU_IF_RETURN(
      indatetime == Null<Datetime>() || indatetime == lastDatetime(),
      getFunds(ktype));

  Datetime datetime(indatetime.year(), indatetime.month(), indatetime.day(), 23,
                    59);
  price_t market_value = 0.0;
  price_t short_market_value = 0.0;
  if (datetime > lastDatetime()) {
    // Adjust the position according to the ex-rights/ex-dividend data
    updateWithWeight(datetime);

    // When the query date is later than or equal to the last trade date,
    // calculate the market value of the currently held securities directly
    position_map_type::const_iterator iter = ledger_.position_.begin();
    for (; iter != ledger_.position_.end(); ++iter) {
      price_t price = iter->second.stock.getMarketValue(datetime, ktype);
      market_value = roundEx(market_value + price * iter->second.number *
                                                iter->second.stock.unit(),
                             precision);
    }

    iter = ledger_.short_position_.begin();
    for (; iter != ledger_.short_position_.end(); ++iter) {
      price_t price = iter->second.stock.getMarketValue(datetime, ktype);
      short_market_value =
          roundEx(short_market_value +
                      price * iter->second.number * iter->second.stock.unit(),
                  precision);
    }

    funds.cash = ledger_.cash_;
    funds.market_value = market_value;
    funds.short_market_value = short_market_value;
    funds.base_cash = ledger_.checkin_cash_ - ledger_.checkout_cash_;
    funds.base_asset = ledger_.checkin_stock_ - ledger_.checkout_stock_;
    funds.borrow_cash = ledger_.borrow_cash_;
    funds.borrow_asset = 0;
    borrow_stock_map_type::iterator bor_iter = ledger_.borrow_stock_.begin();
    for (; bor_iter != ledger_.borrow_stock_.end(); ++bor_iter) {
      funds.borrow_asset += bor_iter->second.value;
    }

    return funds;
  }  // if datetime >= lastDatetime()

  // When the query date is earlier than the last trade date, traverse the trade
  // records and calculate the market value and the cash of that day
  price_t cash = ledger_.init_cash_;
  struct Stock_Number {
    Stock_Number() : number(0) {}
    Stock_Number(const Stock& stock, size_t number)
        : stock(stock), number(number) {}

    Stock stock;
    double number;
  };

  price_t checkin_cash = 0.0;
  price_t checkout_cash = 0.0;
  price_t checkin_stock = 0.0;
  price_t checkout_stock = 0.0;
  map<uint64_t, Stock_Number> stock_map;
  map<uint64_t, Stock_Number> short_stock_map;
  map<uint64_t, Stock_Number>::iterator stock_iter;
  map<uint64_t, Stock_Number>::iterator short_stock_iter;
  map<uint64_t, BorrowRecord> bor_stock_map;
  map<uint64_t, BorrowRecord>::iterator bor_stock_iter;

  TradeRecordList::const_iterator iter = ledger_.trade_list_.begin();
  for (; iter != ledger_.trade_list_.end(); ++iter) {
    if (iter->datetime > datetime) {
      // If the date of the trade record is later than the given date, break the
      // loop; it is done
      break;
    }

    cash = iter->cash;
    switch (iter->business) {
      case BUSINESS_INIT:
        checkin_cash += iter->realPrice;
        break;

      case BUSINESS_BUY:
      case BUSINESS_GIFT:
      case BUSINESS_SUOGU:
        stock_iter = stock_map.find(iter->stock.id());
        if (stock_iter != stock_map.end()) {
          stock_iter->second.number += iter->number;
        } else {
          stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
        }
        break;

      case BUSINESS_SELL:
        stock_iter = stock_map.find(iter->stock.id());
        if (stock_iter != stock_map.end()) {
          stock_iter->second.number -= iter->number;
        } else {
          HAYAKU_WARN("{} {} Sell error in m_ledger.m_tradeList!", datetime,
                      iter->stock.market_code());
        }
        break;

      case BUSINESS_SELL_SHORT:
        short_stock_iter = short_stock_map.find(iter->stock.id());
        if (short_stock_iter != short_stock_map.end()) {
          short_stock_iter->second.number += iter->number;
        } else {
          short_stock_map[iter->stock.id()] =
              Stock_Number(iter->stock, iter->number);
        }
        break;

      case BUSINESS_BUY_SHORT:
        short_stock_iter = short_stock_map.find(iter->stock.id());
        if (short_stock_iter != short_stock_map.end()) {
          short_stock_iter->second.number -= iter->number;
        } else {
          HAYAKU_WARN("{} {} BuyShort Error in m_ledger.m_tradeList!", datetime,
                      iter->stock.market_code());
        }
        break;

      case BUSINESS_BONUS:
        break;

      case BUSINESS_CHECKIN:
        checkin_cash += iter->realPrice;
        break;

      case BUSINESS_CHECKOUT:
        checkout_cash += iter->realPrice;
        break;

      case BUSINESS_CHECKIN_STOCK:
        stock_iter = stock_map.find(iter->stock.id());
        if (stock_iter != stock_map.end()) {
          stock_map[iter->stock.id()].number += iter->number;
        } else {
          stock_map[iter->stock.id()] = Stock_Number(iter->stock, iter->number);
        }
        checkin_stock = roundEx(
            checkin_stock + iter->realPrice * iter->number * iter->stock.unit(),
            precision);
        break;

      case BUSINESS_CHECKOUT_STOCK:
        stock_iter = stock_map.find(iter->stock.id());
        if (stock_iter != stock_map.end()) {
          stock_map[iter->stock.id()].number -= iter->number;
        } else {
          HAYAKU_WARN("{} {} CheckoutStock Error in m_ledger.m_tradeList!",
                      datetime, iter->stock.market_code());
        }
        checkout_stock =
            roundEx(checkout_stock +
                        iter->realPrice * iter->number * iter->stock.unit(),
                    precision);
        break;

      case BUSINESS_BORROW_CASH:
        funds.borrow_cash += iter->realPrice;
        break;

      case BUSINESS_RETURN_CASH:
        funds.borrow_cash -= iter->realPrice;
        break;

      case BUSINESS_BORROW_STOCK:
        funds.borrow_asset =
            roundEx(funds.borrow_asset +
                        iter->realPrice * iter->number * iter->stock.unit(),
                    precision);
        bor_stock_iter = bor_stock_map.find(iter->stock.id());
        if (bor_stock_iter == bor_stock_map.end()) {
          BorrowRecord bor;
          BorrowRecord::Data data(iter->datetime, iter->realPrice,
                                  iter->number);
          bor.record_list.push_back(data);
          bor_stock_map[iter->stock.id()] = bor;
        } else {
          BorrowRecord::Data data(iter->datetime, iter->realPrice,
                                  iter->number);
          bor_stock_iter->second.record_list.push_back(data);
        }
        break;

      case BUSINESS_RETURN_STOCK:
        bor_stock_iter = bor_stock_map.find(iter->stock.id());
        if (bor_stock_iter == bor_stock_map.end()) {
          HAYAKU_WARN("{} {} Error return stock in m_ledger.m_tradeList!",
                      iter->datetime, iter->stock.market_code());

        } else {
          BorrowRecord& bor = bor_stock_iter->second;
          size_t remain_num = iter->number;
          do {
            list<BorrowRecord::Data>::iterator bor_iter =
                bor.record_list.begin();
            if (remain_num == bor_iter->number) {
              funds.borrow_asset -= roundEx(
                  bor_iter->price * remain_num * iter->stock.unit(), precision);
              bor.record_list.pop_front();
              break;

            } else if (remain_num < bor_iter->number) {
              funds.borrow_asset -= roundEx(
                  bor_iter->price * remain_num * iter->stock.unit(), precision);
              bor_iter->number -= remain_num;
              break;

            } else {  // remain_num > bor_iter->number
              funds.borrow_asset -= roundEx(
                  bor_iter->price * bor_iter->number * iter->stock.unit(),
                  precision);
              remain_num -= bor_iter->number;
              bor.record_list.pop_front();
            }
          } while (!bor.record_list.empty());

          if (bor.record_list.empty()) {
            bor_stock_map.erase(bor_stock_iter);
          }
        }

        break;

      default:
        HAYAKU_WARN("{} {} Unknown business in m_ledger.m_tradeList!", datetime,
                    iter->stock.market_code());
        break;
    }
  }

  stock_iter = stock_map.begin();
  for (; stock_iter != stock_map.end(); ++stock_iter) {
    const size_t& number = stock_iter->second.number;
    if (number == 0) {
      continue;
    }

    price_t price = stock_iter->second.stock.getMarketValue(datetime, ktype);
    market_value =
        roundEx(market_value + price * number * stock_iter->second.stock.unit(),
                precision);
  }

  short_stock_iter = short_stock_map.begin();
  for (; short_stock_iter != short_stock_map.end(); ++short_stock_iter) {
    const size_t& number = short_stock_iter->second.number;
    if (number == 0) {
      continue;
    }

    price_t price =
        short_stock_iter->second.stock.getMarketValue(datetime, ktype);
    short_market_value = roundEx(
        short_market_value + price * number * stock_iter->second.stock.unit(),
        precision);
  }

  funds.cash = cash;
  funds.market_value = market_value;
  funds.short_market_value = short_market_value;
  funds.base_cash = checkin_cash - checkout_cash;
  funds.base_asset = checkin_stock - checkout_stock;
  return funds;
}

PriceList ExecutionRuntime::getProfitCurve(const DatetimeList& dates,
                                           KQuery::KType ktype) {
  PriceList result;
  result.reserve(dates.size());
  const int accountPrecision = precision();
  for (const auto& datetime : dates) {
    result.emplace_back(
        roundEx(getFunds(datetime, ktype).profit(), accountPrecision));
  }
  return result;
}

/******************************************************************************
 *  Every time a trade operation is executed, adjust the held position and the
 *cash records according to the ex-rights/ex-dividend information first A lazy
 *update strategy is adopted, i.e. the current position and the assets are
 *updated only when the current position information is needed or a sell happens
 *Input parameter: the date of this operation History: 1) added on 2009/12/22
 *****************************************************************************/
void ExecutionRuntime::updateWithWeight(const Datetime& datetime) {
  HAYAKU_IF_RETURN(datetime <= ledger_.last_update_datetime_, void());

  // Query date range of the ex-rights/ex-dividend information
  Datetime start_date(lastDatetime().date() + bd::days(1));
  Datetime end_date(datetime.date() + bd::days(1));

  int precision = getParam<int>("precision");
  TradeRecordList new_trade_buffer;

  // Update the position information and cache the newly added trade records
  position_map_type::iterator position_iter = ledger_.position_.begin();
  for (; position_iter != ledger_.position_.end(); ++position_iter) {
    PositionRecord& position = position_iter->second;
    Stock stock = position.stock;

    StockWeightList weights = stock.getWeight(start_date, end_date);
    StockWeightList::const_iterator weight_iter = weights.begin();
    for (; weight_iter != weights.end(); ++weight_iter) {
      // Skip it when there is no dividend and the numbers of the bonus shares
      // and the capitalized shares are both zero
      if (0.0 == weight_iter->bonus() && 0.0 == weight_iter->countAsGift() &&
          0.0 == weight_iter->increasement() && 0.0 == weight_iter->suogu()) {
        continue;
      }

      // It must be done before the bonus shares are added, because the bonus
      // shares change the position quantity
      if (weight_iter->bonus() != 0.0) {
        price_t bonus =
            roundEx(position.number * weight_iter->bonus() * 0.1, precision);
        position.sellMoney += bonus;
        ledger_.cash_ += bonus;

        TradeRecord record(stock, weight_iter->datetime(), BUSINESS_BONUS,
                           bonus, bonus, 0.0, 0, CostRecord(), 0.0,
                           ledger_.cash_, OrderOrigin::UNSPECIFIED);
        new_trade_buffer.push_back(record);
      }

      double addcount =
          (position.number / 10.0) *
          (weight_iter->countAsGift() + weight_iter->increasement());
      if (addcount != 0.0) {
        position.number += addcount;
        position.totalNumber += addcount;
        TradeRecord record(stock, weight_iter->datetime(), BUSINESS_GIFT, 0.0,
                           0.0, 0.0, addcount, CostRecord(), 0.0, ledger_.cash_,
                           OrderOrigin::UNSPECIFIED);
        new_trade_buffer.push_back(record);
      }

      if (weight_iter->suogu() > 0.0) {
        double suogu_number = position.number * weight_iter->suogu();
        double change_number = 0.0;
        if (suogu_number < position.number) {
          // The share contraction adopts the round-up
          double old_number = position.number;
          position.number = roundUp(suogu_number, 0);
          change_number = position.number - old_number;
        } else if (suogu_number > position.number) {
          // The share expansion adopts the truncation
          double old_number = position.number;
          position.number = roundDown(suogu_number, 0);
          change_number = position.number - old_number;
        }

        if (change_number != 0.0) {
          TradeRecord record(stock, weight_iter->datetime(), BUSINESS_SUOGU,
                             0.0, 0.0, 0.0, change_number, CostRecord(), 0.0,
                             ledger_.cash_, OrderOrigin::UNSPECIFIED);
          new_trade_buffer.push_back(record);
        }
      }

    } /* for weight */
  } /* for position */

  std::sort(
      new_trade_buffer.begin(), new_trade_buffer.end(),
      std::bind(std::less<Datetime>(),
                std::bind(&TradeRecord::datetime, std::placeholders::_1),
                std::bind(&TradeRecord::datetime, std::placeholders::_2)));

  size_t total = new_trade_buffer.size();
  for (size_t i = 0; i < total; ++i) {
    if (new_trade_buffer[i].business == BUSINESS_BONUS) {
      price_t bonus = new_trade_buffer[i].realPrice;
      for (size_t j = i; j < total; ++j) {
        new_trade_buffer[j].cash += bonus;
      }
    }
  }

  for (size_t i = 0; i < total; ++i) {
    ledger_.trade_list_.push_back(new_trade_buffer[i]);
  }

  ledger_.last_update_datetime_ = datetime;
}

void ExecutionRuntime::_saveAction(const TradeRecord& record) {
  HAYAKU_IF_RETURN(getParam<bool>("save_action") == false, void());
  std::stringstream buf(std::stringstream::out);
  string account("account.");
  string sep(", ");
  switch (record.business) {
    case BUSINESS_INIT:
      buf << "execution = "
             "ExecutionEngine(AccountConfig(init_datetime=Datetime('"
          << record.datetime.str() << "'), " << "initial_cash=" << record.cash
          << sep << "cost_policy=";
      if (costfunc_) {
        buf << costfunc_->name() << "("
            << costfunc_->getParameter().getNameValueList() << "), "
            << "name='" << name_ << "'" << "))";
      } else {
        buf << "TC_Zero()))";
      }
      break;

    case BUSINESS_CHECKIN:
      buf << account << "checkin(Datetime('" << record.datetime.str() << "'), "
          << record.cash << ")";
      break;

    case BUSINESS_CHECKOUT:
      buf << account << "checkout(Datetime('" << record.datetime.str() << "'), "
          << record.cash << ")";
      break;

    case BUSINESS_BUY:
      buf << "execution.submit(OrderRequest(OrderSide.BUY, Datetime('"
          << record.datetime.str() << "'), " << "session.data.get_stock('"
          << record.stock.market_code() << "'), " << record.realPrice << sep
          << record.number << sep << record.stoploss << sep << record.goalPrice
          << sep << record.planPrice << sep << "OrderOrigin("
          << static_cast<unsigned>(record.from) << ")" << sep << "\""
          << record.remark << "\"))";
      break;

    case BUSINESS_SELL:
      buf << "execution.submit(OrderRequest(OrderSide.SELL, Datetime('"
          << record.datetime.str() << "'), " << "session.data.get_stock('"
          << record.stock.market_code() << "'), " << record.realPrice << sep
          << record.number << sep << record.stoploss << sep << record.goalPrice
          << sep << record.planPrice << sep << "OrderOrigin("
          << static_cast<unsigned>(record.from) << ")" << sep << "\""
          << record.remark << "\"))";
      break;

    default:
      break;
  }

  ledger_.actions_.push_back(buf.str());
}

void ExecutionRuntime::tocsv(const string& path) {
  string filename1, filename2, filename3, filename4;
  if (name_.empty()) {
    string date = ledger_.init_datetime_.str();
    filename1 = path + "/" + date + "_交易记录.csv";
    filename2 = path + "/" + date + "_已平仓记录.csv";
    filename3 = path + "/" + date + "_未平仓记录.csv";
    filename4 = path + "/" + date + "_actions.txt";
  } else {
    filename1 = path + "/" + name_ + "_交易记录.csv";
    filename2 = path + "/" + name_ + "_已平仓记录.csv";
    filename3 = path + "/" + name_ + "_未平仓记录.csv";
    filename4 = path + "/" + name_ + "_actions.txt";
  }

#if defined(_MSC_VER)
  filename1 = utf8_to_gb(filename1);
  filename2 = utf8_to_gb(filename2);
  filename3 = utf8_to_gb(filename3);
  filename4 = utf8_to_gb(filename4);
#endif

  string sep(",");

  // Export the trade records
  std::ofstream file(filename1.c_str());
  HAYAKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename1);

  file.setf(std::ios_base::fixed);
  file.precision(3);
  file << "#成交日期,证券代码,证券名称,业务名称,计划交易价格,"
          "实际成交价格,目标价格,成交数量,佣金,印花税,过户费,其他成本,"
          "交易总成本,"
          "止损价,现金余额,信号来源,日期,开盘价,最高价,最低价,收盘价,"
          "成交金额,成交量,备注"
       << std::endl;
  TradeRecordList::const_iterator trade_iter = ledger_.trade_list_.begin();
  for (; trade_iter != ledger_.trade_list_.end(); ++trade_iter) {
    const TradeRecord& record = *trade_iter;
    if (record.stock.isNull()) {
      file << record.datetime << sep << sep << sep
           << getBusinessName(record.business) << sep << record.planPrice << sep
           << record.realPrice << sep << record.goalPrice << sep
           << record.number << sep << record.cost.commission << sep
           << record.cost.stamptax << sep << record.cost.transferfee << sep
           << record.cost.others << sep << record.cost.total << sep
           << record.stoploss << sep << record.cash << sep
           << getOrderOriginName(record.from) << sep << sep << sep << sep << sep
           << sep << sep << sep << record.remark << std::endl;
    } else {
      file << record.datetime << sep << record.stock.market_code() << sep
           << record.stock.name() << sep << getBusinessName(record.business)
           << sep << record.planPrice << sep << record.realPrice << sep
           << record.goalPrice << sep << record.number << sep
           << record.cost.commission << sep << record.cost.stamptax << sep
           << record.cost.transferfee << sep << record.cost.others << sep
           << record.cost.total << sep << record.stoploss << sep << record.cash
           << sep << getOrderOriginName(record.from) << sep;
      if (BUSINESS_BUY == record.business || BUSINESS_SELL == record.business) {
        KRecord kdata = record.stock.getKRecord(record.datetime, KQuery::DAY);
        if (kdata.isValid()) {
          file << kdata.datetime << sep << kdata.openPrice << sep
               << kdata.highPrice << sep << kdata.lowPrice << sep
               << kdata.closePrice << sep << kdata.transAmount << sep
               << kdata.transCount << sep << record.remark;
        } else {
          file << sep << sep << sep << sep << sep << sep << sep << sep
               << record.remark;
        }
      } else {
        file << sep << sep << sep << sep << sep << sep << sep << record.remark;
      }
      file << std::endl;
    }
  }
  file.close();

  // Export the closed position records
  file.open(filename2.c_str());
  HAYAKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename2);
  file << "#建仓日期,平仓日期,证券代码,证券名称,累计持仓数量,"
          "累计花费资金,累计交易成本,已转化资金,总盈利,累积风险,赢亏比率,"
          "持仓天数,累计买入次数,"
          "累计卖出次数"
       << std::endl;
  PositionRecordList::const_iterator history_iter =
      ledger_.position_history_.begin();
  for (; history_iter != ledger_.position_history_.end(); ++history_iter) {
    const PositionRecord& record = *history_iter;
    file << record.takeDatetime << sep << record.cleanDatetime << sep
         << record.stock.market_code() << sep << record.stock.name() << sep
         << record.totalNumber << sep << record.buyMoney << sep
         << record.totalCost << sep << record.sellMoney << sep
         << record.sellMoney - record.totalCost - record.buyMoney << sep
         << record.totalRisk << sep
         << record.totalProfit() / (record.buyMoney + record.totalCost) << sep
         << (record.cleanDatetime - record.takeDatetime).days() << sep
         << record.buyCount << sep << record.sellCount << std::endl;
  }
  file.close();

  // Export the open position records
  file.open(filename3.c_str());
  HAYAKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename3);
  file << "#建仓日期,平仓日期,证券代码,证券名称,当前持仓数量,累计持仓数量,"
          "累计花费资金,累计交易成本,已转化资金,累积风险,累计买入次数,"
          "累计卖出次数,"
          "累计浮动盈亏,当前盈亏成本价, 浮动盈亏比率"
       << std::endl;
  position_map_type::const_iterator position_iter = ledger_.position_.begin();
  for (; position_iter != ledger_.position_.end(); ++position_iter) {
    const PositionRecord& record = position_iter->second;
    file << record.takeDatetime << sep << record.cleanDatetime << sep
         << record.stock.market_code() << sep << record.stock.name() << sep
         << record.number << sep << record.totalNumber << sep << record.buyMoney
         << sep << record.totalCost << sep << record.sellMoney << sep
         << record.totalRisk << sep << record.buyCount << sep
         << record.sellCount << sep;
    size_t pos = record.stock.getCount(KQuery::DAY);
    if (pos != 0) {
      KRecord krecord = record.stock.getKRecord(pos - 1, KQuery::DAY);
      price_t bonus = record.buyMoney - record.sellMoney - record.totalCost;
      auto sellCost = getSellCost(krecord.datetime, record.stock,
                                  krecord.closePrice, record.number);
      price_t profit = record.number * krecord.closePrice + record.sellMoney -
                       record.buyMoney - record.totalCost - sellCost.total;
      file << record.number * krecord.closePrice - bonus << sep
           << bonus / record.number << sep
           << profit / (record.buyMoney + record.totalCost + sellCost.total)
           << std::endl;
    }
  }
  file.close();

  // Export the order execution commands
  // Export the closed position records
  file.open(filename4.c_str());
  HAYAKU_ERROR_IF_RETURN(!file, void(), "Can't create file {}!", filename4);
  list<string>::const_iterator action_iter = ledger_.actions_.begin();
  for (; action_iter != ledger_.actions_.end(); ++action_iter) {
    file << *action_iter << std::endl;
  }
  file.close();
}

bool ExecutionRuntime::addPosition(const PositionRecord& pr) {
  HAYAKU_ERROR_IF_RETURN(pr.stock.isNull(), false,
                         "Invalid position record! stock is null!");
  HAYAKU_ERROR_IF_RETURN(pr.cleanDatetime != Null<Datetime>(), false,
                         "Position cleanDatetime({}) must be Null!",
                         pr.cleanDatetime);
  HAYAKU_ERROR_IF_RETURN(pr.takeDatetime < initDatetime(), false,
                         "Position takeDatetime({}) > initDatetime({})",
                         pr.takeDatetime, initDatetime());
  HAYAKU_ERROR_IF_RETURN(!ledger_.trade_list_.empty(), false,
                         "Exist trade list!");

  auto iter = ledger_.position_.find(pr.stock.id());
  HAYAKU_ERROR_IF_RETURN(iter != ledger_.position_.end(), false,
                         "The stock({}) has position!", pr.stock.market_code());

  ledger_.position_[pr.stock.id()] = pr;
  if (pr.takeDatetime > ledger_.init_datetime_) {
    ledger_.init_datetime_ = pr.takeDatetime;
  }
  return true;
}

bool ExecutionRuntime::addTradeRecord(const TradeRecord& tr) {
  HAYAKU_IF_RETURN(BUSINESS_INIT == tr.business, _add_init_tr(tr));
  HAYAKU_ERROR_IF_RETURN(tr.datetime < lastDatetime(), false,
                         "tr.datetime must be >= lastDatetime({})!",
                         lastDatetime());

  updateWithWeight(tr.datetime);

  switch (tr.business) {
    case BUSINESS_INIT:
      return false;

    case BUSINESS_BUY:
      return _add_buy_tr(tr);

    case BUSINESS_SELL:
      return _add_sell_tr(tr);

    case BUSINESS_GIFT:
    case BUSINESS_SUOGU:
      return true;

    case BUSINESS_BONUS:
      return true;

    case BUSINESS_CHECKIN:
      return _add_checkin_tr(tr);

    case BUSINESS_CHECKOUT:
      return _add_checkout_tr(tr);

    case BUSINESS_CHECKIN_STOCK:
      return _add_checkin_stock_tr(tr);

    case BUSINESS_CHECKOUT_STOCK:
      return _add_checkout_stock_tr(tr);

    case BUSINESS_BORROW_CASH:
      return _add_borrow_cash_tr(tr);

    case BUSINESS_RETURN_CASH:
      return _add_return_cash_tr(tr);

    case BUSINESS_BORROW_STOCK:
      return _add_borrow_stock_tr(tr);

    case BUSINESS_RETURN_STOCK:
      return _add_return_stock_tr(tr);

    case BUSINESS_SELL_SHORT:
      return _add_sell_short_tr(tr);

    case BUSINESS_BUY_SHORT:
      return _add_buy_short_tr(tr);

    case BUSINESS_INVALID:
    default:
      HAYAKU_ERROR("tr.business is invalid({})!", int(tr.business));
      return false;
  }

  return false;
}

bool ExecutionRuntime::_add_init_tr(const TradeRecord& tr) {
  assert(BUSINESS_INIT == tr.business);

  ledger_.init_datetime_ = tr.datetime;
  ledger_.init_cash_ = roundEx(tr.realPrice, getParam<int>("precision"));
  reset();

  return true;
}

bool ExecutionRuntime::_add_buy_tr(const TradeRecord& tr) {
  HAYAKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is null!");
  HAYAKU_ERROR_IF_RETURN(tr.number == 0, false, "tr.number is zero!");
  HAYAKU_ERROR_IF_RETURN(tr.number < tr.stock.minTradeNumber() ||
                             tr.number > tr.stock.maxTradeNumber(),
                         false, "tr.number out of range!");

  int precision = getParam<int>("precision");
  TradeRecord new_tr(tr);
  price_t money =
      roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

  HAYAKU_WARN_IF_RETURN(
      ledger_.cash_ < roundEx(money + tr.cost.total, precision), false,
      "Don't have enough money! {} < {}, {}", ledger_.cash_,
      roundEx(money + tr.cost.total, precision), tr);

  ledger_.cash_ = roundEx(ledger_.cash_ - money - tr.cost.total, precision);
  new_tr.cash = ledger_.cash_;
  ledger_.trade_list_.push_back(new_tr);

  // Update the current position record
  position_map_type::iterator pos_iter = ledger_.position_.find(tr.stock.id());
  if (pos_iter == ledger_.position_.end()) {
    PositionRecord position(
        tr.stock, tr.datetime, Null<Datetime>(), tr.number, tr.stoploss,
        tr.goalPrice, tr.number, money, tr.cost.total,
        roundEx((tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(),
                precision),
        0.0);
    position.buyCount = 1;
    ledger_.position_[tr.stock.id()] = position;
  } else {
    PositionRecord& position = pos_iter->second;
    position.number += tr.number;
    position.stoploss = tr.stoploss;
    position.goalPrice = tr.goalPrice;
    position.totalNumber += tr.number;
    position.buyMoney = roundEx(money + position.buyMoney, precision);
    position.totalCost = roundEx(tr.cost.total + position.totalCost, precision);
    position.totalRisk =
        roundEx(position.totalRisk +
                    (tr.realPrice - tr.stoploss) * tr.number * tr.stock.unit(),
                precision);
    position.buyCount++;
    ;
  }

  _saveAction(new_tr);

  return true;
}

bool ExecutionRuntime::_add_sell_tr(const TradeRecord& tr) {
  HAYAKU_ERROR_IF_RETURN(tr.stock.isNull(), false, "tr.stock is Null!");
  HAYAKU_ERROR_IF_RETURN(tr.number == 0, false, "tr.number is zero!");

  // There is no position
  position_map_type::iterator pos_iter = ledger_.position_.find(tr.stock.id());
  HAYAKU_ERROR_IF_RETURN(pos_iter == ledger_.position_.end(), false,
                         "No position!");

  PositionRecord& position = pos_iter->second;

  // The quantity to be sold is greater than the current position quantity
  HAYAKU_ERROR_IF_RETURN(position.number < tr.number, false,
                         "Try sell number greater position!");

  int precision = getParam<int>("precision");
  price_t money =
      roundEx(tr.realPrice * tr.number * tr.stock.unit(), precision);

  // Update the cash balance
  ledger_.cash_ = roundEx(ledger_.cash_ + money - tr.cost.total, precision);

  // Update the trade records
  TradeRecord new_tr(tr);
  new_tr.cash = ledger_.cash_;
  ledger_.trade_list_.push_back(new_tr);

  // Update the current position
  position.number -= tr.number;
  position.stoploss = tr.stoploss;
  position.goalPrice = tr.goalPrice;
  // position.buyMoney = position.buyMoney;
  position.totalCost = roundEx(position.totalCost + tr.cost.total, precision);
  position.sellMoney = roundEx(position.sellMoney + money, precision);
  position.sellCount++;

  if (position.number == 0) {
    position.cleanDatetime = tr.datetime;
    ledger_.position_history_.push_back(position);
    // Delete the current position
    ledger_.position_.erase(tr.stock.id());
  }

  _saveAction(new_tr);

  return true;
}

bool ExecutionRuntime::_add_checkin_tr(const TradeRecord& tr) {
  HAYAKU_ERROR_IF_RETURN(tr.realPrice <= 0.0, false, "tr.realPrice <= 0.0!");
  int precision = getParam<int>("precision");
  price_t in_cash = roundEx(tr.realPrice, precision);
  ledger_.cash_ = roundEx(ledger_.cash_ + in_cash, precision);
  ledger_.checkin_cash_ = roundEx(ledger_.checkin_cash_ + in_cash, precision);
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), tr.datetime, BUSINESS_CHECKIN, in_cash, in_cash, 0.0, 0,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  _saveAction(ledger_.trade_list_.back());
  return true;
}

bool ExecutionRuntime::_add_checkout_tr(const TradeRecord& tr) {
  HAYAKU_ERROR_IF_RETURN(tr.realPrice <= 0.0, false, "tr.realPrice <= 0.0!");

  int precision = getParam<int>("precision");
  price_t out_cash = roundEx(tr.realPrice, precision);
  HAYAKU_ERROR_IF_RETURN(out_cash > ledger_.cash_, false,
                         "Checkout money > current cash!");

  ledger_.cash_ = roundEx(ledger_.cash_ - out_cash, precision);
  ledger_.checkout_cash_ =
      roundEx(ledger_.checkout_cash_ + out_cash, precision);
  ledger_.trade_list_.push_back(TradeRecord(
      Null<Stock>(), tr.datetime, BUSINESS_CHECKOUT, out_cash, out_cash, 0.0, 0,
      CostRecord(), 0.0, ledger_.cash_, OrderOrigin::UNSPECIFIED));
  _saveAction(ledger_.trade_list_.back());
  return true;
}

bool ExecutionRuntime::_add_checkin_stock_tr(const TradeRecord& tr) {
  // TODO: ExecutionRuntime::_add_checkin_stock_tr
  return false;
}

bool ExecutionRuntime::_add_checkout_stock_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_borrow_cash_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_return_cash_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_borrow_stock_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_return_stock_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_sell_short_tr(const TradeRecord& tr) {
  return false;
}

bool ExecutionRuntime::_add_buy_short_tr(const TradeRecord& tr) {
  return false;
}

} /* namespace hayaku */
