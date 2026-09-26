#include "Bindings.h"

#include <application/SystemInfo.h>
#include <common/Lang.h>
#include <extensions/telemetry/Telemetry.h>
#include <hayaku.h>

/* Domain binding registrations. */

// Registration group: _Constant
/*
 * _Constant.cpp
 *
 *  Created on: 2013-4-10
 *      Author: fasiondog
 */

#include <data/MarketTypes.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

#ifdef STOCKTYPE_BLOCK
#undef STOCKTYPE_BLOCK
#endif

#ifdef STOCKTYPE_A
#undef STOCKTYPE_A
#endif

#ifdef STOCKTYPE_INDEX
#undef STOCKTYPE_INDEX
#endif

#ifdef STOCKTYPE_B
#undef STOCKTYPE_B
#endif

#ifdef STOCKTYPE_FUND
#undef STOCKTYPE_FUND
#endif

#ifdef STOCKTYPE_ETF
#undef STOCKTYPE_ETF
#endif

#ifdef STOCKTYPE_ND
#undef STOCKTYPE_ND
#endif

#ifdef STOCKTYPE_BOND
#undef STOCKTYPE_BOND
#endif

#ifdef STOCKTYPE_GEM
#undef STOCKTYPE_GEM
#endif

#ifdef STOCKTYPE_START
#undef STOCKTYPE_START
#endif

#ifdef STOCKTYPE_CRYPTO
#undef STOCKTYPE_CRYPTO
#endif

#ifdef STOCKTYPE_A_BJ
#undef STOCKTYPE_A_BJ
#endif

#ifdef STOCKTYPE_TMP
#undef STOCKTYPE_TMP
#endif

struct Constant {
  Constant()
      : null_datetime(Null<Datetime>()),
        inf(std::numeric_limits<double>::infinity()),
        infa(-std::numeric_limits<double>::infinity()),
        nan(std::numeric_limits<double>::quiet_NaN()),
        null_double(Null<double>()),
        max_double(std::numeric_limits<double>::max()),
        null_price(Null<price_t>()),
        null_int(Null<int>()),
        null_size(Null<size_t>()),
        null_int64(Null<int64_t>()),
        STOCKTYPE_BLOCK(0),
        STOCKTYPE_A(1),
        STOCKTYPE_INDEX(2),
        STOCKTYPE_B(3),
        STOCKTYPE_FUND(4),
        STOCKTYPE_ETF(5),
        STOCKTYPE_ND(6),
        STOCKTYPE_BOND(7),
        STOCKTYPE_GEM(8),
        STOCKTYPE_START(9),
        STOCKTYPE_CRYPTO(10),
        STOCKTYPE_A_BJ(11),
        STOCKTYPE_TMP(999) {
#if HAYAKU_PYTHON_SUPPORT_PICKLE
    pickle_support = true;
#else
    pickle_support = false;
#endif /* HAYAKU_PYTHON_SUPPORT_PICKLE */
  }

  Datetime null_datetime;
  double inf;
  double infa;  // The negative infinity
  double nan;
  double null_double;
  double max_double;
  double null_price;
  int null_int;
  size_t null_size;
  int64_t null_int64;
  bool pickle_support;  // Whether pickle is supported

  int STOCKTYPE_BLOCK;   /// Block
  int STOCKTYPE_A;       /// A-share
  int STOCKTYPE_INDEX;   /// Index
  int STOCKTYPE_B;       /// B-share
  int STOCKTYPE_FUND;    /// Fund
  int STOCKTYPE_ETF;     /// ETF
  int STOCKTYPE_ND;      /// Treasury bond
  int STOCKTYPE_BOND;    /// Bond
  int STOCKTYPE_GEM;     /// ChiNext
  int STOCKTYPE_START;   /// STAR Market
  int STOCKTYPE_CRYPTO;  /// Crypto
  int STOCKTYPE_A_BJ;    /// A-share of the Beijing Stock Exchange
  int STOCKTYPE_TMP;     /// Temporary Stock
};

