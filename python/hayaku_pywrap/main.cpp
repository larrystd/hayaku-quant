/*
 * main.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <application/SystemInfo.h>
#include <extensions/realtime/ShmClientHook.h>
#include <extensions/telemetry/Telemetry.h>
#include <hayaku.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <cstdint>

#include "Bindings.h"

using namespace hayaku;
namespace py = pybind11;

#if PY_MINOR_VERSION == 8
PYBIND11_MODULE(core38, m) {
#elif PY_MINOR_VERSION == 9
PYBIND11_MODULE(core39, m) {
#elif PY_MINOR_VERSION == 10
PYBIND11_MODULE(core310, m) {
#elif PY_MINOR_VERSION == 11
PYBIND11_MODULE(core311, m) {
#elif PY_MINOR_VERSION == 12
// #warning "current python version: 3.12"
PYBIND11_MODULE(core312, m) {
#elif PY_MINOR_VERSION == 13
// #warning "current python version: 3.13"
PYBIND11_MODULE(core313, m) {
#elif PY_MINOR_VERSION == 14
// #warning "current python version: 3.14"
PYBIND11_MODULE(core314, m) {
#else
PYBIND11_MODULE(core, m) {
#endif

  HAYAKU_INFO("current python version: {}", PY_VERSION);

  py::register_exception<hayaku::exception>(m, "HAYAKUException");

  // Set the system running state
  setRunningInPython(true);

  // Register the interrupt checker for the long IPC blocking waits (e.g.
  // waiting for the data server readiness), responding to Ctrl+C; The waits
  // happen in the C++ code that has released the GIL; the GIL needs to be
  // re-acquired here before the signals can be checked.
  ipc::setInterruptChecker([]() {
    py::gil_scoped_acquire gil;
    if (PyErr_CheckSignals() != 0) {
      // Raise the pending exception (such as KeyboardInterrupt), which pybind11
      // converts to a Python exception
      throw py::error_already_set();
    }
    return false;
  });

  // arrow::py::import_pyarrow();

  bindCommon(m);
  bindData(m);
  bindOperators(m);
  bindExecution(m);
  bindMetrics(m);
  bindStrategy(m);
  bindExtensions(m);
  bindApplication(m);
}
