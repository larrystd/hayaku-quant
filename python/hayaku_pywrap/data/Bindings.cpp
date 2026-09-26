#include "Bindings.h"
#include <hayaku.h>
#include <cstdint>

/* Domain binding registrations. */

// Registration group: _Block
/*
 * _Block.cpp
 *
 *  Created on: 2015-02-10
 *      Author: fasiondog
 */

#include <common/serialization/Block_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

string (Block::*getCategory)() const = &Block::category;
void (Block::*setCategory)(const string&) = &Block::category;
string (Block::*getName)() const = &Block::name;
void (Block::*setName)(const string&) = &Block::name;

void export_Block(py::module& m) {
  py::class_<Block>(
      m, "Block",
      "The block class, which can be regarded as a container of the securities")
      .def(py::init<>())
      .def(py::init<const string&, const string&>(), py::arg("category"),
           py::arg("name"))
      .def(py::init<const string&, const string&, const string&>(),
           py::arg("category"), py::arg("name"), py::arg("index_code"))
      .def(py::init<const Block&>())
      .def(py::init<const StockList&>())
      .def(py::init<const StringList&>())

      .def("__str__", to_py_str<Block>)
      .def("__repr__", to_py_str<Block>)

      .def_property("category", getCategory, setCategory, "The block category")
      .def_property("name", getName, setName, "The block name")
      .def_property("index_stock", &Block::getIndexStock, &Block::setIndexStock,
                    py::return_value_policy::copy, "The corresponding index")

      .def("is_null", &Block::isNull, R"(is_null(self)

    Whether it is a null value)")

      .def("empty", &Block::empty, R"(empty(self)

    Whether it is empty)")

      .def("add", py::overload_cast<const Stock&>(&Block::add),
           R"(add(self, stock)

    Add the specified security

    :param Stock stock: the security to add
    :return: whether it was added successfully
    :rtype: bool)")

      .def("add", py::overload_cast<const string&>(&Block::add),
           R"(add(self, market_code)

    Add the specified security by "market abbreviation + security code"

    :param str market_code: the market abbreviation + the security code
    :return: whether it was added successfully
    :rtype: bool)")

      .def(
          "add",
          [](Block& blk, py::sequence stks) {
            auto total = len(stks);
            HAYAKU_IF_RETURN(total == 0, true);
            StockList stks_list = get_stock_list_from_python(stks);
            return blk.add(stks_list);
          },
          R"(add(self, sequence)

    Add the specified security list

    :param sequence stks: a sequence composed entirely of the Stocks, or a sequence composed entirely of the string "market abbreviation + security code"
    :return: True all succeed | False some fail)")

      .def("remove", py::overload_cast<const Stock&>(&Block::remove),
           R"(remove(self, stock)

    Remove the specified security

    :param Stock stock: the specified security
    :return: whether it was successful
    :rtype: bool)")

      .def("remove", py::overload_cast<const string&>(&Block::remove),
           R"(remove(market_code)

    Remove the specified security

    :param str market_code: the market abbreviation + the security code
    :return: True success | False failure
    :rtype: bool)")

      .def("clear", &Block::clear, "Remove all the contained securities")

      .def("__len__", &Block::size, "The number of the contained securities")

      .def("__getitem__", &Block::get, R"(__getitem__(self, market_code)

    :param str market_code: the security code
    :return: the Stock instance)")

      .def(
          "__iter__",
          [](const Block& blk) {
            return py::make_iterator<
                py::return_value_policy::reference_internal, StockMapIterator,
                StockMapIterator, const Stock&>(blk.begin(), blk.end());
          },
          py::keep_alive<0, 1>())

      .def(
          "get_stock_list",
          [](const Block& self, py::object filter) {
            StockList ret;
            if (filter.is_none()) {
              ret = self.getStockList();
            } else {
              HAYAKU_CHECK(py::hasattr(filter, "__call__"),
                           "filter not callable!");
              py::object filter_func = filter.attr("__call__");
              ret = self.getStockList([&](const Stock& stk) {
                return filter_func(stk).cast<bool>();
              });
            }
            return ret;
          },
          py::arg("filter") = py::none(), R"(get_stock_list(self[, filter=None])

    Get the security list

    :param func filter: a filter function whose input parameter is the stock and which returns True | False)")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)

          DEF_PICKLE(Block);
}

// Registration group: _DataEngine
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include <data/DataEngine.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_DataEngine(py::module& m) {
  py::class_<DataEngine>(m, "DataEngine", "Read-only market data facade")
      .def_property_readonly("ready", &DataEngine::ready,
                             "Whether all data is ready")
      .def_property_readonly("initializing", &DataEngine::initializing,
                             "Whether data initialization is in progress")
      .def_property_readonly("size", &DataEngine::size, "Number of securities")
      .def("__len__", &DataEngine::size)
      .def("__getitem__", &DataEngine::getStock, py::arg("market_code"))
      .def("__iter__",
           [](const DataEngine& self) {
             return py::iter(py::cast(self.getStockList()));
           })
      .def("wait_ready", &DataEngine::waitReady,
           py::call_guard<py::gil_scoped_release>())
      .def("get_stock", &DataEngine::getStock, py::arg("market_code"))
      .def("get_stock_list", &DataEngine::getStockList)
      .def("get_kdata", &DataEngine::getKData, py::arg("market_code"),
           py::arg("query"))
      .def("get_market_info", &DataEngine::getMarketInfo, py::arg("market"))
      .def("get_market_stock", &DataEngine::getMarketStock, py::arg("market"))
      .def("get_market_list", &DataEngine::getMarketList)
      .def("get_stock_type_info", &DataEngine::getStockTypeInfo,
           py::arg("stock_type"))
      .def("get_stock_type_info_list", &DataEngine::getStockTypeInfoList)
      .def("get_block_category_list", &DataEngine::getBlockCategoryList)
      .def("get_block", &DataEngine::getBlock, py::arg("category"),
           py::arg("name"))
      .def("get_block_list", &DataEngine::getBlockList,
           py::arg("category") = "")
      .def("get_stock_belongs", &DataEngine::getStockBelongs, py::arg("stock"),
           py::arg("category") = "")
      .def("get_trading_calendar",
           py::overload_cast<const KQuery&, const string&>(
               &DataEngine::getTradingCalendar, py::const_),
           py::arg("query"), py::arg("market") = "SH")
      .def("get_trading_calendar",
           py::overload_cast<const StockList&, const KQuery&>(
               &DataEngine::getTradingCalendar, py::const_),
           py::arg("stocks"), py::arg("query"))
      .def("is_holiday", &DataEngine::isHoliday, py::arg("datetime"))
      .def("is_trading_hours", &DataEngine::isTradingHours, py::arg("datetime"),
           py::arg("market") = "SH")
      .def("get_zh_bond10", &DataEngine::getZhBond10,
           py::return_value_policy::reference_internal)
      .def("get_stock_weight_list", &DataEngine::getStockWeightList,
           py::arg("stock"), py::arg("start"), py::arg("end"))
      .def("get_history_finance_field_name",
           &DataEngine::getHistoryFinanceFieldName, py::arg("index"),
           py::return_value_policy::copy)
      .def("get_history_finance_field_index",
           &DataEngine::getHistoryFinanceFieldIndex, py::arg("name"))
      .def("get_history_finance_all_fields",
           &DataEngine::getHistoryFinanceAllFields)
      .def("get_history_finance", &DataEngine::getHistoryFinance,
           py::arg("stock"), py::arg("start"), py::arg("end"));
}

// Registration group: _DataType
/*
 * _DataType.cpp
 *
 *  Created on: 2012-9-29
 *      Author: fasiondog
 */

#include <data/MarketTypes.h>

#include <cmath>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if !defined(_MSVC_VER)
bool (*isnan_func)(price_t) = std::isnan;
bool (*isinf_func)(price_t) = std::isinf;
#endif

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_DataType(py::module& m) {
#if defined(_MSVC_VER)
  m.def("isnan", std::isnan<price_t>, "Whether it is not a number");
  m.def("isinf", std::isinf<price_t>,
        "Whether it is the infinity or the negative infinity");
#else
  m.def("isnan", isnan_func, "Whether it is not a number");
  m.def("isinf", isinf_func,
        "Whether it is the infinity or the negative infinity");
#endif

  m.def(
      "toPriceList",
      [](const py::sequence obj) {
        return python_list_to_vector<price_t>(obj);
      },
      "Convert a python list/tuple/np.array object to a PriceList object");
}

