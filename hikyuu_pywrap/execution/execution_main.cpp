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
