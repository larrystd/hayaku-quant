/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "Bindings.h"

namespace py = pybind11;

#if PY_MINOR_VERSION == 8
PYBIND11_MODULE(ingest38, m) {
#elif PY_MINOR_VERSION == 9
PYBIND11_MODULE(ingest39, m) {
#elif PY_MINOR_VERSION == 10
PYBIND11_MODULE(ingest310, m) {
#elif PY_MINOR_VERSION == 11
PYBIND11_MODULE(ingest311, m) {
#elif PY_MINOR_VERSION == 12
PYBIND11_MODULE(ingest312, m) {
#elif PY_MINOR_VERSION == 13
PYBIND11_MODULE(ingest313, m) {
#elif PY_MINOR_VERSION == 14
PYBIND11_MODULE(ingest314, m) {
#else
PYBIND11_MODULE(ingest, m) {
#endif
  // Core registers Datetime, Query and record types consumed by these bindings.
  py::module_::import("hayaku.core");
  bindIngest(m);
}