// Registration group: _KData
/*
 * _KData.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <common/serialization/KData_serialization.h>
#include <operators/SeriesOperators.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

const KRecord& (KData::*KData_getKRecord1)(size_t pos) const =
    &KData::getKRecord;
const KRecord& (KData::*KData_getKRecord2)(Datetime datetime) const =
    &KData::getKRecord;

void export_KData(py::module& m) {
  int64_t null_int64 = Null<int64_t>();
  py::class_<KData>(
      m, "KData",
      "The K-line data obtained through Stock.getKData; it is an array "
      "composed of KRecords and can be traversed like a list")
      .def(py::init<>())
      .def("__str__", &KData::toString)
      .def("__repr__", &KData::toString)

      .def_property_readonly(
          "start_pos", &KData::startPos,
          "Get the corresponding start position in the original K-line "
          "records; if the KData is empty, return 0")
      .def_property_readonly("end_pos", &KData::endPos,
                             "Get the position of the next record after the "
                             "range in the original K-line records; if it is "
                             "empty, return 0, otherwise it equals lastPos + 1")
      .def_property_readonly(
          "last_pos", &KData::lastPos,
          "Get the position of the last record in the original K-line records; "
          "if it is empty, return 0, otherwise it equals endPos - 1")

      .def_property_readonly("open", &KData::open,
                             "Return the Indicator instance containing the "
                             "open prices, equivalent to OPEN(k)")
      .def_property_readonly("close", &KData::close,
                             "Return the Indicator instance containing the "
                             "close prices, equivalent to CLOSE(k)")
      .def_property_readonly("high", &KData::high,
                             "Return the Indicator instance containing the "
                             "high prices, equivalent to HIGH(k)")
      .def_property_readonly("low", &KData::low,
                             "Return the Indicator instance containing the low "
                             "prices, equivalent to LOW(k)")
      .def_property_readonly("amo", &KData::amo,
                             "Return the Indicator instance containing the "
                             "amounts, equivalent to AMO(k)")
      .def_property_readonly("vol", &KData::vol,
                             "Return the Indicator instance containing the "
                             "volumes, equivalent to VOL(k)")

      .def("get_datetime_list", &KData::getDatetimeList,
           R"(get_datetime_list(self)

        Return the trading date list

        :rtype: DatetimeList)")

      .def("get", KData_getKRecord1, py::return_value_policy::copy,
           R"(get(self, pos)

        Get the K-line record at the specified index position

        :param int pos: the position index
        :rtype: KRecord)")

      .def("get_by_datetime", KData_getKRecord2, py::return_value_policy::copy,
           R"(get_by_datetime(self, datetime)

        Get the K-line record at the specified time.

        :param Datetime datetime: the specified date
        :rtype: KRecord)")

      .def(
          "get_pos",
          [](const KData& self, const Datetime& d) {
            size_t pos = self.getPos(d);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
              ret = py::int_(pos);
            }
            return ret;
          },
          R"(get_pos(self, datetime)

        Get the index position of the K-line record at the specified time; if it is out of the data range, return None

        :param Datetime datetime: the specified date
        :rtype: int)")

      .def(
          "get_pos_in_stock",
          [](const KData& self, Datetime datetime) {
            size_t pos = self.getPosInStock(datetime);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
              ret = py::int_(pos);
            }
            return ret;
          },
          R"(get_pos_in_stock(self, datetime)

        Get the index position in the original K-line corresponding to the specified time

        :param Datetime datetime: the specified time
        :return: the corresponding index position; if it is out of the data range, return None)")

      .def("empty", &KData::empty, R"(empty(self)

        Judge whether it is empty

        :rtype: bool)")

      .def("get_query", &KData::getQuery, py::return_value_policy::copy,
           R"(get_query(self)

        Get the associated query condition

        :rtype: KQuery)")

      .def("get_stock", &KData::getStock, py::return_value_policy::copy,
           R"(get_stock(self)

        Get the associated Stock

        :rtype: Stock)")

      .def(
          "get_kdata",
          py::overload_cast<const KQuery::KType&>(&KData::getKData, py::const_),
          py::arg("ktype"), R"(get_kdata(self, ktype

        Get the K-line data of the other type within the same time range, e.g. the minute-line data corresponding under the daily line

        :param KQuery::KType ktype: the specified needed K-line type)")

      .def("get_kdata",
           py::overload_cast<const Datetime&, const Datetime&>(&KData::getKData,
                                                               py::const_),
           R"(get_kdata(self, start_date, end_date)

        Get a new KData that keeps the data type and the recovery type unchanged through the current KData (note that it is not a subset of the original KData)

        :param Datetime start: the new start date
        :param Datetime end: the new end date
        :rtype: KData)")

      .def("get_kdata",
           py::overload_cast<const KQuery&>(&KData::getKData, py::const_),
           R"(get_kdata(query)

        Get another KData through the current KData, which is not necessarily a subset of it

        :rtype: KData)")

      .def("get_sub_kdata", &KData::getSubKData, py::arg("start"),
           py::arg("end") = null_int64,
           R"(get_sub_kdata(start, end = Null<int64_t>)

        Get a subset of itself by the index

        :param int start: the start index
        :param int end: the end index
        :rtype: KData)")

      .def("tocsv", &KData::tocsv, R"(tocsv(self, filename)

        Save the data to a CSV file

        :param str filename: the specified file name to save)")

      .def("__len__", &KData::size)

      .def(py::self == py::self)
      .def(py::self != py::self)

      .def("__getitem__",
           [](const KData& self, py::object obj) {
             py::object ret;
             if (py::isinstance<py::int_>(obj)) {
               int64_t i = obj.cast<int64_t>();
               int64_t length = self.size();
               int64_t index = i < 0 ? length + i : i;
               if (index < 0 || index >= length)
                 throw std::out_of_range(
                     fmt::format("index out of range: {}", i));
               ret = py::cast(self.getKRecord(index));
               return ret;
             } else if (py::isinstance<Datetime>(obj)) {
               Datetime dt = py::cast<Datetime>(obj);
               auto krecord = self.getKRecord(dt);
               if (!krecord.isValid()) {
                 throw std::out_of_range(
                     fmt::format("datetime out of range: {}", dt));
               }
               ret = py::cast(krecord);
               return ret;
             } else if (py::isinstance<py::str>(obj)) {
               Datetime dt = Datetime(py::cast<std::string>(obj));
               auto krecord = self.getKRecord(dt);
               if (!krecord.isValid()) {
                 throw std::out_of_range(
                     fmt::format("datetime out of range: {}", dt));
               }
               ret = py::cast(krecord);
               return ret;
             } else if (py::isinstance<py::slice>(obj)) {
               py::slice slice = py::cast<py::slice>(obj);
               size_t start, stop, step, length;

               if (!slice.compute(self.size(), &start, &stop, &step, &length)) {
                 throw std::invalid_argument("Invalid slice parameters");
               }

               KRecordList result;
               result.reserve(length);
               for (size_t i = 0; i < length; ++i) {
                 size_t index = start + i * step;
                 result.push_back(self[static_cast<size_t>(index)]);
               }

               ret = py::cast(result);
               return ret;
             }

             throw std::out_of_range("Error index type");
           })

      .def(
          "__iter__",
          [](const KData& self) {
            return py::make_iterator<
                py::return_value_policy::reference_internal>(self.cbegin(),
                                                             self.cend());
          },
          py::keep_alive<0, 1>())

      .def(
          "to_np",
          [](const KData& kdata) {
            size_t total = kdata.size();
            HAYAKU_IF_RETURN(total == 0, py::array());

            struct RawData {
              int64_t datetime;  // The converted millisecond timestamp
              double open;
              double high;
              double low;
              double close;
              double amount;
              double volume;
            };

            RawData* data =
                static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
            for (size_t i = 0; i < total; i++) {
              const KRecord& k = kdata[i];
              data[i].datetime = k.datetime.timestamp() * 1000LL;
              data[i].open = k.openPrice;
              data[i].high = k.highPrice;
              data[i].low = k.lowPrice;
              data[i].close = k.closePrice;
              data[i].amount = k.transAmount;
              data[i].volume = k.transCount;
            }

            // Define the NumPy structured data type
            auto dtype = py::dtype(
                vector_to_python_list<string>({"datetime", "open", "high",
                                               "low", "close", "amount",
                                               "volume"}),
                vector_to_python_list<string>(
                    {"datetime64[ns]", "d", "d", "d", "d", "d", "d"}),
                vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

            return py::array(dtype, total, static_cast<RawData*>(data),
                             py::capsule(data, [](void* p) { std::free(p); }));
          },
          "Convert the KData to a NumPy array")

      .def(
          "to_df",
          [](const KData& self, bool with_stock) {
            size_t total = self.size();
            if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
            }

            // Create the array
            py::array_t<int64_t> datetime_arr(total);
            py::array_t<double> open_arr(total);
            py::array_t<double> high_arr(total);
            py::array_t<double> low_arr(total);
            py::array_t<double> close_arr(total);
            py::array_t<double> amount_arr(total);
            py::array_t<double> vol_arr(total);

            // Get the buffer and fill the data
            auto datetime_buf = datetime_arr.request();
            auto open_buf = open_arr.request();
            auto high_buf = high_arr.request();
            auto low_buf = low_arr.request();
            auto close_buf = close_arr.request();
            auto amount_buf = amount_arr.request();
            auto vol_buf = vol_arr.request();

            int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
            double* open_ptr = static_cast<double*>(open_buf.ptr);
            double* high_ptr = static_cast<double*>(high_buf.ptr);
            double* low_ptr = static_cast<double*>(low_buf.ptr);
            double* close_ptr = static_cast<double*>(close_buf.ptr);
            double* amount_ptr = static_cast<double*>(amount_buf.ptr);
            double* vol_ptr = static_cast<double*>(vol_buf.ptr);

            auto* ks = self.data();
            for (size_t i = 0; i < total; i++) {
              datetime_ptr[i] = ks[i].datetime.timestamp() * 1000LL;
              open_ptr[i] = ks[i].openPrice;
              high_ptr[i] = ks[i].highPrice;
              low_ptr[i] = ks[i].lowPrice;
              close_ptr[i] = ks[i].closePrice;
              amount_ptr[i] = ks[i].transAmount;
              vol_ptr[i] = ks[i].transCount;
            }

            // Build the DataFrame
            auto pandas = py::module_::import("pandas");
            py::dict columns;
            if (with_stock) {
              py::list code_list(total);
              py::list name_list(total);
              auto stk = self.getStock();
              auto code = py::str(stk.code());
              auto name = py::str(stk.name());
              for (size_t i = 0; i < total; i++) {
                code_list[i] = code;
                name_list[i] = name;
              }
              columns["market_code"] =
                  pandas.attr("Series")(code_list, py::arg("dtype") = "string");
              columns["name"] =
                  pandas.attr("Series")(name_list, py::arg("dtype") = "string");
            }

            columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
            columns["open"] = open_arr;
            columns["high"] = high_arr;
            columns["low"] = low_arr;
            columns["close"] = close_arr;
            columns["amount"] = amount_arr;
            columns["volume"] = vol_arr;

            return pandas.attr("DataFrame")(columns, py::arg("copy") = false);
          },
          py::arg("with_stock") = false,
          R"(to_df(self, with_stock=False) -> pandas.DataFrame

    Convert to a pandas DataFrame

    :param bool with_stock: include the code and the name of the Stock
    :rtype: pandas.DataFrame)")

          DEF_PICKLE(KData);
}

// Registration group: _KQuery
/*
 * _KQuery.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <common/serialization/Datetime_serialization.h>
#include <common/serialization/KQuery_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_KQuery(py::module& m) {
  int64_t null_int = Null<int64_t>();

  py::class_<KQuery> kquery(m, "Query", "The K-line data query condition");
  kquery.def(py::init<>())
      .def("__str__", to_py_str<KQuery>)
      .def("__repr__", to_py_str<KQuery>)
      .def_property_readonly(
          "start", &KQuery::start,
          "The start index; it is invalid when created with the date query way")
      .def_property_readonly(
          "end", &KQuery::end,
          "The end index; it is invalid when created with the date query way")
      .def_property_readonly(
          "start_datetime", &KQuery::startDatetime,
          "The start date; it is invalid when created with the index query way")
      .def_property_readonly(
          "end_datetime", &KQuery::endDatetime,
          "The end date; it is invalid when created with the index query way")
      .def_property_readonly("query_type", &KQuery::queryType, "The query way")
      .def_property_readonly("ktype", &KQuery::kType,
                             py::return_value_policy::copy,
                             "The K-line type queried")
      .def_property_readonly(
          "recover_type", py::overload_cast<>(&KQuery::recoverType, py::const_),
          "The recovery type")
      .def_property_readonly(
          "ktype_in_sec", &KQuery::kTypeInSeconds,
          "Get the number of the seconds corresponding to the ktype")
      .def("is_right_opening", &KQuery::isRightOpening,
           "Judge whether it is a right-open interval, i.e. the end time is "
           "not specified")
      .def_static("is_valid_ktype", &KQuery::isValidKType,
                  "Judge whether the KType is valid")
      .def_static("is_base_ktype", &KQuery::isBaseKType,
                  "Judge whether it is a basic KType")
      .def_static("is_extra_ktype", &KQuery::isExtraKType,
                  "Judge whether it is an extended KType")
      .def_static("get_base_ktype_list", &KQuery::getBaseKTypeList,
                  "Get all the basic KTypes")
      .def_static("get_extra_ktype_list", &KQuery::getExtraKTypeList,
                  "Get all the extended KTypes")
      .def_static("get_ktype_in_min", &KQuery::getKTypeInMin,
                  "Get the number of the minutes corresponding to the ktype")
      .def_static("get_ktype_in_seconds", &KQuery::getKTypeInSeconds,
                  "Get the number of the seconds corresponding to the ktype")

          DEF_PICKLE(KQuery);

  py::enum_<KQuery::RecoverType>(kquery, "RecoverType")
      .value("NO_RECOVER", KQuery::RecoverType::NO_RECOVER, "No recovery")
      .value("FORWARD", KQuery::RecoverType::FORWARD, "The forward recovery")
      .value("BACKWARD", KQuery::RecoverType::BACKWARD, "The backward recovery")
      .value("EQUAL_FORWARD", KQuery::RecoverType::EQUAL_FORWARD,
             "The equal-ratio forward recovery")
      .value("EQUAL_BACKWARD", KQuery::RecoverType::EQUAL_BACKWARD,
             "The equal-ratio backward recovery")
      .value("INVALID", KQuery::RecoverType::INVALID_RECOVER_TYPE,
             "An invalid type")
      .export_values();

  py::enum_<KQuery::QueryType>(kquery, "QueryType")
      .value("INDEX", KQuery::QueryType::INDEX, "Query by the index way")
      .value("DATE", KQuery::QueryType::DATE, "Query by the date way")
      .value("INVALID", KQuery::QueryType::INVALID, "An invalid type")
      .export_values();

  // An internal enumeration type is used; the enumeration type needs to be
  // registered first, otherwise an error occurs when loading
  kquery.def(py::init<int64_t, int64_t, KQuery::KType, KQuery::RecoverType>(),
             py::arg("start"), py::arg("end") = null_int,
             py::arg("ktype") = KQuery::DAY,
             py::arg("recover_type") = KQuery::NO_RECOVER,
             "\tBuild the condition for getting the K-line data by the index "
             "[start, end) way");

  Datetime null_date;
  kquery.def(py::init<const Datetime&, const Datetime&, KQuery::KType,
                      KQuery::RecoverType>(),
             py::arg("start"), py::arg("end") = null_date,
             py::arg("ktype") = KQuery::DAY,
             py::arg("recover_type") = KQuery::NO_RECOVER,
             "\tBuild the condition for getting the K-line data by the date "
             "[start, end) way");

  kquery.attr("DAY") = "DAY";
  kquery.attr("WEEK") = "WEEK";
  kquery.attr("MONTH") = "MONTH";
  kquery.attr("QUARTER") = "QUARTER";
  kquery.attr("HALFYEAR") = "HALFYEAR";
  kquery.attr("YEAR") = "YEAR";
  kquery.attr("MIN") = "MIN";
  kquery.attr("MIN5") = "MIN5";
  kquery.attr("MIN15") = "MIN15";
  kquery.attr("MIN30") = "MIN30";
  kquery.attr("MIN60") = "MIN60";
  kquery.attr("HOUR2") = "HOUR2";

  kquery.attr("DAY3") = "DAY3";
  kquery.attr("DAY5") = "DAY5";
  kquery.attr("DAY7") = "DAY7";
  kquery.attr("MIN3") = "MIN3";
  kquery.attr("HOUR4") = "HOUR4";
  kquery.attr("HOUR6") = "HOUR6";
  kquery.attr("HOUR12") = "HOUR12";
  kquery.attr("TIMELINE") = "TIMELINE";
  kquery.attr("TRANS") = "TRANS";
}

// Registration group: _KRecord
/*
 * _KRecord.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <common/serialization/KRecord_serialization.h>

#include "DataFrameConversion.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_KRecord(py::module& m) {
  py::class_<KRecord>(m, "KRecord",
                      "The K-line record, composing the K-line data; the "
                      "attributes are readable and writable")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t,
                    price_t, price_t>())

      .def("__str__", to_py_str<KRecord>)
      .def("__repr__", to_py_str<KRecord>)

      .def_readwrite("datetime", &KRecord::datetime, "The time")
      .def_readwrite("open", &KRecord::openPrice, "The open price")
      .def_readwrite("high", &KRecord::highPrice, "The high price")
      .def_readwrite("low", &KRecord::lowPrice, "The low price")
      .def_readwrite("close", &KRecord::closePrice, "The close price")
      .def_readwrite("amount", &KRecord::transAmount, "The amount")
      .def_readwrite("volume", &KRecord::transCount, "The volume")

      .def("is_valid", &KRecord::isValid, "Whether the KRecord is valid")

      .def(py::self == py::self)
      .def(py::self != py::self)

          DEF_PICKLE(KRecord);

  m.def("krecords_to_np", [](const KRecordList& kdata) {
    size_t total = kdata.size();
    HAYAKU_IF_RETURN(total == 0, py::array());

    struct RawData {
      int64_t datetime;  // The converted millisecond timestamp
      double open;
      double high;
      double low;
      double close;
      double amount;
      double volume;
    };

    RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
    for (size_t i = 0; i < total; i++) {
      const KRecord& k = kdata[i];
      data[i].datetime = k.datetime.timestamp() / 1000LL;
      data[i].open = k.openPrice;
      data[i].high = k.highPrice;
      data[i].low = k.lowPrice;
      data[i].close = k.closePrice;
      data[i].amount = k.transAmount;
      data[i].volume = k.transCount;
    }

    // Define the NumPy structured data type
    py::dtype dtype = py::dtype(
        vector_to_python_list<string>(
            {"datetime", "open", "high", "low", "close", "amount", "volume"}),
        vector_to_python_list<string>(
            {"datetime64[ms]", "d", "d", "d", "d", "d", "d"}),
        vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48}), 56);

    return py::array(dtype, total, static_cast<RawData*>(data),
                     py::capsule(data, [](void* p) { std::free(p); }));
  });

  m.def("krecords_to_df", [](const KRecordList& kdata) {
    size_t total = kdata.size();
    if (total == 0) {
      return py::module_::import("pandas").attr("DataFrame")();
    }

    // Create the array
    py::array_t<int64_t> datetime_arr(total);
    py::array_t<double> open_arr(total);
    py::array_t<double> high_arr(total);
    py::array_t<double> low_arr(total);
    py::array_t<double> close_arr(total);
    py::array_t<double> amount_arr(total);
    py::array_t<double> vol_arr(total);

    // Get the buffer and fill the data
    auto datetime_buf = datetime_arr.request();
    auto open_buf = open_arr.request();
    auto high_buf = high_arr.request();
    auto low_buf = low_arr.request();
    auto close_buf = close_arr.request();
    auto amount_buf = amount_arr.request();
    auto vol_buf = vol_arr.request();

    int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
    double* open_ptr = static_cast<double*>(open_buf.ptr);
    double* high_ptr = static_cast<double*>(high_buf.ptr);
    double* low_ptr = static_cast<double*>(low_buf.ptr);
    double* close_ptr = static_cast<double*>(close_buf.ptr);
    double* amount_ptr = static_cast<double*>(amount_buf.ptr);
    double* vol_ptr = static_cast<double*>(vol_buf.ptr);

    auto* ks = kdata.data();
    for (size_t i = 0; i < total; i++) {
      datetime_ptr[i] = ks[i].datetime.timestamp() * 1000LL;
      open_ptr[i] = ks[i].openPrice;
      high_ptr[i] = ks[i].highPrice;
      low_ptr[i] = ks[i].lowPrice;
      close_ptr[i] = ks[i].closePrice;
      amount_ptr[i] = ks[i].transAmount;
      vol_ptr[i] = ks[i].transCount;
    }

    // Build the DataFrame
    py::dict columns;
    columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
    columns["open"] = open_arr;
    columns["high"] = high_arr;
    columns["low"] = low_arr;
    columns["close"] = close_arr;
    columns["amount"] = amount_arr;
    columns["volume"] = vol_arr;

    return py::module_::import("pandas").attr("DataFrame")(
        columns, py::arg("copy") = false);
  });

  m.def("df_to_krecords", df_to_krecords,
        R"(df_to_krecords(df: pd.DataFrame[, columns: dict]) -> KRecordList

    Convert a DataFrame to a KRecordList; the column names must be specified in order, defaulting to: ("datetime", "open", "high", "low", "close", "amount", "volume")

    :param DataFrame df: the input DataFrame
    :param dict columns: specify the column names of the DataFrame, corresponding to the member variable names of the KRecord
    :return: the converted KRecordList)",
        py::arg("df"),
        py::arg("columns") = StringList{"datetime", "open", "high", "low",
                                        "close", "amount", "volume"});
}

// Registration group: _MarketInfo
/*
 * _MarketInfo.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <common/serialization/MarketInfo_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_MarketInfo(py::module& m) {
  py::class_<MarketInfo>(m, "MarketInfo", "The market information record")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&, const string&,
                    const Datetime&, TimeDelta, TimeDelta, TimeDelta,
                    TimeDelta>())

      .def("__str__", &MarketInfo::toString)
      .def("__repr__", &MarketInfo::toString)

      .def_property_readonly(
          "market", py::overload_cast<>(&MarketInfo::market, py::const_),
          py::return_value_policy::copy,
          "The market identifier (e.g.: the Shanghai market 'SH', the Shenzhen "
          "market 'SZ')")
      .def_property_readonly(
          "name", py::overload_cast<>(&MarketInfo::name, py::const_),
          py::return_value_policy::copy, "The full name of the market")
      .def_property_readonly(
          "description",
          py::overload_cast<>(&MarketInfo::description, py::const_),
          py::return_value_policy::copy, "The description")
      .def_property_readonly("code",
                             py::overload_cast<>(&MarketInfo::code, py::const_),
                             py::return_value_policy::copy,
                             "The main index code corresponding to this "
                             "market, used to get the trading calendar")

      .def_property_readonly(
          "last_datetime", &MarketInfo::lastDate,
          "The last trading date of the K-line data of this market")
      .def_property_readonly("open_time1", &MarketInfo::openTime1,
                             "The open time 1")
      .def_property_readonly("close_time1", &MarketInfo::closeTime1,
                             "The close time 1")
      .def_property_readonly("open_time2", &MarketInfo::openTime2,
                             "The open time 2")
      .def_property_readonly("close_time2", &MarketInfo::closeTime2,
                             "The close time 2")

          DEF_PICKLE(MarketInfo);
}

// Registration group: _Stock
/*
 * _Stock.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <common/serialization/Stock_serialization.h>
#include <data/KData.h>

#include "DataFrameConversion.h"

using namespace hayaku;
namespace py = pybind11;

// BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(getIndex_overloads, getIndex, 1, 2)

KRecord (Stock::*getKRecord1)(size_t pos, const KQuery::KType& kType) const =
    &Stock::getKRecord;
KRecord (Stock::*getKRecord2)(
    const Datetime&, const KQuery::KType& kType) const = &Stock::getKRecord;

void export_Stock(py::module& m) {
  py::class_<Stock>(m, "Stock", "The security object")
      .def(py::init<>())
      .def(py::init<const string&, const string&, const string&>(),
           py::arg("market"), py::arg("code"), py::arg("name"))
      .def(py::init<const Stock&>())

      .def("__str__", &Stock::toString)
      .def("__repr__", &Stock::toString)

      .def_property_readonly("id", &Stock::id, "The internal id")
      .def_property("market", py::overload_cast<>(&Stock::market, py::const_),
                    py::overload_cast<const string&>(&Stock::market),
                    py::return_value_policy::copy,
                    "The market abbreviation it belongs to; the market "
                    "abbreviation is the unique identifier of the market")
      .def_property("code", py::overload_cast<>(&Stock::code, py::const_),
                    py::overload_cast<const string&>(&Stock::code),
                    py::return_value_policy::copy, "The security code")
      .def_property_readonly(
          "market_code", py::overload_cast<>(&Stock::market_code, py::const_),
          "The market abbreviation + the security code, e.g.: sh000001")
      .def_property("name", py::overload_cast<>(&Stock::name, py::const_),
                    py::overload_cast<const string&>(&Stock::name),
                    py::return_value_policy::copy, "The security name")
      .def_property("type", py::overload_cast<>(&Stock::type, py::const_),
                    py::overload_cast<uint32_t>(&Stock::type),
                    "The security type, see: constant")
      .def_property("valid", py::overload_cast<>(&Stock::valid, py::const_),
                    py::overload_cast<bool>(&Stock::valid),
                    "Whether the security is currently valid")
      .def_property("start_datetime",
                    py::overload_cast<>(&Stock::startDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&Stock::startDatetime),
                    py::return_value_policy::copy,
                    "The start date of the security")
      .def_property("last_datetime",
                    py::overload_cast<>(&Stock::lastDatetime, py::const_),
                    py::overload_cast<const Datetime&>(&Stock::lastDatetime),
                    py::return_value_policy::copy,
                    "The last date of the security")
      .def_property("tick", py::overload_cast<>(&Stock::tick, py::const_),
                    py::overload_cast<price_t>(&Stock::tick),
                    "The minimum tick")
      .def_property("tick_value",
                    py::overload_cast<>(&Stock::tickValue, py::const_),
                    py::overload_cast<price_t>(&Stock::tickValue),
                    "The minimum tick value")
      .def_property_readonly("unit", &Stock::unit,
                             "The per-unit value = tickValue / tick")
      .def_property(
          "precision", py::overload_cast<>(&Stock::precision, py::const_),
          py::overload_cast<int>(&Stock::precision), "The price precision")
      .def_property("atom", py::overload_cast<>(&Stock::atom, py::const_),
                    py::overload_cast<double>(&Stock::atom),
                    "The minimum trading quantity, the same as min_tradeNumber")
      .def_property("min_trade_number",
                    py::overload_cast<>(&Stock::minTradeNumber, py::const_),
                    py::overload_cast<double>(&Stock::minTradeNumber),
                    "The minimum trading quantity")
      .def_property("max_trade_number",
                    py::overload_cast<>(&Stock::maxTradeNumber, py::const_),
                    py::overload_cast<double>(&Stock::maxTradeNumber),
                    "The maximum trading quantity")

      .def("is_null", &Stock::isNull, R"(is_null(self)

        Whether it is Null

        :rtype: bool)")

      .def("is_buffer", &Stock::isBuffer,
           R"(Whether the K-line data of the specified type is cached)")

      .def(
          "get_index_range",
          [](const Stock& self, const KQuery& query) {
            size_t start, end;
            self.getIndexRange(query, start, end);
            return py::make_tuple(start, end);
          },
          R"(get_index_range(self, query) -> (size_t, size_t)

        Get the corresponding K-line position range [start_pos, end_pos) according to the condition specified by the KQuery

        :param query [in] the specified query condition
        :return (start_pos, end_pos)")

      .def("get_kdata", &Stock::getKData, R"(get_kdata(self, query)

        Get the K-line data

        :param Query query: the query condition
        :return: the K-line data satisfying the query condition
        :rtype: KData)")

      .def("get_timeline_list", &Stock::getTimeLineList,
           R"(get_timeline_list(self, query)

        Get the time-line

        :param Query query: the query condition(the K-line type and the recovery type parameters in the query condition are useless at this time)
        :rtype: TimeLineList)")

      .def("get_trans_list", &Stock::getTransList,
           R"(get_trans_list(self, query)

        Get the historical tick data

        :param Query query: the query condition(the K-line type and the recovery type parameters in the query condition are useless at this time)
        :rtype: TransList)")

      .def("get_count", &Stock::getCount, py::arg("ktype") = KQuery::DAY,
           R"(get_count(self, [ktype=Query.DAY])

        Get the amount of the K-line data of the different types

        :param Query.KType ktype: the K-line data category
        :return: the number of the K-line records
        :rtype: int)")

      .def("get_market_value", &Stock::getMarketValue,
           R"(get_market_value(self, date, ktype)

        Get the market value at the specified moment, i.e. the close price of the last record less than or equal to the specified moment

        :param Datetime date: the specified moment
        :param Query.KType ktype: the K-line data category
        :return: the market value at the specified moment
        :rtype: float)")

      .def("get_krecord", getKRecord1, py::arg("pos"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, pos[, ktype=Query.DAY])

        Get the K-line data record at the specified index, without the out-of-bounds check

        :param int pos: the specified index position
        :param Query.KType ktype: the K-line data category
        :return: the K-line record
        :rtype: KRecord)")

      .def("get_krecord", getKRecord2, py::arg("date"),
           py::arg("ktype") = KQuery::DAY,
           R"(get_krecord(self, date[, ktype=Query.DAY])

        Get the KRecord at the specified moment according to the data type (the daily line, the weekly line, etc.)

        :param Datetime date: the specified date-time
        :param Query.KType ktype: the K-line data category
        :return: the K-line record
        :rtype: KRecord)")

      .def("get_krecord_list", &Stock::getKRecordList,
           R"(get_krecord_list(self, start, end,
          ktype)

        Get the K-line records [start, end); it is generally not used directly.

        :param int start: the start position
        :param int end: the end position
        :param Query.KType ktype: the K-line category
        :return: the K-line record list
        :rtype: KRecordList)")

      .def("get_datetime_list", &Stock::getDatetimeList,
           R"(get_datetime_list(self, query)

        Get the date list

        :param Query query: the query condition
        :rtype: DatetimeList)")

      .def("get_finance_info", &Stock::getFinanceInfo, R"(get_finance_info(self)

        Get the current finance information

        :rtype: Parameter)")

      .def("realtime_update", &Stock::realtimeUpdate, py::arg("krecord"),
           py::arg("ktype") = KQuery::DAY,
           R"(realtime_update(self, krecord)

        Only used to update the daily-line data in the cache

        :param KRecord krecord: the newly added real-time K-line record
        :param KQuery.KType ktype: the K-line type)")

      .def("get_last_update_time", &Stock::getLastUpdateTime,
           py::arg("ktype") = KQuery::DAY,
           R"(get_last_update_time(self, [ktype=Query.DAY])

        Get the last update moment of the specified type of the K-line data. In the client mode, the ordinary securities are forwarded to the master process
        to take its buffer refresh moment; the temporary securities (setKRecordList) return the local writing moment.

        :param KQuery.KType ktype: the K-line type
        :rtype: Datetime)")

      .def("get_weight", &Stock::getWeight, py::arg("start") = Datetime::min(),
           py::arg("end") = Datetime(),
           R"(get_weight(self, [start, end])

        Get the dividend information within the specified time range [start, end). When the start and the end moments are not specified, get all the dividend records.

        :param Datetime start: the start moment
        :param Datetime end: the end moment
        :rtype: StockWeightList)")

      .def(
          "get_belong_to_block_list",
          [](Stock& stk, const py::object& category) {
            string c_category;
            if (!category.is_none()) {
              c_category = category.cast<string>();
            }
            return stk.getBelongToBlockList(c_category);
          },
          py::arg("category") = py::none(),
          R"(get_belong_to_block_list(self[, category=None])

      Get the list of the belonging blocks

      :param str category: the specified block category; when it is None, return the belonging blocks under all the block categories
      :rtype: list)")

      .def(
          "get_history_finance",
          [](const Stock& stk) {
            auto finances = stk.getHistoryFinance();
            py::list ret;
            for (const auto& f : finances) {
              ret.append(py::make_tuple(f.fileDate, f.reportDate, f.values));
            }
            return ret;
          },
          R"(get_history_finance(self)

        Get all the historical finance records; query field information through the corresponding DataEngine methods.
        For the daily use, it is recommended to use the FINANCE indicator directly to get the finance data)")

      .def("get_trading_calendar", &Stock::getTradingCalendar, py::arg("query"),
           R"(get_trading_calendar(self, query)

        Get the trading calendar of its own market (not its own trading dates)

        :param KQuery query: the Query condition
        :return: the date list
        :rtype: DatetimeList)")

      .def("load_kdata_to_buffer", &Stock::loadKDataToBuffer,
           R"(load_kdata_to_buffer(self,
          ktype)

        Load the K-line data of the specified category into the memory cache; if the cache already exists, you need to release_kdata_buffer first

        :param Query.KType ktype: the K-line type)")

      .def("release_kdata_buffer", &Stock::releaseKDataBuffer,
           R"(release_kdata_buffer(self,
          ktype)

        Release the memory K-line data of the specified category

        :param Query.KType ktype: the K-line type)")

      .def(
          "set_krecord_list",
          [](Stock& self, const py::object& obj, const KQuery::KType& ktype) {
            if (py::isinstance<KRecordList>(obj)) {
              const auto& ks = obj.cast<const KRecordList&>();
              self.setKRecordList(ks, ktype);
            } else if (py::isinstance<py::sequence>(obj)) {
              auto seq = obj.cast<py::sequence>();
              auto ks = python_list_to_vector<KRecord>(seq);
              self.setKRecordList(std::move(ks), ktype);
            } else {
              HAYAKU_THROW("Unusable input data type");
            }
          },
          py::arg("krecord_list"), py::arg("ktype") = KQuery::DAY,
          R"(set_krecord_list(self, krecord_list[, ktype=Query.DAY])

      "Call with caution!!! Set the current memory KRecordList directly; it is only used to set the K-line data for the external Stocks that need to be added temporarily

      :param krecord_list: KRecordList or list of KRecord
      :param Query.KType ktype: the K-line category)")

      .def(
          "set_kdata_from_df",
          [](Stock& self, const py::object& df, const StringList& cols,
             const KQuery::KType& ktype) {
            auto ks = df_to_krecords(df, cols);
            self.setKRecordList(std::move(ks), ktype);
          },
          py::arg("df"),
          py::arg("cols") = StringList{"datetime", "open", "high", "low",
                                       "close", "amount", "volume"},
          py::arg("ktype") = KQuery::DAY,
          R"(set_kdata_from_df(self, df, cols, [ktype=Query.DAY])

      Call with caution!!! Set the current memory data directly, which means the basic data of the Stock is changed.
      Get the KRecordList from the DataFrame and set it to the current Stock. df must specify the column names in order, defaulting to: ("datetime", "open", "high", "low", "close", "amount", "volume"))")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)

          DEF_PICKLE(Stock);
}

// Registration group: _StockTypeInfo
/*
 * _StockTypeInfo.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <common/serialization/StockTypeInfo_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_StockTypeInfo(py::module& m) {
  py::class_<StockTypeInfo>(m, "StockTypeInfo", "The stock type detail record")
      .def(py::init<>())
      .def(py::init<uint32_t, const string&, price_t, price_t, int, double,
                    double>())

      .def("__str__", &StockTypeInfo::toString)
      .def("__repr__", &StockTypeInfo::toString)

      .def_property_readonly("type", &StockTypeInfo::type, "The security type")

      .def_property_readonly(
          "description",
          py::overload_cast<>(&StockTypeInfo::description, py::const_),
          "The description information")

      .def_property_readonly("tick", &StockTypeInfo::tick, "The minimum tick")
      .def_property_readonly("tick_value", &StockTypeInfo::tickValue,
                             "The price of each tick")
      .def_property_readonly("unit", &StockTypeInfo::unit,
                             "The price of each minimum change, i.e. the unit "
                             "price = tick_value/tick")
      .def_property_readonly("precision", &StockTypeInfo::precision,
                             "The price precision")
      .def_property_readonly("min_trade_num", &StockTypeInfo::minTradeNumber,
                             "The minimum trading quantity per order")
      .def_property_readonly("max_trade_num", &StockTypeInfo::maxTradeNumber,
                             "The maximum trading quantity per order")

          DEF_PICKLE(StockTypeInfo);
}

// Registration group: _StockWeight
/*
 * StockWeight.cpp
 *
 *  Created on: 2012-9-28
 *      Author: fasiondog
 */