void export_Constant(py::module& m) {
  py::class_<Constant>(m, "Constant")
      .def_readonly("null_datetime", &Constant::null_datetime,
                    "An invalid Datetime")
      .def_readonly("inf", &Constant::inf, "The infinity")
      .def_readonly("infa", &Constant::infa, "The negative infinity")
      .def_readonly("nan", &Constant::nan, "Not a number")
      .def_readonly("null_double", &Constant::null_double, "The same as nan")
      .def_readonly("max_double", &Constant::max_double,
                    "The maximum double value")
      .def_readonly("null_price", &Constant::null_price, "The same as nan")
      .def_readonly("null_int", &Constant::null_int, "An invalid int")
      .def_readonly("null_size", &Constant::null_size, "An invalid size")
      .def_readonly("null_int64", &Constant::null_int64, "An invalid int64_t")
      .def_readonly("pickle_support", &Constant::pickle_support,
                    "Whether pickle is supported")

      .def_readonly("STOCKTYPE_BLOCK", &Constant::STOCKTYPE_BLOCK, "Block")
      .def_readonly("STOCKTYPE_A", &Constant::STOCKTYPE_A, "A-share")
      .def_readonly("STOCKTYPE_INDEX", &Constant::STOCKTYPE_INDEX, "Index")
      .def_readonly("STOCKTYPE_B", &Constant::STOCKTYPE_B, "B-share")
      .def_readonly("STOCKTYPE_FUND", &Constant::STOCKTYPE_FUND, "Fund")
      .def_readonly("STOCKTYPE_ETF", &Constant::STOCKTYPE_ETF, "ETF")
      .def_readonly("STOCKTYPE_ND", &Constant::STOCKTYPE_ND, "Treasury bond")
      .def_readonly("STOCKTYPE_BOND", &Constant::STOCKTYPE_BOND, "Bond")
      .def_readonly("STOCKTYPE_GEM", &Constant::STOCKTYPE_GEM, "ChiNext")
      .def_readonly("STOCKTYPE_START", &Constant::STOCKTYPE_START,
                    "STAR Market")
      .def_readonly("STOCKTYPE_CRYPTO", &Constant::STOCKTYPE_START, "Crypto")
      .def_readonly("STOCKTYPE_A_BJ", &Constant::STOCKTYPE_A_BJ,
                    "A-share of the Beijing Stock Exchange")
      .def_readonly("STOCKTYPE_TMP", &Constant::STOCKTYPE_TMP,
                    "Temporary Stock");

  m.attr("constant") = Constant();
}

// Registration group: _Datetime
/*
 * _Datetime.cpp
 *
 *  Created on: 2012-9-27
 *      Author: fasiondog
 */

