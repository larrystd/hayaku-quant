#include "Bindings.h"

/* Domain binding registrations. */

// Registration group: _misc
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_misc(py::module&) {
  // Retired batch-runtime helpers are intentionally not exposed.
}

// Registration group: analysis_main
/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20240907 added by fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_analysis_main(py::module& m) {
  // Analysis helpers operate on immutable BacktestResult/AccountSnapshot values
  // in Python.
}

void bindMetrics(py::module_& m) {
  export_misc(m);
  export_analysis_main(m);
}