#include <common/serialization/StockWeight_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_StockWeight(py::module& m) {
  py::class_<StockWeight>(m, "StockWeight", "The dividend record")
      .def(py::init<>())
      .def(py::init<const Datetime&>())
      .def(py::init<const Datetime&, price_t, price_t, price_t, price_t,
                    price_t, price_t, price_t, price_t>())

      .def("__str__", to_py_str<StockWeight>)
      .def("__repr__", to_py_str<StockWeight>)

      .def_property_readonly("datetime", &StockWeight::datetime,
                             "The dividend date")
      .def_property_readonly("count_as_gift", &StockWeight::countAsGift,
                             "X shares sent per 10 shares")
      .def_property_readonly("count_for_sell", &StockWeight::countForSell,
                             "X shares allotted per 10 shares")
      .def_property_readonly("price_for_sell", &StockWeight::priceForSell,
                             "The allotment price")
      .def_property_readonly("bonus", &StockWeight::bonus,
                             "The dividend per 10 shares")
      .def_property_readonly("increasement", &StockWeight::increasement,
                             "X shares converted per 10 shares")
      .def_property_readonly("total_count", &StockWeight::totalCount,
                             "The total share capital (10,000 shares)")
      .def_property_readonly("free_count", &StockWeight::freeCount,
                             "The circulating shares (10,000 shares)")
      .def_property_readonly("suogu", &StockWeight::suogu,
                             "The share expansion/contraction ratio")

          DEF_PICKLE(StockWeight);

  m.def("weights_to_np", [](const StockWeightList& sw) {
    size_t total = sw.size();
    HAYAKU_IF_RETURN(total == 0, py::array());

    struct RawData {
      int64_t date;  // The date (only up to the day)
      double countAsGift;
      double countForSell;
      double priceForSell;
      double bonus;
      double countOfIncreasement;
      double totalCount;
      double freeCount;
      double suogu;
    };

    // Allocate the memory with malloc
    RawData* data = static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
    for (size_t i = 0, len = sw.size(); i < len; i++) {
      const StockWeight& w = sw[i];
      data[i].date = (w.datetime() - Datetime(1970, 1, 1)).days();
      data[i].countAsGift = w.countAsGift();
      data[i].countForSell = w.countForSell();
      data[i].priceForSell = w.priceForSell();
      data[i].bonus = w.bonus();
      data[i].countOfIncreasement = w.increasement();
      data[i].totalCount = w.totalCount();
      data[i].freeCount = w.freeCount();
      data[i].suogu = w.suogu();
    }

    py::dtype dtype = py::dtype(
        vector_to_python_list<string>(
            {"date", "countAsGift", "countForSell", "priceForSell", "bonus",
             "countOfIncreasement", "totalCount", "freeCount", "suogu"}),
        vector_to_python_list<string>(
            {"datetime64[D]", "d", "d", "d", "d", "d", "d", "d", "d"}),
        vector_to_python_list<int64_t>({0, 8, 16, 24, 32, 40, 48, 56, 64}), 72);

    // Manage the memory with the capsule
    return py::array(dtype, total, static_cast<RawData*>(data),
                     py::capsule(data, [](void* p) { std::free(p); }));
  });

  m.def("weights_to_df", [](const StockWeightList& sw) {
    size_t total = sw.size();
    if (total == 0) {
      return py::module_::import("pandas").attr("DataFrame")();
    }

    // Create the array
    py::array_t<int64_t> datetime_arr(total);
    py::array_t<double> countAsGift_arr(total);
    py::array_t<double> countForSell_arr(total);
    py::array_t<double> priceForSell_arr(total);
    py::array_t<double> bonus_arr(total);
    py::array_t<double> countOfIncreasement_arr(total);
    py::array_t<double> totalCount_arr(total);
    py::array_t<double> freeCount_arr(total);
    py::array_t<double> suogu_arr(total);

    // Get the buffer and fill the data
    auto datetime_buf = datetime_arr.request();
    auto countAsGift_buf = countAsGift_arr.request();
    auto countForSell_buf = countForSell_arr.request();
    auto priceForSell_buf = priceForSell_arr.request();
    auto bonus_buf = bonus_arr.request();
    auto countOfIncreasement_buf = countOfIncreasement_arr.request();
    auto totalCount_buf = totalCount_arr.request();
    auto freeCount_buf = freeCount_arr.request();
    auto suogu_buf = suogu_arr.request();

    int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
    double* countAsGift_ptr = static_cast<double*>(countAsGift_buf.ptr);
    double* countForSell_ptr = static_cast<double*>(countForSell_buf.ptr);
    double* priceForSell_ptr = static_cast<double*>(priceForSell_buf.ptr);
    double* bonus_ptr = static_cast<double*>(bonus_buf.ptr);
    double* countOfIncreasement_ptr =
        static_cast<double*>(countOfIncreasement_buf.ptr);
    double* totalCount_ptr = static_cast<double*>(totalCount_buf.ptr);
    double* freeCount_ptr = static_cast<double*>(freeCount_buf.ptr);
    double* suogu_ptr = static_cast<double*>(suogu_buf.ptr);

    for (size_t i = 0; i < total; i++) {
      const StockWeight& w = sw[i];
      datetime_ptr[i] = w.datetime().timestamp() * 1000LL;
      countAsGift_ptr[i] = w.countAsGift();
      countForSell_ptr[i] = w.countForSell();
      priceForSell_ptr[i] = w.priceForSell();
      bonus_ptr[i] = w.bonus();
      countOfIncreasement_ptr[i] = w.increasement();
      totalCount_ptr[i] = w.totalCount();
      freeCount_ptr[i] = w.freeCount();
      suogu_ptr[i] = w.suogu();
    }

    // Build the DataFrame
    py::dict columns;
    columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
    columns["countAsGift"] = countAsGift_arr;
    columns["countForSell"] = countForSell_arr;
    columns["priceForSell"] = priceForSell_arr;
    columns["bonus"] = bonus_arr;
    columns["countOfIncreasement"] = countOfIncreasement_arr;
    columns["totalCount"] = totalCount_arr;
    columns["freeCount"] = freeCount_arr;
    columns["suogu"] = suogu_arr;

    return py::module_::import("pandas").attr("DataFrame")(
        columns, py::arg("copy") = false);
  });
}