#include <common/serialization/Datetime_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_Datetime(py::module& m) {
  py::class_<Datetime>(
      m, "Datetime",
      R"(The date-time class (accurate to the microsecond), built in the following ways:

    - From a string: Datetime("2010-1-1 10:00:00"), Datetime("2001-1-1"),
                 Datetime("20010101")、Datetime("20010101T232359)
    - From a Python date: Datetime(date(2010,1,1))
    - From a Python datetime: Datetime(datetime(2010,1,1,10)
    - From an integer in the YYYYMMDDHHMMSS, YYYYMMDDHHMM or YYYYMMDD form: Datetime(201001011000), Datetime(20010101)
    - Datetime(year, month, day, hour=0, minute=0, second=0, millisecond=0, microsecond=0))")

      .def(py::init<>())
      .def(py::init<const std::string&>())
      .def(py::init<unsigned long long>())
      .def(py::init<const Datetime&>())
      .def(py::init<long, long, long, long, long, long, long, long>(),
           py::arg("year"), py::arg("month"), py::arg("day"),
           py::arg("hour") = 0, py::arg("minute") = 0, py::arg("second") = 0,
           py::arg("millisecond") = 0, py::arg("microsecond") = 0)
      .def("__init__",
           [](Datetime& self, const py::object& source) {
             self = pydatetime_to_Datetime(source);
           })

      .def("__str__", &Datetime::str)
      .def("__repr__", &Datetime::repr)

      .def_property_readonly("year", &Datetime::year, "Year")
      .def_property_readonly("month", &Datetime::month, "Month")
      .def_property_readonly("day", &Datetime::day, "Day")
      .def_property_readonly("hour", &Datetime::hour, "Hour")
      .def_property_readonly("minute", &Datetime::minute, "Minute")
      .def_property_readonly("second", &Datetime::second, "Second")
      .def_property_readonly("millisecond", &Datetime::millisecond,
                             "Millisecond")
      .def_property_readonly("microsecond", &Datetime::microsecond,
                             "Microsecond")
      .def_property_readonly("number", &Datetime::number,
                             "Return the number displayed as YYYYMMDDhhmm")
      .def_property_readonly(
          "hex", &Datetime::hex,
          "Return a 64-bit integer in which the last 7 bytes represent the "
          "century, the century year, the month, the day, the hour, the minute "
          "and the second")
      .def_property_readonly("ym", &Datetime::ym,
                             "Return the number displayed as YYYYMM")
      .def_property_readonly("ymd", &Datetime::ymd,
                             "Return the number displayed as YYYYMMDD")
      .def_property_readonly("ymdh", &Datetime::ymdh,
                             "Return the number displayed as YYYYMMDDhh")
      .def_property_readonly("ymdhm", &Datetime::ymdhm,
                             "Return the number displayed as YYYYMMDDhhmm")
      .def_property_readonly("ymdhms", &Datetime::ymdhms,
                             "Return the number displayed as YYYYMMDDhhmmss")
      .def_property_readonly(
          "ticks", &Datetime::ticks,
          "Return the microseconds elapsed since the minimum date")

      .def("is_null", &Datetime::isNull,
           "\nWhether it is a Null value, equal to the object created directly "
           "by Datetime()")

      .def("day_of_week", &Datetime::dayOfWeek,
           "\nReturn the day of the week; Sunday is 0 and Monday is 1")
      .def("day_of_year", &Datetime::dayOfYear,
           "\nReturn the day of the year; January 1st is the 1st day of the "
           "year")
      .def("start_of_day", &Datetime::startOfDay,
           "\nReturn 00:00:00 of the current day")
      .def("end_of_day", &Datetime::endOfDay,
           "\nReturn 23:59:59 of the current day")
      .def("next_day", &Datetime::nextDay, "\nReturn the next natural day")
      .def("next_week", &Datetime::nextWeek,
           "\nReturn the Monday date of the next week")
      .def("next_month", &Datetime::nextMonth,
           "\nReturn the first day of the next month")
      .def("next_quarter", &Datetime::nextQuarter,
           "\nReturn the first day of the next quarter")
      .def("next_halfyear", &Datetime::nextHalfyear,
           "\nReturn the first day of the next half-year")
      .def("next_year", &Datetime::nextYear,
           "\nReturn the first day of the next year")
      .def("pre_day", &Datetime::preDay, "\nReturn the previous natural day")
      .def("pre_week", &Datetime::preWeek,
           "\nReturn the Monday date of the previous week")
      .def("pre_month", &Datetime::preMonth,
           "\nReturn the first day of the previous month")
      .def("pre_quarter", &Datetime::preQuarter,
           "\nReturn the first day of the previous quarter")
      .def("pre_halfyear", &Datetime::preHalfyear,
           "\nReturn the first day of the previous half-year")
      .def("pre_year", &Datetime::preYear,
           "\nReturn the first day of the previous year")
      .def("date_of_week", &Datetime::dateOfWeek,
           R"(
    Return the date of the specified day of this week; Sunday is day 0 and Saturday is day 6

    :param int day: indicate the day of this week; if it is less than 0, it is considered day 0, and if it is greater than 6, it is considered day 6)")

      .def("start_of_week", &Datetime::startOfWeek,
           "\nReturn the start date of the week (Monday)")
      .def("end_of_week", &Datetime::endOfWeek,
           "\nReturn the end date of the week (Sunday)")
      .def("start_of_month", &Datetime::startOfMonth,
           "\nReturn the start date of the month")
      .def("end_of_month", &Datetime::endOfMonth,
           "\nReturn the last day of the month")
      .def("start_of_quarter", &Datetime::startOfQuarter,
           "\nReturn the start date of the quarter")
      .def("end_of_quarter", &Datetime::endOfQuarter,
           "\nReturn the end date of the quarter")
      .def("start_of_halfyear", &Datetime::startOfHalfyear,
           "\nReturn the start date of the half-year")
      .def("end_of_halfyear", &Datetime::endOfHalfyear,
           "\nReturn the end date of the half-year")
      .def("start_of_year", &Datetime::startOfYear,
           "\nReturn the start date of the year")
      .def("endOfYear", &Datetime::endOfYear,
           "\nReturn the end date of the year")
      .def("timestamp", &Datetime::timestamp,
           "\nReturn the timestamp (at the microsecond level)")
      .def("timestamp_utc", &Datetime::timestampUTC,
           "\nReturn the timestamp (at the microsecond level), deducting the "
           "local UTC offset time")
      .def_static("min", &Datetime::min,
                  "\nGet the minimum supported date, Datetime(1400, 1, 1)")
      .def_static("max", &Datetime::max,
                  "\nGet the maximum supported date, Datetime(9999, 12, 31)")
      .def_static("now", &Datetime::now, "\nGet the current system date-time")
      .def_static("today", &Datetime::today, "\nGet the current date")
      .def_static("from_hex", &Datetime::fromHex,
                  "\nCompatible with the oracle datetime represented by the "
                  "last 7 bytes")
      .def_static(
          "from_timestamp", &Datetime::fromTimestamp,
          "\nCreate a Datetime object from the timestamp (in microseconds)")
      .def_static("from_timestamp_utc", &Datetime::fromTimestampUTC,
                  "\nCreate a Datetime object from the timestamp (in "
                  "microseconds), adding the local UTC offset")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self < py::self)

      .def(py::self - py::self)

      .def(py::self + TimeDelta())
      //.def(other<TimeDelta>() + self) extended and supported in python
      .def(py::self - TimeDelta())

          DEF_PICKLE(Datetime);

  m.def("get_date_range", getDateRange, py::arg("start"), py::arg("end"),
        R"(get_date_range(start, end)

    Get the list of the natural calendar dates in the specified [start, end) date-time range, supported only up to the day
    Note: if the end date is empty, the maximum date of Datetime will be used, which may use excessive memory

    :param Datetime start: the start date
    :param Datetime end: the end date
    :rtype: DatetimeList)");

  m.def(
      "dates_to_np",
      [](const DatetimeList& datelist) {
        size_t total = datelist.size();
        HAYAKU_IF_RETURN(total == 0, py::array());

        // Allocate the memory with malloc
        int64_t* data =
            static_cast<int64_t*>(std::malloc(total * sizeof(int64_t)));
        for (size_t i = 0; i < total; i++) {
          data[i] = datelist[i].timestamp() * 1000LL;
        }

        // Define the NumPy structured data type
        py::dtype dtype;
        dtype = py::dtype(vector_to_python_list<string>({"datetime"}),
                          vector_to_python_list<string>({"datetime64[ns]"}),
                          vector_to_python_list<int64_t>({0}), 8);

        // Manage the memory with the capsule
        return py::array(dtype, total, data,
                         py::capsule(data, [](void* p) { std::free(p); }));
      },
      "Convert a DatetimeList to a NumPy tuple");
}

