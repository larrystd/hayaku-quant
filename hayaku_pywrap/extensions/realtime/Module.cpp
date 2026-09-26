/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "Bindings.h"

namespace py = pybind11;

#if PY_MINOR_VERSION == 8
PYBIND11_MODULE(realtime38, m) {
#elif PY_MINOR_VERSION == 9
PYBIND11_MODULE(realtime39, m) {
#elif PY_MINOR_VERSION == 10
PYBIND11_MODULE(realtime310, m) {
#elif PY_MINOR_VERSION == 11
PYBIND11_MODULE(realtime311, m) {
#elif PY_MINOR_VERSION == 12
PYBIND11_MODULE(realtime312, m) {
#elif PY_MINOR_VERSION == 13
PYBIND11_MODULE(realtime313, m) {
#elif PY_MINOR_VERSION == 14
PYBIND11_MODULE(realtime314, m) {
#else
PYBIND11_MODULE(realtime, m) {
#endif
  // Datetime, Stock, Query and SpotRecord are owned by the core binding.
  py::module_::import("hayaku.core");
  bindRealtime(m);
}