// Registration group: _StrategyContext
/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include <data/StrategyContext.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_StrategyContext(py::module& m) {
  py::class_<StrategyContext>(m, "StrategyContext", "The strategy context")
      .def(py::init<>())
      .def(py::init<const vector<string>&>())
      .def(py::init<const vector<string>&, const vector<KQuery::KType>&,
                    const unordered_map<string, int64_t>&>(),
           py::arg("stock_list"), py::arg("ktype_list"),
           py::arg("preload_num") = unordered_map<string, int64_t>(),
           R"(__init__(self, stock_list, ktype_list, [preload_num={}])

  Create the strategy context

  :param stock_list: the security code list to load, e.g.: ["sz000001", "sz000002"]; if it contains 'ALL', it means loading all
  :param ktype_list: the K-line type list to load, e.g.: ["day", "min"]; when unspecified, take the default value configured in the global configuration file
  :param preload_num: the preloading quantity, defaulting to empty, e.g.: {"min_max": 100, "day_max": 200}. When unspecified, take the default value configured in the global configuration file
  :return: the strategy context object)")

      .def("__str__", &StrategyContext::str)
      .def("__repr__", &StrategyContext::str)

      .def_property_readonly(
          "start_datetime",
          py::overload_cast<>(&StrategyContext::startDatetime, py::const_),
          py::return_value_policy::copy, "The start date")
      .def_property(
          "stock_list",
          py::overload_cast<>(&StrategyContext::getStockCodeList, py::const_),
          &StrategyContext::setStockCodeList, py::return_value_policy::copy,
          "The stock code list")
      .def_property(
          "ktype_list",
          py::overload_cast<>(&StrategyContext::getKTypeList, py::const_),
          &StrategyContext::setKTypeList, py::return_value_policy::copy,
          "The needed K-line types")
      .def_property(
          "preload_num",
          py::overload_cast<>(&StrategyContext::getPreloadNum, py::const_),
          &StrategyContext::setPreloadNum, py::return_value_policy::copy,
          "The preloading quantity")

      .def("empty", &StrategyContext::empty,
           "Whether the security code list in the context is empty");
}

