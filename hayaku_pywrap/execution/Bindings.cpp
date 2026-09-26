#include "Bindings.h"

/* Domain binding registrations. */

// Registration group: _BorrowRecord
/*
 * _BorrowRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <execution/BorrowRecord.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_BorrowRecord(py::module& m) {
  py::class_<BorrowRecord>(m, "BorrowRecord",
                           "Record the currently borrowed stock information")
      .def(py::init<>())
      .def(py::init<const Stock&, double, price_t>())
      .def("__str__", to_py_str<BorrowRecord>)
      .def("__repr__", to_py_str<BorrowRecord>)
      .def_readwrite("stock", &BorrowRecord::stock, "The borrowed security")
      .def_readwrite("number", &BorrowRecord::number,
                     "The total borrowed quantity")
      .def_readwrite("value", &BorrowRecord::value, "The total borrowed value")

          DEF_PICKLE(BorrowRecord);
}

// Registration group: _CostRecord
/*
 * _CostRecord.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <execution/CostRecord.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_CostRecord(py::module& m) {
  py::class_<CostRecord>(m, "CostRecord", R"(The cost record

    The total cost = the commission + the stamp tax + the transfer fee + the other fees

    This structure is mainly used to store the cost record results, generally used directly as a struct,
    and the class itself does not calculate the total cost, nor guarantee that the above formula holds)")

      .def(py::init<>())
      .def(py::init<price_t, price_t, price_t, price_t, price_t>(),
           py::arg("commission"), py::arg("stamptax"), py::arg("transferfee"),
           py::arg("others"), py::arg("total"))

      .def("__str__", to_py_str<CostRecord>)
      .def("__repr__", to_py_str<CostRecord>)

      .def_readwrite("commission", &CostRecord::commission, "The commission")
      .def_readwrite("stamptax", &CostRecord::stamptax, "The stamp tax")
      .def_readwrite("transferfee", &CostRecord::transferfee,
                     "The transfer fee")
      .def_readwrite("others", &CostRecord::others, "The other fees")
      .def_readwrite("total", &CostRecord::total,
                     "The total cost (float), = the commission + the stamp tax "
                     "+ the transfer fee + the other fees")
      .def(py::self == py::self)

          DEF_PICKLE(CostRecord);
}

// Registration group: _ExecutionEngine
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <execution/ExecutionEngine.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_ExecutionEngine(py::module& m) {
  py::class_<AccountId>(m, "AccountId", "Stable execution-account identity")
      .def(py::init<std::uint64_t>(), py::arg("value") = 0)
      .def_property_readonly("value", &AccountId::value)
      .def_property_readonly("valid", &AccountId::valid)
      .def("__bool__", &AccountId::valid)
      .def(py::self == py::self);

  py::class_<AccountConfig>(m, "AccountConfig",
                            "Immutable execution-account configuration")
      .def(py::init<Datetime, price_t, TradeCostPtr, string, int, bool, bool,
                    AccountId, std::vector<OrderBrokerPtr>>(),
           py::arg("init_datetime") = Datetime(199001010000LL),
           py::arg("initial_cash") = 100000.0,
           py::arg("cost_policy") = TC_Zero(), py::arg("name") = "SYS",
           py::arg("precision") = 2, py::arg("support_borrow_cash") = false,
           py::arg("support_borrow_stock") = false,
           py::arg("account_id") = AccountId(),
           py::arg("brokers") = std::vector<OrderBrokerPtr>())
      .def_property_readonly("init_datetime", &AccountConfig::initDatetime)
      .def_property_readonly("initial_cash", &AccountConfig::initialCash)
      .def_property_readonly("cost_policy", &AccountConfig::costPolicy)
      .def_property_readonly("name", &AccountConfig::name)
      .def_property_readonly("precision", &AccountConfig::precision)
      .def_property_readonly("support_borrow_cash",
                             &AccountConfig::supportBorrowCash)
      .def_property_readonly("support_borrow_stock",
                             &AccountConfig::supportBorrowStock)
      .def_property_readonly("account_id", &AccountConfig::accountId)
      .def_property_readonly("brokers", &AccountConfig::brokers,
                             py::return_value_policy::reference_internal);

  py::enum_<OrderSide>(m, "OrderSide")
      .value("BUY", OrderSide::BUY)
      .value("SELL", OrderSide::SELL)
      .value("SELL_SHORT", OrderSide::SELL_SHORT)
      .value("BUY_SHORT", OrderSide::BUY_SHORT);

  py::class_<OrderRequest>(m, "OrderRequest",
                           "Immutable synchronous order request")
      .def(py::init<OrderSide, Datetime, Stock, price_t, double, price_t,
                    price_t, price_t, OrderOrigin, string>(),
           py::arg("side"), py::arg("datetime"), py::arg("stock"),
           py::arg("real_price"), py::arg("number"), py::arg("stoploss") = 0.0,
           py::arg("goal_price") = 0.0, py::arg("plan_price") = 0.0,
           py::arg("origin") = OrderOrigin::UNSPECIFIED, py::arg("remark") = "")
      .def_property_readonly("side", &OrderRequest::side)
      .def_property_readonly("datetime", &OrderRequest::datetime,
                             py::return_value_policy::copy)
      .def_property_readonly("stock", &OrderRequest::stock,
                             py::return_value_policy::copy)
      .def_property_readonly("real_price", &OrderRequest::realPrice)
      .def_property_readonly("number", &OrderRequest::number)
      .def_property_readonly("stoploss", &OrderRequest::stoploss)
      .def_property_readonly("goal_price", &OrderRequest::goalPrice)
      .def_property_readonly("plan_price", &OrderRequest::planPrice)
      .def_property_readonly("origin", &OrderRequest::origin)
      .def_property_readonly("remark", &OrderRequest::remark,
                             py::return_value_policy::copy);

  py::enum_<ExecutionStatus>(m, "ExecutionStatus")
      .value("FILLED", ExecutionStatus::FILLED)
      .value("REJECTED", ExecutionStatus::REJECTED);

  py::class_<ExecutionReport>(m, "ExecutionReport", "Synchronous order result")
      .def_property_readonly("status", &ExecutionReport::status)
      .def_property_readonly("filled", &ExecutionReport::filled)
      .def_property_readonly("rejected", &ExecutionReport::rejected)
      .def_property_readonly("trade", &ExecutionReport::trade,
                             py::return_value_policy::reference_internal);

  py::class_<AccountSnapshot>(m, "AccountSnapshot",
                              "Read-only account value snapshot")
      .def_property_readonly("funds", &AccountSnapshot::funds,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("positions", &AccountSnapshot::positions,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("short_positions",
                             &AccountSnapshot::shortPositions,
                             py::return_value_policy::reference_internal);

  py::class_<AccountView>(m, "AccountView",
                          "Value-owned read-only account view")
      .def_property_readonly("account_id", &AccountView::id)
      .def_property_readonly("init_datetime", &AccountView::initDatetime)
      .def_property_readonly("last_datetime", &AccountView::lastDatetime)
      .def_property_readonly("funds", &AccountView::funds,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("positions", &AccountView::positions,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("short_positions", &AccountView::shortPositions,
                             py::return_value_policy::reference_internal);

  py::class_<ExecutionEngine>(m, "ExecutionEngine",
                              "Order execution and account facade")
      .def(py::init<const AccountConfig&>(), py::arg("account_config"))
      .def("submit", &ExecutionEngine::submit, py::arg("request"))
      .def("snapshot", &ExecutionEngine::snapshot)
      .def("view", &ExecutionEngine::view)
      .def("history", &ExecutionEngine::history)
      .def_property_readonly("account_id", &ExecutionEngine::accountId);
}

// Registration group: _FundsRecord
/*
 * _FundsRecord.cpp
 *
 *  Created on: 2013-5-2
 *      Author: fasiondog
 */

