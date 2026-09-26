/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "common/pybind_utils.h"

namespace py = pybind11;

void export_DataEngine(py::module& m);

void export_data_main(py::module& m) {
    export_DataEngine(m);
}