// Registration group: _TimeLineRecord
/*
 * _KRecord.cpp
 *
 *  Created on: 2019-1-27
 *      Author: fasiondog
 */

#include <common/serialization/TimeLineRecord_serialization.h>
#include <pybind11/numpy.h>
#include <pybind11/pybind11.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TimeLineRecord(py::module& m) {
  py::class_<TimeLineRecord>(
      m, "TimeLineRecord",
      "The time-line record; the attributes are readable and writable")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t>())
      .def("__str__", to_py_str<TimeLineRecord>)
      .def("__repr__", to_py_str<TimeLineRecord>)
      .def_readwrite("date", &TimeLineRecord::datetime, "The date-time")
      .def_readwrite("price", &TimeLineRecord::price, "The price")
      .def_readwrite("vol", &TimeLineRecord::vol, "The volume")
      .def(py::self == py::self)

          DEF_PICKLE(TimeLineRecord);

  m.def(
      "timeline_to_np",
      [](const TimeLineList& timeline) {
        size_t total = timeline.size();
        HAYAKU_IF_RETURN(total == 0, py::array());

        struct RawData {
          int64_t datetime;  // The converted millisecond timestamp
          double price;
          double vol;
        };

        // Allocate the memory with malloc
        RawData* data =
            static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, len = timeline.size(); i < len; i++) {
          const TimeLineRecord& record = timeline[i];
          data[i].datetime = record.datetime.timestamp() * 1000LL;
          data[i].price = record.price;
          data[i].vol = record.vol;
        }

        // Define the NumPy structured data type
        auto dtype = py::dtype(
            vector_to_python_list<string>({"datetime", "price", "vol"}),
            vector_to_python_list<string>({"datetime64[ns]", "d", "d"}),
            vector_to_python_list<int64_t>({0, 8, 16}), 24);

        // Manage the memory with the capsule
        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
      },
      "Convert the time-line records to a NumPy tuple");

  m.def(
      "timeline_to_df",
      [](const TimeLineList& timeline) {
        size_t total = timeline.size();
        if (total == 0) {
          return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the array
        py::array_t<int64_t> datetime_arr(total);
        py::array_t<double> price_arr(total);
        py::array_t<double> vol_arr(total);

        // Get the buffer and fill the data
        auto datetime_buf = datetime_arr.request();
        auto price_buf = price_arr.request();
        auto vol_buf = vol_arr.request();

        int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
        double* price_ptr = static_cast<double*>(price_buf.ptr);
        double* vol_ptr = static_cast<double*>(vol_buf.ptr);

        for (size_t i = 0; i < total; i++) {
          const TimeLineRecord& record = timeline[i];
          datetime_ptr[i] = record.datetime.timestamp() * 1000LL;
          price_ptr[i] = record.price;
          vol_ptr[i] = record.vol;
        }

        // Build the DataFrame
        py::dict columns;
        columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
        columns["price"] = price_arr;
        columns["vol"] = vol_arr;

        return py::module_::import("pandas").attr("DataFrame")(
            columns, py::arg("copy") = false);
      },
      "Convert the time-line records to a DataFrame");
}

