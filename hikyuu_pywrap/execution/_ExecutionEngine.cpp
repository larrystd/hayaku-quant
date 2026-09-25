/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <execution/ExecutionEngine.h>
#include "common/pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_ExecutionEngine(py::module& m) {
    py::class_<AccountId>(m, "AccountId", "Stable execution-account identity")
      .def(py::init<std::uint64_t>(), py::arg("value") = 0)
      .def_property_readonly("value", &AccountId::value)
      .def_property_readonly("valid", &AccountId::valid)
      .def("__bool__", &AccountId::valid)
      .def(py::self == py::self);

    py::class_<AccountConfig>(m, "AccountConfig", "Immutable execution-account configuration")
      .def(py::init<Datetime, price_t, TradeCostPtr, string, int, bool, bool, AccountId,
                    std::vector<OrderBrokerPtr>>(),
           py::arg("init_datetime") = Datetime(199001010000LL),
           py::arg("initial_cash") = 100000.0, py::arg("cost_policy") = TC_Zero(),
           py::arg("name") = "SYS", py::arg("precision") = 2,
           py::arg("support_borrow_cash") = false,
           py::arg("support_borrow_stock") = false, py::arg("account_id") = AccountId(),
           py::arg("brokers") = std::vector<OrderBrokerPtr>())
      .def_property_readonly("init_datetime", &AccountConfig::initDatetime)
      .def_property_readonly("initial_cash", &AccountConfig::initialCash)
      .def_property_readonly("cost_policy", &AccountConfig::costPolicy)
      .def_property_readonly("name", &AccountConfig::name)
      .def_property_readonly("precision", &AccountConfig::precision)
      .def_property_readonly("support_borrow_cash", &AccountConfig::supportBorrowCash)
      .def_property_readonly("support_borrow_stock", &AccountConfig::supportBorrowStock)
      .def_property_readonly("account_id", &AccountConfig::accountId)
      .def_property_readonly("brokers", &AccountConfig::brokers,
                             py::return_value_policy::reference_internal);

    py::enum_<OrderSide>(m, "OrderSide")
      .value("BUY", OrderSide::BUY)
      .value("SELL", OrderSide::SELL)
      .value("SELL_SHORT", OrderSide::SELL_SHORT)
      .value("BUY_SHORT", OrderSide::BUY_SHORT);

    py::class_<OrderRequest>(m, "OrderRequest", "Immutable synchronous order request")
      .def(py::init<OrderSide, Datetime, Stock, price_t, double, price_t, price_t, price_t,
                    OrderOrigin, string>(),
           py::arg("side"), py::arg("datetime"), py::arg("stock"), py::arg("real_price"),
           py::arg("number"), py::arg("stoploss") = 0.0, py::arg("goal_price") = 0.0,
           py::arg("plan_price") = 0.0, py::arg("origin") = OrderOrigin::UNSPECIFIED,
           py::arg("remark") = "")
      .def_property_readonly("side", &OrderRequest::side)
      .def_property_readonly("datetime", &OrderRequest::datetime, py::return_value_policy::copy)
      .def_property_readonly("stock", &OrderRequest::stock, py::return_value_policy::copy)
      .def_property_readonly("real_price", &OrderRequest::realPrice)
      .def_property_readonly("number", &OrderRequest::number)
      .def_property_readonly("stoploss", &OrderRequest::stoploss)
      .def_property_readonly("goal_price", &OrderRequest::goalPrice)
      .def_property_readonly("plan_price", &OrderRequest::planPrice)
      .def_property_readonly("origin", &OrderRequest::origin)
      .def_property_readonly("remark", &OrderRequest::remark, py::return_value_policy::copy);

    py::enum_<ExecutionStatus>(m, "ExecutionStatus")
      .value("FILLED", ExecutionStatus::FILLED)
      .value("REJECTED", ExecutionStatus::REJECTED);

    py::class_<ExecutionReport>(m, "ExecutionReport", "Synchronous order result")
      .def_property_readonly("status", &ExecutionReport::status)
      .def_property_readonly("filled", &ExecutionReport::filled)
      .def_property_readonly("rejected", &ExecutionReport::rejected)
      .def_property_readonly("trade", &ExecutionReport::trade,
                             py::return_value_policy::reference_internal);

    py::class_<AccountSnapshot>(m, "AccountSnapshot", "Read-only account value snapshot")
      .def_property_readonly("funds", &AccountSnapshot::funds,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("positions", &AccountSnapshot::positions,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("short_positions", &AccountSnapshot::shortPositions,
                             py::return_value_policy::reference_internal);

    py::class_<AccountView>(m, "AccountView", "Value-owned read-only account view")
      .def_property_readonly("account_id", &AccountView::id)
      .def_property_readonly("init_datetime", &AccountView::initDatetime)
      .def_property_readonly("last_datetime", &AccountView::lastDatetime)
      .def_property_readonly("funds", &AccountView::funds,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("positions", &AccountView::positions,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("short_positions", &AccountView::shortPositions,
                             py::return_value_policy::reference_internal);

    py::class_<ExecutionEngine>(m, "ExecutionEngine", "Order execution and account facade")
      .def(py::init<const AccountConfig&>(), py::arg("account_config"))
      .def("submit", &ExecutionEngine::submit, py::arg("request"))
      .def("snapshot", &ExecutionEngine::snapshot)
      .def("view", &ExecutionEngine::view)
      .def("history", &ExecutionEngine::history)
      .def_property_readonly("account_id", &ExecutionEngine::accountId);
}