#include <execution/FundsRecord.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_FundsRecord(py::module& m) {
  py::class_<FundsRecord>(m, "FundsRecord",
                          "The current asset situation record")
      .def(py::init<>())
      .def("__str__", to_py_str<FundsRecord>)
      .def("__repr__", to_py_str<FundsRecord>)

      .def_readwrite("cash", &FundsRecord::cash, "The current cash (float)")
      .def_readwrite("market_value", &FundsRecord::market_value,
                     "The current long market value (float)")
      .def_readwrite("short_market_value", &FundsRecord::short_market_value,
                     "The current short position market value (float)")
      .def_readwrite("base_cash", &FundsRecord::base_cash,
                     "The current invested principal (float)")
      .def_readwrite("base_asset", &FundsRecord::base_asset,
                     "The current invested asset value (float)")
      .def_readwrite("borrow_cash", &FundsRecord::borrow_cash,
                     "The currently borrowed funds (float), i.e. the debt")
      .def_readwrite("borrow_asset", &FundsRecord::borrow_asset,
                     "The currently borrowed securities asset value (float)")

      .def_property_readonly("total_assets", &FundsRecord::total_assets,
                             "The total assets")
      .def_property_readonly("net_assets", &FundsRecord::net_assets,
                             "The net assets")
      .def_property_readonly("total_borrow", &FundsRecord::total_borrow,
                             "The total debt")
      .def_property_readonly("total_base", &FundsRecord::total_base,
                             "The invested principal assets")
      .def_property_readonly("profit", &FundsRecord::profit,
                             "The current profit")

      .def(py::self + py::self)
      .def(py::self += py::self)

          DEF_PICKLE(FundsRecord);
}

// Registration group: _LoanRecord
/*
 * _LoanRecord.cpp
 *
 *  Created on: 2013-5-24
 *      Author: fasiondog
 */

#include <execution/LoanRecord.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_LoanRecord(py::module& m) {
  py::class_<LoanRecord>(m, "LoanRecord",
                         "The loan record (the financing record)")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t>())
      .def("__str__", to_py_str<LoanRecord>)
      .def("__repr__", to_py_str<LoanRecord>)
      .def_readwrite("datetime", &LoanRecord::datetime, "The loan time")
      .def_readwrite("value", &LoanRecord::value, "The loan amount")

          DEF_PICKLE(LoanRecord);
}

// Registration group: _OrderBroker
/*
 * _OrderBroker.cpp
 *
 *  Created on: 2017-06-28
 *      Author: fasiondog
 */

#include <execution/broker/OrderBrokerBase.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyOrderBrokerBase : public OrderBrokerBase {
 public:
  using OrderBrokerBase::OrderBrokerBase;

  void _buy(Datetime datetime, const string& market, const string& code,
            price_t price, double num, price_t stoploss, price_t goalPrice,
            OrderOrigin from, const string& remark) override {
    PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _buy, datetime, market, code,
                           price, num, stoploss, goalPrice, from, remark);
  }

  void _sell(Datetime datetime, const string& market, const string& code,
             price_t price, double num, price_t stoploss, price_t goalPrice,
             OrderOrigin from, const string& remark) override {
    PYBIND11_OVERLOAD_PURE(void, OrderBrokerBase, _sell, datetime, market, code,
                           price, num, stoploss, goalPrice, from, remark);
  }

  string _getAssetInfo() override {
    PYBIND11_OVERLOAD_NAME(string, OrderBrokerBase, "_get_asset_info",
                           _getAssetInfo);
  }
};

