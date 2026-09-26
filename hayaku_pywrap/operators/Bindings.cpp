#include "Bindings.h"

/* Domain binding registrations. */

// Registration group: _Factor
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <operators/Factor.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_Factor(py::module& m) {
  py::class_<Factor>(m, "Factor", "The factor metadata")
      .def(py::init<>(), R"(__init__(self)

    The default constructor, creating an empty factor object)")

      .def(py::init<const string&, const KQuery::KType&>(), py::arg("name"),
           py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, name[, ktype=KQuery.DAY])

    The constructor specifying only the factor name and the K-line type, which will try to load the factor from the database automatically

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line)")

      .def(
          py::init([](const string& name, const Indicator& formula,
                      const KQuery::KType& ktype, const string& brief,
                      const string& details, bool save_value,
                      const Datetime& start_date, const py::object& block,
                      KQuery::RecoverType recover_type) {
            Block c_block = get_block_from_python(block);
            return Factor(name, formula, ktype, brief, details, save_value,
                          start_date, c_block, recover_type);
          }),
          py::arg("name"), py::arg("formula"), py::arg("ktype") = KQuery::DAY,
          py::arg("brief") = "", py::arg("details") = "",
          py::arg("need_save_value") = false,
          py::arg("start_date") = Datetime::min(), py::arg("block") = Block(),
          py::arg("recover_type") = KQuery::NO_RECOVER,
          R"(__init__(self, name, formula[, ktype=KQuery.DAY[, brief=""[, details=""[, need_save_value=False[, start_date=Datetime.min()[, block=Block()]]]]]])

    The constructor creating a new factor object (the factor name + the K-line type is the unique identifier of the factor)

    :param str name: the factor name
    :param Indicator formula: the calculation formula indicator, which cannot be changed once created
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param str brief: the brief description, defaulting to empty
    :param str details: the detailed description, defaulting to empty
    :param bool need_save_value: whether the factor value data needs to be persisted, defaulting to False
    :param Datetime start_date: the start date, the starting date when storing the data, defaulting to the minimum date
    :param Block block: the block information, the security set; if it is empty, it is all, defaulting to empty
    :param KQuery.RecoverType recover_type: the recovery type, defaulting to NO_RECOVER
    :note: the factor name is not case-sensitive, with name + ktype as the unique identifier)")

      .def("__str__", &Factor::str)
      .def("__repr__", &Factor::str)

      .def_property("name", py::overload_cast<>(&Factor::name, py::const_),
                    py::overload_cast<const string&>(&Factor::name),
                    py::return_value_policy::copy, "The factor name")
      .def_property("ktype", py::overload_cast<>(&Factor::ktype, py::const_),
                    py::overload_cast<const string&>(&Factor::ktype),
                    py::return_value_policy::copy, "The factor frequency type")
      .def_property("create_at",
                    py::overload_cast<>(&Factor::createAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::createAt),
                    py::return_value_policy::copy, "The creation date")
      .def_property("update_at",
                    py::overload_cast<>(&Factor::updateAt, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::updateAt),
                    py::return_value_policy::copy, "The modification date")
      .def_property("formula",
                    py::overload_cast<>(&Factor::formula, py::const_),
                    py::overload_cast<const Indicator&>(&Factor::formula),
                    py::return_value_policy::copy, "The factor formula")
      .def_property("start_date",
                    py::overload_cast<>(&Factor::startDate, py::const_),
                    py::overload_cast<const Datetime&>(&Factor::startDate),
                    "The start date of the data storage")
      .def_property("block", py::overload_cast<>(&Factor::block, py::const_),
                    py::overload_cast<const Block&>(&Factor::block),
                    py::return_value_policy::copy, "The security set")
      .def_property("brief", py::overload_cast<>(&Factor::brief, py::const_),
                    py::overload_cast<const string&>(&Factor::brief),
                    py::return_value_policy::copy, "The basic description")
      .def_property("details",
                    py::overload_cast<>(&Factor::details, py::const_),
                    py::overload_cast<const string&>(&Factor::details),
                    py::return_value_policy::copy, "The detailed description")
      .def_property("need_save_value",
                    py::overload_cast<>(&Factor::needSaveValue, py::const_),
                    py::overload_cast<bool>(&Factor::needSaveValue),
                    "Whether to persist the factor value data")
      .def_property(
          "recover_type", py::overload_cast<>(&Factor::recoverType, py::const_),
          py::overload_cast<KQuery::RecoverType>(&Factor::recoverType),
          py::return_value_policy::copy, "The recovery type")

      .def("is_null", &Factor::isNull, "Whether it is an empty factor")

      .def("save_to_db", &Factor::save_to_db, py::arg("update_before") = true,
           R"(save_to_db(self[, update_before=True])

    Save the factor metadata to the database; if the factor already exists, update it, otherwise insert a new record

    :note: the factor name is not case-sensitive, with name + ktype as the unique identifier

    :param bool update_before: whether to check and update the existing factor before saving, defaulting to True). Note: it usually must be true, otherwise it will cause the data errors, unless you are certain that all the factor values have been updated)")

      .def("save_special_values_to_db",
           py::overload_cast<const Stock&, const Indicator&, bool>(
               &Factor::save_special_values_to_db),
           py::arg("stock"), py::arg("values"), py::arg("replace") = false)

      .def(
          "save_special_values_to_db",
          py::overload_cast<const Stock&, const DatetimeList&, const PriceList&,
                            bool>(&Factor::save_special_values_to_db),
          py::arg("stock"), py::arg("dates"), py::arg("values"),
          py::arg("replace") = false,
          R"(save_special_values_to_db(self, stock, values[, replace=False])
save_special_values_to_db(self, stock, dates, values[, replace=False])

    Save the special factor values to the database, supporting two input formats:
    1. Save the result data of an Indicator object directly (usually a PRICELIST), extracting the dates and the values from the Indicator automatically
    2. Save the pre-calculated date-value pair data, applicable when there are already independent date and price lists
       (such as the externally imported finance data or the machine learning prediction results), without needing to be wrapped into an Indicator first

    Overload 1 - save the Indicator object:
    :param Stock stock: the security object
    :param Indicator values: the already calculated indicator object (it must have been bound to the K-line data)
    :param bool replace: whether to replace the existing data, defaulting to False

    Overload 2 - save the pre-calculated data:
    :param Stock stock: the security object
    :param DatetimeList dates: the special factor date list
    :param PriceList values: the special factor value list
    :param bool replace: whether to replace the existing data, defaulting to False

    Usage scenarios:
    - Save the composite indicator calculation results
    - Save the externally imported finance data
    - Save the machine learning model prediction results
    - Save the manually annotated special factor values)")

      .def("remove_from_db", &Factor::remove_from_db,
           R"(remove_from_db(self)

    Delete the factor and its data from the database. Note: to prevent the misoperations, the values of the special factors will not be deleted; you need to delete them manually yourself.

    :note: delete with name + ktype as the unique identifier)")

      .def(
          "get_all_values", &Factor::getAllValues, py::arg("query"),
          py::arg("align") = false, py::arg("fill_null") = false,
          py::arg("tovalue") = false, py::arg("align_dates") = DatetimeList{},
          R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=False[, align_dates=DatetimeList()]]]])

    Get all the calculation results of the specified query parameters

    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results of all the stocks
    :rtype: list)")

      .def(
          "get_value",
          py::overload_cast<const KData&, bool, bool, bool, bool,
                            const DatetimeList&>(&Factor::getValue, py::const_),
          py::arg("kdata"), py::arg("align") = false,
          py::arg("fill_null") = false, py::arg("tovalue") = false,
          py::arg("check") = false, py::arg("align_dates") = DatetimeList{})
      .def(
          "get_value",
          py::overload_cast<const Stock&, const KQuery&, bool, bool, bool, bool,
                            const DatetimeList&>(&Factor::getValue, py::const_),
          py::arg("stock"), py::arg("query"), py::arg("align") = false,
          py::arg("fill_null") = false, py::arg("tovalue") = false,
          py::arg("check") = false, py::arg("align_dates") = DatetimeList{},
          R"(get_value(self, stock, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])

    Get the calculation result of the specified stock with the specified query parameters

    :param Stock stock: the security object
    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param bool check: whether to check that the stock belongs to the block specified by itself, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the calculation result indicator
    :rtype: Indicator)")

      .def(
          "get_values",
          [](Factor& self, const py::object& stks, const KQuery& query,
             bool align, bool fill_null, bool tovalue, bool check,
             const DatetimeList& align_dates) {
            return self.getValues(get_stock_list_from_python(stks), query,
                                  align, fill_null, tovalue, check,
                                  align_dates);
          },
          py::arg("stocks"), py::arg("query"), py::arg("align") = false,
          py::arg("fill_null") = false, py::arg("tovalue") = false,
          py::arg("check") = false, py::arg("align_dates") = DatetimeList{},
          R"(get_values(self, stocks, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])

    Get the calculation results of the specified stock list with the specified query parameters

    :param list stocks: the security list
    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param bool check: whether to check that the stock list belongs to the block specified by itself, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results arranged by the stock order
    :rtype: list)")

      .def(py::hash(py::self))

          DEF_PICKLE(Factor);
}

