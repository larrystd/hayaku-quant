#include "Bindings.h"

/* Domain binding registrations. */

// Registration group: _Condition
/*
 * _Condition.cpp
 *
 *  Created on: 2013-3-10
 *      Author: fasiondog
 */

#include <strategy/decision/Conditions.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyConditionBase : public ConditionBase {
  PY_CLONE(PyConditionBase, ConditionBase)

 public:
  PyConditionBase() : ConditionBase() { is_python_object_ = true; }

  PyConditionBase(const string& name) : ConditionBase(name) {
    is_python_object_ = true;
  }

  PyConditionBase(const ConditionBase& base) : ConditionBase(base) {
    is_python_object_ = true;
  }

  void _calculate() override {
    PYBIND11_OVERLOAD_PURE(void, ConditionBase, _calculate, );
  }

  void _reset() override { PYBIND11_OVERLOAD(void, ConditionBase, _reset, ); }
};

void export_Condition(py::module& m) {
  py::class_<ConditionBase, ConditionPtr, PyConditionBase>(
      m, "ConditionBase", py::dynamic_attr(),
      R"(The system valid condition base class; the custom system valid condition interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const ConditionBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<ConditionBase>)
      .def("__repr__", to_py_str<ConditionBase>)

      .def_property("name",
                    py::overload_cast<>(&ConditionBase::name, py::const_),
                    py::overload_cast<const string&>(&ConditionBase::name),
                    py::return_value_policy::copy, "Name")

      .def_property("to", &ConditionBase::getTO, &ConditionBase::setTO,
                    "Set or get the trading object")
      .def_property("sg", &ConditionBase::getSG, &ConditionBase::setSG,
                    "Set or get the trading signal generator")

      .def("get_param", &ConditionBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def(
          "set_param",
          static_cast<void (ConditionBase::*)(
              const std::string&, const boost::any&)>(&ConditionBase::setParam),
          R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ConditionBase::haveParam,
           "Whether the specified parameter exists")

      .def("is_valid", &ConditionBase::isValid, R"(is_valid(self, datetime)

    Whether the system is valid at the specified time

    :param Datetime datetime: the specified time
    :return: True valid | False invalid)")

      .def("reset", &ConditionBase::reset, "The reset operation")
      .def("clone", &ConditionBase::clone, "The clone operation")

      .def("get_datetime_list", &ConditionBase::getDatetimeList,
           R"(get_datetime_list(self)

    Get the dates when the system is valid. Note that only the list of the dates when the system is valid is returned, which is not the same length as the trading object)")

      .def("get_values", &ConditionBase::getValues, R"(get_values(self)

    Get the actual values in the form of an indicator, with the same length as the trading object; 0 means invalid, and 1 means the system is valid)")

      .def("_add_valid", &ConditionBase::_addValid, py::arg("datetime"),
           py::arg("value") = 1.0,
           R"(_add_valid(self, datetime)

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time)")

      .def("_calculate", &ConditionBase::_calculate,
           "[Overload interface] The subclass calculation interface")
      .def("_reset", &ConditionBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the "
           "internal private "
           "variables")

      .def("__len__", &ConditionBase::size)

      .def("__getitem__",
           [](const ConditionPtr& self, int64_t i) {
             size_t total = self->size();
             int64_t pos = i < 0 ? total + i : i;
             return self->at(pos);
           })

      .def("__and__", [](const ConditionPtr& self,
                         const ConditionPtr& other) { return self & other; })

      .def("__or__", [](const ConditionPtr& self,
                        const ConditionPtr& other) { return self | other; })

      .def("__add__", [](const ConditionPtr& self,
                         const ConditionPtr& other) { return self + other; })

      .def("__sub__", [](const ConditionPtr& self,
                         const ConditionPtr& other) { return self - other; })

      .def("__mul__", [](const ConditionPtr& self,
                         const ConditionPtr& other) { return self * other; })

      .def("__truediv__",
           [](const ConditionPtr& self, const ConditionPtr& other) {
             return self / other;
           })

          DEF_PICKLE(ConditionPtr);

  m.def("CN_OPLine", CN_OPLine, R"(CN_OPLine(ind)

    Fixedly use the minimum trading quantity of the stock to trade; calculate the ind value of the equity curve; when the equity curve is higher than ind, the system is valid, otherwise invalid.

    :param Indicator ind: the Indicator instance
    :return: the system valid condition instance
    :rtype: ConditionBase)");

  m.def("CN_Bool", CN_Bool, R"(CN_Bool(ind)

    The boolean signal generator system valid condition; if the corresponding position in the indicator is >0, it means the system is valid, otherwise invalid

    :param Indicator ind: a bool-type indicator, with the KData as the input
    :return: the system valid condition instance
    :rtype: ConditionBase)");
}

// Registration group: _Environment
/*
 * _Environment.cpp
 *
 *  Created on: 2013-3-2
 *      Author: fasiondog
 */

/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <strategy/decision/EnvironmentBase.h>
#include <strategy/decision/Environments.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyEnvironmentBase : public EnvironmentBase {
  PY_CLONE(PyEnvironmentBase, EnvironmentBase)

 public:
  PyEnvironmentBase() : EnvironmentBase() { is_python_object_ = true; }

  PyEnvironmentBase(const string& name) : EnvironmentBase(name) {
    is_python_object_ = true;
  }

  PyEnvironmentBase(const EnvironmentBase& base) : EnvironmentBase(base) {
    is_python_object_ = true;
  }

  void _calculate() override {
    PYBIND11_OVERLOAD_PURE(void, EnvironmentBase, _calculate, );
  }

  void _reset() override { PYBIND11_OVERLOAD(void, EnvironmentBase, _reset, ); }
};

void export_Environment(py::module& m) {
  py::class_<EnvironmentBase, EnvironmentPtr, PyEnvironmentBase>(
      m, "EnvironmentBase", py::dynamic_attr(),
      R"(The market environment strategy base class

The custom market environment strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const EnvironmentBase&>())
      .def(py::init<const string&>())

      .def("__str__", to_py_str<EnvironmentBase>)
      .def("__repr__", to_py_str<EnvironmentBase>)

      .def_property("name",
                    py::overload_cast<>(&EnvironmentBase::name, py::const_),
                    py::overload_cast<const string&>(&EnvironmentBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &EnvironmentBase::getQuery,
                    &EnvironmentBase::setQuery, py::return_value_policy::copy,
                    "Set or get the query condition")

      .def("get_param", &EnvironmentBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (EnvironmentBase::*)(const std::string&,
                                                 const boost::any&)>(
               &EnvironmentBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &EnvironmentBase::haveParam,
           "Whether the specified parameter exists")

      .def("is_valid", &EnvironmentBase::isValid, R"(is_valid(self, datetime)

    Whether the system is valid at the specified time

    :param Datetime datetime: the specified time
    :return: True valid | False invalid)")

      .def("_add_valid", &EnvironmentBase::_addValid, py::arg("datetime"),
           py::arg("value") = 1.0,
           R"(_add_valid(self, datetime)

    Add a valid time, called in _calculate

    :param Datetime datetime: the valid time
    :param float value: defaulting to 1.0; greater than 0 means valid, and less than or equal to 0 means invalid)")

      .def("reset", &EnvironmentBase::reset, "The reset operation")
      .def("clone", &EnvironmentBase::clone, "The clone operation")
      .def("_reset", &EnvironmentBase::_reset,
           "[Overload interface] The subclass reset interface, used to reset "
           "the internal private "
           "variables")
      .def("_calculate", &EnvironmentBase::_calculate,
           "[Overload interface] The subclass calculation interface")

      .def("__and__", [](const EnvironmentPtr& self,
                         const EnvironmentPtr& other) { return self & other; })

      .def("__or__", [](const EnvironmentPtr& self,
                        const EnvironmentPtr& other) { return self | other; })

      .def("__add__", [](const EnvironmentPtr& self,
                         const EnvironmentPtr& other) { return self + other; })

      .def("__sub__", [](const EnvironmentPtr& self,
                         const EnvironmentPtr& other) { return self - other; })

      .def("__mul__", [](const EnvironmentPtr& self,
                         const EnvironmentPtr& other) { return self * other; })

      .def("__truediv__",
           [](const EnvironmentPtr& self, const EnvironmentPtr& other) {
             return self / other;
           })

          DEF_PICKLE(EnvironmentPtr);

  m.def("EV_TwoLine", EV_TwoLine, py::arg("fast"), py::arg("slow"),
        py::arg("market") = "SH",
        R"(EV_TwoLine(fast, slow[, market = 'SH'])

    The fast/slow line strategy; when the fast line of the market index is greater than the slow line, the market is valid, otherwise invalid.

    :param Indicator fast: the fast line indicator
    :param Indicator slow: the slow line indicator
    :param string market: the market name)");

  m.def("EV_Bool", EV_Bool, py::arg("ind"), py::arg("market") = "SH",
        R"(EV_Bool(ind, market='SH')

    The boolean signal generator market environment

    :param Indicator ind: a bool-type indicator; if the corresponding position in the indicator is >0, it means the market is valid, otherwise invalid
    :param str market: the specified market, used to get the corresponding trading calendar)");
}

// Registration group: _MoneyManager
/*
 * _MoneyManager.cpp
 *
 *  Created on: 2013-3-13
 *      Author: fasiondog
 */

#include <strategy/risk/MoneyManagers.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyMoneyManagerBase : public MoneyManagerBase {
  PY_CLONE(PyMoneyManagerBase, MoneyManagerBase)

 public:
  PyMoneyManagerBase() : MoneyManagerBase() { is_python_object_ = true; }

  PyMoneyManagerBase(const string& name) : MoneyManagerBase(name) {
    is_python_object_ = true;
  }

  PyMoneyManagerBase(const MoneyManagerBase& base) : MoneyManagerBase(base) {
    is_python_object_ = true;
  }

  void _reset() override {
    PYBIND11_OVERLOAD(void, MoneyManagerBase, _reset, );
  }

  void _buyNotify(const TradeRecord& tr) override {
    PYBIND11_OVERLOAD_NAME(void, MoneyManagerBase, "_buy_notify", _buyNotify,
                           tr);
  }

  void _sellNotify(const TradeRecord& tr) override {
    PYBIND11_OVERLOAD_NAME(void, MoneyManagerBase, "_sell_notify", _sellNotify,
                           tr);
  }

  double _getBuyNumber(const Datetime& datetime, const Stock& stock,
                       price_t price, price_t risk,
                       OrderOrigin origin) override {
    PYBIND11_OVERLOAD_PURE_NAME(double, MoneyManagerBase, "_get_buy_num",
                                _getBuyNumber, datetime, stock, price, risk,
                                origin);
  }

  double _getSellNumber(const Datetime& datetime, const Stock& stock,
                        price_t price, price_t risk,
                        OrderOrigin origin) override {
    PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_sell_num",
                           _getSellNumber, datetime, stock, price, risk,
                           origin);
  }

  double _getSellShortNumber(const Datetime& datetime, const Stock& stock,
                             price_t price, price_t risk,
                             OrderOrigin origin) override {
    PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_sell_short_num",
                           _getSellShortNumber, datetime, stock, price, risk,
                           origin);
  }

  double _getBuyShortNumber(const Datetime& datetime, const Stock& stock,
                            price_t price, price_t risk,
                            OrderOrigin origin) override {
    PYBIND11_OVERLOAD_NAME(double, MoneyManagerBase, "_get_buy_short_num",
                           _getBuyShortNumber, datetime, stock, price, risk,
                           origin);
  }
};

