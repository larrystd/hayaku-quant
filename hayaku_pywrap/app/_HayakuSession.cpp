/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <application/HayakuSession.h>
#include "common/pybind_utils.h"

using namespace hayaku;
namespace py = pybind11;

void export_HayakuSession(py::module& m) {
    py::class_<HayakuSession>(m, "HayakuSession", "Explicit Hayaku runtime session")
      .def_static(
        "open",
        py::overload_cast<const string&, bool, const StrategyContext&>(&HayakuSession::open),
        py::arg("filename"), py::arg("ignore_preload") = false,
        py::arg("context") = StrategyContext({"all"}), py::call_guard<py::gil_scoped_release>())
      .def_static(
        "open",
        [](const string& filename, const AccountConfig& accountConfig, bool ignorePreload,
           const StrategyContext& context) {
            return HayakuSession::open(SessionOptions::fromIni(filename, ignorePreload, context),
                                       accountConfig);
        },
        py::arg("filename"), py::arg("account_config"), py::arg("ignore_preload") = false,
        py::arg("context") = StrategyContext({"all"}), py::call_guard<py::gil_scoped_release>())
      .def("close", &HayakuSession::close)
      .def_property_readonly("opened", &HayakuSession::isOpen)
      .def_property_readonly("ready", &HayakuSession::ready)
      .def_property_readonly("data", py::overload_cast<>(&HayakuSession::data),
                             py::return_value_policy::reference_internal)
      .def_property_readonly("has_execution", &HayakuSession::hasExecution)
      .def_property_readonly("has_strategy", &HayakuSession::hasStrategy)
      .def_property_readonly("execution", py::overload_cast<>(&HayakuSession::execution),
                             py::return_value_policy::reference_internal)
      .def_property_readonly("strategy", py::overload_cast<>(&HayakuSession::strategy),
                             py::return_value_policy::reference_internal)
      .def("wait_ready", &HayakuSession::waitReady, py::call_guard<py::gil_scoped_release>())
      .def(
        "__enter__", [](HayakuSession& self) -> HayakuSession& { return self; },
        py::return_value_policy::reference_internal)
      .def("__exit__",
           [](HayakuSession& self, const py::object&, const py::object&, const py::object&) {
               self.close();
               return false;
           });
}
