/*
 *  Copyright(C) 2021 hikyuu.org
 *
 *  Create on: 2021-02-10
 *     Author: fasiondog
 */

#include "StrategyContext.h"

namespace hayaku {

HAYAKU_API std::ostream& operator<<(std::ostream& os,
                                    const StrategyContext& context) {
  os << context.str();
  return os;
}

string StrategyContext::str() const {
  std::stringstream os;
  os << "StrategyContext{\n"
     << "  start datetime: " << start_datetime_ << ",\n"
     << "  stock code list: [";
  size_t len = stock_code_list_.size();
  if (len > 5) {
    len = 5;
  }
  for (size_t i = 0; i < len; i++) {
    os << "\"" << stock_code_list_[i] << "\", ";
  }
  if (stock_code_list_.size() >= 5) {
    os << "...";
  }
  os << "],\n"
     << "  ktype list: [";
  for (size_t i = 0, total = ktype_list_.size(); i < total; i++) {
    os << "\"" << ktype_list_[i] << "\", ";
  }
  os << "],\n"
     << "  default load: [";
  for (size_t i = 0, total = must_load_.size(); i < total; i++) {
    os << "\"" << must_load_[i] << "\", ";
  }
  os << "],\n"
     << "}";
  return os.str();
}

StrategyContext::StrategyContext(const vector<string>& stockCodeList) {
  _removeDuplicateCode(stockCodeList);
}

StrategyContext::StrategyContext(
    const vector<string>& stockCodeList, const vector<KQuery::KType>& ktypeList,
    const unordered_map<string, int64_t>& preloadNum) {
  _removeDuplicateCode(stockCodeList);
  _checkAndRemoveDuplicateKType(ktypeList);
  setPreloadNum(preloadNum);
}

void StrategyContext::setPreloadNum(
    const unordered_map<string, int64_t>& preloadNum) {
  preload_num_.clear();
  preload_num_.reserve(preloadNum.size());
  for (auto it = preloadNum.cbegin(); it != preloadNum.cend(); ++it) {
    string key = it->first;
    to_lower(key);
    if (it->second > 0) {
      preload_num_[key] = it->second;
    } else {
      HAYAKU_WARN("Invalid preload number {}: {}", key, it->second);
    }
  }
}

void StrategyContext::_removeDuplicateCode(
    const vector<string>& stockCodeList) {
  stock_code_list_.clear();
  stock_code_list_.reserve(stockCodeList.size());
  std::set<string> code_set;
  for (const auto& code : stockCodeList) {
    if (code_set.find(code) == code_set.end()) {
      stock_code_list_.push_back(code);
    } else {
      code_set.insert(code);
    }
  }
}

void StrategyContext::_checkAndRemoveDuplicateKType(
    const vector<KQuery::KType>& ktypeList) {
  ktype_list_.clear();
  ktype_list_.reserve(ktypeList.size());
  std::set<KQuery::KType> ktype_set;
  for (const auto& ktype : ktypeList) {
    auto upktype = ktype;
    to_upper(upktype);
    HAYAKU_CHECK(KQuery::isBaseKType(upktype), "Invalid ktype: {}", upktype);
    if (ktype_set.find(upktype) == ktype_set.end()) {
      ktype_list_.push_back(upktype);
    } else {
      ktype_set.insert(upktype);
    }
  }
}

bool StrategyContext::isAll() const noexcept {
  return std::find_if(stock_code_list_.begin(), stock_code_list_.end(),
                      [](string val) {
                        to_upper(val);
                        return val == "ALL";
                      }) != stock_code_list_.end();
}

vector<string> StrategyContext::getAllNeedLoadStockCodeList() const noexcept {
  vector<string> ret;
  std::set<string> code_set;
  for (const auto& code : must_load_) {
    ret.push_back(code);
    code_set.insert(code);
  }
  for (const auto& code : stock_code_list_) {
    if (code_set.find(code) == code_set.end()) {
      ret.push_back(code);
    }
  }
  return ret;
}

}  // namespace hayaku