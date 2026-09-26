/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <execution/OrderOrigin.h>
#include "common/pybind_utils.h"

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