// Registration group: _TransRecord
/*
 * _KRecord.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include <common/serialization/TransRecord_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#if defined(_MSC_VER)
#pragma warning(disable : 4267)
#endif

void export_TransRecord(py::module& m) {
  py::class_<TransRecord>(m, "TransRecord")
      .def(py::init<>())
      .def(py::init<const Datetime&, price_t, price_t, int>())
      .def("__str__", to_py_str<TransRecord>)
      .def("__repr__", to_py_str<TransRecord>)
      .def_readwrite("date", &TransRecord::datetime, "The time")
      .def_readwrite("price", &TransRecord::price, "The price")
      .def_readwrite("vol", &TransRecord::vol, "The volume")
      .def_readwrite("direct", &TransRecord::direct,
                     "The nature of the buy/sell order: 1--sell 0--buy 2--call "
                     "auction, others unknown")
      .def(py::self == py::self)

          DEF_PICKLE(TransRecord);

  m.def(
      "translist_to_np",
      [](const TransList& trans) {
        size_t total = trans.size();
        HAYAKU_IF_RETURN(total == 0, py::array());

        struct RawData {
          int64_t datetime;
          double price;
          double vol;
          int64_t direct;
        };

        // Allocate the memory with malloc
        RawData* data =
            static_cast<RawData*>(std::malloc(total * sizeof(RawData)));
        for (size_t i = 0, len = trans.size(); i < len; i++) {
          const TransRecord& record = trans[i];
          data[i].datetime = record.datetime.timestamp() * 1000LL;
          data[i].price = record.price;
          data[i].vol = record.vol;
          data[i].direct = record.direct;
        }

        // Define the NumPy structured data type
        auto dtype = py::dtype(
            vector_to_python_list<string>(
                {"datetime", "price", "vol", "direct"}),
            vector_to_python_list<string>({"datetime64[ns]", "d", "d", "i4"}),
            vector_to_python_list<int64_t>({0, 8, 16, 24}), 32);

        // Manage the memory with the capsule
        return py::array(dtype, total, static_cast<RawData*>(data),
                         py::capsule(data, [](void* p) { std::free(p); }));
      },
      "Convert the tick records to a NumPy tuple");

  m.def(
      "translist_to_df",
      [](const TransList& trans) {
        size_t total = trans.size();
        if (total == 0) {
          return py::module_::import("pandas").attr("DataFrame")();
        }

        // Create the array
        py::array_t<int64_t> datetime_arr(total);
        py::array_t<double> price_arr(total);
        py::array_t<double> vol_arr(total);
        py::array_t<int64_t> direct_arr(total);

        // Get the buffer and fill the data
        auto datetime_buf = datetime_arr.request();
        auto price_buf = price_arr.request();
        auto vol_buf = vol_arr.request();
        auto direct_buf = direct_arr.request();

        int64_t* datetime_ptr = static_cast<int64_t*>(datetime_buf.ptr);
        double* price_ptr = static_cast<double*>(price_buf.ptr);
        double* vol_ptr = static_cast<double*>(vol_buf.ptr);
        int64_t* direct_ptr = static_cast<int64_t*>(direct_buf.ptr);

        for (size_t i = 0; i < total; i++) {
          const TransRecord& record = trans[i];
          datetime_ptr[i] = record.datetime.timestamp() * 1000LL;
          price_ptr[i] = record.price;
          vol_ptr[i] = record.vol;
          direct_ptr[i] = record.direct;
        }

        // Build the DataFrame
        py::dict columns;
        columns["datetime"] = datetime_arr.attr("astype")("datetime64[ns]");
        columns["price"] = price_arr;
        columns["vol"] = vol_arr;
        columns["direct"] = direct_arr;
        return py::module_::import("pandas").attr("DataFrame")(
            columns, py::arg("copy") = false);
      },
      "Convert the tick records to a DataFrame");
}

// Registration group: data_main
/*
 * Copyright (c) 2026 hikyuu.org
 */