// Registration group: _Log
/*
 * _log.cpp
 *
 *  Created on: 2019-2-11
 *      Author: fasiondog
 */

#include "common/Log.h"
#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_log(py::module& m) {
  py::enum_<LOG_LEVEL>(m, "LOG_LEVEL")
      .value("DEBUG", LOG_LEVEL::LOG_DEBUG)
      .value("TRACE", LOG_LEVEL::LOG_TRACE)
      .value("INFO", LOG_LEVEL::LOG_INFO)
      .value("WARN", LOG_LEVEL::LOG_WARN)
      .value("ERROR", LOG_LEVEL::LOG_ERROR)
      .value("FATAL", LOG_LEVEL::LOG_FATAL)
      .value("OFF", LOG_LEVEL::LOG_OFF)
      .export_values();

  m.def("get_log_level", get_log_level, "Get the current log level");
  m.def("set_log_level", set_log_level, "Set the current log level");
}

// Registration group: _Parameter
/*
 * _Parameter.cpp
 *
 *  Created on: 2013-2-28
 *      Author: fasiondog
 */

#include <common/Parameter.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

bool (*parameter_eq)(const Parameter&, const Parameter&) = &operator==;
bool (*parameter_ne)(const Parameter&, const Parameter&) = &operator!=;
bool (*parameter_lt)(const Parameter&, const Parameter&) = &operator<;