// Registration group: _FactorSet
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-24
 *      Author: fasiondog
 */

#include <operators/FactorSet.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_FactorSet(py::module& m) {
  py::class_<FactorSet>(m, "FactorSet", "The factor metadata")
      .def(py::init<>(), R"(__init__(self)

    The default constructor, creating an empty factor set)")

      .def(py::init<const string&, const KQuery::KType&, const Block&>(),
           py::arg("name"), py::arg("ktype") = KQuery::DAY,
           py::arg("block") = Block(),
           R"(__init__(self, name[, ktype=KQuery.DAY[, block=Block()]])

    The constructor creating the factor set with the specified name and type

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param Block block: the block information, the security set, defaulting to empty)")

      .def(py::init<const IndicatorList&, const KQuery::KType&>(),
           py::arg("inds"), py::arg("ktype") = KQuery::DAY,
           R"(__init__(self, inds[, ktype=KQuery.DAY])

    The constructor creating the factor set with the specified indicator list, where the factor names default to the indicator names

    :note: the indicators with the same name will be overwritten, and finally the last indicator with the same name is kept
    :param list inds: the indicator list
    :param KQuery.KType ktype: the K-line type of the factor set, defaulting to the daily line)")

      .def(
          py::init([](const py::sequence& factors, const KQuery::KType& ktype,
                      const Block& block, const string& name) {
            FactorList factors_list = python_list_to_vector<Factor>(factors);
            return FactorSet(factors_list, ktype, block, name);
          }),
          py::arg("factors"), py::arg("ktype") = KQuery::DAY,
          py::arg("block") = Block(), py::arg("name") = "",
          R"(__init__(self, factors[, ktype=KQuery.DAY[, block=Block(), name='']])

    The constructor creating the factor set with the specified factor list

    :note: the factors with the same name will be overwritten, and finally the last factor with the same name is kept
    :param list factors: the factor list
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :param Block block: the block, defaulting to empty
    :param str name: the factor set name, defaulting to empty)")

      .def("__str__", &FactorSet::str)
      .def("__repr__", &FactorSet::str)

      .def_property("name", py::overload_cast<>(&FactorSet::name, py::const_),
                    py::overload_cast<const string&>(&FactorSet::name),
                    py::return_value_policy::copy, "The factor name")
      .def_property("ktype", py::overload_cast<>(&FactorSet::ktype, py::const_),
                    py::overload_cast<const string&>(&FactorSet::ktype),
                    py::return_value_policy::copy, "The factor frequency type")

      .def_property("block", py::overload_cast<>(&FactorSet::block, py::const_),
                    py::overload_cast<const Block&>(&FactorSet::block),
                    py::return_value_policy::copy,
                    "The block corresponding to the factor set")

      .def("is_null", &FactorSet::isNull, R"(is_null(self)

    Whether it is a null value)")

      .def("empty", &FactorSet::empty, R"(empty(self)

    Whether it is empty)")

      .def("clear", &FactorSet::clear, R"(clear(self))

    Clear the factor metadata)")

      .def("have", &FactorSet::have)
      .def("remove", &FactorSet::remove)
      .def("add", py::overload_cast<const Factor&>(&FactorSet::add))
      .def("add",
           py::overload_cast<const string&, const Indicator&>(&FactorSet::add))
      .def("add", py::overload_cast<const Indicator&>(&FactorSet::add))
      .def("add", py::overload_cast<const IndicatorList&>(&FactorSet::add))
      .def("add", py::overload_cast<const FactorList&>(&FactorSet::add))
      .def("add", py::overload_cast<const std::map<string, Indicator>&>(
                      &FactorSet::add))

      .def("get_factors", &FactorSet::getAllFactors,
           py::return_value_policy::copy, "Get the factor list")

      .def(
          "get_all_values", &FactorSet::getAllValues, py::arg("query"),
          py::arg("align") = false, py::arg("fill_null") = false,
          py::arg("tovalue") = true, py::arg("align_dates") = DatetimeList{},
          R"(get_all_values(self, query[, align=False[, fill_null=False[, tovalue=True[, align_dates=DatetimeList()]]]])

    Get the calculation results of all the factors with the specified query parameters

    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to True
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results of all the factors
    :rtype: list)")

      .def(
          "get_values",
          [](FactorSet& self, const py::object& stks, const KQuery& query,
             bool align = false, bool fill_null = false, bool tovalue = true,
             bool check = false,
             const DatetimeList& align_dates = DatetimeList{}) {
            return self.getValues(get_stock_list_from_python(stks), query,
                                  align, fill_null, tovalue, check,
                                  align_dates);
          },
          py::arg("stocks"), py::arg("query"), py::arg("align") = false,
          py::arg("fill_null") = false, py::arg("tovalue") = false,
          py::arg("check") = false, py::arg("align_dates") = DatetimeList{},
          R"(get_values(self, stocks, query[, align=False[, fill_null=False[, tovalue=False[, check=False[, align_dates=DatetimeList()]]]]])

    Get the calculation results of the specified stock list with the specified query parameters

    :param list stocks: the security list
    :param Query query: the query parameters
    :param bool align: whether to align the dates (e.g. by the specified align_dates or the default trading calendar), defaulting to False
    :param bool fill_null: whether to fill the empty values, defaulting to False
    :param bool tovalue: whether to convert to the values, defaulting to False
    :param bool check: whether to check that the stock list belongs to the block specified by itself, defaulting to False
    :param DatetimeList align_dates: the aligned date list, defaulting to empty
    :return: the list of the calculation results arranged by the stock order
    :rtype: list)")

      .def("save_to_db", &FactorSet::save_to_db,
           R"(save_to_db(self)

    Save the factor set to the database

    :note: with name + ktype as the unique identifier)")

      .def("remove_from_db", &FactorSet::remove_from_db,
           R"(remove_from_db(self)

    Delete the factor set from the database

    :note: with name + ktype as the unique identifier)")

      .def("load_from_db", &FactorSet::load_from_db,
           R"(load_from_db(self)

    Load the factor set from the database

    :note: with name + ktype as the unique identifier; if it does not exist, the current object is not modified)")

      .def("__getitem__",
           py::overload_cast<const string&>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__getitem__",
           py::overload_cast<size_t>(&FactorSet::get, py::const_),
           py::return_value_policy::copy)
      .def("__len__", &FactorSet::size, "The number of the contained factors")
      .def(
          "__iter__",
          [](const FactorSet& self) {
            return py::make_iterator(self.begin(), self.end());
          },
          py::keep_alive<0, 1>())

          DEF_PICKLE(FactorSet);
}