void export_OrderBroker(py::module& m) {
  py::class_<BrokerPositionRecord>(m, "BrokerPositionRecord")
      .def(py::init<>())
      .def(py::init<const Stock&, price_t, price_t>())
      .def("__str__", &BrokerPositionRecord::str)
      .def("__repr__", &BrokerPositionRecord::str)
      .def_readwrite("stock", &BrokerPositionRecord::stock,
                     "The position object")
      .def_readwrite("number", &BrokerPositionRecord::number,
                     "The position quantity")
      .def_readwrite("money", &BrokerPositionRecord::money,
                     "The total funds spent on buying");

  py::class_<OrderBrokerBase, OrderBrokerPtr, PyOrderBrokerBase>(
      m, "OrderBrokerBase",
      R"(The order broker wrapper base class; users can refer to it to customize their own order brokers, adding the extra processing

    :param bool real: whether to re-fetch the real-time tick data before placing the order
    :param float slip: if the absolute difference between the current ask price and the instructed buy price does not exceed slip, place the order, otherwise ignore; it is invalid for the sell operation, selling immediately at the current price)")

      .def(py::init<>())
      .def(py::init<const string&>(), R"(
    :param str name: the broker name)")

      .def("__str__", to_py_str<OrderBrokerBase>)
      .def("__repr__", to_py_str<OrderBrokerBase>)

      .def_property(
          "name", py::overload_cast<>(&OrderBrokerBase::name, py::const_),
          py::overload_cast<const string&>(&OrderBrokerBase::name),
          py::return_value_policy::copy, "The name (readable and writable)")

      .def("buy", &OrderBrokerBase::buy,
           "For the details, see the subclass implementation interface: _buy")
      .def("sell", &OrderBrokerBase::sell,
           "For the details, see the subclass implementation interface: _sell")
      .def("get_asset_info", &OrderBrokerBase::getAssetInfo,
           "For the details, see the subclass implementation interface: "
           "_get_asset_info")

      .def(
          "_buy", &OrderBrokerBase::_buy,
          R"(_buy(self, datetime, market, code, price, num, stoploss, goal_price, origin, remark)

    [Subclass interface] Execute the buy operation

    :param Datetime datetime: the strategy instruction time
    :param str market: the market identifier
    :param str code: the security code
    :param float price: the buy price
    :param float num: the buy quantity
    :param float stoploss: the planned stop-loss price
    :param float goal_price: the planned profit target price
    :param OrderOrigin origin: the order origin,
    :param str remark: the order remark)")

      .def(
          "_sell", &OrderBrokerBase::_sell,
          R"(_sell(self, datetime, market, code, price, num, stoploss, goal_price, origin, remark)

    [Subclass interface] Execute the sell operation

    :param Datetime datetime: the strategy instruction time
    :param str market: the market identifier
    :param str code: the security code
    :param float price: the sell price
    :param float num: the sell quantity
    :param float stoploss: the planned stop-loss price
    :param float goal_price: the planned profit target price
    :param OrderOrigin origin: the order origin
    :param str remark: the order remark)")

      .def("_get_asset_info", &OrderBrokerBase::_getAssetInfo,
           R"(_get_asset_info(self)

    [Subclass interface] Get the current asset information; the subclass needs to return a json string conforming to the following specification:

    {
        "datetime": "2001-01-01 18:00:00.12345",
        "cash": 0.0,
        "positions": [
            {"market": "SZ", "code": "000001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
             "cost_price": 0.0},
            {"market": "SH", "code": "600001", "number": 100.0, "stoploss": 0.0, "goal_price": 0.0,
             "cost_price": 0.0},
         ]
    }

    :return: return the current asset information as a string (in json format)
    :rtype: str)");
}

// Registration group: _OrderOrigin
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <execution/OrderOrigin.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_OrderOrigin(py::module& m) {
  py::enum_<OrderOrigin>(m, "OrderOrigin")
      .value("ENVIRONMENT", OrderOrigin::ENVIRONMENT)
      .value("CONDITION", OrderOrigin::CONDITION)
      .value("SIGNAL", OrderOrigin::SIGNAL)
      .value("STOP_LOSS", OrderOrigin::STOP_LOSS)
      .value("TAKE_PROFIT", OrderOrigin::TAKE_PROFIT)
      .value("MONEY_MANAGEMENT", OrderOrigin::MONEY_MANAGEMENT)
      .value("PROFIT_GOAL", OrderOrigin::PROFIT_GOAL)
      .value("SLIPPAGE", OrderOrigin::SLIPPAGE)
      .value("ALLOCATION", OrderOrigin::ALLOCATION)
      .value("PORTFOLIO", OrderOrigin::PORTFOLIO)
      .value("UNSPECIFIED", OrderOrigin::UNSPECIFIED);

  m.def("get_order_origin_name", &getOrderOriginName, py::arg("origin"));
  m.def("get_order_origin_enum", &getOrderOriginEnum, py::arg("name"));
}