void export_Parameter(py::module& m) {
  py::class_<Parameter>(m, "Parameter",
                        "The parameter class, used by the classes that need "
                        "the named parameter settings, similar to a dict")
      .def(py::init<>())
      .def("__str__", to_py_str<Parameter>)
      .def("__repr__", to_py_str<Parameter>)

      .def("__contains__", &Parameter::have)
      .def("__setitem__",
           static_cast<void (Parameter::*)(const std::string&,
                                           const boost::any&)>(&Parameter::set))
      .def("__getitem__", &Parameter::get<boost::any>)
      .def("have", &Parameter::have,
           "Return True if there is a parameter for the specified name.")
      .def("set", static_cast<void (Parameter::*)(
                      const std::string&, const boost::any&)>(&Parameter::set))
      .def("get", &Parameter::get<boost::any>)
      .def("type", &Parameter::type,
           "Get the type name of the specified parameter, return 'string' | "
           "'int' | 'double' | "
           "'bool' | 'Stock' | 'KQuery' | 'KData' | 'PriceList' | "
           "'DatetimeList'")
      .def("get_name_list", &Parameter::getNameList,
           "Get all the parameter names list")
      .def("get_name_value_list", &Parameter::getNameValueList,
           "Return a string, like 'name1=val1,name2=val2,...'")

      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self < py::self)

          DEF_PICKLE(Parameter);
}

// Registration group: _TimeDelta
/*
 * _TimeDelta.cpp
 *
 *  Copyright (C) 2019 hikyuu.org
 *
 *  Created on: 2019-12-14
 *      Author: fasiondog
 */

#include <common/serialization/TimeDelta_serialization.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

double (TimeDelta::*TimeDelta_div_1)(TimeDelta) const = &TimeDelta::operator/;
TimeDelta (TimeDelta::*TimeDelta_div_2)(double) const = &TimeDelta::operator/;

TimeDelta (TimeDelta::*TimeDelta_pos)() const = &TimeDelta::operator+;
TimeDelta (TimeDelta::*TimeDelta_neg)() const = &TimeDelta::operator-;