// Registration group: factor_main
/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-18
 *      Author: fasiondog
 */

#include <operators/FactorStore.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_Factor(py::module& m);
void export_FactorSet(py::module& m);

void export_factor_main(py::module& m) {
  export_Factor(m);
  export_FactorSet(m);

  m.def("has_factor", &hasFactor, py::arg("name"),
        py::arg("ktype") = KQuery::DAY,
        R"(has_factor(name[, ktype=KQuery.DAY])

    Check whether the factor exists

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: if the factor exists, return True; otherwise, return False
    :rtype: bool)");

  m.def("get_factor", &getFactor, py::arg("name"),
        py::arg("ktype") = KQuery::DAY,
        R"(get_factor(name[, ktype=KQuery.DAY])

    Get the factor metadata

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: the factor object; if it does not exist, return an empty factor
    :rtype: Factor)");

  m.def("save_factor", &saveFactor, py::arg("factor"),
        py::arg("update_before") = true,
        R"(save_factor(factor[, update_before=True])

    Save the factor to the database

    :param Factor factor: the factor object to save
    :param bool update_before: whether to check and update the existing factor before saving, defaulting to True). Note: it usually must be true, otherwise it will cause the data errors, unless you are certain that all the factor values have been updated
    :note: with name + ktype as the unique identifier)");

  m.def("remove_factor", &removeFactor, py::arg("name"), py::arg("ktype"),
        R"(remove_factor(name, ktype)

    Delete the factor from the database

    :param str name: the factor name
    :param KQuery.KType ktype: the K-line type
    :note: with name + ktype as the unique identifier)");

  m.def("get_all_factors", &getAllFactors,
        R"(get_all_factors()

    Get all the factor metadata

    :return: the list of all the factor objects
    :rtype: list)");

  m.def("update_all_factors_values", &updateAllFactorsValues,
        py::arg("ktype") = KQuery::DAY,
        R"(update_all_factors_values([ktype=KQuery.DAY])

    Update all the factor values

    :param KQuery.KType ktype: the K-line type, defaulting to the daily line)");

  m.def("save_factorset", &saveFactorSet, py::arg("set"),
        R"(save_factorset(set)

    Save the factor set to the database

    :param FactorSet set: the factor set object to save
    :note: with name + ktype as the unique identifier)");

  m.def("get_factorset", &getFactorSet, py::arg("name"),
        py::arg("ktype") = KQuery::DAY,
        R"(get_factorset(name[, ktype=KQuery.DAY])

    Get the factor set

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type, defaulting to the daily line
    :return: the factor set object; if it does not exist, return an empty factor set
    :rtype: FactorSet)");

  m.def("remove_factorset", &removeFactorSet, py::arg("name"), py::arg("ktype"),
        R"(remove_factorset(name, ktype)

    Delete the factor set from the database

    :param str name: the factor set name
    :param KQuery.KType ktype: the K-line type
    :note: with name + ktype as the unique identifier)");

  m.def("get_all_factorsets", &getAllFactorSets,
        R"(get_all_factorsets()

    Get all the factor sets

    :return: the list of all the factor set objects
    :rtype: list)");
}

