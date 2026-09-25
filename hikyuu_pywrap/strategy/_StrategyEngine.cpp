/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <strategy/engine/StrategyEngine.h>
#include "common/pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_StrategyEngine(py::module& m) {
    py::class_<StrategyDefinition>(m, "StrategyDefinition",
                                   "Explicit immutable strategy component graph")
      .def(
        py::init<MoneyManagerPtr, SignalPtr, string, EnvironmentPtr, ConditionPtr, StoplossPtr,
                 StoplossPtr, ProfitGoalPtr, SlippagePtr, Parameter>(),
        py::arg("money_manager"), py::arg("signal"),
        py::arg("name") = "Strategy", py::arg("environment") = EnvironmentPtr(),
        py::arg("condition") = ConditionPtr(), py::arg("stoploss") = StoplossPtr(),
        py::arg("take_profit") = StoplossPtr(), py::arg("profit_goal") = ProfitGoalPtr(),
        py::arg("slippage") = SlippagePtr(), py::arg("parameters") = Parameter())
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

    py::class_<BacktestRequest>(m, "BacktestRequest", "Immutable strategy run request")
      .def(py::init<const KData&, bool, bool>(), py::arg("kdata"), py::arg("reset") = true,
           py::arg("reset_all") = false)
      .def_property_readonly("kdata", &BacktestRequest::kdata,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("reset", &BacktestRequest::reset)
      .def_property_readonly("reset_all", &BacktestRequest::resetAll);

    py::class_<BacktestResult>(m, "BacktestResult", "Stable strategy result snapshot")
      .def_property_readonly("stock", &BacktestResult::stock,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("query", &BacktestResult::query,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("trades", &BacktestResult::trades,
                             py::return_value_policy::reference_internal)
      .def_property_readonly("trade_count", &BacktestResult::tradeCount)
      .def_property_readonly("empty", &BacktestResult::empty);

    py::class_<StrategyEngine>(m, "StrategyEngine", "Narrow strategy orchestration facade")
      .def(py::init<StrategyDefinition, ExecutionEngine&>(), py::arg("definition"),
           py::arg("execution"), py::keep_alive<1, 3>())
      .def("run", &StrategyEngine::run, py::arg("request"), py::call_guard<py::gil_scoped_release>())
      .def("stop", &StrategyEngine::stop)
      .def_property_readonly("running", &StrategyEngine::running);
}