void export_TimeDelta(py::module& m) {
  py::class_<TimeDelta>(
      m, "TimeDelta",
      R"(The time duration, used for the time calculation. It can be built in the following ways:

    - Build from a datetime.timedelta. TimeDelta(a timedelta instance)
    - TimeDelta(days=0, hours=0, minutes=0, seconds=0, milliseconds=0, microseconds=0)

        - -99999999 <= days <= 99999999
        - -100000 <= hours <= 100000
        - -100000 <= minutes <= 100000
        - -8639900 <= seconds <= 8639900
        - -86399000000 <= milliseconds <= 86399000000
        - -86399000000 <= microseconds <= 86399000000

    The parameter limits above are mainly to prevent a possible overflow when summing the total microseconds. When only one parameter is used and the limits above are not desired, the shortcut functions can be used:
    Days, Hours, Minutes, Seconds, Milliseconds, Microseconds)")

      .def(py::init<>())
      .def(py::init<int64_t, int64_t, int64_t, int64_t, int64_t, int64_t>(),
           py::arg("days") = 0, py::arg("hours") = 0, py::arg("minutes") = 0,
           py::arg("seconds") = 0, py::arg("milliseconds") = 0,
           py::arg("microseconds") = 0)

      .def("__str__", &TimeDelta::str)
      .def("__repr__", &TimeDelta::repr)

      .def_property_readonly("days", &TimeDelta::days,
                             "The number of days [-99999999, 99999999]")
      .def_property_readonly("hours", &TimeDelta::hours,
                             "The number of hours [0, 23]")
      .def_property_readonly("minutes", &TimeDelta::minutes,
                             "The number of minutes [0, 59]")
      .def_property_readonly("seconds", &TimeDelta::seconds,
                             "The number of seconds [0, 59]")
      .def_property_readonly("milliseconds", &TimeDelta::milliseconds,
                             "The number of milliseconds [0, 999]")
      .def_property_readonly("microseconds", &TimeDelta::microseconds,
                             "The number of microseconds [0, 999]")
      .def_property_readonly("ticks", &TimeDelta::ticks,
                             "The same as the total microseconds")

      .def("isNegative", &TimeDelta::isNegative, R"(isNegative(self)

    Whether it is a negative duration

    :rtype: bool)")

      .def("total_days", &TimeDelta::total_days, R"(total_days(self)

    Get the total number of days with decimals

    :rtype: float)")

      .def("total_hours", &TimeDelta::total_hours, R"(total_hours(self)

    Get the total number of hours with decimals

    :rtype: float)")

      .def("total_minutes", &TimeDelta::total_minutes, R"(total_minutes(self)

    Get the total number of minutes with decimals

    :rtype: float)")

      .def("total_seconds", &TimeDelta::total_seconds, R"(total_seconds(self)

    Get the total number of seconds with decimals

    :rtype: float)")

      .def("total_milliseconds", &TimeDelta::total_milliseconds,
           R"(total_milliseconds(self)

    Get the total number of milliseconds with decimals

    :rtype: float)")

      .def("max", &TimeDelta::max, R"(max()

    The maximum supported duration

    :return: TimeDelta(99999999, 23, 59, 59, 999, 999))")

      .def("min", &TimeDelta::min, R"(min()

    The minimum supported duration

    :return: TimeDelta(-99999999, 0, 0, 0, 0, 0))")

      .def("resolution", &TimeDelta::resolution, R"(resolution()

    The minimum supported precision

    :return: TimeDelta(0, 0, 0, 0, 0, 1))")

      .def("max_ticks", &TimeDelta::maxTicks, R"(max_ticks()

    The maximum supported ticks (i.e. the number of microseconds)

    :rtype: int)")

      .def("min_ticks", &TimeDelta::minTicks, R"(min_ticks()

    The minimum supported ticks (i.e. the number of microseconds)

    :rtype: int)")

      .def("from_ticks", &TimeDelta::fromTicks, R"(from_ticks(ticks)

    Create with the ticks (i.e. the number of microseconds) value

    :param int ticks: the number of microseconds
    :rtype: TimeDelta)")

      .def(py::hash(py::self))
      .def(py::self == py::self)
      .def(py::self != py::self)
      .def(py::self >= py::self)
      .def(py::self <= py::self)
      .def(py::self > py::self)
      .def(py::self < py::self)
      .def(-py::self)
      .def(+py::self)
      .def(py::self + py::self)
      .def(py::self - py::self)
      .def(py::self % py::self)
      .def(py::self * float())

      .def("__abs__", &TimeDelta::abs)
      .def("__rmul__", &TimeDelta::operator*)
      .def("__truediv__", TimeDelta_div_1)
      .def("__truediv__", TimeDelta_div_2)
      .def("__floordiv__", &TimeDelta::floorDiv)

          DEF_PICKLE(TimeDelta);

  m.def("Days", Days, R"(Days(days)

      The TimeDelta shortcut creation function

      :param int days: the number of days [-99999999, 99999999]
      :rtype: TimeDelta)");

  m.def("Hours", Hours, R"(Hours(hours)

      The TimeDelta shortcut creation function

      :param int hours: the number of hours
      :rtype: TimeDelta)");

  m.def("Minutes", Minutes, R"(Minutes(mins)

      The TimeDelta shortcut creation function

      :param int mins: the number of minutes
      :rtype: TimeDelta)");

  m.def("Seconds", Seconds, R"(Seconds(secs)

      The TimeDelta shortcut creation function

      :param int secs: the number of seconds
      :rtype: TimeDelta)");

  m.def("Milliseconds", Milliseconds, R"(Milliseconds(milliseconds)

      The TimeDelta shortcut creation function

      :param int milliseconds: the number of milliseconds
      :rtype: TimeDelta)");

  m.def("Microseconds", Microseconds, R"(Microseconds(microsecs)

      The TimeDelta shortcut creation function

      :param int microsecs: the number of microseconds
      :rtype: TimeDelta)");

  m.def("UTCOffset", UTCOffset, R"(UTCOffset()

      Get the current system UTC offset

      :rtype: TimeDelta)");
}

