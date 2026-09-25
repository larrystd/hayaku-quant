/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <app/HikyuuSession.h>
#include "common/pybind_utils.h"

using namespace hku;
namespace py = pybind11;

void export_HikyuuSession(py::module& m) {
    py::class_<HikyuuSession>(m, "HikyuuSession", "Explicit Hikyuu runtime session")
      .def_static(
        "open",
        py::overload_cast<const string&, bool, const StrategyContext&>(&HikyuuSession::open),
        py::arg("filename"), py::arg("ignore_preload") = false,
        py::arg("context") = StrategyContext({"all"}), py::call_guard<py::gil_scoped_release>())
      .def_static(
        "open",
        [](const string& filename, const AccountConfig& accountConfig, bool ignorePreload,
           const StrategyContext& context) {
            return HikyuuSession::open(SessionOptions::fromIni(filename, ignorePreload, context),
                                       accountConfig);
        },
        py::arg("filename"), py::arg("account_config"), py::arg("ignore_preload") = false,
        py::arg("context") = StrategyContext({"all"}), py::call_guard<py::gil_scoped_release>())
      .def("close", &HikyuuSession::close)
      .def_property_readonly("opened", &HikyuuSession::isOpen)
      .def_property_readonly("ready", &HikyuuSession::ready)
      .def_property_readonly("data", py::overload_cast<>(&HikyuuSession::data),
                             py::return_value_policy::reference_internal)
      .def_property_readonly("has_execution", &HikyuuSession::hasExecution)
      .def_property_readonly("has_strategy", &HikyuuSession::hasStrategy)
      .def_property_readonly("execution", py::overload_cast<>(&HikyuuSession::execution),
                             py::return_value_policy::reference_internal)
      .def_property_readonly("strategy", py::overload_cast<>(&HikyuuSession::strategy),
                             py::return_value_policy::reference_internal)
      .def("wait_ready", &HikyuuSession::waitReady, py::call_guard<py::gil_scoped_release>())
      .def(
        "__enter__", [](HikyuuSession& self) -> HikyuuSession& { return self; },
        py::return_value_policy::reference_internal)
      .def("__exit__",
           [](HikyuuSession& self, const py::object&, const py::object&, const py::object&) {
               self.close();
               return false;
           });
}