void export_MoneyManager(py::module& m) {
  py::class_<MoneyManagerBase, MMPtr, PyMoneyManagerBase>(
      m, "MoneyManagerBase", py::dynamic_attr(),
      R"(The money management strategy base class

Common parameters:

    - auto-checkin=False (bool): when the account cash is insufficient to buy the quantity indicated by the money management strategy, automatically deposit (checkin) enough cash into the account.
    - max-stock=20000 (int): the maximum number of the kinds of the held securities (i.e. how many stocks are held, not the position size of each stock)
    - disable_ev_force_clean_position=False (bool): disable forcibly clearing the positions when the market environment becomes invalid
    - disable_cn_force_clean_position=False (bool): disable forcibly clearing the positions when the system valid condition becomes invalid

The custom money management strategy interfaces:

    - _buyNotify : [Optional] Receive the actual buy notification, reserved for the multiple position increase/decrease processing
    - _sellNotify : [Optional] Receive the actual sell notification, reserved for the multiple position increase/decrease processing
    - _getBuyNumber : [Required] Get the quantity that can be bought of the specified trading object
    - _getSellNumber : [Optional] Get the quantity that can be sold of the specified trading object; if it is not overloaded, default to selling all the held quantity
    - _reset : [Optional] Reset the private attributes
    - _clone : [Required] The clone interface)")
      .def(py::init<>())
      .def(py::init<const MoneyManagerBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<MoneyManagerBase>)
      .def("__repr__", to_py_str<MoneyManagerBase>)

      .def_property("name",
                    py::overload_cast<>(&MoneyManagerBase::name, py::const_),
                    py::overload_cast<const string&>(&MoneyManagerBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &MoneyManagerBase::getQuery,
                    &MoneyManagerBase::setQuery, py::return_value_policy::copy,
                    "Set or get the query condition")

      .def("current_buy_count", &MoneyManagerBase::currentBuyCount,
           "The current consecutive buy count")
      .def("current_sell_count", &MoneyManagerBase::currentSellCount,
           "The current consecutive sell count")

      .def("get_param", &MoneyManagerBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (MoneyManagerBase::*)(const std::string&,
                                                  const boost::any&)>(
               &MoneyManagerBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &MoneyManagerBase::haveParam,
           "Whether the specified parameter exists")

      .def("reset", &MoneyManagerBase::reset, "The reset operation")
      .def("clone", &MoneyManagerBase::clone, "The clone operation")

      .def("_buy_notify", &MoneyManagerBase::_buyNotify,
           R"(_buy_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual buy operation, notify the trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual buy trade record when the actual buying occurs)")

      .def("_sell_notify", &MoneyManagerBase::_sellNotify,
           R"(_sell_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual sell operation, notify the actual trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual sell trade record when the actual selling occurs)")

      .def("get_buy_num", &MoneyManagerBase::getBuyNumber,
           R"(get_buy_num(self, datetime, stock, price, risk, origin)

    Get the quantity that can be bought of the specified trading object

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param OrderOrigin origin: the strategy component origin
    :return: the quantity that can be bought
    :rtype: float)")

      .def("get_sell_num", &MoneyManagerBase::getSellNumber,
           R"(get_sell_num(self, datetime, stock, price, risk, origin)

    Get the quantity that can be sold of the specified trading object

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the new risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param OrderOrigin origin: the strategy component origin
    :return: the quantity that can be sold
    :rtype: float)")

      .def("_get_buy_num", &MoneyManagerBase::_getBuyNumber,
           R"(_get_buy_num(self, datetime, stock, price, risk, origin)

    [Overload interface] Get the quantity that can be bought of the specified trading object

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param OrderOrigin origin: the strategy component origin
    :return: the quantity that can be bought
    :rtype: float)")

      .def("_get_sell_num", &MoneyManagerBase::_getSellNumber,
           R"(_get_sell_num(self, datetime, stock, price, risk, origin)

    [Overload interface] Get the quantity that can be sold of the specified trading object. If it is not overloaded, default to selling all the held quantity.

    :param Datetime datetime: the trading time
    :param Stock stock: the trading object
    :param float price: the trading price
    :param float risk: the new risk taken in the trade; if it is 0, it means a total loss, i.e. the market value falls to 0 yuan
    :param OrderOrigin origin: the strategy component origin
    :return: the quantity that can be sold
    :rtype: float)")

      .def("get_sell_short_num", &MoneyManagerBase::getSellShortNumber)
      .def("get_buy_short_num", &MoneyManagerBase::getBuyShortNumber)
      .def("_get_sell_short_num", &MoneyManagerBase::_getSellShortNumber)
      .def("_get_buy_short_num", &MoneyManagerBase::_getBuyShortNumber)

      .def(
          "_reset", &MoneyManagerBase::_reset,
          R"([Overload interface] The subclass reset interface, resetting the internal private variables)")

          DEF_PICKLE(MMPtr);

  m.def("MM_Nothing", MM_Nothing, R"(MM_Nothing()

    A special money management strategy, equivalent to no money management; buy as much as the money available.)");

  m.def("MM_FixedRisk", MM_FixedRisk, py::arg("risk") = 1000.00,
        R"(MM_FixedRisk([risk = 1000.00])

    The fixed risk money management strategy limits each trade to a pre-determined or fixed capital risk, e.g. a fixed risk of 1000 yuan per trade. The formula: the trading quantity = the fixed risk / the trading risk.

    :param float risk: the fixed risk
    :return: the money management strategy instance)");

  m.def("MM_FixedCapital", MM_FixedCapital, py::arg("capital") = 10000.00,
        R"(MM_FixedCapital([capital = 10000.0])

    The fixed capital money management strategy. The buy quantity = the current cash / capital

    :param float capital: the fixed capital unit
    :return: the money management strategy instance)");

  m.def("MM_FixedCapitalFunds", MM_FixedCapitalFunds,
        py::arg("capital") = 10000.00,
        R"(MM_FixedCapitalFunds([capital = 10000.0])

    The fixed total capital money management strategy. The buy quantity = the current total assets / capital

    :param float capital: the fixed capital unit
    :return: the money management strategy instance)");

  m.def("MM_FixedCount", MM_FixedCount, py::arg("n") = 100,
        R"(MM_FixedCount([n = 100])

    The fixed trading quantity money management strategy. Buy a fixed quantity each time.

    :param float n: the quantity to buy each time (it should be an integer multiple of the minimum trading quantity of the trading object; the program does not check this here)
    :return: the money management strategy instance)");

  m.def("MM_FixedPercent", MM_FixedPercent, py::arg("p") = 0.03,
        R"(MM_FixedPercent([p = 0.03])

    The fixed percentage risk model. The formula: P (the position size) = the account balance * the percentage / R (the trading risk per share). [BOOK3]_, [BOOK4]_ .

    :param float p: the percentage
    :return: the money management strategy instance)");

  m.def("MM_FixedUnits", MM_FixedUnits, py::arg("n") = 33,
        R"(MM_FixedUnits([n = 33])

    The fixed unit money management strategy. The formula: the buy quantity = the current cash / n / the current risk

    :param int n: n capital units
    :return: the money management strategy instance)");

  m.def("MM_WilliamsFixedRisk", MM_WilliamsFixedRisk, py::arg("p") = 0.1,
        py::arg("max_loss") = 1000.0,
        R"( MM_WilliamsFixedRisk([p=0.1, max_loss=1000.0])

    The Williams fixed risk money management strategy. The buy quantity = (the account balance × the risk percentage p) ÷ the maximum loss (max_loss)

    :param float p: the risk percentage
    :param float max_loss: the maximum loss
    :return: the money management strategy instance)");

  m.def("MM_FixedCountTps", MM_FixedCountTps, py::arg("buy_counts"),
        py::arg("sell_counts"),
        R"(MM_FixedCountTps([buy_counts, sell_counts])

    The consecutive buy/sell fixed quantity money management strategy.

    :param list buy_counts: the buy quantity list
    :param list sell_counts: the sell quantity list
    :return: the money management strategy instance)");
}

// Registration group: _MultiFactor
/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-13
 *      Author: fasiondog
 */

#include <strategy/selection/MultiFactors.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

#ifdef _MSC_VER
#define HIDDEN
#else
#define HIDDEN __attribute__((visibility("hidden")))
#endif

class HIDDEN PyMultiFactor : public MultiFactorBase {
  PY_CLONE(PyMultiFactor, MultiFactorBase)

 public:
  PyMultiFactor() : MultiFactorBase() { is_python_object_ = true; }

  PyMultiFactor(const string& name) : MultiFactorBase(name) {
    is_python_object_ = true;
  }

  PyMultiFactor(const MultiFactorBase& base) : MultiFactorBase(base) {
    is_python_object_ = true;
  }

  virtual ~PyMultiFactor() override {}

  IndicatorList _calculate(const vector<IndicatorList>& all_stk_inds) override {
    // PYBIND11_OVERLOAD_PURE_NAME(IndicatorList, MultiFactorBase, "_calculate",
    // _calculate,
    //                             all_stk_inds);
    auto self = py::cast(this);
    auto func = self.attr("_calculate")();
    auto py_all_stk_inds = vector_to_python_list<IndicatorList>(all_stk_inds);
    auto py_ret = func(py_all_stk_inds);
    return py_ret.cast<IndicatorList>();
  }

 public:
  void set_norm(py::object norm) {
    py::gil_scoped_acquire gil;
    auto tmp = norm;
    setNormalize(norm.cast<NormPtr>());
    tmp.release();
  }

  void add_special_norm(const string& name, py::object norm,
                        const string& category,
                        const IndicatorList& style_inds) {
    py::gil_scoped_acquire gil;
    HAYAKU_INFO_IF_RETURN(!norm || norm.is_none(), void(), "norm is None");
    auto tmp = norm;
    addSpecialNormalize(name, norm.cast<NormPtr>(), category, style_inds);
    tmp.release();
  }
};

void export_MultiFactor(py::module& m) {
  py::class_<ScoreRecord>(m, "ScoreRecord", "")
      .def(py::init<>())
      .def(py::init<const Stock&, ScoreRecord::value_t>())
      .def("__str__", to_py_str<ScoreRecord>)
      .def("__repr__", to_py_str<ScoreRecord>)
      .def_readwrite("stock", &ScoreRecord::stock, "The security")
      .def_readwrite("value", &ScoreRecord::value, "The score");

  m.def("scorerecords_to_np", [](const ScoreRecordList& scs) {
    size_t total = scs.size();
    HAYAKU_IF_RETURN(total == 0, py::array());

    struct alignas(8) RawData {
      int32_t code[10];
      int32_t name[20];
      double value;
    };

    // Allocate the memory with malloc
    RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
    std::string ucode, uname;
    for (size_t i = 0, len = scs.size(); i < len; i++) {
      const ScoreRecord& sc = scs[i];
      utf8_to_utf32(sc.stock.market_code(), data[i].code, 10);
      utf8_to_utf32(sc.stock.name(), data[i].name, 20);
      data[i].value = sc.value;
    }

    // Define the NumPy structured data type
    py::dtype dtype = py::dtype(
        vector_to_python_list<string>(
            {htr("market_code"), htr("name"), htr("score")}),
        vector_to_python_list<string>({"U10", "U20", "d"}),
        vector_to_python_list<int64_t>({0, 40, 120}), sizeof(RawData));

    // Manage the memory with the capsule
    return py::array(dtype, total, static_cast<RawData*>(data),
                     py::capsule(data, [](void* p) { std::free(p); }));
  });

  m.def("scorerecords_to_df", [](const ScoreRecordList& scs) {
    size_t total = scs.size();
    if (total == 0) {
      return py::module_::import("pandas").attr("DataFrame")();
    }

    // Create the python string object array
    py::list code_list(total);
    py::list name_list(total);
    py::array_t<double> value_arr(total);

    // Get the buffer of the value array
    auto value_buf = value_arr.request();
    double* value_ptr = static_cast<double*>(value_buf.ptr);

    // Fill the data
    for (size_t i = 0; i < total; i++) {
      const ScoreRecord& sc = scs[i];
      code_list[i] = py::str(sc.stock.market_code());
      name_list[i] = py::str(sc.stock.name());
      value_ptr[i] = sc.value;
    }

    // Build the DataFrame
    auto pandas = py::module_::import("pandas");
    py::dict columns;
    columns[htr("market_code").c_str()] =
        pandas.attr("Series")(code_list, py::arg("dtype") = "string");
    columns[htr("name").c_str()] =
        pandas.attr("Series")(name_list, py::arg("dtype") = "string");
    columns["score"] = value_arr;

    return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
  });

  py::class_<MultiFactorBase, MultiFactorPtr, PyMultiFactor>(
      m, "MultiFactorBase", py::dynamic_attr(),
      R"(The multi-factor model base class

The custom multi-factor model override hooks:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reset the internal member variables)")
      .def(py::init<>())
      .def(py::init<const MultiFactorBase&>())

      .def("__str__", to_py_str<MultiFactorBase>)
      .def("__repr__", to_py_str<MultiFactorBase>)

      .def_property("name",
                    py::overload_cast<>(&MultiFactorBase::name, py::const_),
                    py::overload_cast<const string&>(&MultiFactorBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("query", &MultiFactorBase::getQuery,
                    &MultiFactorBase::setQuery, py::return_value_policy::copy,
                    R"(The query condition)")

      .def("get_param", &MultiFactorBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (MultiFactorBase::*)(const std::string&,
                                                 const boost::any&)>(
               &MultiFactorBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &MultiFactorBase::haveParam,
           "Whether the specified parameter exists")

      .def("get_ref_stock", &MultiFactorBase::getRefStock,
           py::return_value_policy::copy, "Get the reference security")
      .def("set_ref_stock", &MultiFactorBase::setRefStock,
           R"(set_ref_stock(self, stk)

    Set the reference security

    :param Stock stk: the reference security)")

      .def("get_datetime_list", &MultiFactorBase::getDatetimeList,
           py::return_value_policy::copy,
           "Get the reference date list (obtained from the reference security "
           "through the query "
           "condition)")

      .def("get_stock_list", &MultiFactorBase::getStockList,
           py::return_value_policy::copy,
           "Get the security list specified at the creation")
      .def("set_stock_list", &MultiFactorBase::setStockList,
           R"(set_stock_list(self, stks)

    Set the security list specified for the calculation range

    :param list stks: the new security list to calculate)")

      .def("get_stock_list_num", &MultiFactorBase::getStockListNumber,
           "Get the number of the securities in the security list specified at "
           "the creation")

      .def("get_ref_factorset", &MultiFactorBase::getRefFactorSet,
           py::return_value_policy::copy,
           "Get the original factor set input at the creation")

      .def(
          "set_ref_factorset",
          [](MultiFactorBase& self, const py::sequence& inds,
             const KQuery::KType& ktype) {
            IndicatorList ind_list = python_list_to_vector<Indicator>(inds);
            FactorSet factorset = FactorSet(ind_list, ktype);
            self.setRefFactorSet(factorset);
          },
          py::arg("inds"), py::arg("ktype") = KQuery::DAY)
      .def(
          "set_ref_factorset",
          [](MultiFactorBase& self, const py::dict& inds,
             const KQuery::KType& ktype) {
            std::unordered_map<string, Indicator> inds_dict;
            for (auto iter = inds.begin(); iter != inds.end(); ++iter) {
              inds_dict[iter->first.cast<string>()] =
                  iter->second.cast<Indicator>();
            }
            FactorSet factorset = FactorSet(inds_dict, ktype);
            self.setRefFactorSet(factorset);
          },
          py::arg("inds"), py::arg("ktype") = KQuery::DAY)
      .def(
          "set_ref_factorset",
          [](MultiFactorBase& self, FactorSet factorset) {
            self.setRefFactorSet(factorset);
          },
          R"(set_ref_factorset(self, factorset)

    Set the original factor set

    :param FactorSet factorset: the new original factor set)")

      .def("get_factor", &MultiFactorBase::getFactor,
           py::return_value_policy::copy, py::arg("stock"),
           R"(get_factor(self, stock)

    Get the new composed factor of the specified security

    :param Stock stock: the specified security)")

      .def("get_all_factors", &MultiFactorBase::getAllFactors,
           py::return_value_policy::copy,
           R"(get_all_factors(self)

    Get the list of the composed factors of all the securities

    :return: [factor1, factor2, ...] in the same order as the reference securities)")

      .def(
          "set_normalize",
          [](PyMultiFactor& self, py::object norm) { self.set_norm(norm); },
          py::arg("norm"),
          R"(set_normalize(self, norm)

    Set the standardization or normalization method (affecting all the factors)

    :param NormalizeBase norm: the standardization or normalization method instance)")

      .def(
          "add_special_normalize",
          [](PyMultiFactor& self, const string& name, py::object norm,
             const string& category, const IndicatorList& style_inds) {
            self.add_special_norm(name, norm, category, style_inds);
          },
          py::arg("name"), py::arg("norm") = NormPtr(),
          py::arg("category") = "", py::arg("style_inds") = IndicatorList(),
          R"(add_special_normalize(self, name[, norm=None, category="", style_inds=[]])

    Apply a specific standardization/normalization, industry neutralization or style factor neutralization operation to the indicator with the specified name. The standardization operation, the industry neutralization and the style factor neutralization are independent of each other; they can be specified together or separately.

    :param str name: the special normalization method name
    :param Normalize norm: the special normalization method
    :param str category: for the industry neutralization, specify the block category
    :param list[Indicator] style_inds: the list of the style indicators used for the neutralization)")

      .def("get_ic", &MultiFactorBase::getIC, py::arg("ndays") = 0,
           R"(get_ic(self[, ndays=0])

    Get the IC of the composed factor, with the same length as the reference dates

    For the new factors weighted with IC/ICIR, it is best to keep ndays consistent with ic_n,
    but for the new factors calculated with the equal weights, it is not necessarily required to calculate with ic_n.
    Therefore, ndays has a special value 0, which means calculating the IC directly with the ic_n parameter

    :rtype: Indicator)")

      .def("get_icir", &MultiFactorBase::getICIR, py::arg("ir_n"),
           py::arg("ic_n") = 0,
           R"(get_icir(self, ir_n[, ic_n=0])

    Get the ICIR of the composed factor

    :param int ir_n: the n window for calculating the IR
    :param int ic_n: the n window for calculating the IC (the same as ndays in get_ic))")

      .def("clone", &MultiFactorBase::clone, "The clone operation")

      .def(
          "get_scores",
          [](MultiFactorBase& self, const Datetime& date, size_t start,
             py::object end, py::object filter) {
            size_t cend = end.is_none() ? Null<size_t>() : end.cast<size_t>();
            if (filter.is_none()) {
              return self.getScores(date, start, cend,
                                    std::function<bool(const ScoreRecord&)>());
            }
            HAYAKU_CHECK(py::hasattr(filter, "__call__"),
                         "filter not callable!");
            py::object filter_func = filter.attr("__call__");
            ScoreRecord sc;
            try {
              filter_func(sc);
              return self.getScores(date, start, cend,
                                    [&](const ScoreRecord& score_) {
                                      return filter_func(score_).cast<bool>();
                                    });
            } catch (...) {
              filter_func(date, sc);
              return self.getScores(
                  date, start, cend,
                  [&](const Datetime& date_, const ScoreRecord& score_) {
                    return filter_func(date_, score_).cast<bool>();
                  });
            }
          },
          py::arg("date"), py::arg("start") = 0, py::arg("end") = py::none(),
          py::arg("filter") = py::none(),
          R"(get_score(self, date[, start=0, end=Null])

    Get all the factor values of the cross-section on the specified date, already sorted descending, equivalent to the cross-section scores of the securities on that date.

    :param Datetime date: the specified date
    :param int start: the start of the daily ranking to take
    :param int end: the end of the daily ranking to take (exclusive)
    :param function filter: a callable object with the prototype (ScoreRecord)->bool or (Datetime, ScoreRecord)->bool
    :rtype: ScoreRecordList)")

      .def("get_all_scores", &MultiFactorBase::getAllScores,
           py::return_value_policy::copy,
           R"(get_all_scores(self)

    Get all the scores of all the dates, with the same length as the reference dates

    :return: ScoreRecordList)")

      .def("get_all_src_factors", &MultiFactorBase::getAllSrcFactors,
           R"(get_all_src_factors(self)

    Get the list of all the original factors (if the standardization or the industry neutralization is specified, the returned list is the processed factor list)

    :rtype: list
    :return: list IndicatorList stks x inds)")

          DEF_PICKLE(MultiFactorPtr);

  m.def("MF_EqualWeight", py::overload_cast<>(MF_EqualWeight));
  m.def(
      "MF_EqualWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, bool spearman, int mode,
         bool save_all_factors) {
        StockList c_stks = get_stock_list_from_python(stks);
        Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

        // Judge the input type
        if (py::isinstance<FactorSet>(input)) {
          // The input is a FactorSet
          FactorSet factset = input.cast<FactorSet>();
          return MF_EqualWeight(factset, c_stks, query, ref_stock, ic_n,
                                spearman, mode, save_all_factors);
        } else if (py::isinstance<py::sequence>(input)) {
          // The input is a sequence (assumed to be an Indicator list)
          IndicatorList c_inds = python_list_to_vector<Indicator>(input);
          return MF_EqualWeight(c_inds, c_stks, query, ref_stock, ic_n,
                                spearman, mode, save_all_factors);
        } else {
          throw std::invalid_argument(
              "First parameter must be either FactorSet or sequence of "
              "Indicator");
        }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5,
      py::arg("spearman") = true, py::arg("mode") = 0,
      py::arg("save_all_factors") = false,
      R"(MF_EqualWeight(input, stks, query, ref_stk[, ic_n=5])

    Compose the factor with the equal weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_EqualWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_EqualWeight(factor_set, stocks, query))");

  m.def("MF_Weight", py::overload_cast<>(MF_Weight));
  m.def(
      "MF_Weight",
      [](const py::object& input, const py::object& stks,
         const py::object& weights_obj, const KQuery& query,
         const py::object& ref_stk, int ic_n, bool spearman, int mode,
         bool save_all_factors) {
        StockList c_stks = get_stock_list_from_python(stks);
        Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();
        PriceList c_weights = python_list_to_vector<price_t>(weights_obj);

        // Judge the input type
        if (py::isinstance<FactorSet>(input)) {
          // The input is a FactorSet
          FactorSet factset = input.cast<FactorSet>();
          return MF_Weight(factset, c_weights, c_stks, query, ref_stock, ic_n,
                           spearman, mode, save_all_factors);
        } else if (py::isinstance<py::sequence>(input)) {
          // The input is a sequence (assumed to be an Indicator list)
          IndicatorList c_inds = python_list_to_vector<Indicator>(input);
          return MF_Weight(c_inds, c_weights, c_stks, query, ref_stock, ic_n,
                           spearman, mode, save_all_factors);
        } else {
          throw std::invalid_argument(
              "First parameter must be either FactorSet or sequence of "
              "Indicator");
        }
      },
      py::arg("input"), py::arg("stks"), py::arg("weights"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5,
      py::arg("spearman") = true, py::arg("mode") = 0,
      py::arg("save_all_factors") = false,
      R"(MF_Weight(input, stks, weights, query, ref_stk[, ic_n=5, spearman=True, mode=0, save_all_factors=False])

    Compose the factor by the specified weights = ind1 * weight1 + ind2 * weight2 + ... + indn * weightn, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param sequence(float) weights: the weight list (must be the same length as the number of the factors)
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        weights = [0.6, 0.4]
        mf1 = MF_Weight(indicators, stocks, weights, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_Weight(factor_set, stocks, weights, query))");

  m.def("MF_ICWeight", py::overload_cast<>(MF_ICWeight));
  m.def(
      "MF_ICWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n, bool spearman,
         int mode, bool save_all_factors) {
        StockList c_stks = get_stock_list_from_python(stks);
        Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

        // Judge the input type
        if (py::isinstance<FactorSet>(input)) {
          // The input is a FactorSet
          FactorSet factset = input.cast<FactorSet>();
          return MF_ICWeight(factset, c_stks, query, ref_stock, ic_n,
                             ic_rolling_n, spearman, mode, save_all_factors);
        } else if (py::isinstance<py::sequence>(input)) {
          // The input is a sequence (assumed to be an Indicator list)
          IndicatorList c_inds = python_list_to_vector<Indicator>(input);
          return MF_ICWeight(c_inds, c_stks, query, ref_stock, ic_n,
                             ic_rolling_n, spearman, mode, save_all_factors);
        } else {
          throw std::invalid_argument(
              "First parameter must be either FactorSet or sequence of "
              "Indicator");
        }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5,
      py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling IC weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICWeight(factor_set, stocks, query))");

  m.def("MF_ICIRWeight", py::overload_cast<>(MF_ICIRWeight));
  m.def(
      "MF_ICIRWeight",
      [](const py::object& input, const py::object& stks, const KQuery& query,
         const py::object& ref_stk, int ic_n, int ic_rolling_n, bool spearman,
         int mode, bool save_all_factors) {
        StockList c_stks = get_stock_list_from_python(stks);
        Stock ref_stock = ref_stk.is_none() ? Stock() : ref_stk.cast<Stock>();

        // Judge the input type
        if (py::isinstance<FactorSet>(input)) {
          // The input is a FactorSet
          FactorSet factset = input.cast<FactorSet>();
          return MF_ICIRWeight(factset, c_stks, query, ref_stock, ic_n,
                               ic_rolling_n, spearman, mode, save_all_factors);
        } else if (py::isinstance<py::sequence>(input)) {
          // The input is a sequence (assumed to be an Indicator list)
          IndicatorList c_inds = python_list_to_vector<Indicator>(input);
          return MF_ICIRWeight(c_inds, c_stks, query, ref_stock, ic_n,
                               ic_rolling_n, spearman, mode, save_all_factors);
        } else {
          throw std::invalid_argument(
              "First parameter must be either FactorSet or sequence of "
              "Indicator");
        }
      },
      py::arg("input"), py::arg("stks"), py::arg("query"),
      py::arg("ref_stk") = py::none(), py::arg("ic_n") = 5,
      py::arg("ic_rolling_n") = 120, py::arg("spearman") = true,
      py::arg("mode") = 0, py::arg("save_all_factors") = false,
      R"(MF_ICIRWeight(input, stks, query, ref_stk[, ic_n=5, ic_rolling_n=120])

    Compose the factor with the rolling ICIR weights, supporting several input types

    :param input: the factor input, which can be a FactorSet object or an Indicator sequence
    :param sequence(stock) stks: the list of the securities to calculate
    :param Query query: the date range
    :param Stock ref_stk: the reference security used for the date alignment (when unspecified, defaults to sh000001)
    :param int ic_n: the N-day return corresponding to the default IC
    :param int ic_rolling_n: the IC rolling period
    :param bool spearman: use spearman to calculate the correlation coefficient by default, otherwise pearson
    :param int mode: the sorting mode when getting the cross-section data: 0-descending, 1-ascending, 2-no sorting
    :param bool save_all_factors: whether to save all the factor values, affecting the get_actor/get_all_factors methods
    :rtype: MultiFactorBase

    .. code-block:: python

        # Use an Indicator list
        indicators = [MA(CLOSE(), 5), MA(CLOSE(), 10)]
        mf1 = MF_ICIRWeight(indicators, stocks, query)

        # Use a FactorSet
        factor_set = FactorSet(indicators)
        mf2 = MF_ICIRWeight(factor_set, stocks, query))");
}

// Registration group: _Normalize
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-04
 *      Author: fasiondog
 */

#include <strategy/selection/Normalizers.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyNormalizeBase : public NormalizeBase {
  PY_CLONE(PyNormalizeBase, NormalizeBase)

 public:
  PyNormalizeBase() : NormalizeBase() { is_python_object_ = true; }

  PyNormalizeBase(const string& name) : NormalizeBase(name) {
    is_python_object_ = true;
  }

  PyNormalizeBase(const NormalizeBase& base) : NormalizeBase(base) {
    is_python_object_ = true;
  }

  PriceList normalize(const PriceList& data) override {
    PYBIND11_OVERRIDE_PURE(PriceList, NormalizeBase, normalize, data);
  }
};

void export_Normalize(py::module& m) {
  py::class_<NormalizeBase, NormalizePtr, PyNormalizeBase>(
      m, "NormalizeBase", py::dynamic_attr(),
      R"(The cross-section standardization operation used for the MF)")
      .def(py::init<>())
      .def(py::init<const NormalizeBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<NormalizeBase>)
      .def("__repr__", to_py_str<NormalizeBase>)

      .def_property("name",
                    py::overload_cast<>(&NormalizeBase::name, py::const_),
                    py::overload_cast<const string&>(&NormalizeBase::name),
                    py::return_value_policy::copy, "Name")

      .def("get_param", &NormalizeBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def(
          "set_param",
          static_cast<void (NormalizeBase::*)(
              const std::string&, const boost::any&)>(&NormalizeBase::setParam),
          R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &NormalizeBase::haveParam,
           "Whether the specified parameter exists")

      .def("clone", &NormalizeBase::clone, "The clone operation")

      .def("normalize", &NormalizeBase::normalize,
           "[Overload interface] The subclass calculation interface")

          DEF_PICKLE(NormalizePtr);

  m.def("NORM_NOTHING", &NORM_NOTHING,
        "No cross-section standardization operation");
  m.def("NORM_MinMax", &NORM_MinMax, "The min-max standardization operation");
  m.def("NORM_Zscore", &NORM_Zscore, py::arg("out_extreme") = false,
        py::arg("nsigma") = 3.0, py::arg("recursive") = false,
        R"(The Z-score standardization operation

    :param out_extreme: whether to remove the outliers
    :param nsigma: the outlier judgment multiple ±3.0
    :param recursive: whether to process the outliers recursively)");

  m.def("NORM_Quantile", &NORM_Quantile, py::arg("quantile_min") = 0.01,
        py::arg("quantile_max") = 0.99,
        R"(The quantile cross-section standardization operation

    :param quantile_min: the minimum quantile
    :param quantile_max: the maximum quantile)");

  m.def(
      "NORM_Quantile_Uniform", &NORM_Quantile_Uniform,
      py::arg("quantile_min") = 0.01, py::arg("quantile_max") = 0.99,
      R"(The quantile cross-section uniform distribution standardization operation

    :param quantile_min: the minimum quantile
    :param quantile_max: the maximum quantile)");
}

// Registration group: _ProfitGoal
/*
 * _ProfitGoal.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <strategy/risk/ProfitGoals.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyProfitGoalBase : public ProfitGoalBase {
  PY_CLONE(PyProfitGoalBase, ProfitGoalBase)

 public:
  PyProfitGoalBase() : ProfitGoalBase() { is_python_object_ = true; }

  PyProfitGoalBase(const string& name) : ProfitGoalBase(name) {
    is_python_object_ = true;
  }

  PyProfitGoalBase(const ProfitGoalBase& base) : ProfitGoalBase(base) {
    is_python_object_ = true;
  }

  void buyNotify(const TradeRecord& tr) override {
    PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "buy_notify", buyNotify, tr);
  }

  void sellNotify(const TradeRecord& tr) override {
    PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "sell_notify", sellNotify, tr);
  }

  price_t getGoal(const Datetime& datetime, price_t price) override {
    PYBIND11_OVERLOAD_PURE_NAME(price_t, ProfitGoalBase, "get_goal", getGoal,
                                datetime, price);
  }

  price_t getShortGoal(const Datetime& date, price_t price) override {
    PYBIND11_OVERLOAD_NAME(price_t, ProfitGoalBase, "get_short_goal",
                           getShortGoal, date, price);
  }

  void _reset() override { PYBIND11_OVERLOAD(void, ProfitGoalBase, _reset, ); }

  void _calculate() override {
    PYBIND11_OVERLOAD_NAME(void, ProfitGoalBase, "_calculate", _calculate, );
  };
};

void export_ProfitGoal(py::module& m) {
  py::class_<ProfitGoalBase, PGPtr, PyProfitGoalBase>(
      m, "ProfitGoalBase", py::dynamic_attr(),
      R"(The profit goal strategy base class

The custom profit goal strategy interfaces:

- getGoal : [Required] Get the target price
- _calculate : [Required] The subclass calculation interface
- _clone : [Required] The clone interface
- _reset : [Optional] Reload the private variables
- buyNotify : [Optional] Receive the actual buy notification, reserved for the multiple position increase/decrease processing
- sellNotify : [Optional] Receive the actual sell notification, reserved for the multiple position increase/decrease processing)")

      .def(py::init<>())
      .def(py::init<const ProfitGoalBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<ProfitGoalBase>)
      .def("__repr__", to_py_str<ProfitGoalBase>)

      .def_property("name",
                    py::overload_cast<>(&ProfitGoalBase::name, py::const_),
                    py::overload_cast<const string&>(&ProfitGoalBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &ProfitGoalBase::getTO, &ProfitGoalBase::setTO,
                    "Set or get the trading object")
      .def("get_param", &ProfitGoalBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (ProfitGoalBase::*)(const std::string&,
                                                const boost::any&)>(
               &ProfitGoalBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ProfitGoalBase::haveParam,
           "Whether the specified parameter exists")

      .def("buy_notify", &ProfitGoalBase::buyNotify,
           R"(buy_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual buy operation, notify the trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual buy trade record when the actual buying occurs)")

      .def("sell_notify", &ProfitGoalBase::sellNotify,
           R"(sell_notify(self, trade_record)

    [Overload interface] When the trading system performs the actual sell operation, notify the actual trade changes; it only needs to be overloaded when there are multiple position increases/decreases

    :param TradeRecord trade_record: the actual sell trade record when the actual selling occurs)")

      .def("get_goal", &ProfitGoalBase::getGoal,
           R"(get_goal(self, datetime, price)

    [Overload interface] Get the profit goal price; returning constant.null_price means the goal is not limited; returning 0 means it needs to be sold

    :param Datetime datetime: the current time
    :param float price: the current price
    :return: the target price
    :rtype: float)")

      //.def("getShortGoal", &ProfitGoalBase::getShortGoal,
      //&ProfitGoalWrap::default_getShortGoal)

      .def("reset", &ProfitGoalBase::reset, "The reset operation")
      .def("clone", &ProfitGoalBase::clone, "The clone operation")
      .def("_calculate", &ProfitGoalBase::_calculate,
           "[Overload interface] The subclass calculation interface")
      .def("_reset", &ProfitGoalBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the "
           "internal private variables")

          DEF_PICKLE(PGPtr);

  m.def("PG_NoGoal", PG_NoGoal, R"(PG_NoGoal()

    The no profit goal strategy, usually for testing or comparison.

    :return: the profit goal strategy instance)");

  m.def("PG_FixedPercent", PG_FixedPercent, py::arg("p") = 0.2,
        R"(PG_FixedPercent([p = 0.2])

    The fixed percentage profit goal; the target price = the buy price * (1 + p)

    :param float p: the percentage
    :return: the profit goal strategy instance)");

  m.def("PG_FixedHoldDays", PG_FixedHoldDays, py::arg("days") = 5,
        R"(PG_FixedHoldDays([days=5])

    The fixed holding days profit goal strategy

    :param int days: the allowed holding days (counted by the trading days), defaulting to 5 days
    :return: the profit goal strategy instance)");
}

// Registration group: _SCFilter
/*
 *  Copyright (c) 2025 hikyuu.org
 *
 *  Created on: 2025-10-04
 *      Author: fasiondog
 */

#include <strategy/selection/ScoreFilters.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyScoresFilterBase : public ScoresFilterBase {
  PY_CLONE(PyScoresFilterBase, ScoresFilterBase)

 public:
  PyScoresFilterBase() : ScoresFilterBase() { is_python_object_ = true; }

  PyScoresFilterBase(const string& name) : ScoresFilterBase(name) {
    is_python_object_ = true;
  }

  PyScoresFilterBase(const ScoresFilterBase& base) : ScoresFilterBase(base) {
    is_python_object_ = true;
  }

  virtual ScoreRecordList _filter(const ScoreRecordList& scores,
                                  const Datetime& date,
                                  const KQuery& query) override {
    PYBIND11_OVERRIDE_PURE(ScoreRecordList, ScoresFilterBase, _filter, scores,
                           date, query);
  }
};

void export_SCFilter(py::module& m) {
  py::class_<ScoresFilterBase, ScoresFilterPtr, PyScoresFilterBase>(
      m, "ScoresFilterBase", py::dynamic_attr(),
      R"(The cross-section standardization operation used for the MF)")
      .def(py::init<>())
      .def(py::init<const ScoresFilterBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<ScoresFilterBase>)
      .def("__repr__", to_py_str<ScoresFilterBase>)

      .def_property("name",
                    py::overload_cast<>(&ScoresFilterBase::name, py::const_),
                    py::overload_cast<const string&>(&ScoresFilterBase::name),
                    py::return_value_policy::copy, "Name")

      .def("get_param", &ScoresFilterBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (ScoresFilterBase::*)(const std::string&,
                                                  const boost::any&)>(
               &ScoresFilterBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &ScoresFilterBase::haveParam,
           "Whether the specified parameter exists")

      .def("clone", &ScoresFilterBase::clone, "The clone operation")

      .def("filter", &ScoresFilterBase::filter,
           R"(filter(self, scores, date, query)

    The cross-section filter
    :param list scores: the cross-section data
    :param Datetime date: the cross-section date
    :param KQuery query: the query parameters
    :return: the cross-section data
    :rtype: ScoreRecordList)")

      .def("_filter", &ScoresFilterBase::_filter,
           "[Overload interface] The subclass calculation interface")

      .def("__or__", [](const ScoresFilterPtr& self,
                        const ScoresFilterPtr& other) { return self | other; })

          DEF_PICKLE(ScoresFilterPtr);

  m.def("SCFilter_IgnoreNan", &SCFilter_IgnoreNan,
        R"(SCFilter_IgnoreNan() -> ScoresFilterPtr

    Ignore the NAN values in the cross-section)");

  m.def("SCFilter_LessOrEqualValue", &SCFilter_LessOrEqualValue,
        py::arg("value") = 0.0,
        R"(SCFilter_LessOrEqualValue([value = 0.0])

    Filter out the cross-sections whose score is less than or equal to the specified value)");

  m.def("SCFilter_TopN", &SCFilter_TopN, py::arg("topn") = 10,
        R"(SCFilter_TopN([topn: int=10])

    Get the first topn of the score list

    :param int topn: the first topn)");

  m.def("SCFilter_Group", &SCFilter_Group, py::arg("group") = 10,
        py::arg("group_index") = 0,
        R"(SCFilter_Group([group: int=10, group_index: int=0])

    Group and filter by the cross-section
    :param int group: the number of the groups
    :param int group_index: the group index
    :return: the cross-section filter
    :rtype: ScoresFilterPtr)");

  m.def("SCFilter_AmountLimit", &SCFilter_AmountLimit,
        py::arg("min_amount_percent_limit") = 0.1,
        R"(SCFilter_AmountLimit([min_amount_percent_limit: float = 0.1])

    Filter out the cross-sections whose amount is within the percentage range at the end of the score list

    Note: it is related to the order of the passed cross-section score list; if it is descending, the system score records with the smaller amounts are filtered; otherwise, the records with the larger amounts

    :param double min_amount_percent_limit: the minimum amount percentage limit
    :return: the cross-section filter
    :rtype: ScoresFilterPtr)");

  m.def("SCFilter_Price", &SCFilter_Price, py::arg("min_price") = 10.,
        py::arg("max_price") = 100000.,
        R"(SCFilter_Price([min_price = 10., max_price = 100000.])

    Keep only the targets whose prices are within [min_price, max_price]

    Note: it is related to the order of the passed cross-section score list; if it is descending, the system score records with the smaller prices are filtered; otherwise, the records with the larger prices

    :param double min_price: the minimum price limit
    :param double max_price: the maximum price limit)");
}

// Registration group: _Signal
/*
 * _Signal.cpp
 *
 *  Created on: 2013-3-18
 *      Author: fasiondog
 */

#include <strategy/decision/Signals.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PySignalBase : public SignalBase {
  PY_CLONE(PySignalBase, SignalBase)

 public:
  PySignalBase() : SignalBase() { is_python_object_ = true; }

  PySignalBase(const string& name) : SignalBase(name) {
    is_python_object_ = true;
  }

  PySignalBase(const SignalBase& base) : SignalBase(base) {
    is_python_object_ = true;
  }

  void _calculate(const KData& kdata) override {
    PYBIND11_OVERLOAD_PURE(void, SignalBase, _calculate, kdata);
  }

  void _reset() override { PYBIND11_OVERLOAD(void, SignalBase, _reset, ); }
};

void export_Signal(py::module& m) {
  py::class_<SignalBase, SGPtr, PySignalBase>(m, "SignalBase",
                                              py::dynamic_attr(),
                                              R"(The signal generator base class
    The signal generator is responsible for generating the buy and the sell signals.

Common parameters:

    - alternate (bool|True): whether the buy and the sell signals appear alternately. The single-line signals usually judge the generation of the signals through the inflection points, the slopes, etc.; in this case, the consecutive buy signals or the consecutive sell signals may appear, and this parameter can control whether the buy and the sell signals appear alternately. The double-line cross signals usually have the buys and the sells already alternating, in which case this parameter is invalid.

The custom signal generator interfaces:

    - _calculate : [Required] the subclass calculation interface
    - _clone : [Required] the clone interface
    - _reset : [Optional] reload the private variables)")

      .def(py::init<>())
      .def(py::init<const string&>())
      .def(py::init<const SignalBase&>())

      .def("__str__", to_py_str<SignalBase>)
      .def("__repr__", to_py_str<SignalBase>)

      .def_property("name", py::overload_cast<>(&SignalBase::name, py::const_),
                    py::overload_cast<const string&>(&SignalBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &SignalBase::getTO, &SignalBase::setTO,
                    py::return_value_policy::copy,
                    "Set or get the trading object")

      .def("get_param", &SignalBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (SignalBase::*)(
               const std::string&, const boost::any&)>(&SignalBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &SignalBase::haveParam,
           "Whether the specified parameter exists")

      .def("should_buy", &SignalBase::shouldBuy, R"(should_buy(self, datetime)

    Whether it can be bought at the specified moment

    :param Datetime datetime: the specified moment
    :rtype: bool)")

      .def("should_sell", &SignalBase::shouldSell,
           R"(should_sell(self, datetime)

    Whether it can be sold at the specified moment

    :param Datetime datetime: the specified moment
    :rtype: bool)")

      .def("next_time_should_buy", &SignalBase::nextTimeShouldBuy,
           R"(next_time_should_buy(self)

    Whether it can be bought at the next moment, equivalent to whether the last moment indicates buying)")

      .def("next_time_should_sell", &SignalBase::nextTimeShouldSell,
           R"(next_time_should_sell(self)

    Whether it can be sold at the next moment, equivalent to whether the last moment indicates selling)")

      .def("get_buy_signal", &SignalBase::getBuySignal, R"(get_buy_signal(self)

    Get the list of all the buy indication dates

    :rtype: DatetimeList)")

      .def("get_sell_signal", &SignalBase::getSellSignal,
           R"(get_sell_signal(self)

    Get the list of all the sell indication dates

    :rtype: DatetimeList)")

      .def("_add_signal", &SignalBase::_addSignal, py::arg("datetime"),
           py::arg("value"), R"()")

      .def("_add_buy_signal", &SignalBase::_addBuySignal, py::arg("datetime"),
           py::arg("value") = 1.0,
           R"(_add_buy_signal(self, datetime)

    Add a buy signal, called in _calculate

    :param Datetime datetime: the date indicating buying)")

      .def("_add_sell_signal", &SignalBase::_addSellSignal, py::arg("datetime"),
           py::arg("value") = -1.0, R"(_add_sell_signal(self, datetime)

    Add a sell signal, called in _calculate

    :param Datetime datetime: the date indicating selling)")

      .def("reset", &SignalBase::reset, "The reset operation")
      .def("clone", &SignalBase::clone, "The clone operation")
      .def("_calculate", &SignalBase::_calculate, R"(_calculate(self, kdata)

    [Overload interface] The subclass calculation interface)")

      .def("_reset", &SignalBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the "
           "internal private "
           "variables")

      .def("__add__", [](const SignalPtr& self,
                         const SignalPtr& other) { return self + other; })
      .def("__add__",
           [](const SignalPtr& self, double other) { return self + other; })
      .def("__radd__",
           [](const SignalPtr& self, double other) { return other + self; })
      .def("__sub__", [](const SignalPtr& self,
                         const SignalPtr& other) { return self - other; })
      .def("__sub__",
           [](const SignalPtr& self, double other) { return self - other; })
      .def("__rsub__",
           [](const SignalPtr& self, double other) { return other - self; })
      .def("__mul__", [](const SignalPtr& self,
                         const SignalPtr& other) { return self * other; })
      .def("__mul__",
           [](const SignalPtr& self, double other) { return self * other; })
      .def("__rmul__",
           [](const SignalPtr& self, double other) { return other * self; })
      .def("__truediv__", [](const SignalPtr& self,
                             const SignalPtr& other) { return self / other; })
      .def("__truediv__",
           [](const SignalPtr& self, double other) { return self / other; })
      .def("__rtruediv__",
           [](const SignalPtr& self, double other) { return other / self; })
      .def("__and__", [](const SignalPtr& self,
                         const SignalPtr& other) { return self & other; })
      .def("__or__", [](const SignalPtr& self,
                        const SignalPtr& other) { return self | other; })

          DEF_PICKLE(SGPtr);

  m.def("SG_Bool", SG_Bool, py::arg("buy"), py::arg("sell"),
        py::arg("alternate") = true,
        R"(SG_Bool(buy, sell)

    The boolean signal generator, using the Indicators whose operation results are like bool arrays as the buy and the sell indications respectively.

    :param Indicator buy: the buy indication (if the corresponding position in the result Indicator is >0, it means buying)
    :param Indicator sell: the sell indication (if the corresponding position in the result Indicator is >0, it means selling)
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

  m.def("SG_Single", SG_Single, py::arg("ind"), py::arg("filter_n") = 10,
        py::arg("filter_p") = 0.1,
        R"(SG_Single(ind[, filter_n = 10, filter_p = 0.1])

    Generate the single-line inflection point signal generator. It uses the curve inflection point algorithm given in Trade Your Way to Financial Freedom [BOOK1]_ to judge the curve trend; the formula is as follows::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - AMA[1] > filter
        or Buy When AMA - AMA[2] > filter
        or Buy When AMA - AMA[3] > filter

    :param Indicator ind: the input indicator
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator)");

  m.def("SG_Single2", SG_Single2, py::arg("ind"), py::arg("filter_n") = 10,
        py::arg("filter_p") = 0.1,
        R"(SG_Single2(ind[, filter_n = 10, filter_p = 0.1])

    Generate the single-line inflection point signal generator 2 [BOOK1]_::

        filter = percentage * STDEV((AMA-AMA[1], N)

        Buy  When AMA - @lowest(AMA,n) > filter
        Sell When @highest(AMA, n) - AMA > filter

    :param Indicator ind: the input indicator
    :param int filter_n: the N-day period
    :param float filter_p: the filter percentage
    :return: the signal generator)");

  m.def("SG_Cross", SG_Cross, py::arg("fast"), py::arg("slow"),
        R"(SG_Cross(fast, slow)

    The double-line cross indicator; when the fast line crosses the slow line from below upward, buy; when the fast line crosses the slow line from above downward, sell. E.g.: buy when the 5-day MA crosses above the 10-day MA, and sell when the 5-day MA crosses below the 10-day MA::

        SG_Cross(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator)");

  m.def("SG_CrossGold", SG_CrossGold, py::arg("fast"), py::arg("slow"),
        R"(SG_CrossGold(fast, slow)

    The golden cross indicator; when the fast line crosses the slow line from below upward and both the fast line and the slow line are heading upward, it is a golden cross, buy;
    when the fast line crosses the slow line from above downward and both the fast line and the slow line are heading downward, it is a death cross, sell. ::

        SG_CrossGold(MA(C, n=10), MA(C, n=30))

    :param Indicator fast: the fast line
    :param Indicator slow: the slow line
    :return: the signal generator)");

  m.def("SG_Flex", SG_Flex, py::arg("ind"), py::arg("slow_n"),
        R"(SG_Flex(ind, slow_n)

    Use its own EMA(slow_n) as the slow line and itself as the fast line; buy when the fast line crosses the slow line upward, and sell when the fast line crosses the slow line downward.

    :param Indicator ind: the input indicator
    :param int slow_n: the EMA period of the slow line
    :return: the signal generator)");

  m.def("SG_Band",
        py::overload_cast<const Indicator&, const Indicator&, const Indicator&>(
            SG_Band),
        py::arg("ind"), py::arg("lower"), py::arg("upper"));
  m.def("SG_Band",
        py::overload_cast<const Indicator&, price_t, price_t>(SG_Band),
        py::arg("ind"), py::arg("lower"), py::arg("upper"),
        R"(SG_Band(ind, lower, upper)

    The indicator range indicator; when the indicator exceeds the upper band, buy;
    when the indicator is below the lower band, sell. ::

        SG_Band(MA(C, n=10), 100, 200)
        SG_Band(CLOSE, MA(LOW), MA(HIGH)))");

  m.def("SG_AllwaysBuy", SG_AllwaysBuy, R"(SG_AllwaysBuy()

    A special SG that issues the buy signal every day continuously, usually used together with the PF)");

  m.def("SG_Cycle", SG_Cycle, R"(SG_Cycle()

    A special SG, used together with the PF, with the PF position adjustment period as the buy signal)");

  m.def("SG_OneSide", SG_OneSide, py::arg("ind"), py::arg("is_buy"),
        R"(SG_OneSide(ind, is_buy)

    Build the one-sided signal (containing only the buy or the sell signal) from the input indicator; if the indicator value is greater than 0, add the signal. The SG_Buy or the SG_Sell functions can also be used.

    :param Indicator ind: the input indicator
    :param bool is_buy: what is built is the buy signal; otherwise, it is the sell signal
    :return: the signal generator)");

  m.def("SG_Buy", SG_Buy, py::arg("ind"), R"(SG_Buy(ind)

    Generate the one-sided buy signal

    :param Indicator ind: the input indicator
    :return: the signal generator)");

  m.def("SG_Sell", SG_Sell, py::arg("ind"), R"(SG_Sell(ind)

    Generate the one-sided sell signal

    :param Indicator ind: the input indicator
    :return: the signal generator)");

  m.def(
      "SG_Add",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_Add(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_Add",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Add),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_Add(sg1, sg2, alternate)

    Generate the signal of the sum of the two indicators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

  m.def(
      "SG_Sub",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_Sub(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_Sub",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Sub),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_Sub(sg1, sg2, alternate)

    Generate the signal of the difference of the two indicators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True
    :return: the signal generator)");

  m.def(
      "SG_Mul",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_Mul(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_Mul",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Mul),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_Mul(sg1, sg2, alternate)

    Generate the signal of the product of the two signal generators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

  m.def(
      "SG_Div",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_Div(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_Div",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Div),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_Div(sg1, sg2, alternate)

    Generate the signal of the quotient of the two signal generators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

  m.def(
      "SG_And",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_And(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_And",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_And),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_And(sg1, sg2, alternate)

    Generate the signal of the AND of the two indicators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");

  m.def(
      "SG_Or",
      [](const py::sequence& sg_list, bool alternate) {
        vector<SignalPtr> sg_vec = python_list_to_vector<SignalPtr>(sg_list);
        return SG_Or(sg_vec, alternate);
      },
      py::arg("sg_list"), py::arg("alternate"));
  m.def("SG_Or",
        py::overload_cast<const SignalPtr&, const SignalPtr&, bool>(SG_Or),
        py::arg("sg1"), py::arg("sg2"), py::arg("alternate"),
        R"(SG_Or(sg1, sg2, alternate)

    Generate the signal of the logical OR of the two signal generators

    Since the alternate of the SG defaults to True, when using the form like "sg1 + sg2 + sg3", it is easy to overlook the alternate attribute of sg1 + sg2,
    it is recommended to use: SG_Add(sg1, sg2, False) + sg3 to avoid the alternate problem

    :param SignalBase sg1: the input signal 1
    :param SignalBase sg2: the input signal 2
    :param bool alternate: whether to buy and sell alternately, defaulting to True)");
}

// Registration group: _Slippage
/*
 * _Slippage.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <execution/pricing/SlippageBase.h>
#include <execution/pricing/SlippageModels.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PySlippageBase : public SlippageBase {
  PY_CLONE(PySlippageBase, SlippageBase)

 public:
  PySlippageBase() : SlippageBase() { is_python_object_ = true; }

  PySlippageBase(const string& name) : SlippageBase(name) {
    is_python_object_ = true;
  }

  PySlippageBase(const SlippageBase& base) : SlippageBase(base) {
    is_python_object_ = true;
  }

  void _calculate() override {
    PYBIND11_OVERLOAD_PURE(void, SlippageBase, _calculate, );
  }

  void _reset() override { PYBIND11_OVERLOAD(void, SlippageBase, _reset, ); }

  price_t getRealBuyPrice(const Datetime& datetime,
                          price_t planPrice) override {
    PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_buy_price",
                                getRealBuyPrice, datetime, planPrice);
  }

  price_t getRealSellPrice(const Datetime& datetime,
                           price_t planPrice) override {
    PYBIND11_OVERLOAD_PURE_NAME(price_t, SlippageBase, "get_real_sell_price",
                                getRealSellPrice, datetime, planPrice);
  }
};

void export_Slippage(py::module& m) {
  py::class_<SlippageBase, SPPtr, PySlippageBase>(
      m, "SlippageBase", py::dynamic_attr(),
      R"(The slippage algorithm base class

The custom slippage interfaces:

    - getRealBuyPrice : [Required] Calculate the actual buy price
    - getRealSellPrice : [Required] Calculate the actual sell price
    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")

      .def(py::init<>())
      .def(py::init<const SlippageBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<SlippageBase>)
      .def("__repr__", to_py_str<SlippageBase>)

      .def_property("name",
                    py::overload_cast<>(&SlippageBase::name, py::const_),
                    py::overload_cast<const string&>(&SlippageBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &SlippageBase::getTO, &SlippageBase::setTO,
                    "The associated trading object")

      .def("get_param", &SlippageBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (SlippageBase::*)(
               const std::string&, const boost::any&)>(&SlippageBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &SlippageBase::haveParam,
           "Whether the specified parameter exists")

      .def("get_real_buy_price", &SlippageBase::getRealBuyPrice,
           R"(get_real_buy_price(self, datetime, price)

    [Overload interface] Calculate the actual buy price

    :param Datetime datetime: the buy time
    :param float price: the planned buy price
    :return: the actual buy price
    :rtype: float)")

      .def("get_real_sell_price", &SlippageBase::getRealSellPrice,
           R"(get_real_sell_price(self, datetime, price)

    [Overload interface] Calculate the actual sell price

    :param Datetime datetime: the sell time
    :param float price: the planned sell price
    :return: the actual sell price
    :rtype: float)")

      .def("reset", &SlippageBase::reset, "The reset operation")
      .def("clone", &SlippageBase::clone, "The clone operation")
      .def("_calculate", &SlippageBase::_calculate,
           "[Overload interface] The subclass calculation interface")
      .def("_reset", &SlippageBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the "
           "internal private variables")

          DEF_PICKLE(SPPtr);

  m.def("SP_FixedPercent", &SP_FixedPercent, py::arg("p") = 0.001,
        R"(SP_FixedPercent([p=0.001])

    The fixed percentage slippage algorithm; the actual buy price = the planned buy price * (1 + p), and the actual sell price = the planned sell price * (1 - p)

    :param float p: the fixed percentage of the offset
    :return: the slippage algorithm instance)");

  m.def("SP_FixedValue", &SP_FixedValue, py::arg("value") = 0.01,
        R"(SP_FixedValue([value=0.01])

    The fixed price slippage algorithm; the actual buy price = the planned buy price + the offset price, and the actual sell price = the planned sell price - the offset price

    :param float value: the offset price
    :return: the slippage algorithm instance)");

  m.def("SP_Uniform", &SP_Uniform, py::arg("min_value") = -0.05,
        py::arg("max_value") = 0.05,
        R"(SP_Uniform([min_value=-0.05, max_value=0.05])

    The uniform distribution random price slippage algorithm; the buy and the sell operations are the random price offsets uniformly distributed within the range [min_value, max_value]

    :param float min_value: the minimum offset price
    :param float max_value: the maximum offset price
    :return: the slippage algorithm instance)");

  m.def("SP_Normal", &SP_Normal, py::arg("mean") = 0.0,
        py::arg("stddev") = 0.05,
        R"(SP_Normal([mean=0.0, stddev=0.05])

    The normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the normal distribution [mean, stddev] range

    :param float mean: the mean of the normal distribution
    :param float stddev: the standard deviation of the normal distribution
    :return: the slippage algorithm instance)");

  m.def("SP_LogNormal", &SP_LogNormal, py::arg("mean") = 0.0,
        py::arg("stddev") = 0.05,
        R"(SP_LogNormal([mean=0.0, stddev=0.05])

    The log-normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the log-normal distribution [mean, stddev] range

    :param float mean: the mean of the log-normal distribution
    :param float stddev: the standard deviation of the log-normal distribution
    :return: the slippage algorithm instance)");

  m.def(
      "SP_TruncNormal", &SP_TruncNormal, py::arg("mean") = 0.0,
      py::arg("stddev") = 0.05, py::arg("min_value") = -0.11,
      py::arg("max_value") = 0.1,
      R"(SP_TruncNormal([mean=0.0, stddev=0.05, min_value=-0.1, max_value=0.1])

    The truncated normal distribution random price slippage algorithm; the buy and the sell operations are the random price offsets within the truncated normal distribution [mean, stddev, min_value, max_value] range

    :param float mean: the mean of the truncated normal distribution
    :param float stddev: the standard deviation of the truncated normal distribution
    :param float min_value: the minimum truncation value
    :param float max_value: the maximum truncation value
    :return: the slippage algorithm instance)");
}

// Registration group: _Stoploss
/*
 * _Stoploss.cpp
 *
 *  Created on: 2013-3-21
 *      Author: fasiondog
 */

#include <strategy/risk/StoplossRules.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyStoplossBase : public StoplossBase {
  PY_CLONE(PyStoplossBase, StoplossBase)

 public:
  PyStoplossBase() : StoplossBase() { is_python_object_ = true; }

  PyStoplossBase(const string& name) : StoplossBase(name) {
    is_python_object_ = true;
  }

  PyStoplossBase(const StoplossBase& base) : StoplossBase(base) {
    is_python_object_ = true;
  }

  void _calculate() override {
    PYBIND11_OVERLOAD(void, StoplossBase, _calculate, );
  }

  void _reset() override { PYBIND11_OVERLOAD(void, StoplossBase, _reset, ); }

  price_t getPrice(const Datetime& datetime, price_t price) override {
    PYBIND11_OVERLOAD_PURE_NAME(price_t, StoplossBase, "get_price", getPrice,
                                datetime, price);
  }

  price_t getShortPrice(const Datetime& datetime, price_t price) override {
    PYBIND11_OVERLOAD_NAME(price_t, StoplossBase, "get_short_price",
                           getShortPrice, datetime, price);
  }
};

void export_Stoploss(py::module& m) {
  py::class_<StoplossBase, StoplossPtr, PyStoplossBase>(
      m, "StoplossBase", py::dynamic_attr(),
      R"(The stop-loss/take-profit algorithm base class
The custom stop-loss/take-profit strategy interfaces:

    - _calculate : [Required] The subclass calculation interface
    - _clone : [Required] The clone interface
    - _reset : [Optional] Reload the private variables)")
      .def(py::init<>())
      .def(py::init<const StoplossBase&>())
      .def(py::init<const string&>(), R"(The initialization constructor

    :param str name: the name)")

      .def("__str__", to_py_str<StoplossBase>)
      .def("__repr__", to_py_str<StoplossBase>)

      .def_property("name",
                    py::overload_cast<>(&StoplossBase::name, py::const_),
                    py::overload_cast<const string&>(&StoplossBase::name),
                    py::return_value_policy::copy, "Name")
      .def_property("to", &StoplossBase::getTO, &StoplossBase::setTO,
                    "The associated trading object")

      .def("get_param", &StoplossBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (StoplossBase::*)(
               const std::string&, const boost::any&)>(&StoplossBase::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &StoplossBase::haveParam,
           "Whether the specified parameter exists")

      .def("get_price", &StoplossBase::getPrice,
           R"(get_price(self, datetime, price)

    [Overload interface] Get the planned stop-loss price of this expected trade (buy); if there is no stop-loss price, return 0. It is used by the system to query the planned stop-loss price of this trade from the stop-loss strategy module before executing the trade.

    .. note::
        Generally, the stop-loss and the take-profit algorithms can be interchanged, but the getPrice of the stop-loss can be passed the planned trading price, e.g. using 30% of the buy price as the stop-loss. The take-profit does not consider the passed price parameter, i.e. it considers the price to be 0.0. In fact, even for the stop-loss, it is not recommended to use the price parameter; e.g. if 30% of the previous day's lowest price can be used as the stop-loss, the price parameter does not need to be considered.

    :param Datetime datetime: the trading time
    :param float price: the planned buy price
    :return: the stop-loss price
    :rtype: float)")

      .def("get_short_price", &StoplossBase::getShortPrice)

      .def("reset", &StoplossBase::reset, "The reset operation")
      .def("clone", &StoplossBase::clone, "The clone operation")
      .def("_calculate", &StoplossBase::_calculate,
           "[Overload interface] The subclass calculation interface")
      .def("_reset", &StoplossBase::_reset,
           "[Overload interface] The subclass reset interface, resetting the "
           "internal private variables")

          DEF_PICKLE(StoplossPtr);

  m.def("ST_FixedPercent", ST_FixedPercent, py::arg("p") = 0.03,
        R"(ST_FixedPercent([p=0.03])

    The fixed percentage stop-loss strategy, i.e. stopping the loss when the price is below a certain percentage of the buy price

    :param float p: the percentage(0,1]
    :return: the stop-loss/take-profit strategy instance)");

  m.def("ST_Indicator", ST_Indicator, py::arg("ind"),
        R"(ST_Indicator(ind)

    Use a technical indicator as the stop-loss price. E.g. using the 10-day EMA as the stop-loss::

        ST_Indicator(EMA(CLOSE(), n=10))

    :param Indicator ind:
    :return: the stop-loss/take-profit strategy instance)");

  m.def("ST_Saftyloss", ST_Saftyloss, py::arg("n1") = 10, py::arg("n2") = 3,
        py::arg("p") = 2.0,
        R"(ST_Saftyloss([n1=10, n2=3, p=2.0])

    See Come Into My Trading Room (2007, 地震出版社) by Alexander Elder, P202
    The calculation description: within the lookback period (generally 10 to 20 days), sum all the lengths of the downward penetrations and divide by the number of the downward penetrations,
    to get the average noise (i.e. the total length of all the lowest prices below the lowest price of the previous day within the lookback period divided by the number of the times), and subtract
    (the average noise of the previous day multiplied by a multiple) from the lowest price of today to get the stop line. To offset the fluctuation and ensure the upward movement of the stop line,
    take the highest value within the N days (generally 3 days) on the basis of the above result

    :param int n1: the lookback time window for calculating the average noise, defaulting to 10 days
    :param int n2: take the highest value within the n2 days for the preliminary stop line, defaulting to 3
    :param double p: the noise coefficient, defaulting to 2
    :return: the stop-loss/take-profit strategy instance)");
}

// Registration group: _StrategyEngine
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <strategy/StrategyEngine.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_StrategyEngine(py::module& m) {
  py::class_<StrategyDefinition>(m, "StrategyDefinition",
                                 "Explicit immutable strategy component graph")
      .def(py::init<MoneyManagerPtr, SignalPtr, string, EnvironmentPtr,
                    ConditionPtr, StoplossPtr, StoplossPtr, ProfitGoalPtr,
                    SlippagePtr, Parameter>(),
           py::arg("money_manager"), py::arg("signal"),
           py::arg("name") = "Strategy",
           py::arg("environment") = EnvironmentPtr(),
           py::arg("condition") = ConditionPtr(),
           py::arg("stoploss") = StoplossPtr(),
           py::arg("take_profit") = StoplossPtr(),
           py::arg("profit_goal") = ProfitGoalPtr(),
           py::arg("slippage") = SlippagePtr(),
           py::arg("parameters") = Parameter())
      .def_property_readonly("name", &StrategyDefinition::name)
      .def_property_readonly("money_manager", &StrategyDefinition::moneyManager,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("signal", &StrategyDefinition::signal,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("environment", &StrategyDefinition::environment,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("condition", &StrategyDefinition::condition,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("stoploss", &StrategyDefinition::stoploss,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("take_profit", &StrategyDefinition::takeProfit,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("profit_goal", &StrategyDefinition::profitGoal,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("slippage", &StrategyDefinition::slippage,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("parameters", &StrategyDefinition::parameters,
                             py::return_value_policy::reference_internal);

  py::class_<BacktestRequest>(m, "BacktestRequest",
                              "Immutable strategy run request")
      .def(py::init<const KData&, bool, bool>(), py::arg("kdata"),
           py::arg("reset") = true, py::arg("reset_all") = false)
      .def_property_readonly("kdata", &BacktestRequest::kdata,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("reset", &BacktestRequest::reset)
      .def_property_readonly("reset_all", &BacktestRequest::resetAll);

  py::class_<BacktestResult>(m, "BacktestResult",
                             "Stable strategy result snapshot")
      .def_property_readonly("stock", &BacktestResult::stock,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("query", &BacktestResult::query,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("trades", &BacktestResult::trades,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("trade_count", &BacktestResult::tradeCount)
      .def_property_readonly("empty", &BacktestResult::empty);

  py::class_<StrategyEngine>(m, "StrategyEngine",
                             "Narrow strategy orchestration facade")
      .def(py::init<StrategyDefinition, ExecutionEngine&>(),
           py::arg("definition"), py::arg("execution"), py::keep_alive<1, 3>())
      .def("run", &StrategyEngine::run, py::arg("request"),
           py::call_guard<py::gil_scoped_release>())
      .def("stop", &StrategyEngine::stop)
      .def_property_readonly("running", &StrategyEngine::running);
}

// Registration group: _strategy_main
/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-16
 *     Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_Environment(py::module& m);
void export_Condition(py::module& m);
void export_MoneyManager(py::module& m);
void export_Signal(py::module& m);
void export_Stoploss(py::module& m);
void export_ProfitGoal(py::module& m);
void export_Slippage(py::module& m);
void export_SCFilter(py::module& m);
void export_Normalize(py::module& m);
void export_MultiFactor(py::module& m);
void export_StrategyEngine(py::module& m);

void export_strategy_main(py::module& m) {
  export_Environment(m);
  export_Condition(m);
  export_MoneyManager(m);
  export_Signal(m);
  export_Stoploss(m);
  export_ProfitGoal(m);
  export_Slippage(m);
  export_SCFilter(m);
  export_Normalize(m);
  export_MultiFactor(m);
  export_StrategyEngine(m);
}

void bindStrategy(py::module_& m) {
  export_strategy_main(m);
}
