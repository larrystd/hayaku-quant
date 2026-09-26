/*
 * KDataDriver.cpp
 *
 *  Created on: 2014-9-2
 *      Author: fasiondog
 */

#include "KDataDriver.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const KDataDriver& driver) {
  os << "KDataDriver(" << driver.name() << ", " << driver.getParameter() << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const KDataDriverPtr& driver) {
  if (driver) {
    os << *driver;
  } else {
    os << "KDataDriver(NULL)";
  }

  return os;
}

KDataDriver::KDataDriver() : m_name("") {}

KDataDriver::KDataDriver(const Parameter& params) : m_params(params) {}

KDataDriver::KDataDriver(const string& name) : m_name(name) {
  to_upper(m_name);
}

shared_ptr<KDataDriver> KDataDriver::clone() {
  shared_ptr<KDataDriver> ptr = _clone();
  ptr->m_params = m_params;
  ptr->m_name = m_name;
  ptr->m_is_python_object = m_is_python_object;
  ptr->_init();
  return ptr;
}

bool KDataDriver::checkType() {
  bool result = false;
  try {
    string type = getParam<string>("type");
    to_upper(type);
    if (type == m_name) {
      result = true;
    } else {
      result = false;
      HAYAKU_WARN("Type of driver mismatch! ({} != {})", type, m_name);
    }

  } catch (...) {
    result = false;
    HAYAKU_ERROR("Can't get type of driver!");
  }

  return result;
}

bool KDataDriver::init(const Parameter& params) {
  HAYAKU_IF_RETURN(m_params == params, true);
  m_params = params;
  HAYAKU_IF_RETURN(!checkType(), false);
  return _init();
}

size_t KDataDriver::getCount(const string& market, const string& code,
                             const KQuery::KType& kType) {
  HAYAKU_INFO("The getCount method has not been implemented! (KDataDriver: {})",
              m_name);
  return 0;
}

bool KDataDriver::getIndexRangeByDate(const string& market, const string& code,
                                      const KQuery& query, size_t& out_start,
                                      size_t& out_end) {
  HAYAKU_INFO(
      "The getIndexRangeByDate method has not been implemented! (KDataDriver: "
      "{})",
      m_name);
  return false;
}

KRecordList KDataDriver::getKRecordList(const string& market,
                                        const string& code,
                                        const KQuery& query) {
  HAYAKU_INFO(
      "The getKRecordList method has not been implemented! (KDataDriver: {})",
      m_name);
  return KRecordList();
}

bool KDataDriver::tryGetKRecordView(const string& market, const string& code,
                                    const KQuery::KType& kType, size_t start_ix,
                                    size_t end_ix, KRecordView& out) {
  // The zero-copy view is not supported by default
  return false;
}

TimeLineList KDataDriver::getTimeLineList(const string& market,
                                          const string& code,
                                          const KQuery& query) {
  HAYAKU_INFO(
      "The getTimeLineList method has not been implemented! (KDataDriver: {})",
      m_name);
  return TimeLineList();
}

TransList KDataDriver::getTransList(const string& market, const string& code,
                                    const KQuery& query) {
  HAYAKU_INFO(
      "The getTransList method has not been implemented! (KDataDriver: {})",
      m_name);
  return TransList();
}

std::unordered_map<std::string, KRecordList> KDataDriver::getAllKRecordList(
    const KQuery::KType& ktype, const Datetime& start_date,
    const std::atomic_bool& cancel_flag) {
  HAYAKU_INFO(
      "The getAllKRecordList method has not been implemented! (KDataDriver: "
      "{})",
      m_name);
  return std::unordered_map<std::string, KRecordList>();
}

} /* namespace hayaku */