#include "common/PybindSupport.h"

namespace py = pybind11;

void export_DataEngine(py::module& m);

void export_data_main(py::module& m) { export_DataEngine(m); }

// Registration group: _BaseInfoDriver
/*
 * _BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include "storage/BaseInfoDriverWrap.h"

using namespace hayaku;
namespace py = pybind11;

static string BaseInfoDriver_to_str(const BaseInfoDriver& v) {
  std::stringstream out;
  out << v;
  return out.str();
}

void export_BaseInfoDriver(py::module& m) {
  py::class_<BaseInfoDriver, BaseInfoDriverPtr, PyBaseInfoDriver>(
      m, "BaseInfoDriver",
      R"(The basic information data driver base class

    The subclass interfaces:
        - _init(self) (Required)
        - getAllStockInfo(self) (Required)
        - getStockInfo(self, market, code) (Required)
        - getMarketInfo(self, market) (Required)
        - getAllMarketInfo(self) (Required)
        - getAllStockTypeInfo(self) (Required)
        - getStockTypeInfo(self, type) (Required)
        - getAllHolidays(self) (Required)
        - getAllZhBond10(self) (Required)
        - getStockWeightList(self, market, code, start, end)
        - getAllStockWeightList(self)
        - getHistoryFinance(self, market, code, start, end)
        - getHistoryFinanceField(self)
        - getFinanceInfo(self, market, code)
    )")
      .def(py::init<const string&>(), R"(Initialize

    :param str name: the driver name)")
      .def_property_readonly("name", &BaseInfoDriver::name,
                             py::return_value_policy::copy, "The driver name")
      .def("__str__", BaseInfoDriver_to_str)
      .def("__repr__", BaseInfoDriver_to_str)

      .def("get_param", &BaseInfoDriver::getParam<boost::any>,
           "Get the specified parameter")
      .def("set_param",
           static_cast<void (BaseInfoDriver::*)(const std::string&,
                                                const boost::any&)>(
               &BaseInfoDriver::setParam),
           "Set the specified parameter")
      .def("have_param", &BaseInfoDriver::haveParam,
           "Whether the specified parameter exists")

      .def("_init", &BaseInfoDriver::_init,
           "[Subclass interface (Required)] Initialize the driver")
      .def("getAllStockInfo", &BaseInfoDriver::getAllStockInfo,
           "[Subclass interface (Required)] Get the detailed information of "
           "all the stocks")
      .def(
          "getStockInfo", &BaseInfoDriver::getStockInfo, py::arg("market"),
          py::arg("code"),
          R"([Subclass interface (Required)] Get the specified security information

    :param str market: the market abbreviation
    :param str code: the security code)")
      .def("getStockWeightList", &BaseInfoDriver::getStockWeightList,
           py::arg("market"), py::arg("code"), py::arg("start"), py::arg("end"),
           R"(Get the dividend list within the specified date range [start, end)

    :param str market: the market abbreviation
    :param str code: the security code
    :param Datetime start: the start date
    :param Datetime end: the end date)")
      .def("getAllStockWeightList", &BaseInfoDriver::getAllStockWeightList,
           "Get the dividend lists of all the stocks")
      .def("getHistoryFinance", &BaseInfoDriver::getHistoryFinance,
           py::arg("market"), py::arg("code"), py::arg("start"), py::arg("end"),
           R"(Get the historical finance information

    :param str market: the market abbreviation
    :param str code: the security code
    :param Datetime start: the start date of the finance report publishing
    :param Datetime end: the end date of the query)")
      .def("getHistoryFinanceField", &BaseInfoDriver::getHistoryFinanceField,
           "Get the indexes and the names of the historical finance "
           "information fields")
      .def("getFinanceInfo", &BaseInfoDriver::getFinanceInfo, py::arg("market"),
           py::arg("code"),
           R"(Get the current finance information

    :param str market: the market identifier
    :param str code: the security code)")
      .def("getMarketInfo", &BaseInfoDriver::getMarketInfo, py::arg("market"),
           R"([Subclass interface (Required)] Get the specified MarketInfo

    :param str market: the market abbreviation
    :return: if it is not found, return Null<MarketInfo>())")
      .def("getAllMarketInfo", &BaseInfoDriver::getAllMarketInfo,
           "[Subclass interface (Required)] Get all the market information")
      .def("getAllStockTypeInfo", &BaseInfoDriver::getAllStockTypeInfo,
           "[Subclass interface (Required)] Get all the security type "
           "information")
      .def(
          "getStockTypeInfo", &BaseInfoDriver::getStockTypeInfo,
          py::arg("type"),
          R"([Subclass interface (Required)] Get the detailed information of the corresponding security type

    :param int type: the security type
    :return: the corresponding security type information; if it does not exist, return Null<StockTypeInfo>())")
      .def("getAllHolidays", &BaseInfoDriver::getAllHolidays,
           "[Subclass interface (Required)] Get all the holiday dates")
      .def("getAllZhBond10", &BaseInfoDriver::getAllZhBond10,
           "[Subclass interface (Required)] Get all the Chinese 10-year "
           "treasury bond information");
}

// Registration group: _BlockInfoDriver
/*
 * _BlockInfoDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include "storage/BlockInfoDriverWrap.h"

#include <data/storage/BlockInfoDriver.h>

using namespace hayaku;
namespace py = pybind11;

static string BlockInfoDriver_to_str(const BlockInfoDriver& v) {
  std::stringstream out;
  out << v;
  return out.str();
}

BlockList (BlockInfoDriver::*get_block_list_1)(const string&) =
    &BlockInfoDriver::getBlockList;
BlockList (BlockInfoDriver::*get_block_list_2)() =
    &BlockInfoDriver::getBlockList;

void export_BlockInfoDriver(py::module& m) {
  py::class_<BlockInfoDriver, BlockInfoDriverPtr, PyBlockInfoDriver>(
      m, "BlockInfoDriver",
      R"(The block data driver base class

    The subclass interfaces:
        - _init(self) (Required)
        - getAllCategory(self) (Required)
        - getBlock(self, category, name) (Required)
        - getBlockList(self, category=None) (Required)
        - save(self, block) (Required)
        - remove(self, category, name) (Required)
    )")
      .def(py::init<const string&>(), R"(Initialize

    :param str name: the driver name)")
      .def_property_readonly("name", &BlockInfoDriver::name,
                             py::return_value_policy::copy, "The driver name")
      .def("__str__", BlockInfoDriver_to_str)
      .def("__repr__", BlockInfoDriver_to_str)

      .def("get_param", &BlockInfoDriver::getParam<boost::any>,
           "Get the specified parameter")
      .def("set_param",
           static_cast<void (BlockInfoDriver::*)(const std::string&,
                                                 const boost::any&)>(
               &BlockInfoDriver::setParam),
           "Set the specified parameter")
      .def("have_param", &BlockInfoDriver::haveParam,
           "Whether the specified parameter exists")

      .def("_init", &BlockInfoDriver::_init,
           "[Subclass interface (Required)] Initialize the driver")
      .def("getAllCategory", &BlockInfoDriver::getAllCategory,
           "[Subclass interface (Required)] Get all the block categories")
      .def("getBlock", &BlockInfoDriver::getBlock, py::arg("category"),
           py::arg("name"),
           R"([Subclass interface (Required)] Get the specified block

    :param str category: the specified block category
    :param str name: the block name)")
      .def(
          "_getBlockList",
          (BlockList (BlockInfoDriver::*)(
              const string&))&BlockInfoDriver::getBlockList,
          py::arg("category"),
          R"([Subclass interface (Required)] Get the block list of the specified category

    :param str category: the block category)")
      .def("getBlockList", get_block_list_1, py::arg("category"),
           "Get the block list of the specified category")
      .def("getBlockList", get_block_list_2, "Get all the block lists")
      .def("save", &BlockInfoDriver::save, py::arg("block"),
           R"([Subclass interface (Required)] Save the specified block

    :param Block block: the block object
    :note: if a block with the same name already exists, it will be overwritten; if the block category or the name has been modified, you need to delete the original block manually before the modification)")
      .def("remove", &BlockInfoDriver::remove, py::arg("category"),
           py::arg("name"),
           R"([Subclass interface (Required)] Delete the specified block

    :param str category: the block category
    :param str name: the block name)");
}

// Registration group: _DataDriverFactory
/*
 * _DataDriverFactory.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include <data/storage/DataDriverFactory.h>

#include "storage/BaseInfoDriverWrap.h"
#include "storage/BlockInfoDriverWrap.h"
#include "storage/KDataDriverWrap.h"

using namespace hayaku;
namespace py = pybind11;

void export_DataDriverFactory(py::module& m) {
  py::class_<DataDriverFactory>(m, "DataDriverFactory",
                                "The data driver factory class")
      .def_static("getBaseInfoDriver", &DataDriverFactory::getBaseInfoDriver)
      .def_static("removeBaseInfoDriver",
                  &DataDriverFactory::removeBaseInfoDriver)
      .def_static("getKDataDriverPool", &DataDriverFactory::getKDataDriverPool)
      .def_static("removeKDataDriver", &DataDriverFactory::removeKDataDriver)
      .def_static("getBlockDriver", &DataDriverFactory::getBlockDriver)
      .def_static("removeBlockDriver", &DataDriverFactory::removeBlockDriver)

      .def_static("regBaseInfoDriver",
                  [](py::object pydriver) {
                    auto keep_python_state_alive =
                        std::make_shared<py::object>(pydriver);
                    auto ptr = pydriver.cast<PyBaseInfoDriver*>();
                    auto driver =
                        BaseInfoDriverPtr(keep_python_state_alive, ptr);
                    DataDriverFactory::regBaseInfoDriver(driver);
                  })

      .def_static("regBlockDriver",
                  [](py::object pydriver) {
                    auto keep_python_state_alive =
                        std::make_shared<py::object>(pydriver);
                    auto ptr = pydriver.cast<PyBlockInfoDriver*>();
                    auto driver =
                        BlockInfoDriverPtr(keep_python_state_alive, ptr);
                    DataDriverFactory::regBlockDriver(driver);
                  })

      .def_static("regKDataDriver",
                  [](py::object pydriver) {
                    auto keep_python_state_alive =
                        std::make_shared<py::object>(pydriver);
                    auto ptr = pydriver.cast<PyKDataDriver*>();
                    auto driver = KDataDriverPtr(keep_python_state_alive, ptr);
                    DataDriverFactory::regKDataDriver(driver);
                  })

      .def_static("init", &DataDriverFactory::init,
                  "Initialize the supported default drivers")
      .def_static(
          "release", &DataDriverFactory::release,
          "Actively release the resources, mainly used for the memory leak "
          "detection; clean up actively at the exit to avoid false positives");
}

// Registration group: _KDataDriver
/*
 * _KDataDriver.cpp
 *
 *  Created on: 2017-10-07
 *      Author: fasiondog
 */

