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
void export_Normlize(py::module& m);
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
    export_Normlize(m);
    export_MultiFactor(m);
    export_StrategyEngine(m);
}