// Registration group: _PositionRecord
/*
 * _PositionRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <execution/PositionExtInfo.h>
#include <execution/PositionRecord.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_PositionRecord(py::module& m) {
  py::class_<PositionRecord>(m, "PositionRecord", "The position record")
      .def(py::init<>())
      .def(py::init<const Stock&, const Datetime&, const Datetime&, double,
                    price_t, price_t, double, price_t, price_t, price_t,
                    price_t>())

      .def("__str__", &PositionRecord::str)
      .def("__repr__", &PositionRecord::str)

      .def_readwrite("stock", &PositionRecord::stock,
                     "The trading object (Stock)")
      .def_readwrite("take_datetime", &PositionRecord::takeDatetime,
                     "The initial position building moment (Datetime)")
      .def_readwrite("clean_datetime", &PositionRecord::cleanDatetime,
                     "The closing date; in the current position records it is "
                     "constant.null_datetime")
      .def_readwrite("number", &PositionRecord::number,
                     "The current position quantity (float)")
      .def_readwrite("stoploss", &PositionRecord::stoploss,
                     "The current stop-loss price (float)")
      .def_readwrite("goal_price", &PositionRecord::goalPrice,
                     "The current target price (float)")
      .def_readwrite("total_number", &PositionRecord::totalNumber,
                     "The cumulative position quantity (float)")
      .def_readwrite("buy_money", &PositionRecord::buyMoney,
                     "The cumulative buy funds (float)")
      .def_readwrite("total_cost", &PositionRecord::totalCost,
                     "The cumulative total trading cost (float)")
      .def_readwrite(
          "total_risk", &PositionRecord::totalRisk,
          "The cumulative trading risk = each (the buy price - the stop-loss) "
          "* the buy quantity, excluding the trading costs")
      .def_readwrite("sell_money", &PositionRecord::sellMoney,
                     "The cumulative sell funds (float)")
      .def_readwrite("buy_count", &PositionRecord::buyCount,
                     "The cumulative buy count (size_t)")
      .def_readwrite("sell_count", &PositionRecord::sellCount,
                     "The cumulative sell count (size_t)")
      .def_property_readonly("total_profit", &PositionRecord::totalProfit,
                             R"(total_profit(self):

    The cumulative profit = the cumulative sell funds - the cumulative buy funds - the cumulative trading cost
    Note: it is only valid for the closed records; the open records return 0  )")

          DEF_PICKLE(PositionRecord);

  py::class_<PositionExtInfo>(m, "PositionExtInfo",
                              "The extended position information")
      .def(py::init<>())
      .def_readwrite("position", &PositionExtInfo::position, "PositionRecord")
      .def_readwrite("current_close_price", &PositionExtInfo::currentClosePrice,
                     "The current close price (float)")
      .def_readwrite("max_high_price", &PositionExtInfo::maxHighPrice,
                     "The maximum of the highest prices in the period")
      .def_readwrite("min_low_price", &PositionExtInfo::minLowPrice,
                     "The minimum of the lowest prices in the period")
      .def_readwrite("max_close_price", &PositionExtInfo::maxClosePrice,
                     "The highest close price in the period")
      .def_readwrite("min_close_price", &PositionExtInfo::minClosePrice,
                     "The lowest close price in the period")
      .def_readwrite("current_close_price", &PositionExtInfo::currentClosePrice,
                     "The current close price")
      .def_readwrite(
          "max_pull_back1", &PositionExtInfo::maxPullBack1,
          "The maximum drawdown ratio 1 (calculated only with the maximum "
          "close price and the lowest close price) (a negative number)")
      .def_readwrite("max_pull_back2", &PositionExtInfo::maxPullBack2,
                     "The maximum drawdown ratio 2 (calculated with the "
                     "maximum of the highest prices and the minimum of the "
                     "lowest prices in the period) (a negative number)")
      .def_readwrite("current_profit", &PositionExtInfo::currentProfit,
                     "The current floating profit and loss (excluding the "
                     "estimated sell cost)")

      .def("current_pull_back1", &PositionExtInfo::currentPullBack1,
           "The current drawdown percentage 1 (calculated only with the "
           "maximum close price and the current close price)")
      .def("current_pull_back2", &PositionExtInfo::currentPullBack2,
           "The current drawdown percentage 2 (calculated with the maximum of "
           "the highest prices in the period and the current close price)")
      .def("max_floating_profit1", &PositionExtInfo::maxFloatingProfit1,
           "The maximum floating profit 1 in the period (a positive number, "
           "calculated only with the close price, excluding the estimated sell "
           "cost; the statistics are inaccurate when buying and selling "
           "multiple times)")
      .def("max_floating_profit2", &PositionExtInfo::maxFloatingProfit2,
           "The maximum floating profit 2 in the period (a positive number, "
           "calculated with the maximum of the highest prices, excluding the "
           "estimated sell cost; the statistics are inaccurate when buying and "
           "selling multiple times)")
      .def("min_loss_profit1", &PositionExtInfo::minLossProfit1,
           "The maximum floating loss 1 in the period (a negative number, "
           "calculated only with the close price, excluding the estimated sell "
           "cost; the statistics are inaccurate when buying and selling "
           "multiple times)")
      .def("min_loss_profit2", &PositionExtInfo::minLossProfit2,
           "The maximum floating loss 2 in the period (a negative number, "
           "calculated only with the lowest price in the period, excluding the "
           "estimated sell cost; the statistics are inaccurate when buying and "
           "selling multiple times)")

          DEF_PICKLE(PositionExtInfo);

  m.def(
      "positions_to_np",
      [](const PositionRecordList& positions) {
        size_t total = positions.size();
        HAYAKU_IF_RETURN(total == 0, py::array());

        struct alignas(8) RawData {
          int32_t code[10];
          int32_t name[20];
          int64_t take_datetime;        // The buy date
          int64_t hold_days;            // The held days
          double number;                // The current position quantity
          double invest;                // The current invested amount
          double current_market_value;  // The current market value
          double profit;                // The current profit/loss amount
          double profit_ratio;          // The current profit/loss ratio
          double stoploss;              // The current stop-loss price
          double goal_price;            // The current target price
          int64_t clean_datetime;       // The sell date
          double total_number;          // The cumulative position quantity
          double total_cost;            // The cumulative trading cost
          double total_risk;  // The cumulative trading risk = each (the buy
                              // price - the stop-loss) * the buy quantity,
                              // excluding the trading costs
          double buy_money;   // The cumulative invested amount
          double sell_money;  // The cumulative sell funds
        };

        RawData* data =
            static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, total = positions.size(); i < total; i++) {
          const PositionRecord& p = positions[i];
          utf8_to_utf32(p.stock.market_code(), data[i].code, 10);
          utf8_to_utf32(p.stock.name(), data[i].name, 20);
          data[i].take_datetime = p.takeDatetime.timestamp() * 1000LL;
          data[i].number = p.number;
          data[i].invest = p.buyMoney - p.sellMoney + p.totalCost;
          if (p.cleanDatetime.isNull()) {
            data[i].hold_days = (Datetime::now() - p.takeDatetime).days();
            double cur_price =
                p.stock.getMarketValue(Datetime::now(), KQuery::DAY);
            data[i].current_market_value = cur_price * p.number;
            data[i].profit = data[i].current_market_value - data[i].invest;
          } else {
            data[i].hold_days = (p.cleanDatetime - p.takeDatetime).days();
            data[i].current_market_value = 0.0;
            data[i].profit = p.totalProfit();
          }
          data[i].profit_ratio = roundEx(
              100. *
                  (data[i].invest != 0.0 ? data[i].profit / data[i].invest : 0),
              2);
          data[i].stoploss = p.stoploss;
          data[i].goal_price = p.goalPrice;
          data[i].clean_datetime = p.cleanDatetime.isNull()
                                       ? std::numeric_limits<int64_t>::min()
                                       : p.cleanDatetime.timestamp() * 1000LL;
          data[i].total_number = p.totalNumber;
          data[i].total_cost = p.totalCost;
          data[i].total_risk = p.totalRisk;
          data[i].buy_money = p.buyMoney;
          data[i].sell_money = p.sellMoney;
        }

        py::dtype dtype = py::dtype(
            vector_to_python_list<string>(
                {htr("market_code"), htr("stock_name"), htr("take_time"),
                 htr("hold_days"), htr("hold_number"), htr("invest"),
                 htr("market_value"), htr("profit"), htr("profit_percent"),
                 htr("stoploss"), htr("goal_price"), htr("clean_time"),
                 htr("total_number"), htr("total_cost"), htr("total_risk"),
                 htr("buy_money"), htr("sell_money")}),
            vector_to_python_list<string>(
                {"U10", "U20", "datetime64[ns]", "i8", "d", "d", "d", "d", "d",
                 "d", "d", "datetime64[ns]", "d", "d", "d", "d", "d"}),
            vector_to_python_list<int64_t>({0, 40, 120, 128, 136, 144, 152, 160,
                                            168, 176, 184, 192, 200, 208, 216,
                                            224, 232}),
            240);

        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
      },
      R"(Convert the position list to Numpy

    Note: the calculated values such as the current market value, the profit and the profit/loss are all calculated by the daily line; when backtesting with a level below the daily line, you need to recalculate the open position records yourself!)");

  m.def(
      "positions_to_df",
      [](const PositionRecordList& positions) {
        size_t total = positions.size();
        if (total == 0) {
          return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the data containers of each column
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<int64_t> take_time_arr(total);
        py::array_t<int64_t> hold_days_arr(total);
        py::array_t<double> hold_number_arr(total);
        py::array_t<double> invest_arr(total);
        py::array_t<double> market_value_arr(total);
        py::array_t<double> profit_arr(total);
        py::array_t<double> profit_percent_arr(total);
        py::array_t<double> stoploss_arr(total);
        py::array_t<double> goal_price_arr(total);
        py::array_t<int64_t> clean_time_arr(total);
        py::array_t<double> total_number_arr(total);
        py::array_t<double> total_cost_arr(total);
        py::array_t<double> total_risk_arr(total);
        py::array_t<double> buy_money_arr(total);
        py::array_t<double> sell_money_arr(total);

        // Get the buffers of each array
        auto take_time_buf = take_time_arr.request();
        auto hold_days_buf = hold_days_arr.request();
        auto hold_number_buf = hold_number_arr.request();
        auto invest_buf = invest_arr.request();
        auto market_value_buf = market_value_arr.request();
        auto profit_buf = profit_arr.request();
        auto profit_percent_buf = profit_percent_arr.request();
        auto stoploss_buf = stoploss_arr.request();
        auto goal_price_buf = goal_price_arr.request();
        auto clean_time_buf = clean_time_arr.request();
        auto total_number_buf = total_number_arr.request();
        auto total_cost_buf = total_cost_arr.request();
        auto total_risk_buf = total_risk_arr.request();
        auto buy_money_buf = buy_money_arr.request();
        auto sell_money_buf = sell_money_arr.request();

        int64_t* take_time_ptr = static_cast<int64_t*>(take_time_buf.ptr);
        int64_t* hold_days_ptr = static_cast<int64_t*>(hold_days_buf.ptr);
        double* hold_number_ptr = static_cast<double*>(hold_number_buf.ptr);
        double* invest_ptr = static_cast<double*>(invest_buf.ptr);
        double* market_value_ptr = static_cast<double*>(market_value_buf.ptr);
        double* profit_ptr = static_cast<double*>(profit_buf.ptr);
        double* profit_percent_ptr =
            static_cast<double*>(profit_percent_buf.ptr);
        double* stoploss_ptr = static_cast<double*>(stoploss_buf.ptr);
        double* goal_price_ptr = static_cast<double*>(goal_price_buf.ptr);
        int64_t* clean_time_ptr = static_cast<int64_t*>(clean_time_buf.ptr);
        double* total_number_ptr = static_cast<double*>(total_number_buf.ptr);
        double* total_cost_ptr = static_cast<double*>(total_cost_buf.ptr);
        double* total_risk_ptr = static_cast<double*>(total_risk_buf.ptr);
        double* buy_money_ptr = static_cast<double*>(buy_money_buf.ptr);
        double* sell_money_ptr = static_cast<double*>(sell_money_buf.ptr);

        // Fill the data
        for (size_t i = 0; i < total; i++) {
          const PositionRecord& p = positions[i];
          if (!p.stock.isNull()) {
            code_list[i] = py::str(p.stock.market_code());
            name_list[i] = py::str(p.stock.name());
          } else {
            code_list[i] = py::str("");
            name_list[i] = py::str("");
          }

          take_time_ptr[i] = p.takeDatetime.timestamp() * 1000LL;

          int64_t hold_days;
          double current_market_value;
          double profit;

          if (p.cleanDatetime.isNull()) {
            hold_days = (Datetime::now() - p.takeDatetime).days();
            double cur_price =
                p.stock.getMarketValue(Datetime::now(), KQuery::DAY);
            current_market_value = cur_price * p.number;
            profit =
                current_market_value - (p.buyMoney - p.sellMoney + p.totalCost);
          } else {
            hold_days = (p.cleanDatetime - p.takeDatetime).days();
            current_market_value = 0.0;
            profit = p.totalProfit();
          }

          hold_days_ptr[i] = hold_days;
          hold_number_ptr[i] = p.number;

          double invest = p.buyMoney - p.sellMoney + p.totalCost;
          invest_ptr[i] = invest;
          market_value_ptr[i] = current_market_value;
          profit_ptr[i] = profit;

          double profit_ratio =
              invest != 0.0 ? roundEx(100. * (profit / invest), 2) : 0.0;
          profit_percent_ptr[i] = profit_ratio;

          stoploss_ptr[i] = p.stoploss;
          goal_price_ptr[i] = p.goalPrice;
          clean_time_ptr[i] = p.cleanDatetime.isNull()
                                  ? std::numeric_limits<int64_t>::min()
                                  : p.cleanDatetime.timestamp() * 1000LL;
          total_number_ptr[i] = p.totalNumber;
          total_cost_ptr[i] = p.totalCost;
          total_risk_ptr[i] = p.totalRisk;
          buy_money_ptr[i] = p.buyMoney;
          sell_money_ptr[i] = p.sellMoney;
        }

        // Build the DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("market_code").c_str()] =
            pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("stock_name").c_str()] =
            pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns[htr("take_time").c_str()] =
            take_time_arr.attr("astype")("datetime64[ns]");
        columns[htr("hold_days").c_str()] = hold_days_arr;
        columns[htr("hold_number").c_str()] = hold_number_arr;
        columns[htr("invest").c_str()] = invest_arr;
        columns[htr("market_value").c_str()] = market_value_arr;
        columns[htr("profit").c_str()] = profit_arr;
        columns[htr("profit_percent").c_str()] = profit_percent_arr;
        columns[htr("stoploss").c_str()] = stoploss_arr;
        columns[htr("goal_price").c_str()] = goal_price_arr;
        columns[htr("clean_time").c_str()] =
            clean_time_arr.attr("astype")("datetime64[ns]");
        columns[htr("total_number").c_str()] = total_number_arr;
        columns[htr("total_cost").c_str()] = total_cost_arr;
        columns[htr("total_risk").c_str()] = total_risk_arr;
        columns[htr("buy_money").c_str()] = buy_money_arr;
        columns[htr("sell_money").c_str()] = sell_money_arr;

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      R"(positions_to_df(positions)

    Convert the position record list to a pandas DataFrame

    Note: the calculated values such as the current market value, the profit and the profit/loss are all calculated by the daily line; when backtesting with a level below the daily line, you need to recalculate the open position records yourself!

    :param PositionRecordList positions: the position record list
    :return: a pandas DataFrame containing the position records
    :rtype: pandas.DataFrame)");
}

// Registration group: _TradeCost
/*
 * _TradeCost.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <execution/pricing/TradeCostBase.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyTradeCostBase : public TradeCostBase {
  PY_CLONE(PyTradeCostBase, TradeCostBase)

 public:
  PyTradeCostBase() : TradeCostBase("PyTradeCostBase") {
    is_python_object_ = true;
  }

  PyTradeCostBase(const string& name) : TradeCostBase(name) {
    is_python_object_ = true;
  }

  CostRecord getBuyCost(const Datetime& datetime, const Stock& stock,
                        price_t price, double num) const override {
    PYBIND11_OVERLOAD_PURE(CostRecord, TradeCostBase, getBuyCost, datetime,
                           stock, price, num);
  }

  CostRecord getSellCost(const Datetime& datetime, const Stock& stock,
                         price_t price, double num) const override {
    PYBIND11_OVERLOAD_PURE(CostRecord, TradeCostBase, getSellCost, datetime,
                           stock, price, num);
  }

  CostRecord getBorrowCashCost(const Datetime& datetime,
                               price_t cash) const override {
    PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getBorrowCashCost, datetime,
                      cash);
  }

  CostRecord getReturnCashCost(const Datetime& borrow_datetime,
                               const Datetime& return_datetime,
                               price_t cash) const override {
    PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getReturnCashCost,
                      borrow_datetime, return_datetime, cash);
  }

  CostRecord getBorrowStockCost(const Datetime& datetime, const Stock& stock,
                                price_t price, double num) const override {
    PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getBorrowStockCost, datetime,
                      stock, price, num);
  }

  CostRecord getReturnStockCost(const Datetime& borrow_datetime,
                                const Datetime& return_datetime,
                                const Stock& stock, price_t price,
                                double num) const override {
    PYBIND11_OVERLOAD(CostRecord, TradeCostBase, getReturnStockCost,
                      borrow_datetime, return_datetime, stock, price, num);
  }
};

void export_TradeCost(py::module& m) {
  py::class_<TradeCostBase, TradeCostPtr, PyTradeCostBase>(
      m, "TradeCostBase",
      R"(The trade cost algorithm base class

    The custom trade cost algorithm interfaces:

    :py:meth:`TradeCostBase.getBuyCost` - [Required] Get the buy cost
    :py:meth:`TradeCostBase.getSellCost` - [Required] Get the sell cost
    :py:meth:`TradeCostBase._clone` - [Required] The subclass clone interface)")

      .def(py::init<const string&>())

      .def("__str__", to_py_str<TradeCostBase>)
      .def("__repr__", to_py_str<TradeCostBase>)

      .def_property_readonly("name", &TradeCostBase::name,
                             py::return_value_policy::copy,
                             "The cost algorithm name")

      .def("get_param", &TradeCostBase::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def(
          "set_param",
          static_cast<void (TradeCostBase::*)(
              const std::string&, const boost::any&)>(&TradeCostBase::setParam),
          R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("clone", &TradeCostBase::clone, "The clone operation")

      .def("get_buy_cost", &TradeCostBase::getBuyCost, py::arg("date"),
           py::arg("stock"), py::arg("price"), py::arg("num"),
           R"(get_buy_cost(self, date, stock, price, num)

        [Overload interface] Get the buy cost

        :param Datetime date: the buy moment
        :param Stock stock: the buy object
        :param float price: the buy price
        :param int num: the buy quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      .def("get_sell_cost", &TradeCostBase::getSellCost, py::arg("date"),
           py::arg("stock"), py::arg("price"), py::arg("num"),
           R"(get_sell_cost(self, date, stock, price, num)

        [Overload interface] Get the sell cost

        :param Datetime date: the sell moment
        :param Stock stock: the sell object
        :param float price: the sell price
        :param int num: the sell quantity
        :return: the trade cost record
        :rtype: CostRecord)")

      //.def("getBorrowCashCost", &TradeCostBase::getBorrowCashCost,
      //     &TradeCostWrap::default_getBorrowCashCost)

      //.def("getReturnCashCost", &TradeCostBase::getReturnCashCost,
      //     &TradeCostWrap::default_getReturnCashCost)

      //.def("getBorrowStockCost", &TradeCostBase::getBorrowStockCost,
      //&TradeCostWrap::default_getBorrowStockCost) .def("getReturnStockCost",
      //&TradeCostBase::getReturnStockCost,
      //&TradeCostWrap::default_getReturnStockCost)

      DEF_PICKLE(TradeCostPtr);
}

// Registration group: _TradeRecord
/*
 * _TradeRecord.cpp
 *
 *  Created on: 2013-2-25
 *      Author: fasiondog
 */

