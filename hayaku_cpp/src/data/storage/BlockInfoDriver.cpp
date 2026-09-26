/*
 * BaseInfoDriver.cpp
 *
 *  Created on: 2017-10-8
 *      Author: fasiondog
 */

#include "BlockInfoDriver.h"

namespace hayaku {

std::ostream& operator<<(std::ostream& os, const BlockInfoDriver& driver) {
  os << "BlockInfoDriver(" << driver.name() << ", " << driver.getParameter()
     << ")";
  return os;
}

std::ostream& operator<<(std::ostream& os, const BlockInfoDriverPtr& driver) {
  if (driver) {
    os << *driver;
  } else {
    os << "BlockInfoDriver(NULL)";
  }

  return os;
}

BlockInfoDriver::BlockInfoDriver(const string& name) : name_(name) {
  to_upper(name_);
}

bool BlockInfoDriver::checkType() {
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

bool BlockInfoDriver::init(const Parameter& params) {
  HAYAKU_IF_RETURN(params_ == params, true);
  params_ = params;
  HAYAKU_IF_RETURN(!checkType(), false);
  return _init();
}

} /* namespace hayaku */