#include "storage/KDataDriverWrap.h"

using namespace hayaku;
namespace py = pybind11;

void export_KDataDriver(py::module& m) {
  py::class_<KDataDriver, KDataDriverPtr, PyKDataDriver>(
      m, "KDataDriver",
      R"(The K-line data driver base class

  The subclass interfaces:
    - _init(self)
    - isIndexFirst(self) (Required)
    - canParallelLoad(self) (Required)
    - getCount(self, market, code, ktype)
    - _getIndexRangeByDate(self, market, code, query)
    - _getKRecordList(self, market, code, query)
    - _getTimeLineList(self, market, code, query)
    - _getTransList(self, market, code, query)
  )")
      .def(py::init<>())
      .def(py::init<const string&>())
      .def_property_readonly("name", &KDataDriver::name,
                             py::return_value_policy::copy, "The driver name")

      .def("__str__", to_py_str<KDataDriver>)
      .def("__repr__", to_py_str<KDataDriver>)

      .def("get_param", &KDataDriver::getParam<boost::any>,
           "Get the value of the specified parameter")
      .def("set_param",
           static_cast<void (KDataDriver::*)(
               const std::string&, const boost::any&)>(&KDataDriver::setParam),
           "Set the parameter")
      .def("have_param", &KDataDriver::haveParam,
           "Whether the specified parameter exists")

      .def("clone", &KDataDriver::clone, "Clone the driver")

      .def("_init", &KDataDriver::_init,
           "[Subclass interface] Initialize the driver")
      .def("isIndexFirst", &KDataDriver::isIndexFirst,
           "[Subclass interface (Required)] Judge whether this engine is "
           "faster when querying by the position index way, or faster when "
           "querying by the date way")
      .def("canParallelLoad", &KDataDriver::canParallelLoad,
           "[Subclass interface (Required)] Whether the parallel data loading "
           "is supported")
      .def("getCount", &KDataDriver::getCount, py::arg("market"),
           py::arg("code"), py::arg("ktype"),
           R"(Get the amount of the K-line data of the specified type

    :param str market: the market abbreviation
    :param str code: the security code
    :param Query.KType ktype: the K-line type
    :rtype int)")
      .def(
          "_getIndexRangeByDate",
          [](KDataDriver& self, const string& market, const string& code,
             const KQuery& query) {
            size_t start = 0, end = 0;
            self.getIndexRangeByDate(market, code, query, start, end);
            return py::make_tuple(start, end);
          },
          py::arg("market"), py::arg("code"), py::arg("query"),
          R"([Subclass interface] Get the K-line record indexes corresponding to the specified date range

    :param str market: the market abbreviation
    :param str code: the security code
    :param KQuery query: the query condition
    :return: the (start, end) corresponding K-line record positions)")
      .def("_getKRecordList", &KDataDriver::getKRecordList, py::arg("market"),
           py::arg("code"), py::arg("query"),
           "[Subclass interface] Get the K-line data")
      .def("_getTimeLineList", &KDataDriver::getTimeLineList, py::arg("market"),
           py::arg("code"), py::arg("query"),
           "[Subclass interface] Get the time-line data")
      .def("_getTransList", &KDataDriver::getTransList, py::arg("market"),
           py::arg("code"), py::arg("query"),
           "[Subclass interface] Get the historical tick data")
      .def("isColumnFirst", &KDataDriver::isColumnFirst,
           "Whether it is column-first (the column database stores the K-line "
           "data)");
}

// Registration group: data_driver_main
/*
 * data_driver_main.cpp
 *
 *  Created on: 2017-10-7
 *      Author: fasiondog
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

void export_DataDriverFactory(py::module& m);
void export_KDataDriver(py::module& m);
void export_BaseInfoDriver(py::module& m);
void export_BlockInfoDriver(py::module& m);

void export_data_driver_main(py::module& m) {
  export_BaseInfoDriver(m);
  export_BlockInfoDriver(m);
  export_KDataDriver(m);
  export_DataDriverFactory(m);
}

void bindData(py::module_& m) {
  export_DataType(m);
  export_MarketInfo(m);
  export_StockTypeInfo(m);
  export_StockWeight(m);
  export_StrategyContext(m);
  export_KQuery(m);
  export_KRecord(m);
  export_TimeLineRecord(m);
  export_TransRecord(m);
  export_KData(m);
  export_Stock(m);
  export_Block(m);
  export_data_main(m);
  export_data_driver_main(m);

  m.def("get_stock", getStock,
        R"(get_stock(market_code)

        Get the corresponding security instance by "market abbreviation + security code"

        :param str market_code: the format: "market abbreviation + security code", e.g. "sh000001"
        :return: the corresponding security instance; if the instance does not exist, return an empty instance, i.e. Stock(), without raising an exception
        :rtype: Stock)");

  int64_t null_int64 = Null<int64_t>();
  Datetime null_date = Null<Datetime>();

  m.def("get_block", getBlock, R"(get_block(category: str, name: str)

    Get the predefined block

    :param str category: the block category
    :param str name: the block name
    :rtype: Block)");

  m.def("get_kdata", py::overload_cast<const string&, const KQuery&>(getKData));

  m.def(
      "get_kdata",
      py::overload_cast<const string&, int64_t, int64_t, const KQuery::KType&,
                        KQuery::RecoverType>(getKData),
      py::arg("market_code"), py::arg("start") = 0, py::arg("end") = null_int64,
      py::arg("ktype") = KQuery::DAY,
      py::arg("recover_type") = KQuery::NO_RECOVER,
      R"(Get the K-line data within the [start, end) range by the security code and the start/end positions

    :param str market_code: the security code, e.g.: 'sh000001'
    :param int start: the start index
    :param int end: the end index
    :param Query.KType ktype: the K-line type, 'DAY'|'WEEK'|'MONTH'|'QUARTER'|'HALFYEAR'|'YEAR'|'MIN'|'MIN5'|'MIN15'|'MIN30'|'MIN60'
    :param Query.RecoverType recover_type: the recovery type)");

  m.def(
      "get_kdata",
      py::overload_cast<const string&, const Datetime&, const Datetime&,
                        const KQuery::KType&, KQuery::RecoverType>(getKData),
      py::arg("market_code"), py::arg("start") = Datetime::min(),
      py::arg("end") = null_date, py::arg("ktype") = KQuery::DAY,
      py::arg("recover_type") = KQuery::NO_RECOVER,
      R"(Get the K-line data within the [start, end) range by the security code and the start/end dates

    :param str market_code: the security code, e.g.: 'sh000001'
    :param int start: the start date
    :param int end: the end date
    :param Query.KType ktype: the K-line type, 'DAY'|'WEEK'|'MONTH'|'QUARTER'|'HALFYEAR'|'YEAR'|'MIN'|'MIN5'|'MIN15'|'MIN30'|'MIN60'
    :param Query.RecoverType recover_type: the recovery type)");}
