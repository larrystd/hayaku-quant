/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <data/DataEngine.h>
#include "common/pybind_utils.h"

using namespace hayaku;
namespace py = pybind11;

void export_DataEngine(py::module& m) {
    py::class_<DataEngine>(m, "DataEngine", "Read-only market data facade")
      .def_property_readonly("ready", &DataEngine::ready, "Whether all data is ready")
      .def_property_readonly("initializing", &DataEngine::initializing,
                             "Whether data initialization is in progress")
      .def_property_readonly("size", &DataEngine::size, "Number of securities")
      .def("__len__", &DataEngine::size)
      .def("__getitem__", &DataEngine::getStock, py::arg("market_code"))
      .def("__iter__",
           [](const DataEngine& self) { return py::iter(py::cast(self.getStockList())); })
      .def("wait_ready", &DataEngine::waitReady, py::call_guard<py::gil_scoped_release>())
      .def("get_stock", &DataEngine::getStock, py::arg("market_code"))
      .def("get_stock_list", &DataEngine::getStockList)
      .def("get_kdata", &DataEngine::getKData, py::arg("market_code"), py::arg("query"))
      .def("get_market_info", &DataEngine::getMarketInfo, py::arg("market"))
      .def("get_market_stock", &DataEngine::getMarketStock, py::arg("market"))
      .def("get_market_list", &DataEngine::getMarketList)
      .def("get_stock_type_info", &DataEngine::getStockTypeInfo, py::arg("stock_type"))
      .def("get_stock_type_info_list", &DataEngine::getStockTypeInfoList)
      .def("get_block_category_list", &DataEngine::getBlockCategoryList)
      .def("get_block", &DataEngine::getBlock, py::arg("category"), py::arg("name"))
      .def("get_block_list", &DataEngine::getBlockList, py::arg("category") = "")
      .def("get_stock_belongs", &DataEngine::getStockBelongs, py::arg("stock"),
           py::arg("category") = "")
      .def("get_trading_calendar",
           py::overload_cast<const KQuery&, const string&>(&DataEngine::getTradingCalendar,
                                                           py::const_),
           py::arg("query"), py::arg("market") = "SH")
      .def("get_trading_calendar",
           py::overload_cast<const StockList&, const KQuery&>(&DataEngine::getTradingCalendar,
                                                              py::const_),
           py::arg("stocks"), py::arg("query"))
      .def("is_holiday", &DataEngine::isHoliday, py::arg("datetime"))
      .def("is_trading_hours", &DataEngine::isTradingHours, py::arg("datetime"),
           py::arg("market") = "SH")
      .def("get_zh_bond10", &DataEngine::getZhBond10, py::return_value_policy::reference_internal)
      .def("get_stock_weight_list", &DataEngine::getStockWeightList, py::arg("stock"),
           py::arg("start"), py::arg("end"))
      .def("get_history_finance_field_name", &DataEngine::getHistoryFinanceFieldName,
           py::arg("index"), py::return_value_policy::copy)
      .def("get_history_finance_field_index", &DataEngine::getHistoryFinanceFieldIndex,
           py::arg("name"))
      .def("get_history_finance_all_fields", &DataEngine::getHistoryFinanceAllFields)
      .def("get_history_finance", &DataEngine::getHistoryFinance, py::arg("stock"),
           py::arg("start"), py::arg("end"));
}
