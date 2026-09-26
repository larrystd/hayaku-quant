/*
 * KQuery.cpp
 *
 *  Created on: 2012-9-23
 *      Author: fasiondog
 */

#include "KQuery.h"

#include <xxhash.h>

#include <boost/functional/hash.hpp>

#include "KDataExtension.h"

namespace hayaku {

const string KQuery::MIN("MIN");
const string KQuery::MIN5("MIN5");
const string KQuery::MIN15("MIN15");
const string KQuery::MIN30("MIN30");
const string KQuery::MIN60("MIN60");
const string KQuery::DAY("DAY");
const string KQuery::WEEK("WEEK");
const string KQuery::MONTH("MONTH");
const string KQuery::QUARTER("QUARTER");
const string KQuery::HALFYEAR("HALFYEAR");
const string KQuery::YEAR("YEAR");

const string KQuery::DAY3{"DAY3"};
const string KQuery::DAY5{"DAY5"};
const string KQuery::DAY7{"DAY7"};
const string KQuery::MIN3("MIN3");
const string KQuery::HOUR2("HOUR2");
const string KQuery::HOUR4("HOUR4");
const string KQuery::HOUR6("HOUR6");
const string KQuery::HOUR12("HOUR12");
const string KQuery::TIMELINE("TIMELINE");  // Time-sharing
const string KQuery::TRANS("TRANS");        // Tick

// All the base K-line types (i.e. the K-line types with an actual physical
// storage)
static std::unordered_set<string> g_all_base_ktype{
    KQuery::MIN,      KQuery::MIN5,     KQuery::MIN15, KQuery::MIN30,
    KQuery::MIN60,    KQuery::DAY,      KQuery::WEEK,  KQuery::MONTH,
    KQuery::QUARTER,  KQuery::HALFYEAR, KQuery::YEAR,  KQuery::HOUR2,
    KQuery::TIMELINE, KQuery::TRANS};

static unordered_map<string, int32_t> g_ktype2min{
    {KQuery::MIN, 1},
    {KQuery::MIN3, 3},

    {KQuery::MIN5, 5},
    {KQuery::MIN15, 15},
    {KQuery::MIN30, 30},
    {KQuery::MIN60, 60},
    {KQuery::HOUR2, 60 * 2},

    {KQuery::DAY, 60 * 24},
    {KQuery::WEEK, 60 * 24 * 7},
    {KQuery::MONTH, 60 * 24 * 30},
    {KQuery::QUARTER, 60 * 24 * 30 * 3},
    {KQuery::HALFYEAR, 60 * 24 * 30 * 6},
    {KQuery::YEAR, 60 * 24 * 365},

    {KQuery::TIMELINE, 1},
    //   {KQuery::TRANS, 1}
};

static unordered_map<string, int64_t> g_ktype2sec{
    {KQuery::TRANS, 3},
};

// Get all the KTypes
vector<KQuery::KType> KQuery::getBaseKTypeList() noexcept {
  vector<KQuery::KType> ret;
  for (const auto& v : g_all_base_ktype) {
    ret.push_back(v);
  }
  return ret;
}

vector<KQuery::KType> KQuery::getExtraKTypeList() {
  return hayaku::getExtraKTypeList();
}

int32_t KQuery::getKTypeInMin(const KType& ktype) {
  string nktype(ktype);
  to_upper(nktype);
  auto iter = g_ktype2min.find(nktype);
  HAYAKU_IF_RETURN(iter != g_ktype2min.end(), iter->second);

  int32_t sec = getKTypeExtraMinutes(nktype);
  HAYAKU_WARN_IF(sec <= 0, "Can't get KType in minutes: {}, will return 0",
                 nktype);
  return sec;
}

int32_t KQuery::getBaseKTypeInMin(const KType& ktype) noexcept {
  string nktype(ktype);
  to_upper(nktype);
  auto iter = g_ktype2min.find(nktype);
  if (iter != g_ktype2min.end()) {
    return iter->second;
  }
  HAYAKU_WARN("Can't get base KType in minutes: {}, will return 0", nktype);
  return 0;
}

int64_t KQuery::getKTypeInSeconds(const KType& ktype) {
  string nktype(ktype);
  to_upper(nktype);
  auto iter = g_ktype2min.find(nktype);
  HAYAKU_IF_RETURN(iter != g_ktype2min.end(), iter->second * 60);

  auto sec = getKTypeExtraMinutes(nktype) * 60;
  if (sec <= 0) {
    auto sec_iter = g_ktype2sec.find(nktype);
    HAYAKU_IF_RETURN(sec_iter != g_ktype2sec.end(), sec_iter->second);
    HAYAKU_WARN("Can't get KType in seconds: {}, will return 0", nktype);
  }
  return sec;
}

bool KQuery::isValidKType(const string& ktype) {
  return isBaseKType(ktype) || hayaku::isExtraKType(ktype);
}

bool KQuery::isBaseKType(const string& ktype) noexcept {
  string nktype(ktype);
  to_upper(nktype);
  auto iter = g_all_base_ktype.find(nktype);
  return iter != g_all_base_ktype.end();
}

bool KQuery::isExtraKType(const string& ktype) {
  return hayaku::isExtraKType(ktype);
}

KQuery::KQuery(Datetime start, Datetime end, const KType& ktype,
               RecoverType recoverType)
    : start_(start.ymdhms()),
      end_(end.ymdhms()),
      query_type_(KQuery::DATE),
      data_type_(ktype),
      recover_type_(recoverType) {
  to_upper(data_type_);
}

Datetime KQuery::startDatetime() const {
  HAYAKU_IF_RETURN(query_type_ != DATE || (uint64_t)start_ == Null<uint64_t>(),
                   Null<Datetime>());
  return Datetime(start_);
}

Datetime KQuery::endDatetime() const {
  HAYAKU_IF_RETURN(query_type_ != DATE || (uint64_t)end_ == Null<uint64_t>(),
                   Null<Datetime>());
  return Datetime(end_);
}

uint64_t KQuery::hash() const {
  XXH64_state_t* state = XXH64_createState();
  HAYAKU_IF_RETURN(!state, 0);

  uint64_t seed = 0;
  XXH64_reset(state, seed);
  XXH64_update(state, &start_, sizeof(start_));
  XXH64_update(state, &end_, sizeof(end_));
  XXH64_update(state, &query_type_, sizeof(query_type_));
  XXH64_update(state, &recover_type_, sizeof(recover_type_));
  XXH64_update(state, data_type_.data(), data_type_.size());

  // Get the final hash value
  uint64_t result = XXH64_digest(state);
  XXH64_freeState(state);
  return result;
}

string KQuery::getQueryTypeName(QueryType queryType) {
  switch (queryType) {
    case INDEX:
      return "INDEX";
    case DATE:
      return "DATE";
    default:
      return "INVALID";
  }
}

KQuery::QueryType KQuery::getQueryTypeEnum(const string& arg) {
  string name(arg);
  to_upper(name);
  HAYAKU_IF_RETURN("INDEX" == name, INDEX);
  HAYAKU_IF_RETURN("DATE" == name, DATE);
  return INVALID;
}

string KQuery::getKTypeName(const KType& dataType) {
  string result(dataType);
  to_upper(result);
  return result;
}

KQuery::KType KQuery::getKTypeEnum(const string& arg) {
  string name(arg);
  to_upper(name);
  return name;
}

string KQuery::getRecoverTypeName(RecoverType recoverType) {
  switch (recoverType) {
    case NO_RECOVER:
      return "NO_RECOVER";
    case FORWARD:
      return "FORWARD";
    case BACKWARD:
      return "BACKWARD";
    case EQUAL_FORWARD:
      return "EQUAL_FORWARD";
    case EQUAL_BACKWARD:
      return "EQUAL_BACKWARD";
    default:
      return "INVALID_RECOVER_TYPE";
  }
}

KQuery::RecoverType KQuery::getRecoverTypeEnum(const string& arg) {
  string name(arg);
  to_upper(name);
  HAYAKU_IF_RETURN("NO_RECOVER" == name, NO_RECOVER);
  HAYAKU_IF_RETURN("FORWARD" == name, FORWARD);
  HAYAKU_IF_RETURN("BACKWARD" == name, BACKWARD);
  HAYAKU_IF_RETURN("EQUAL_FORWARD" == name, EQUAL_FORWARD);
  HAYAKU_IF_RETURN("EQUAL_BACKWARD" == name, EQUAL_BACKWARD);
  return INVALID_RECOVER_TYPE;
}

std::ostream& operator<<(std::ostream& os, const KQuery& query) {
  string strip(", ");
  if (query.queryType() == KQuery::INDEX) {
    os << "KQuery(" << query.start() << strip << query.end() << strip
       << KQuery::getQueryTypeName(query.queryType()) << strip
       << KQuery::getKTypeName(query.kType()) << strip
       << KQuery::getRecoverTypeName(query.recoverType()) << ")";
  } else {
    os << "KQueryByDate(" << query.startDatetime() << strip
       << query.endDatetime() << strip
       << KQuery::getQueryTypeName(query.queryType()) << strip
       << KQuery::getKTypeName(query.kType()) << strip
       << KQuery::getRecoverTypeName(query.recoverType()) << ")";
  }
  return os;
}

bool operator!=(const KQuery& q1, const KQuery& q2) noexcept {
  // cppcheck-suppress [mismatchingContainerExpression]
  HAYAKU_IF_RETURN(q1.queryType() != q2.queryType(), true);
  if (q1.queryType() == KQuery::DATE) {
    return q1.kType() != q2.kType() || q1.recoverType() != q2.recoverType() ||
           q1.startDatetime() != q2.startDatetime() ||
           q1.endDatetime() != q2.endDatetime();
  }
  return q1.kType() != q2.kType() || q1.recoverType() != q2.recoverType() ||
         q1.start() != q2.start() || q1.end() != q2.end();
}

bool operator==(const KQuery& q1, const KQuery& q2) noexcept {
  // cppcheck-suppress [mismatchingContainerExpression]
  HAYAKU_IF_RETURN(q1.queryType() != q2.queryType(), false);
  if (q1.queryType() == KQuery::DATE) {
    return q1.kType() == q2.kType() && q1.recoverType() == q2.recoverType() &&
           q1.startDatetime() == q2.startDatetime() &&
           q1.endDatetime() == q2.endDatetime();
  }
  return q1.kType() == q2.kType() && q1.recoverType() == q2.recoverType() &&
         q1.start() == q2.start() && q1.end() == q2.end();
}

}  // namespace hayaku