// Registration group: _IndParam
/*
 *  Copyright (c) 2019 hikyuu.org
 *
 *  Created on: 2022-02-07
 *      Author: fasiondog
 */

#include <operators/Indicator.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_IndParam(py::module& m) {
  py::class_<IndParam>(m, "IndParam", "The technical indicator")
      .def(py::init<>())
      .def(py::init<IndicatorImpPtr>())
      .def(py::init<Indicator>())
      .def("__str__", to_py_str<IndParam>)
      .def("__repr__", to_py_str<IndParam>)

      .def("get", &IndParam::get)
      .def("get_imp", &IndParam::getImp);
}

// Registration group: _Indicator
/*
 * _Indicator.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <operators/Indicator.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

string (Indicator::*ind_read_name)() const = &Indicator::name;
void (Indicator::*ind_write_name)(const string&) = &Indicator::name;

void (Indicator::*setContext_1)(const Stock&,
                                const KQuery&) = &Indicator::setContext;
void (Indicator::*setContext_2)(const KData&) = &Indicator::setContext;

Indicator (Indicator::*ind_call_1)(const Indicator&) = &Indicator::operator();
Indicator (Indicator::*ind_call_2)(const KData&) const = &Indicator::operator();
Indicator (Indicator::*ind_call_3)() = &Indicator::operator();

void (Indicator::*setIndParam1)(const string&,
                                const Indicator&) = &Indicator::setIndParam;
void (Indicator::*setIndParam2)(const string&,
                                const IndParam&) = &Indicator::setIndParam;

void export_Indicator(py::module& m) {
  py::class_<Indicator>(m, "Indicator", "The technical indicator")
      .def(py::init<>())
      .def(py::init<IndicatorImpPtr>(), py::keep_alive<1, 2>())
      .def("__str__", &Indicator::str)
      .def("__repr__", &Indicator::str)

      .def_property_static(
          "enable_increment_calculate",
          [](py::object) { return Indicator::enableIncrementCalculate(); },
          [](py::object cls, bool flag) {
            Indicator::enableIncrementCalculate(flag);
          },
          "Enable/disable the indicator incremental calculation")

      .def_property("name", ind_read_name, ind_write_name, "The indicator name")
      .def_property_readonly(
          "long_name", &Indicator::long_name,
          "Return in the form: Name(param1_val,param2_val,...)")
      .def_property_readonly(
          "discard", &Indicator::discard,
          "The number of the points to discard in the result")
      .def_property_readonly(
          "optype",
          [](const Indicator& ind) { return getOPTypeName(ind.getOPType()); })

      .def("set_discard", &Indicator::setDiscard, R"(set_discard(self, discard)

    Set the number to discard; if it is smaller than the original discard, it is invalid
    :param int discard: the number of the points to discard, greater than 0)")

      .def("get_param", &Indicator::getParam<boost::any>,
           R"(get_param(self, name)

    Get the specified parameter

    :param str name: the parameter name
    :return: the parameter value
    :raises out_of_range: no such parameter)")

      .def("set_param",
           static_cast<void (Indicator::*)(
               const std::string&, const boost::any&)>(&Indicator::setParam),
           R"(set_param(self, name, value)

    Set the parameter

    :param str name: the parameter name
    :param value: the parameter value
    :type value: int | bool | float | string | Query | KData | Stock | DatetimeList
    :raises logic_error: Unsupported type! The parameter type is not supported)")

      .def("have_param", &Indicator::haveParam,
           "Whether the specified parameter exists")

      .def("have_ind_param", &Indicator::haveIndParam,
           "Whether the specified dynamic period indicator parameter exists")
      .def("get_ind_param", &Indicator::getIndParam,
           R"(get_ind_param(self, name)

    Get the specified dynamic indicator parameter

    :param str name: the parameter name
    :return: the dynamic indicator parameter
    :rtype: IndParam
    :raises out_of_range: no such parameter)")

      .def("set_ind_param", setIndParam1)
      .def("set_ind_param", setIndParam2, R"(set_param(self, name, ind)

    Set the dynamic indicator parameter

    :param str name: the parameter name
    :param Indicator|IndParam: the parameter value (can be an Indicator or an IndParam instance))")

      .def("empty", &Indicator::empty, "Whether it is empty")
      .def("clone", &Indicator::clone, "The clone operation")
      .def("formula", &Indicator::formula, R"(formula(self)

    Print the indicator formula

    :rtype: str)")

      .def("get_result_num", &Indicator::getResultNumber,
           R"(get_result_num(self)

    Get the number of the result sets

    :rtype: int)")

      .def("get", &Indicator::get, py::arg("pos"), py::arg("result_index") = 0,
           R"(get(self, pos[, result_index=0])

    Get the value at the specified position

    :param int pos: the specified index position
    :param int result_index: the specified result set
    :rtype: float)")

      .def(
          "get_pos",
          [](const Indicator& self, const Datetime& d) {
            size_t pos = self.getPos(d);
            py::object ret = py::none();
            if (pos != Null<size_t>()) {
              ret = py::int_(pos);
            }
            return ret;
          },
          R"(get_pos(self, date):

    Get the index position corresponding to the specified date; if there is no corresponding position, return None

    :param Datetime date: the specified date
    :rtype: int)")

      .def("get_datetime", &Indicator::getDatetime, R"(get_datetime(self, pos)

    Get the date at the specified position

    :param int pos: the specified index position
    :rtype: float)")

      .def("get_by_datetime", &Indicator::getByDate, py::arg("datetime"),
           py::arg("result_index") = 0,
           R"(get_by_datetime(self, datetime[, result_index=0])

    Get the value of the specified date. If there is no result for the corresponding date, return constant.null_price

    :param Datetime datetime: the specified date
    :param int result_index: the specified result set
    :rtype: float)")

      .def("get_result", &Indicator::getResult,
           R"(get_result(self, result_index)

    Get the specified result set

    :param int result_index: the specified result set
    :rtype: Indicator)")

      .def("get_result_as_price_list", &Indicator::getResultAsPriceList,
           R"(get_result_as_price_list(self, result_index)

    Get the specified result set

    :param int result_index: the specified result set
    :rtype: PriceList)")

      .def("get_datetime_list", &Indicator::getDatetimeList,
           R"(get_datetime_list(self)

    Return the corresponding date list

    :rtype: DatetimeList)")

      .def("exist_nan", &Indicator::existNan, py::arg("result_idx=0"),
           R"(exist_nan(self, result_idx)

    Judge whether a NaN value exists

    :param int result_idx: the specified result set
    :rtype: bool)")

      .def("set_context", setContext_1)
      .def("set_context", setContext_2, R"(set_context(self, kdata)

    Set the context

    :param KData kdata: the associated context K-line)

set_context(self, stock, query)

    Set the context

    :param Stock stock: the specified Stock
    :param Query query: the specified query condition)")

      .def("get_context", &Indicator::getContext, R"(get_context(self)

    Get the context

    :rtype: KData)")

      .def("extend", &Indicator::extend, R"(extend(self)

    When there is a context, automatically extend the context to the current latest data and calculate)")

      .def("contains", &Indicator::contains, R"(contains(self, name)

    Get whether the indicator formula contains the indicator with the specified name

    :param str name: the specified indicator name
    :rtype: bool)")

      .def("equal", &Indicator::equal)
      .def("is_same", &Indicator::isSame)
      .def("get_imp", &Indicator::getImp)
      .def("__len__", &Indicator::size)

      .def("__call__", ind_call_1)
      .def("__call__", ind_call_2)
      .def("__call__", ind_call_3)

      .def("__hash__",
           [](const Indicator& self) { return std::hash<Indicator>()(self); })

      .def("__getitem__",
           [](const Indicator& self, py::object obj) {
             py::object ret;
             if (py::isinstance<py::int_>(obj)) {
               int64_t i = obj.cast<int64_t>();
               int64_t length = self.size();
               int64_t index = i < 0 ? length + i : i;
               if (index < 0 || index >= length)
                 throw std::out_of_range(
                     fmt::format("index out of range: {}", i));
               ret = py::cast(self[index]);
               return ret;
             } else if (py::isinstance<Datetime>(obj)) {
               Datetime dt = py::cast<Datetime>(obj);
               auto val = self[dt];
               if (val == Null<Indicator::value_t>()) {
                 throw std::out_of_range(
                     fmt::format("datetime out of range: {}", dt));
               }
               ret = py::cast(val);
               return ret;
             } else if (py::isinstance<py::str>(obj)) {
               Datetime dt = Datetime(py::cast<std::string>(obj));
               auto val = self[dt];
               if (val == Null<Indicator::value_t>()) {
                 throw std::out_of_range(
                     fmt::format("datetime out of range: {}", dt));
               }
               ret = py::cast(val);
               return ret;
             } else if (py::isinstance<py::slice>(obj)) {
               py::slice slice = py::cast<py::slice>(obj);
               size_t start, stop, step, length;

               if (!slice.compute(self.size(), &start, &stop, &step, &length)) {
                 throw std::invalid_argument("Invalid slice parameters");
               }

               std::vector<Indicator::value_t> result;
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
          [](const Indicator& self) {
            return py::make_iterator(self.begin(), self.end());
          },
          py::keep_alive<0, 1>())

      .def(
          "to_np",
          [](const Indicator& self) {
            py::array ret;
            auto imp = self.getImp();
            HAYAKU_IF_RETURN(!imp, ret);
            size_t ret_num = imp->getResultNumber();

            uint64_t* buffer = new uint64_t[self.size() * (ret_num + 1)];

            std::vector<string> names;
            std::vector<string> fields;
            std::vector<int64_t> offsets;

            auto dates = imp->getDatetimeList();
            size_t bytes_size;
            if (!dates.empty()) {
              names.push_back("datetime");
              fields.push_back("datetime64[ns]");
              offsets.push_back(0);
              for (size_t i = 0; i < ret_num; i++) {
                names.push_back(fmt::format("value{}", i));
                fields.push_back("d");
                offsets.push_back(offsets.back() + sizeof(Indicator::value_t));
              }
              bytes_size =
                  sizeof(Datetime) + ret_num * sizeof(Indicator::value_t);
            } else {
              for (size_t i = 0; i < ret_num; i++) {
                names.push_back(fmt::format("value{}", i));
                fields.push_back("d");
                if (i == 0) {
                  offsets.push_back(0);
                } else {
                  offsets.push_back(offsets.back() +
                                    sizeof(Indicator::value_t));
                }
              }
              bytes_size = ret_num * sizeof(Indicator::value_t);
            }

            auto dtype =
                py::dtype(vector_to_python_list<string>(names),
                          vector_to_python_list<string>(fields),
                          vector_to_python_list<int64_t>(offsets), bytes_size);

            std::vector<const Indicator::value_t*> src(ret_num);
            for (size_t i = 0; i < ret_num; i++) {
              src[i] = imp->data(i);
            }

            uint64_t* data = buffer;
            double* val = (double*)buffer;
            if (!dates.empty()) {
              size_t x = ret_num + 1;
              for (size_t i = 0, total = imp->size(); i < total; i++) {
                data[i * x] = dates[i].timestamp() * 1000LL;
                for (size_t j = 0; j < ret_num; j++) {
                  val[i * x + j + 1] = src[j][i];
                }
              }
            } else {
              for (size_t i = 0, total = imp->size(); i < total; i++) {
                for (size_t j = 0; j < ret_num; j++) {
                  val[i * ret_num + j] = src[j][i];
                }
              }
            }

            auto capsule = py::capsule(buffer, [](void* ptr) {
              delete[] static_cast<uint64_t*>(ptr);
            });
            ret = py::array(dtype, self.size(), data, capsule);
            return ret;
          },
          "Convert to np.array; if it is a time series, the datetime date "
          "column will be included")

      .def(
          "value_to_np",
          [](const Indicator& self) {
            size_t ret_num = self.getResultNumber();

            // Initialize the array_t and get its internal buffer
            py::array_t<double> ret;
            ret.resize(
                {self.size(), ret_num});  // The 2D shape: [size, ret_num]
            auto buf = ret.request();
            double* buffer = static_cast<double*>(
                buf.ptr);  // Get the pointer from the array_t

            std::vector<std::string> names;
            std::vector<std::string> fields;
            std::vector<int64_t> offsets;
            for (size_t i = 0; i < ret_num; i++) {
              names.push_back(fmt::format("value{}", i));
              fields.push_back("d");
              offsets.push_back(
                  i *
                  sizeof(
                      Indicator::value_t));  // Simplify the offset calculation
            }

            auto dtype = py::dtype(vector_to_python_list<std::string>(names),
                                   vector_to_python_list<std::string>(fields),
                                   vector_to_python_list<int64_t>(offsets),
                                   ret_num * sizeof(Indicator::value_t));

            std::vector<const Indicator::value_t*> src(ret_num);
            for (size_t i = 0; i < ret_num; i++) {
              src[i] = self.data(i);
            }

            // Fill the data into the buffer of the array_t
            for (size_t i = 0, total = self.size(); i < total; i++) {
              for (size_t j = 0; j < ret_num; j++) {
                buffer[i * ret_num + j] = src[j][i];
              }
            }

            return py::array(dtype, {self.size()}, {ret_num * sizeof(double)},
                             buf.ptr, ret);
          },
          "Convert only the values to np.array, without the date column")

      .def(
          "to_array",
          [](const Indicator& self, size_t result_index) {
            HAYAKU_CHECK(result_index < self.getResultNumber(),
                         "result_index out of range");
            auto ret = py::array_t<double>(self.size());
            auto buf = ret.request();
            double* ptr = static_cast<double*>(buf.ptr);
            const auto* src = self.data(result_index);
            for (size_t i = 0; i < self.size(); i++) {
              ptr[i] = src[i];
            }
            return ret;
          },
          py::arg("result_index") = 0,
          "Convert the specified result set to numpy.array")

      .def(
          "to_df",
          [](const Indicator& self) {
            size_t total = self.size();
            if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
            }

            py::dict columns;
            auto dates = self.getDatetimeList();
            if (!dates.empty()) {
              std::vector<int64_t> datetime(total);
              for (size_t i = 0; i < total; i++) {
                datetime[i] = dates[i].timestamp() * 1000LL;
              }
              columns["datetime"] = py::array_t<int64_t>(total, datetime.data())
                                        .attr("astype")("datetime64[ns]");
            }

            size_t ret_num = self.getResultNumber();
            for (size_t i = 0; i < ret_num; i++) {
              py::array_t<double> arr(total);
              auto buf = arr.request();
              double* dst = static_cast<double*>(buf.ptr);
              const auto* src = self.data(i);
              for (size_t j = 0; j < total; j++) {
                dst[j] = src[j];
              }
              columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(
                columns, py::arg("copy") = false);
          },
          "Convert to a DataFrame")

      .def(
          "value_to_df",
          [](const Indicator& self) {
            size_t total = self.size();
            if (total == 0) {
              return py::module_::import("pandas").attr("DataFrame")();
            }

            py::dict columns;
            size_t ret_num = self.getResultNumber();
            for (size_t i = 0; i < ret_num; i++) {
              py::array_t<double> arr(total);
              auto buf = arr.request();
              double* dst = static_cast<double*>(buf.ptr);
              const auto* src = self.data(i);
              for (size_t j = 0; j < total; j++) {
                dst[j] = src[j];
              }
              columns[fmt::format("value{}", i).c_str()] = arr;
            }

            return py::module_::import("pandas").attr("DataFrame")(
                columns, py::arg("copy") = false);
          },
          "Convert to a DataFrame, containing only the values")

      .def(+py::self)
      .def(py::self + py::self)
      .def(py::self + Indicator::value_t())
      .def(Indicator::value_t() + py::self)

      .def(-py::self)
      .def(py::self - py::self)
      .def(py::self - Indicator::value_t())
      .def(Indicator::value_t() - py::self)

      .def(py::self * py::self)
      .def(py::self * Indicator::value_t())
      .def(Indicator::value_t() * py::self)

      .def(py::self / py::self)
      .def(py::self / Indicator::value_t())
      .def(Indicator::value_t() / py::self)

      .def(py::self == py::self)
      .def(py::self == Indicator::value_t())
      .def(Indicator::value_t() == py::self)

      .def(py::self != py::self)
      .def(py::self != Indicator::value_t())
      .def(Indicator::value_t() != py::self)

      .def(py::self >= py::self)
      .def(py::self >= Indicator::value_t())
      .def(Indicator::value_t() >= py::self)

      .def(py::self <= py::self)
      .def(py::self <= Indicator::value_t())
      .def(Indicator::value_t() <= py::self)

      .def(py::self > py::self)
      .def(py::self > Indicator::value_t())
      .def(Indicator::value_t() > py::self)

      .def(py::self < py::self)
      .def(py::self < Indicator::value_t())
      .def(Indicator::value_t() < py::self)

      .def(py::self % py::self)
      .def(py::self % Indicator::value_t())
      .def(Indicator::value_t() % py::self)

      .def(py::self & py::self)
      .def(py::self & Indicator::value_t())
      .def(Indicator::value_t() & py::self)

      .def(py::self | py::self)
      .def(py::self | Indicator::value_t())
      .def(Indicator::value_t() | py::self)

          DEF_PICKLE(Indicator);
}

// Registration group: _IndicatorImp
/*
 * _IndicatorImp.cpp
 *
 *  Created on: 2013-2-13
 *      Author: fasiondog
 */

