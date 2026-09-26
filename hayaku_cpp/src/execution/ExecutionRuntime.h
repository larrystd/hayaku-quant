#pragma once

/*
 * ExecutionRuntime.h
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include "AccountConfig.h"
#include "AccountView.h"
#include "ExecutionAccountPort.h"
#include "ExecutionBrokerPort.h"
#include "Ledger.h"
#include "PortfolioAccountPort.h"
#include "broker/OrderBrokerBase.h"
#include "common/Parameter.h"
#include "pricing/TradeCosts.h"

namespace hayaku {

/**
 * Trade management module of the backtest simulated account; it manages the
 * trade records and the fund usage of the account
 * @details
 * <pre>
 * Default parameters:
 * precision(int): 2 calculation precision
 * support_borrow_cash(bool): false whether cash is borrowed automatically on a
 * buy operation support_borrow_stock(bool): false whether the stock is borrowed
 * automatically when short selling
 * </pre>
 * @ingroup ExecutionRuntimeClass
 */
class ExecutionRuntime final : public internal::PortfolioAccountPort,
                               public internal::ExecutionBrokerPort {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  explicit ExecutionRuntime(const Datetime& datetime = Datetime(199001010000LL),
                            price_t initcash = 100000.0,
                            const TradeCostPtr& costfunc = TC_Zero(),
                            const string& name = "SYS");
  explicit ExecutionRuntime(const AccountConfig& config);
  ~ExecutionRuntime();

  [[nodiscard]] const string& name() const noexcept { return name_; }

  [[nodiscard]] const TradeCostPtr& costFunc() const noexcept {
    return costfunc_;
  }

  [[nodiscard]] CostRecord getSellCost(const Datetime& datetime,
                                       const Stock& stock, price_t price,
                                       double number) const {
    return costfunc_ ? costfunc_->getSellCost(datetime, stock, price, number)
                      : CostRecord();
  }

  [[nodiscard]] CostRecord getBorrowCashCost(const Datetime& datetime,
                                             price_t cash) const {
    return costfunc_ ? costfunc_->getBorrowCashCost(datetime, cash)
                      : CostRecord();
  }

  [[nodiscard]] CostRecord getReturnCashCost(const Datetime& borrowDatetime,
                                             const Datetime& returnDatetime,
                                             price_t cash) const {
    return costfunc_ ? costfunc_->getReturnCashCost(borrowDatetime,
                                                      returnDatetime, cash)
                      : CostRecord();
  }

  [[nodiscard]] CostRecord getBorrowStockCost(const Datetime& datetime,
                                              const Stock& stock, price_t price,
                                              double number) const {
    return costfunc_
               ? costfunc_->getBorrowStockCost(datetime, stock, price, number)
               : CostRecord();
  }

  [[nodiscard]] CostRecord getReturnStockCost(const Datetime& borrowDatetime,
                                              const Datetime& returnDatetime,
                                              const Stock& stock, price_t price,
                                              double number) const {
    return costfunc_
               ? costfunc_->getReturnStockCost(borrowDatetime, returnDatetime,
                                                stock, price, number)
               : CostRecord();
  }

  void regBroker(const OrderBrokerPtr& broker) override {
    if (broker) {
      broker_list_.push_back(broker);
    }
  }

  void clearBroker() override { broker_list_.clear(); }

  [[nodiscard]] Datetime getBrokerLastDatetime() const noexcept override {
    return broker_last_datetime_;
  }

  void setBrokerLastDatetime(const Datetime& datetime) noexcept override {
    broker_last_datetime_ = datetime;
  }

  void fetchAssetInfoFromBroker(
      const OrderBrokerPtr& broker,
      const Datetime& datetime = Null<Datetime>()) override;

  [[nodiscard]] AccountId accountId() const noexcept override {
    return ledger_.account_id_;
  }

  void reset() override { _reset(); }

  [[nodiscard]] ExecutionReport submit(const OrderRequest& request) override;

  [[nodiscard]] int precision() const override {
    return getParam<int>("precision");
  }

  [[nodiscard]] CostRecord getBuyCost(const Datetime& datetime,
                                      const Stock& stock, price_t price,
                                      double number) const override {
    return costfunc_ ? costfunc_->getBuyCost(datetime, stock, price, number)
                      : CostRecord();
  }

  [[nodiscard]] bool supportsBorrowCash() const override {
    return getParam<bool>("support_borrow_cash");
  }

  [[nodiscard]] bool supportsBorrowStock() const override {
    return getParam<bool>("support_borrow_stock");
  }

  [[nodiscard]] AccountView view() const;

  [[nodiscard]] shared_ptr<ExecutionRuntime> cloneRuntime() const;

  [[nodiscard]] internal::PortfolioAccountPortPtr cloneAccount() const override;
  [[nodiscard]] internal::PortfolioAccountPortPtr createChildAccount(
      string name, price_t initialCash = 0.0) const override;

  /** Reset, clearing the trade and position records */
  void _reset();

  /**
   * Get the margin rate of the given security
   * @param datetime date
   * @param stock the given security
   */
  virtual double getMarginRate(const Datetime& datetime, const Stock& stock);

  /** Initial cash */
  price_t initCash() const override { return ledger_.init_cash_; }

  /** Account creation date */
  Datetime initDatetime() const override { return ledger_.init_datetime_; }

  /** Date of the first buy trade; Null<Datetime>() is returned if no trade has
   * happened */
  Datetime firstDatetime() const override;

  /** Date of the last trade, regardless of the trade type; the account creation
   * date is returned if no trade has happened */
  Datetime lastDatetime() const override {
    return ledger_.trade_list_.empty() ? ledger_.init_datetime_
                                        : ledger_.trade_list_.back().datetime;
  }

  /**
   * Update the current positions and trades according to the weight
   * (adjustment) information
   * @note It must be called in chronological order
   * @param datetime the current moment
   */
  void updateWithWeight(const Datetime& datetime) override;

  /**
   * Return the current cash
   * @note Only the current information is returned, it is not adjusted
   * according to the weight information
   */
  price_t currentCash() const override { return ledger_.cash_; }

  /**
   * Get the cash of the given date
   * @note Without the date parameter the positions cannot be adjusted according
   * to the weight information
   */
  price_t cash(const Datetime& datetime,
               KQuery::KType ktype = KQuery::DAY) override;

  /**
   * Whether the given security is currently held
   * @note The date parameter is not used here, so execution in chronological
   * order is required
   * @param stock the given security
   * @return true yes | false no
   */
  bool have(const Stock& stock) const override {
    return ledger_.position_.count(stock.id()) ? true : false;
  }

  /**
   * Whether the given security is currently held in the short position
   * @note The date parameter is not used here, so execution in chronological
   * order is required
   * @param stock the given security
   * @return true yes | false no
   */
  bool haveShort(const Stock& stock) const override {
    return ledger_.short_position_.count(stock.id()) ? true : false;
  }

  /** Number of security types currently held */
  size_t getStockNumber() const override { return ledger_.position_.size(); }

  /** Number of security types currently held short */
  virtual size_t getShortStockNumber() const {
    return ledger_.short_position_.size();
  }

  /** Get the held quantity of a security at the given moment */
  double getHoldNumber(const Datetime& datetime, const Stock& stock) override;

  /** Get the short held quantity of a security at the given moment */
  double getShortHoldNumber(const Datetime& datetime,
                            const Stock& stock) override;

  /** Get the number of borrowed shares at the given moment */
  virtual double getDebtNumber(const Datetime& datetime, const Stock& stock);

  /** Get the amount of borrowed cash at the given moment */
  virtual price_t getDebtCash(const Datetime& datetime);

  /** Get all the trade records */
  TradeRecordList getTradeList() const override { return ledger_.trade_list_; }

  /**
   * Get the trade records within the given date range [start, end)
   * @param start start date
   * @param end end date
   * @return trade record list
   */
  virtual TradeRecordList getTradeList(const Datetime& start,
                                       const Datetime& end) const;

  /** Get all the current position records */
  PositionRecordList getPositionList() const override;

  /** Get all the historical position records, i.e. the closed position records
   */
  PositionRecordList getHistoryPositionList() const override {
    return ledger_.position_history_;
  }

  /** Get all the current short position records */
  virtual PositionRecordList getShortPositionList() const;

  /** Get all the historical short position records */
  virtual PositionRecordList getShortHistoryPositionList() const {
    return ledger_.short_position_history_;
  }

  /**
   * Get the position record of the given security
   * @param date the given date
   * @param stock the given security
   */
  PositionRecord getPosition(const Datetime& date, const Stock& stock) override;

  /** Get the current short position record of the given security;
   * Null<PositionRecord>() is returned if the security is not currently held
   * short */
  PositionRecord getShortPosition(const Stock&) const override;

  /** Get the list of currently borrowed shares */
  virtual BorrowRecordList getBorrowStockList() const;

  /**
   * Deposit funds
   * @param datetime deposit time
   * @param cash deposited amount
   * @return true | false
   */
  bool checkin(const Datetime& datetime, price_t cash) override;

  /**
   * Withdraw funds
   * @param datetime withdrawal time
   * @param cash withdrawn amount
   * @return true | false
   */
  bool checkout(const Datetime& datetime, price_t cash) override;

  /**
   * Deposit assets
   * @param datetime deposit date
   * @param stock the stock to deposit
   * @param price price per share of the deposited stock
   * @param number number of deposited shares
   * @return true | false
   */
  virtual bool checkinStock(const Datetime& datetime, const Stock& stock,
                            price_t price, double number);

  /**
   * Withdraw the current assets
   * @param datetime withdrawal date
   * @param stock the stock to withdraw
   * @param price withdrawal price per share
   * @param number withdrawn quantity
   * @return true | false
   * @note It should never be used
   */
  virtual bool checkoutStock(const Datetime& datetime, const Stock& stock,
                             price_t price, double number);

  /**
   * Buy operation
   * @param datetime buy time
   * @param stock the security to buy
   * @param realPrice actual buy price
   * @param number buy quantity
   * @param stoploss stop-loss price
   * @param goalPrice target price
   * @param planPrice planned buy price
   * @param from records which system part issued the buy instruction
   * @param remark buy remark
   * @return the corresponding trade record; business equals BUSINESS_INVALID if
   * the operation failed
   */
  virtual TradeRecord buy(const Datetime& datetime, const Stock& stock,
                          price_t realPrice, double number,
                          price_t stoploss = 0.0, price_t goalPrice = 0.0,
                          price_t planPrice = 0.0,
                          OrderOrigin from = OrderOrigin::UNSPECIFIED,
                          const string& remark = "");

  /**
   * Sell operation
   * @param datetime sell time
   * @param stock the security to sell
   * @param realPrice actual sell price
   * @param number sell quantity; MAX_DOUBLE means selling everything
   * @param stoploss new stop-loss price
   * @param goalPrice new target price
   * @param planPrice originally planned sell price
   * @param from records which system part issued the sell instruction
   * @param remark sell remark
   * @return the corresponding trade record; business equals BUSINESS_INVALID if
   * the operation failed
   */
  virtual TradeRecord sell(const Datetime& datetime, const Stock& stock,
                           price_t realPrice, double number = MAX_DOUBLE,
                           price_t stoploss = 0.0, price_t goalPrice = 0.0,
                           price_t planPrice = 0.0,
                           OrderOrigin from = OrderOrigin::UNSPECIFIED,
                           const string& remark = "");

  /**
   * Short sell
   * @param datetime short sell time
   * @param stock the security to short sell
   * @param realPrice actual short sell price
   * @param number sell quantity
   * @param stoploss stop-loss price
   * @param goalPrice target price
   * @param planPrice planned short sell price
   * @param from records which system part issued the buy instruction
   * @param remark remark
   * @return the corresponding trade record; business equals BUSINESS_INVALID if
   * the operation failed
   */
  virtual TradeRecord sellShort(const Datetime& datetime, const Stock& stock,
                                price_t realPrice, double number,
                                price_t stoploss = 0.0, price_t goalPrice = 0.0,
                                price_t planPrice = 0.0,
                                OrderOrigin from = OrderOrigin::UNSPECIFIED,
                                const string& remark = "");

  /**
   * Cover a short position
   * @param datetime buy time
   * @param stock the security to buy
   * @param realPrice actual buy price
   * @param number sell quantity; MAX_DOUBLE means selling everything
   * @param stoploss stop-loss price
   * @param goalPrice target price
   * @param planPrice planned buy price
   * @param from records which system part issued the sell instruction
   * @param remark remark
   * @return the corresponding trade record; business equals BUSINESS_INVALID if
   * the operation failed
   */
  virtual TradeRecord buyShort(const Datetime& datetime, const Stock& stock,
                               price_t realPrice, double number = MAX_DOUBLE,
                               price_t stoploss = 0.0, price_t goalPrice = 0.0,
                               price_t planPrice = 0.0,
                               OrderOrigin from = OrderOrigin::UNSPECIFIED,
                               const string& remark = "");

  /**
   * Borrow funds, i.e. funds borrowed from another source, e.g. margin
   * @param datetime borrow time
   * @param cash borrowed cash
   * @return true | false
   */
  virtual bool borrowCash(const Datetime& datetime, price_t cash);

  /**
   * Repay funds
   * @param datetime repayment date
   * @param cash repaid cash
   * @return true | false
   */
  virtual bool returnCash(const Datetime& datetime, price_t cash);

  /**
   * Borrow a security
   * @param datetime borrow time
   * @param stock the borrowed stock
   * @param price price per share when borrowing
   * @param number borrowed quantity
   * @return true | false
   */
  virtual bool borrowStock(const Datetime& datetime, const Stock& stock,
                           price_t price, double number);

  /**
   * Return a borrowed security
   * @param datetime return time
   * @param stock the returned stock
   * @param price price per share when returning
   * @param number returned quantity
   * @return true | false
   */
  virtual bool returnStock(const Datetime& datetime, const Stock& stock,
                           price_t price, double number);

  /**
   * Get the asset details of the account at the current moment
   * @param ktype the type of the date
   * @return asset details
   */
  virtual FundsRecord getFunds(KQuery::KType ktype = KQuery::DAY) const;

  /**
   * Get the market value details of the assets at the given moment
   * @param datetime it must be later than the initial date of the account, or
   * Null<Datetime>()
   * @param ktype the type of the date
   * @return asset details
   * @note When datetime equals Null<Datetime>(), it is the same as
   * getFunds(KType)
   */
  FundsRecord getFunds(const Datetime& datetime,
                       KQuery::KType ktype = KQuery::DAY) override;

  [[nodiscard]] PriceList getProfitCurve(
      const DatetimeList& dates, KQuery::KType ktype = KQuery::DAY) override;

  /**
   * Add a trade record directly
   * @note If an account initialization record is added, all the existing trade
   * and position records are cleared
   * @param tr the trade record to add
   * @return bool true on success | false on failure
   */
  bool addTradeRecord(const TradeRecord& tr) override;

  /**
   * Add a position record directly
   * @note Special purpose: building the initial position, it may cause
   * confusion
   * @param pr position record
   * @return true on success
   * @return false on failure
   */
  virtual bool addPosition(const PositionRecord& pr);

  /** String output */
  virtual string str() const;

  /**
   * Export the trade records, open position records, closed position records
   * and the net value curve of the assets in csv format
   * @param path the directory of the output file
   */
  virtual void tocsv(const string& path);

 private:
  // Save the trade actions in the form of a script, so that they can be
  // corrected and calibrated
  void _saveAction(const TradeRecord&);

  bool _add_init_tr(const TradeRecord&);
  bool _add_buy_tr(const TradeRecord&);
  bool _add_sell_tr(const TradeRecord&);
  bool _add_checkin_tr(const TradeRecord&);
  bool _add_checkout_tr(const TradeRecord&);
  bool _add_checkin_stock_tr(const TradeRecord&);
  bool _add_checkout_stock_tr(const TradeRecord&);
  bool _add_borrow_cash_tr(const TradeRecord&);
  bool _add_return_cash_tr(const TradeRecord&);
  bool _add_borrow_stock_tr(const TradeRecord&);
  bool _add_return_stock_tr(const TradeRecord&);
  bool _add_sell_short_tr(const TradeRecord&);
  bool _add_buy_short_tr(const TradeRecord&);

 protected:
  void copyRuntimeStateTo(ExecutionRuntime& target) const;

 private:
  using borrow_stock_map_type = Ledger::BorrowStockMap;
  using position_map_type = Ledger::PositionMap;

  string name_;
  TradeCostPtr costfunc_;
  Datetime broker_last_datetime_;
  list<OrderBrokerPtr> broker_list_;
  Ledger ledger_;
};

inline void ExecutionRuntime::baseCheckParam(const string& name) const {
  if (name == "precision") {
    HAYAKU_ASSERT(getParam<int>("precision") > 0);
  }
}

inline void ExecutionRuntime::paramChanged() {}

} /* namespace hayaku */
