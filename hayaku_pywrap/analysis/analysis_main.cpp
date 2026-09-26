/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240907 added by fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_analysis_main(py::module& m) {
    // Analysis helpers operate on immutable BacktestResult/AccountSnapshot values in Python.
}