#include <operators/Indicator.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

class PyIndicatorImp : public IndicatorImp {
  PY_CLONE(PyIndicatorImp, IndicatorImp)

 public:
  PyIndicatorImp() : IndicatorImp() { m_is_python_object = true; }

  PyIndicatorImp(const string& name) : IndicatorImp(name) {
    m_is_python_object = true;
  }

  PyIndicatorImp(const string& name, size_t result_num)
      : IndicatorImp(name, result_num) {
    m_is_python_object = true;
  }

  void _calculate(const Indicator& ind) override {
    PYBIND11_OVERLOAD(void, IndicatorImp, _calculate, ind);
  }

  void _dyn_run_one_step(const Indicator& ind, size_t curPos,
                         size_t step) override {
    PYBIND11_OVERLOAD(void, IndicatorImp, _dyn_run_one_step, ind, curPos, step);
  }

  void _dyn_calculate(const Indicator& ind) override {
    PYBIND11_OVERLOAD(void, IndicatorImp, _dyn_calculate, ind);
  }

  bool supportIncrementCalculate() const override {
    PYBIND11_OVERLOAD_NAME(bool, IndicatorImp, "support_increment_calculate",
                           supportIncrementCalculate, );
  }

  size_t min_increment_start() const override {
    PYBIND11_OVERLOAD_NAME(bool, IndicatorImp, "min_increment_start",
                           min_increment_start);
  }

