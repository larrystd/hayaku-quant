/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include "BaseInfoDriver.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const BaseInfoDriver& driver) {
  os << "BaseInfoDriver(" << driver.name() << ", " << driver.getParameter()
     << ")";
  return os;
}

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const BaseInfoDriverPtr& driver) {
  if (driver) {
    os << *driver;
  } else {
    os << "BaseInfoDriver(NULL)";
  }

  return os;
}

BaseInfoDriver::BaseInfoDriver(const string& name) : name_(name) {
  to_upper(name_);
}

bool BaseInfoDriver::checkType() {
  bool result = false;
  try {
    string type = getParam<string>("type");
    to_upper(type);
    if (type == name_) {
      result = true;
    } else {
      result = false;
      HAYAKU_WARN("Type of driver mismatch! ({} != {})", type, name_);
    }

  } catch (...) {
    result = false;
    HAYAKU_ERROR("Can't get type of driver!");
  }

  return result;
}

bool BaseInfoDriver::init(const Parameter& params) {
  HAYAKU_IF_RETURN(params_ == params, true);
  params_ = params;
  HAYAKU_IF_RETURN(!checkType(), false);
  HAYAKU_INFO("Using {} BaseInfoDriver", name());
  return _init();
}

Parameter BaseInfoDriver::getFinanceInfo(const string& market,
                                         const string& code) {
  HAYAKU_INFO(
      "The getFinanceInfo method has not been implemented! (BaseInfoDriver: "
      "{})",
      name_);
  return Parameter();
}

StockWeightList BaseInfoDriver::getStockWeightList(const string& market,
                                                   const string& code,
                                                   Datetime start,
                                                   Datetime end) {
  HAYAKU_INFO(
      "The getStockWeightList method has not been implemented! "
      "(BaseInfoDriver: {})",
      name_);
  return StockWeightList();
}

} /* namespace hayaku */