#include <execution/TradeRecord.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TradeRecord(py::module& m) {
  py::enum_<BUSINESS>(m, "BUSINESS")
      .value("INIT", BUSINESS_INIT)
      .value("BUY", BUSINESS_BUY)
      .value("SELL", BUSINESS_SELL)
      .value("BUY_SHORT", BUSINESS_BUY_SHORT)
      .value("SELL_SHORT", BUSINESS_SELL_SHORT)
      .value("GIFT", BUSINESS_GIFT)
      .value("BONUS", BUSINESS_BONUS)
      .value("CHECKIN", BUSINESS_CHECKIN)
      .value("CHECKOUT", BUSINESS_CHECKOUT)
      .value("CHECKIN_STOCK", BUSINESS_CHECKIN_STOCK)
      .value("CHECKOUT_STOCK", BUSINESS_CHECKOUT_STOCK)
      .value("BORROW_CASH", BUSINESS_BORROW_CASH)
      .value("RETURN_CASH", BUSINESS_RETURN_CASH)
      .value("BORROW_STOCK", BUSINESS_BORROW_STOCK)
      .value("RETURN_STOCK", BUSINESS_RETURN_STOCK)
      .value("SUOGU", BUSINESS_SUOGU)
      .value("INVALID", BUSINESS_INVALID);

  m.def("get_business_name", getBusinessName, R"(get_business_name(business)

    :param BUSINESS business: the trade business type
    :return: the trade business type name ("INIT"|"BUY"|"SELL"|"GIFT"|"BONUS"|"CHECKIN"|"CHECKOUT"|"UNKNOWN"
    :rtype: string)");

  py::class_<TradeRecord>(m, "TradeRecord", "The trade record")
      .def(py::init<>())
      .def(py::init<const Stock&, const Datetime&, BUSINESS, price_t, price_t,
                    price_t, double, const CostRecord&, price_t, price_t,
                    OrderOrigin>())

      .def("__str__", &TradeRecord::toString)
      .def("__repr__", &TradeRecord::toString)

      .def("is_null", &TradeRecord::isNull)

      .def_readwrite("stock", &TradeRecord::stock, "The stock (Stock)")
      .def_readwrite("datetime", &TradeRecord::datetime,
                     " The trading time (Datetime)")
      .def_readwrite("business", &TradeRecord::business,
                     "The trade type (BUSINESS)")
      .def_readwrite("plan_price", &TradeRecord::planPrice,
                     "The planned trading price (float)")
      .def_readwrite("real_price", &TradeRecord::realPrice,
                     "The actual trading price (float)")
      .def_readwrite("goal_price", &TradeRecord::goalPrice,
                     "The target price (float); if it is 0, it means the goal "
                     "is not limited")
      .def_readwrite("number", &TradeRecord::number,
                     "The traded quantity (float)")
      .def_readwrite("cost", &TradeRecord::cost, "The trading cost")
      .def_readwrite("stoploss", &TradeRecord::stoploss,
                     "The stop-loss price (float)")
      .def_readwrite("cash", &TradeRecord::cash, "The cash balance (float)")
      .def_readwrite("origin", &TradeRecord::from,
                     "The execution-domain order origin")
      .def_readwrite("remark", &TradeRecord::remark, "The remark")

          DEF_PICKLE(TradeRecord);

  m.def("trades_to_np", [](const TradeRecordList& trades) {
    size_t total = trades.size();
    HAYAKU_IF_RETURN(total == 0, py::array());

    struct alignas(8) RawData {
      int32_t code[10];
      int32_t name[20];
      int64_t datetime;      // The trading date
      int32_t business[20];  // The business type
      double planPrice;      // The planned trading price
      double realPrice;      // The actual trading price
      double goalPrice;   // The target price; if it is 0 or Null, it means the
                          // goal is not limited
      double number;      // The traded quantity
      double stoploss;    // The stop-loss price
      double cash;        // The cash balance
      double cost_total;  // The total cost
      double cost_commission;   // The commission
      double cost_stamptax;     // The stamp tax
      double cost_transferfee;  // The transfer fee
      double cost_others;       // The other fees
      int32_t origin[20];       // The execution order origin
      int32_t remark[100];      // The remark
    };

    RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
    for (size_t i = 0, total = trades.size(); i < total; i++) {
      const TradeRecord& t = trades[i];
      utf8_to_utf32(t.stock.market_code(), data[i].code, 10);
      utf8_to_utf32(t.stock.name(), data[i].name, 20);
      data[i].datetime = t.datetime.timestamp() * 1000LL;
      utf8_to_utf32(getBusinessName(t.business), data[i].business, 20);
      data[i].planPrice = t.planPrice;
      data[i].realPrice = t.realPrice;
      data[i].goalPrice = t.goalPrice;
      data[i].number = t.number;
      data[i].stoploss = t.stoploss;
      data[i].cash = t.cash;
      data[i].cost_total = t.cost.total;
      data[i].cost_commission = t.cost.commission;
      data[i].cost_stamptax = t.cost.stamptax;
      data[i].cost_transferfee = t.cost.transferfee;
      data[i].cost_others = t.cost.others;
      utf8_to_utf32(getOrderOriginName(t.from), data[i].origin, 20);
      utf8_to_utf32(t.remark, data[i].remark, 100);
    }

    py::dtype dtype =
        py::dtype(vector_to_python_list<string>(
                      {htr("market_code"), htr("stock_name"), htr("datetime"),
                       htr("business"), htr("planPrice"), htr("realPrice"),
                       htr("goalPrice"), htr("number"), htr("stoploss"),
                       htr("cash"), htr("cost_total"), htr("cost_commission"),
                       htr("cost_stamptax"), htr("cost_transferfee"),
                       htr("cost_others"), htr("origin"), htr("remark")}),
                  vector_to_python_list<string>(
                      {"U10", "U20", "datetime64[ns]", "U20", "d", "d", "d",
                       "d", "d", "d", "d", "d", "d", "d", "d", "U20", "U100"}),
                  vector_to_python_list<int64_t>({0, 40, 120, 128, 208, 216,
                                                  224, 232, 240, 248, 256, 264,
                                                  272, 280, 288, 296, 376}),
                  776);

    return py::array(dtype, total, static_cast<RawData*>(data),
                     py::capsule(data, [](void* p) { std::free(p); }));
  });

  m.def(
      "trades_to_df",
      [](const TradeRecordList& trades) {
        size_t total = trades.size();
        if (total == 0) {
          return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the data containers of each column
        py::list code_list(total);
        py::list name_list(total);
        py::array_t<int64_t> datetime_arr(total);
        py::list business_list(total);
        py::array_t<double> planPrice_arr(total);
        py::array_t<double> realPrice_arr(total);
        py::array_t<double> goalPrice_arr(total);
        py::array_t<double> number_arr(total);
        py::array_t<double> stoploss_arr(total);
        py::array_t<double> cash_arr(total);
        py::array_t<double> cost_total_arr(total);
        py::array_t<double> cost_commission_arr(total);
        py::array_t<double> cost_stamptax_arr(total);
        py::array_t<double> cost_transferfee_arr(total);
        py::array_t<double> cost_others_arr(total);
        py::list origin_list(total);
        py::list remark_list(total);

        // Get the buffers of each array
        auto datetime_buf = datetime_arr.request();
        auto planPrice_buf = planPrice_arr.request();
        auto realPrice_buf = realPrice_arr.request();
        auto goalPrice_buf = goalPrice_arr.request();
        auto number_buf = number_arr.request();
        auto stoploss_buf = stoploss_arr.request();
        auto cash_buf = cash_arr.request();
        auto cost_total_buf = cost_total_arr.request();
        auto cost_commission_buf = cost_commission_arr.request();
        auto cost_stamptax_buf = cost_stamptax_arr.request();
        auto cost_transferfee_buf = cost_transferfee_arr.request();
        auto cost_others_buf = cost_others_arr.request();

        int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
        double* planPrice_ptr = static_cast<double*>(planPrice_buf.ptr);
        double* realPrice_ptr = static_cast<double*>(realPrice_buf.ptr);
        double* goalPrice_ptr = static_cast<double*>(goalPrice_buf.ptr);
        double* number_ptr = static_cast<double*>(number_buf.ptr);
        double* stoploss_ptr = static_cast<double*>(stoploss_buf.ptr);
        double* cash_ptr = static_cast<double*>(cash_buf.ptr);
        double* cost_total_ptr = static_cast<double*>(cost_total_buf.ptr);
        double* cost_commission_ptr =
            static_cast<double*>(cost_commission_buf.ptr);
        double* cost_stamptax_ptr = static_cast<double*>(cost_stamptax_buf.ptr);
        double* cost_transferfee_ptr =
            static_cast<double*>(cost_transferfee_buf.ptr);
        double* cost_others_ptr = static_cast<double*>(cost_others_buf.ptr);

        // Fill the data
        for (size_t i = 0; i < total; i++) {
          const TradeRecord& t = trades[i];
          if (!t.stock.isNull()) {
            code_list[i] = py::str(t.stock.market_code());
            name_list[i] = py::str(t.stock.name());
          } else {
            code_list[i] = py::str("");
            name_list[i] = py::str("");
          }
          datetime_ptr[i] = t.datetime.timestamp() * 1000LL;
          business_list[i] = py::str(getBusinessName(t.business));
          planPrice_ptr[i] = t.planPrice;
          realPrice_ptr[i] = t.realPrice;
          goalPrice_ptr[i] = t.goalPrice;
          number_ptr[i] = t.number;
          stoploss_ptr[i] = t.stoploss;
          cash_ptr[i] = t.cash;
          cost_total_ptr[i] = t.cost.total;
          cost_commission_ptr[i] = t.cost.commission;
          cost_stamptax_ptr[i] = t.cost.stamptax;
          cost_transferfee_ptr[i] = t.cost.transferfee;
          cost_others_ptr[i] = t.cost.others;
          origin_list[i] = py::str(getOrderOriginName(t.from));
          remark_list[i] = py::str(t.remark);
        }

        // Build the DataFrame
        auto pandas = py::module_::import("pandas");
        py::dict columns;
        columns[htr("market_code").c_str()] =
            pandas.attr("Series")(code_list, py::arg("dtype") = "string");
        columns[htr("stock_name").c_str()] =
            pandas.attr("Series")(name_list, py::arg("dtype") = "string");
        columns[htr("datetime").c_str()] =
            datetime_arr.attr("astype")("datetime64[ns]");
        columns[htr("business").c_str()] =
            pandas.attr("Series")(business_list, py::arg("dtype") = "string");
        columns[htr("planPrice").c_str()] = planPrice_arr;
        columns[htr("realPrice").c_str()] = realPrice_arr;
        columns[htr("goalPrice").c_str()] = goalPrice_arr;
        columns[htr("number").c_str()] = number_arr;
        columns[htr("stoploss").c_str()] = stoploss_arr;
        columns[htr("cash").c_str()] = cash_arr;
        columns[htr("cost_total").c_str()] = cost_total_arr;
        columns[htr("cost_commission").c_str()] = cost_commission_arr;
        columns[htr("cost_stamptax").c_str()] = cost_stamptax_arr;
        columns[htr("cost_transferfee").c_str()] = cost_transferfee_arr;
        columns[htr("cost_others").c_str()] = cost_others_arr;
        columns[htr("origin").c_str()] =
            pandas.attr("Series")(origin_list, py::arg("dtype") = "string");
        columns[htr("remark").c_str()] =
            pandas.attr("Series")(remark_list, py::arg("dtype") = "string");

        return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
      },
      R"(trades_to_df(trades)

    Convert the trade record list to a pandas DataFrame

    :param TradeRecordList trades: the trade record list
    :return: a pandas DataFrame containing the trade records
    :rtype: pandas.DataFrame)");
}

// Registration group: _cost_factories
/*
 * _cost_factories.cpp
 *
 *  Created on: 2013-2-14
 *      Author: fasiondog
 */

#include <execution/pricing/TradeCosts.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_CostFactories(py::module& m) {
  m.def("TC_TestStub", TC_TestStub, "For testing only");

  m.def(
      "TC_FixedA", TC_FixedA, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.001, py::arg("lowest_transferfee") = 1.0,
      R"(TC_FixedA([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.001, lowest_transferfee=1.0])

    The A-share trade cost algorithm before August 1, 2015

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :param float lowest_transferfee: the lowest transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

  m.def(
      "TC_FixedA2015", TC_FixedA2015, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.00002,
      R"(TC_FixedA2015([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    Since August 1, 2015, the SSE transfer fee is changed to 0.02 per mille of the amount

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

  m.def(
      "TC_FixedA2017", TC_FixedA2017, py::arg("commission") = 0.0018,
      py::arg("lowest_commission") = 5.0, py::arg("stamptax") = 0.001,
      py::arg("transferfee") = 0.00002,
      R"(TC_FixedA2017([commission=0.0018, lowest_commission=5.0, stamptax=0.001, transferfee=0.00002])

    Since January 1, 2017, the SZSE transfer fee item is listed separately, charged in both directions at 0.02‰ of the amount.

    :param float commission: the commission ratio
    :param float lowest_commission: the lowest commission value
    :param float stamptax: the stamp tax
    :param float transferfee: the transfer fee
    :return: a subclass instance of :py:class:`TradeCostBase`)");

  m.def("TC_FixedETF", TC_FixedETF, py::arg("commission") = 0.0001,
        py::arg("lowest_commission") = 5.0,
        R"(TC_FixedETF([commission=0.0001, lowest_commission=5.0])

    The ETF trade cost algorithm; the commission is charged in both the buy and the sell directions, with no stamp tax and no transfer fee.

    :param float commission: the commission ratio, defaulting to 1 per ten thousand
    :param float lowest_commission: the lowest commission value, defaulting to 5 yuan per trade
    :return: a subclass instance of :py:class:`TradeCostBase`)");

  m.def("TC_Zero", TC_Zero, "The zero trade cost algorithm");
}

// Registration group: execution_main
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_ExecutionEngine(py::module& m);
void export_CostFactories(py::module& m);
void export_OrderBroker(py::module& m);
void export_OrderOrigin(py::module& m);
void export_TradeCost(py::module& m);
void export_BorrowRecord(py::module& m);
void export_CostRecord(py::module& m);
void export_FundsRecord(py::module& m);
void export_LoanRecord(py::module& m);
void export_PositionRecord(py::module& m);
void export_TradeRecord(py::module& m);

void export_execution_main(py::module& m) {
  export_OrderOrigin(m);
  export_CostRecord(m);
  export_TradeCost(m);
  export_CostFactories(m);
  export_OrderBroker(m);
  export_PositionRecord(m);
  export_TradeRecord(m);
  export_FundsRecord(m);
  export_BorrowRecord(m);
  export_LoanRecord(m);
  export_ExecutionEngine(m);
}

void bindExecution(py::module_& m) {
  export_execution_main(m);
}