// Registration group: _arithmetic
/*
 * _util.cpp
 *
 *  Created on: 2011-12-4
 *      Author: fasiondog
 */

#include <common/Arithmetic.h>

#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

void export_util(py::module& m) {
  m.def("roundEx", roundEx<float>, py::arg("number"), py::arg("ndigits") = 0);
  m.def("roundEx", roundEx<double>, py::arg("number"), py::arg("ndigits") = 0,
        R"(roundEx(number[, ndigits=0])

    Round half up, with the ROUND_HALF_EVEN banker's rounding

    :param float number  the data to round
    :param int ndigits the number of the decimal places to keep
    :rype: float)");

  m.def("roundUp", roundUp<float>, py::arg("number"), py::arg("ndigits") = 0);
  m.def("roundUp", roundUp<double>, py::arg("number"), py::arg("ndigits") = 0,
        R"(roundUp(number[, ndigits=0])

    Round up, e.g. 10.1 becomes 11 after rounding

    :param float number  the data to process
    :param int ndigits the number of the decimal places to keep
    :rtype: float)");

  m.def("roundDown", roundDown<float>, py::arg("number"),
        py::arg("ndigits") = 0);
  m.def("roundDown", roundDown<double>, py::arg("number"),
        py::arg("ndigits") = 0,
        R"(roundDown(number[, ndigits=0])

    Round down, e.g. 10.1 becomes 10 after rounding

    :param float number  the data to process
    :param int ndigits the number of the decimal places to keep
    :rtype: float)");
}

// Registration group: bind_stl
/*
 *  Copyright (c) 2019~2023, hikyuu.org
 *
 *  History:
 *    1. 20231231 added by fasiondog
 */

#include <hayaku.h>

#include "AnyConversion.h"
#include "common/PybindSupport.h"

using namespace hayaku;
namespace py = pybind11;