  void _increment_calculate(const Indicator& ind, size_t start_pos) override {
    PYBIND11_OVERLOAD(void, IndicatorImp, _increment_calculate, ind, start_pos);
  }
};

const string& (IndicatorImp::*read_name)() const = &IndicatorImp::name;
void (IndicatorImp::*write_name)(const string&) = &IndicatorImp::name;

void (IndicatorImp::*set_ind_param1)(const string&, const Indicator&) =
    &IndicatorImp::setIndParam;
void (IndicatorImp::*set_ind_param2)(const string&, const IndParam&) =
    &IndicatorImp::setIndParam;

void export_IndicatorImp(py::module& m) {
  py::class_<IndicatorImp, IndicatorImpPtr, PyIndicatorImp>(
      m, "IndicatorImp",
      R"(The indicator implementation class; when defining a new indicator, you should inherit from this class

    The subclass needs to implement the following interfaces:

        - _clone() -> IndicatorImp
        - _calculate(ind): the indicator calculation
        - isNeedContext(bool): whether it depends on the context)")
      .def(py::init<>())

      .def(py::init<const string&>(), R"(
    :param str name: the indicator name)")

      .def(py::init<const string&, size_t>(), R"(
    :param str name: the indicator name
    :param int result_num: the number of the indicator result sets)")

      .def("__str__", to_py_str<IndicatorImp>)
      .def("__repr__", to_py_str<IndicatorImp>)

      .def_property("name", read_name, write_name,
                    py::return_value_policy::copy, "The indicator name")
      .def_property_readonly(
          "discard", &IndicatorImp::discard,
          "The number of the points to discard in the result")

      .def("get_parameter", &IndicatorImp::getParameter,
           py::return_value_policy::copy,
           "Get the internal parameter class object")

      .def("have_param", &IndicatorImp::haveParam)
      .def("get_param", &IndicatorImp::getParam<boost::any>)
      .def("set_param",
           static_cast<void (IndicatorImp::*)(
               const std::string&, const boost::any&)>(&IndicatorImp::setParam))
      .def("have_ind_param", &IndicatorImp::haveIndParam)
      .def("get_ind_param", &IndicatorImp::getIndParam)
      .def("set_ind_param", set_ind_param1)
      .def("set_ind_param", set_ind_param2)
      .def("set_discard", &IndicatorImp::setDiscard)
      .def("_set", &IndicatorImp::_set, py::arg("val"), py::arg("pos"),
           py::arg("num") = 0)
      .def("_ready_buffer", &IndicatorImp::_readyBuffer)
      .def("get_result_num", &IndicatorImp::getResultNumber)
      .def("get_result_as_price_list", &IndicatorImp::getResultAsPriceList)
      .def("calculate", &IndicatorImp::calculate)
      .def("clone", &IndicatorImp::clone)
      .def("_calculate", &IndicatorImp::_calculate)
      .def("_dyn_run_one_step", &IndicatorImp::_dyn_run_one_step)
      .def("_dyn_calculate", &IndicatorImp::_dyn_calculate)
      .def("is_need_context", &IndicatorImp::isNeedContext)
      .def("is_leaf", &IndicatorImp::isLeaf)
      .def("is_serial", &IndicatorImp::isSerial)
      .def("contains", &IndicatorImp::contains)
      .def("print_tree", &IndicatorImp::printTree,
           py::arg("show_long_name") = false)
      .def("print_all_sub_trees", &IndicatorImp::printAllSubTrees,
           py::arg("show_long_name") = false)
      .def("print_leaves", &IndicatorImp::printLeaves,
           py::arg("show_long_name") = false)

          DEF_PICKLE(IndicatorImpPtr);
}

