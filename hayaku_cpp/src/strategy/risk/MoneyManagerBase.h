#pragma once

/*
 * MoneyManagerBase.h
 *
 *  Created on: 2013-3-3
 *      Author: fasiondog
 */

#include "common/Parameter.h"
#include "execution/ExecutionAccountPort.h"
#include "execution/OrderOrigin.h"

namespace hayaku {

/**
 * Base class of the money management
 * @ingroup MoneyManager
 */
class MoneyManagerBase : public enable_shared_from_this<MoneyManagerBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  MoneyManagerBase();
  explicit MoneyManagerBase(const string& name);
  MoneyManagerBase(const MoneyManagerBase&) = default;
  virtual ~MoneyManagerBase();

  /** Get the name */
  const string& name() const { return name_; }

  /** Set the name */
  void name(const string& name) { name_ = name; }

  /** Reset */
  void reset();

  /**
   * Set the trade account
   * @param tm the given trade account
   */
  void setAccount(const internal::ExecutionAccountPortPtr& account) {
    account_ = account;
  }

  /**
   * Get the trade account
   * @return
   */
  internal::ExecutionAccountPortPtr getAccount() const { return account_; }

  /** Set the query condition */
  void setQuery(const KQuery& query) { query_ = query; }

  /** Get the K-line type of the trade */
  const KQuery& getQuery() const { return query_; }

  typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
  /** Clone operation */
  MoneyManagerPtr clone();

  /** Receive the actual trade change situation */
  void buyNotify(const TradeRecord& tr);

  /** Interface for the subclass to receive the actual trade change situation;
   * it generally needs to be overloaded only when there are multiple position
   * increases and decreases */
  virtual void _buyNotify(const TradeRecord&) {}

  /** Receive the actual trade change situation */
  void sellNotify(const TradeRecord& tr);

  /** Interface for the subclass to receive the actual trade change situation;
   * it generally needs to be overloaded only when there are multiple position
   * increases and decreases */
  virtual void _sellNotify(const TradeRecord&) {}

  /**
   * Get the quantity of the given trading object that can be sold
   * @param datetime trade date
   * @param stock the trading object
   * @param price trade price
   * @param risk the risk taken by the new trade; if it is 0, it means the whole
   * loss, i.e. the market value drops to 0 yuan
   * @param from signal source
   * @note The default implementation returns MAX_DOUBLE, i.e. selling
   * everything; this interface is needed only for the multiple position
   * reductions
   */
  double getSellNumber(const Datetime& datetime, const Stock& stock,
                       price_t price, price_t risk, OrderOrigin origin);

  /**
   * Get the quantity of the given trading object that can be short sold
   * @param datetime trade date
   * @param stock the trading object
   * @param price trade price
   * @param from signal source
   * @param risk the trade risk taken; Null<price_t> means there is no loss
   * upper limit
   */
  double getSellShortNumber(const Datetime& datetime, const Stock& stock,
                            price_t price, price_t risk, OrderOrigin origin);

  /**
   * Get the buy quantity to cover the short position of the given trading
   * object
   * @param datetime trade date
   * @param stock the trading object
   * @param price trade price
   * @param from signal source
   * @param risk the trade risk taken; Null<price_t> means there is no loss
   * upper limit
   */
  double getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                           price_t price, price_t risk, OrderOrigin origin);

  /**
   * Get the quantity of the given trading object that can be bought
   * @param datetime trade date
   * @param stock the trading object
   * @param price trade price
   * @param from signal source
   * @param risk the risk taken by the trade; if it is 0, it means the whole
   * loss, i.e. the market value drops to 0 yuan
   */
  double getBuyNumber(const Datetime& datetime, const Stock& stock,
                      price_t price, price_t risk, OrderOrigin origin);

  /** Current number of the buy trades; it counts the consecutive buys and is
   * reset to 0 once a sell is received */
  size_t currentBuyCount(const Stock&) const;

  /** Current number of the sell trades; it counts the consecutive sells and is
   * reset to 0 once a buy is received */
  size_t currentSellCount(const Stock&) const;

  virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock,
                               price_t price, price_t risk,
                               OrderOrigin origin) = 0;

  virtual double _getSellNumber(const Datetime& datetime, const Stock& stock,
                                price_t price, price_t risk,
                                OrderOrigin origin);

  virtual double _getSellShortNumber(const Datetime& datetime,
                                     const Stock& stock, price_t price,
                                     price_t risk, OrderOrigin origin);

  virtual double _getBuyShortNumber(const Datetime& datetime,
                                    const Stock& stock, price_t price,
                                    price_t risk, OrderOrigin origin);

  /** Subclass reset interface */
  virtual void _reset() {}

  /** Interface for the subclass to clone its private variables */
  virtual MoneyManagerPtr _clone() = 0;

  bool isPythonObject() const noexcept { return is_python_object_; }

 protected:
  string name_;
  KQuery query_;
  internal::ExecutionAccountPortPtr account_;
  unordered_map<Stock, std::pair<size_t, size_t>> buy_sell_counts_;
  bool is_python_object_{false};

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    // m_query and m_account are set temporarily when the strategy runs, they do
    // not need to be serialized ar & boost::serialization::make_nvp("m_query",
    // query_); ar & boost::serialization::make_nvp("m_account", account_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MoneyManagerBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public MoneyManagerBase {
 *     MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MoneyManager
 */
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION          \
 private:                                                      \
  friend class boost::serialization::access;                   \
  template <class Archive>                                     \
  void serialize(Archive& ar, const unsigned int version) {    \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MoneyManagerBase); \
  }
#else
#define MONEY_MANAGER_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

/**
 * Client programs should all use this pointer type
 * @ingroup MoneyManager
 */
typedef shared_ptr<MoneyManagerBase> MoneyManagerPtr;
typedef shared_ptr<MoneyManagerBase> MMPtr;

#define MONEY_MANAGER_IMP(classname)                                         \
 public:                                                                     \
  virtual MoneyManagerPtr _clone() override {                                \
    return std::make_shared<classname>();                                    \
  }                                                                          \
  virtual double _getBuyNumber(const Datetime& datetime, const Stock& stock, \
                               price_t price, price_t risk,                  \
                               OrderOrigin origin) override;

std::ostream& operator<<(std::ostream&, const MoneyManagerBase&);
std::ostream& operator<<(std::ostream&, const MoneyManagerPtr&);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::MoneyManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::MoneyManagerPtr> : ostream_formatter {};
#endif