// pybind converts between the vector and the list by default, which affects the
// performance when the data volume is too large Only consider exporting the
// types that may affect the performance

void export_bind_stl(py::module& m) {
  // py::bind_vector<PriceList>(m, "PriceList");
  // py::bind_vector<StringList>(m, "StringList");
  py::bind_vector<DatetimeList>(m, "DatetimeList");
  py::bind_vector<KRecordList>(m, "KRecordList");
  // py::bind_vector<StockList>(m, "StockList");
  py::bind_vector<StockWeightList>(m, "StockWeightList");
  // py::bind_vector<IndicatorList>(m, "Indicatorist");
  py::bind_vector<TimeLineList>(m, "TimeLineList");
  py::bind_vector<TransList>(m, "TransList");
  // py::bind_vector<BorrowRecordList>(m, "BorrowRecordList");
  // py::bind_vector<LoanRecordList>(m, "LoanRecordList");
  py::bind_vector<PositionRecordList>(m, "PositionRecordList");
  // py::bind_vector<FundsList>(m, "FundsList");
  py::bind_vector<TradeRecordList>(m, "TradeRecordList");
}

// Registration group: ioredirect
/*
 * ioredirect.cpp
 *
 *  Created on: 2018-08-27
 *      Author: fasiondog
 */

#include <pybind11/iostream.h>

#include "PybindSupport.h"

namespace hayaku {

pybind11::detail::OstreamRedirect OStreamToPython::ms_io_redirect(true, true);
bool OStreamToPython::ms_opened{false};

void open_ostream_to_python() {
  if (!OStreamToPython::ms_opened) {
    OStreamToPython::ms_io_redirect.enter();
    OStreamToPython::ms_opened = true;
  }
}

void close_ostream_to_python() {
  if (OStreamToPython::ms_opened) {
    OStreamToPython::ms_io_redirect.exit();
    OStreamToPython::ms_opened = false;
  }
}

}  // namespace hayaku

namespace py = pybind11;
using namespace hayaku;

void export_io_redirect(py::module& m) {
  m.def("open_ostream_to_python", open_ostream_to_python);
  m.def("close_ostream_to_python", close_ostream_to_python);
}

void bindCommon(py::module_& m) {
  export_bind_stl(m);
  export_Constant(m);
  export_util(m);
  export_log(m);
  export_Datetime(m);
  export_TimeDelta(m);
  export_Parameter(m);
  export_io_redirect(m);

  m.def("set_python_in_jupyter", setPythonInJupyter);
  m.def("set_python_in_interactive", setPythonInInteractive);

  m.def("close_spend_time", close_spend_time,
        "Globally disable the c++ part time-spending printing");
  m.def("open_spend_time", close_spend_time,
        "Globally enable the c++ part time-spending printing");

  m.def("get_version", getVersion, R"(getVersion()

        :return: the current version of hayaku
        :rtype: str)");

  m.def("get_version_with_build", getVersionWithBuild);
  m.def("get_version_git", getVersionWithGit);
  m.def("get_latest_version_info", []() {
    auto info = getLatestVersionInfo();
    py::dict ret;
    ret["version"] = info.version;
    ret["remark"] = info.remark;
    ret["release_date"] = info.release_date;
    return ret;
  });
  m.def("can_upgrade", CanUpgrade);

  m.def(
      "htr", [](const std::string& key) { return lang_htr(key.c_str()); },
      py::arg("key"),
      R"(htr(key)

    Translate the given text into the current runtime language.

    It shares the same gettext domain with the C++ part (the msgid must be in English), so that the
    python-side text, such as the drawing labels, always appears in the same language as the C++
    output, such as Performance.report().

    The placeholders in the msgid use the python style ``{}``, and the caller formats the returned
    text itself::

        htr("Account({}) cumulative return").format(tm.name)

    :param str key: the text in English (the msgid)
    :return: the translated text; the original text is returned when no translation exists
    :rtype: str)");
}
