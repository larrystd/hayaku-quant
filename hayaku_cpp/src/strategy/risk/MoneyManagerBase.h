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
class HAYAKU_API MoneyManagerBase
    : public enable_shared_from_this<MoneyManagerBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  MoneyManagerBase();
  explicit MoneyManagerBase(const string& name);
  MoneyManagerBase(const MoneyManagerBase&) = default;
  virtual ~MoneyManagerBase();

  /** Get the name */
  const string& name() const { return m_name; }

  /** Set the name */
  void name(const string& name) { m_name = name; }

  /** Reset */
  void reset();

  /**
   * Set the trade account
   * @param tm the given trade account
   */
  void setAccount(const internal::ExecutionAccountPortPtr& account) {
    m_account = account;
  }

  /**
   * Get the trade account
   * @return
   */
  internal::ExecutionAccountPortPtr getAccount() const { return m_account; }

  /** Set the query condition */
  void setQuery(const KQuery& query) { m_query = query; }

  /** Get the K-line type of the trade */
  const KQuery& getQuery() const { return m_query; }

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

  bool isPythonObject() const noexcept { return m_is_python_object; }

 protected:
  string m_name;
  KQuery m_query;
  internal::ExecutionAccountPortPtr m_account;
  unordered_map<Stock, std::pair<size_t, size_t>> m_buy_sell_counts;
  bool m_is_python_object{false};

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& BOOST_SERIALIZATION_NVP(m_name);
    ar& BOOST_SERIALIZATION_NVP(m_params);
    ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
    // m_query and m_account are set temporarily when the strategy runs, they do
    // not need to be serialized ar & BOOST_SERIALIZATION_NVP(m_query); ar &
    // BOOST_SERIALIZATION_NVP(m_account);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& BOOST_SERIALIZATION_NVP(m_name);
    ar& BOOST_SERIALIZATION_NVP(m_params);
    ar& BOOST_SERIALIZATION_NVP(m_is_python_object);
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

HAYAKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerBase&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const MoneyManagerPtr&);

} /* namespace hayaku */

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::MoneyManagerBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::MoneyManagerPtr> : ostream_formatter {};
#endif