// Registration group: indicator_main
/*
 * indicator_main.cpp
 *
 *  Created on: 2012-10-18
 *      Author: fasiondog
 */

#include <common/concurrency/ParallelAlgorithms.h>
#include <operators/Indicator.h>
#include <operators/IndicatorSupport.h>

#include "common/PybindSupport.h"

namespace py = pybind11;
using namespace hayaku;

void export_Indicator(py::module& m);
void export_IndicatorImp(py::module& m);
void export_IndParam(py::module& m);
void export_Indicator_build_in(py::module& m);

void export_indicator_main(py::module& m) {
  export_Indicator(m);
  export_IndicatorImp(m);
  export_IndParam(m);
  export_Indicator_build_in(m);

  m.def(
      "batch_calculate_inds",
      [](const py::sequence& inds, const KData& kdata) {
        py::list ret;
        HAYAKU_IF_RETURN(len(inds) == 0, ret);
        IndicatorList cinds = python_list_to_vector<Indicator>(inds);
        ret = vector_to_python_list(global_parallel_for_index(
            0, cinds.size(), [&](size_t i) { return cinds[i](kdata); }));
        return ret;
      },
      R"(batch_calculate_inds(inds, kdata) -> list)

    Calculate multiple indicators in parallel

    :param list inds: the indicator list
    :param KData kdata: the K-line data
    :return: the list of the indicator calculation results
    :rtype: list)");

  m.def(
      "multi_regression",
      [](const Stock& stk, const KQuery& query, const py::args& inds) {
        IndicatorList cinds;
        for (const auto& ind : inds) {
          cinds.push_back(ind.cast<Indicator>());
        }
        return multi_regression(stk, query, cinds);
      },
      R"(multi_regression(stk, query, *inds) -> list)

    Perform a multiple linear regression analysis on the stock, using the return of the stock close price as the dependent variable

    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition
    :param Indicator *inds: one or more indicators as the independent variables
    :return: the list of the regression coefficients; the first element is alpha (the intercept), followed by each beta coefficient
    :rtype: list
    :example:

        >>> stk = getStock('sh000001')
        >>> result = multi_regression(stk, KQuery(-252), MA(CLOSE(), 5), MACD(CLOSE())[0], RSI(CLOSE(), 14))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> beta3 = result[3]
    )");

  m.def(
      "multi_regression_full",
      [](const Stock& stk, const KQuery& query, const py::args& inds) {
        IndicatorList cinds;
        for (const auto& ind : inds) {
          cinds.push_back(ind.cast<Indicator>());
        }
        return multi_regression_full(stk, query, cinds);
      },
      R"(multi_regression_full(stk, query, *inds) -> list)

    Perform a multiple linear regression analysis on the stock (the full version), returning the complete regression result

    :param Stock stk: the stock object
    :param KQuery query: the K-line query condition
    :param Indicator *inds: one or more indicators as the independent variables
    :return: the list of the regression results, in the format:
             [alpha, beta1, beta2, ..., betan, e1, e2, ..., en, RSS, R²]
             - alpha: the intercept
             - beta1~betan: the coefficient of each factor
             - e1~en: the residual of each data point (the actual value - the predicted value)
             - RSS: the residual sum of squares
             - R²: the coefficient of determination
    :rtype: list
    :example:

        >>> stk = getStock('sh000001')
        >>> result = multi_regression_full(stk, KQuery(-252), MA(CLOSE(), 5), MA(CLOSE(), 10))
        >>> alpha = result[0]
        >>> beta1 = result[1]
        >>> beta2 = result[2]
        >>> residuals = result[3:-2]  # the residual sequence
        >>> RSS = result[-2]
        >>> R_squared = result[-1]
    )");
}

void bindOperators(py::module_& m) {
  export_indicator_main(m);
  export_factor_main(m);
}
