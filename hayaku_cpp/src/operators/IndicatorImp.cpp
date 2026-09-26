/*
 * IndicatorImp.cpp
 *
 *  Created on: 2013-2-9
 *      Author: fasiondog
 */
#include <algorithm>
#include <atomic>
#include <forward_list>
#include <stack>
#include <stdexcept>

#include "IndParam.h"
#include "Indicator.h"
#include "common/Log.h"
#include "data/Stock.h"

#if HAYAKU_ENABLE_MIMALLOC
#include <mimalloc.h>
#endif

#if HAYAKU_LOCAL_VECTORIZE
#include <Eigen/Dense>
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_CLASS_EXPORT(hayaku::IndicatorImp)
#endif

namespace hayaku {

bool IndicatorImp::ms_enable_increment_calculate{true};

void IndicatorImp::initEngine() {
#if HAYAKU_ENABLE_MIMALLOC
  mi_option_enable(mi_option_large_os_pages);  // Enable the large pages
  mi_option_enable(mi_option_use_numa_nodes);  // Enable the NUMA support
  // mi_option_set(mi_option_purge_delay, 0);
  // mi_option_set(mi_option_purge_delay, 100);
  // mi_option_disable(mi_option_purge_decommits);

  mi_stats_reset();
// mi_stats_print(NULL);
#endif

#if HAYAKU_LOCAL_VECTORIZE
  fmt::print("simd instruction sets in use: {}\n",
             Eigen::SimdInstructionSetsInUse());
#endif
}

void IndicatorImp::releaseEngine() {
#if HAYAKU_ENABLE_MIMALLOC
// mi_stats_print(NULL);
#endif
}

string HAYAKU_API getOPTypeName(IndicatorImp::OPType op) {
  string name;
  switch (op) {
    case IndicatorImp::LEAF:
      name = "LEAF";
      break;

    case IndicatorImp::OP:
      name = "OP";
      break;

    case IndicatorImp::ADD:
      name = "ADD";
      break;

    case IndicatorImp::SUB:
      name = "SUB";
      break;

    case IndicatorImp::MUL:
      name = "MUL";
      break;

    case IndicatorImp::DIV:
      name = "DIV";
      break;

    case IndicatorImp::MOD:
      name = "MOD";
      break;

    case IndicatorImp::EQ:
      name = "EQ";
      break;

    case IndicatorImp::GT:
      name = "GT";
      break;

    case IndicatorImp::LT:
      name = "LT";
      break;

    case IndicatorImp::NE:
      name = "NE";
      break;

    case IndicatorImp::GE:
      name = "GE";
      break;

    case IndicatorImp::LE:
      name = "LE";
      break;

    case IndicatorImp::AND:
      name = "AND";
      break;

    case IndicatorImp::OR:
      name = "OR";
      break;

    case IndicatorImp::WEAVE:
      name = "WEAVE";
      break;

    case IndicatorImp::OP_IF:
      name = "IF";
      break;

    default:
      name = "UNKNOWN";
      break;
  }
  return name;
}

HAYAKU_API std::ostream &operator<<(std::ostream &os, const IndicatorImp &imp) {
  os << imp.str();
  return os;
}

HAYAKU_API std::ostream &operator<<(std::ostream &os,
                                    const IndicatorImpPtr &imp) {
  if (!imp) {
    os << "Indicator {}";
  } else {
    os << imp->str();
  }
  return os;
}

// The construction id generator. It is defined in this file only (not inline),
// ensuring that there is a single counter across the DLLs: a derived indicator
// may be constructed in a plugin dll (such as extind), but its base class
// constructor is exported and executed by the core dll, so the in-class
// initializer is evaluated on the core side and the id space is unified.
uint64_t IndicatorImp::nextOriginId() noexcept {
  static std::atomic<uint64_t> seq{1};
  return seq.fetch_add(1, std::memory_order_relaxed);
}

uint64_t IndicatorImp::originId() const noexcept { return origin_id_; }

IndicatorImp::IndicatorImp() : name_("IndicatorImp") {
  memset(p_buffer_, 0, sizeof(buffer_t *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name) : name_(name) {
  memset(p_buffer_, 0, sizeof(buffer_t *) * MAX_RESULT_NUM);
}

IndicatorImp::IndicatorImp(const string &name, size_t result_num)
    : name_(name) {
  memset(p_buffer_, 0, sizeof(buffer_t *) * MAX_RESULT_NUM);
  result_num_ = result_num < MAX_RESULT_NUM ? result_num : MAX_RESULT_NUM;
  _readyBuffer(0, result_num_);
}

void IndicatorImp::baseCheckParam(const string &name) const {}

void IndicatorImp::paramChanged() {
  need_calculate_ = true;
  param_changed_ = true;
}

void IndicatorImp::setIndParam(const string &name, const Indicator &ind) {
  IndicatorImpPtr imp = ind.getImp();
  HAYAKU_CHECK(imp, "Invalid input ind, no concrete implementation!");
  ind_params_[name] = imp;
}

void IndicatorImp::setIndParam(const string &name, const IndParam &ind) {
  IndicatorImpPtr imp = ind.getImp();
  HAYAKU_CHECK(imp, "Invalid input ind, no concrete implementation!");
  ind_params_[name] = imp;
}

IndParam IndicatorImp::getIndParam(const string &name) const {
  return IndParam(ind_params_.at(name));
}

const IndicatorImpPtr &IndicatorImp::getIndParamImp(const string &name) const {
  return ind_params_.at(name);
}

bool IndicatorImp::supportIncrementCalculate() const { return false; }

bool IndicatorImp::can_inner_calculate() {
  if (need_calculate_ || !ms_enable_increment_calculate || result_num_ == 0 ||
      context_.empty() || size() < context_.size() ||
      old_context_.size() < context_.size() || !supportIncrementCalculate()) {
    return false;
  }

  if (context_.front().datetime < old_context_.front().datetime ||
      context_.back().datetime > old_context_.back().datetime) {
    return false;
  }

  if (context_.getStock() != old_context_.getStock() ||
      old_context_.getQuery().kType() != context_.getQuery().kType() ||
      old_context_.getQuery().recoverType() !=
          context_.getQuery().recoverType()) {
    return false;
  }

  size_t start_pos = old_context_.getPos(context_.front().datetime);
  if (start_pos == Null<size_t>()) {
    return false;
  }

  size_t last_pos = old_context_.getPos(context_.back().datetime);
  if (last_pos == Null<size_t>()) {
    return false;
  }

  if (start_pos > last_pos) {
    return false;
  }

  size_t total = context_.size();
  if (total != last_pos - start_pos + 1) {
    return false;
  }

  for (size_t r = 0; r < result_num_; ++r) {
    if (p_buffer_[r] == nullptr) {
      return false;
    }
    auto *dst = p_buffer_[r]->data();
    memmove(dst, dst + start_pos, sizeof(value_t) * (total));
    p_buffer_[r]->resize(total);
  }

  discard_ = start_pos >= discard_ ? 0 : discard_ - start_pos;
  need_calculate_ = false;

  return true;
}

void IndicatorImp::setContext(const KData &k) {
  const KData &old_k = getContext();

  // Calculate according to its own identifier when the context has not changed
  if (old_k == k) {
    if (need_calculate_) {
      calculate();
    }
    return;
  }

  onlySetContext(k);
  if (can_inner_calculate()) {
    return;
  }

  need_calculate_ = true;

  // Set the context for the child nodes
  if (left_) left_->setContext(k);
  if (right_) right_->setContext(k);
  if (three_) three_->setContext(k);

  // Set the context for the dynamic parameters
  for (auto iter = ind_params_.begin(); iter != ind_params_.end(); ++iter) {
    iter->second->setContext(k);
  }

  // Start the recalculation
  calculate();

  // Clean up the intermediate calculation data of all the nodes below the root
  // node
  if (!parent_) {
    vector<IndicatorImpPtr> nodes;
    getAllSubNodes(nodes);
    if (ms_enable_increment_calculate) {
      for (const auto &node : nodes) {
        if (!node->need_calculate_ &&
            ((node->optype_ == LEAF || node->optype_ == OP) &&
             !node->supportIncrementCalculate())) {
          node->_clearBuffer();
        }
      }
    } else {
      for (const auto &node : nodes) {
        if (!node->need_calculate_) {
          node->_clearBuffer();
        }
      }
    }
  }
}

void IndicatorImp::_readyBuffer(size_t len, size_t result_num) {
  HAYAKU_CHECK_THROW(result_num <= MAX_RESULT_NUM, std::invalid_argument,
                     "result_num oiverload MAX_RESULT_NUM! {}", name());
  HAYAKU_IF_RETURN(result_num == 0, void());

  value_t null_price = Null<value_t>();
  for (size_t i = 0; i < result_num; ++i) {
    if (!p_buffer_[i]) {
      p_buffer_[i] = new buffer_t(len, null_price);

    } else {
      p_buffer_[i]->resize(len);
      for (size_t j = 0; j < len; ++j) {
        (*p_buffer_[i])[j] = null_price;
      }
    }
  }

  for (size_t i = result_num; i < result_num_; ++i) {
    if (p_buffer_[i]) {
      delete p_buffer_[i];
      p_buffer_[i] = NULL;
    }
  }

  result_num_ = result_num;
}

void IndicatorImp::_clearBuffer() {
  for (size_t i = 0; i < result_num_; ++i) {
    if (p_buffer_[i]) {
      delete p_buffer_[i];
      p_buffer_[i] = NULL;
    }
  }
}

IndicatorImp::~IndicatorImp() {
  for (size_t i = 0; i < result_num_; ++i) {
    delete p_buffer_[i];
  }
}

string IndicatorImp::str() const {
  std::ostringstream os;
  os << "Indicator{\n"
     << "  name: " << name() << "\n  size: " << size()
     << "\n  discard: " << discard()
     << "\n  stock: " << getContext().getStock().market_code()
     << "\n  result sets: " << getResultNumber()
     << "\n  params: " << getParameter()
     << "\n  is python object: " << (isPythonObject() ? "True" : "False");
  const auto &ind_params = getIndParams();
  if (!ind_params.empty()) {
    os << "\n  ind params: {";
    for (auto iter = ind_params.begin(); iter != ind_params.end(); ++iter) {
      os << iter->first << ": " << iter->second->formula() << ", ";
    }
    os << "}";
  }
  os << "\n  formula: " << formula();
  DatetimeList dates = getDatetimeList();
  if (!dates.empty()) {
    os << "\n  first: " << dates.front();
    os << "\n  last: " << dates.back();
  }
  for (size_t r = 0; r < getResultNumber(); ++r) {
    if (p_buffer_[r]) {
      os << "\n  values" << r << ": " << *p_buffer_[r];
    }
  }
  os << "\n}";
  return os.str();
}

void IndicatorImp::swap(IndicatorImp *other) {
  HAYAKU_ASSERT(other != nullptr);
  HAYAKU_IF_RETURN(this == other, void());
  HAYAKU_ASSERT(other->result_num_ == result_num_);
  HAYAKU_ASSERT(other->size() == size());
  for (size_t r = 0; r < result_num_; ++r) {
    buffer_t *tmp = p_buffer_[r];
    p_buffer_[r] = other->p_buffer_[r];
    other->p_buffer_[r] = tmp;
  }
}

void IndicatorImp::swap(IndicatorImp *other, size_t other_result_idx,
                        size_t self_result_idx) {
  HAYAKU_ASSERT(other != nullptr);
  HAYAKU_ASSERT(other->size() == size());
  HAYAKU_ASSERT(other_result_idx < other->result_num_);
  HAYAKU_ASSERT(self_result_idx < result_num_);
  buffer_t *tmp = p_buffer_[self_result_idx];
  p_buffer_[self_result_idx] = other->p_buffer_[other_result_idx];
  other->p_buffer_[other_result_idx] = tmp;
}

IndicatorImpPtr IndicatorImp::clone() {
  IndicatorImpPtr p = _clone();
  p->params_ = params_;
  p->name_ = name_;
  p->origin_id_ =
      origin_id_;  // Copy the origin id: the clone chain shares the identity
  p->is_python_object_ = is_python_object_;
  p->need_self_alike_compare_ = need_self_alike_compare_;
  p->is_serial_ = is_serial_;
  p->discard_ = discard_;
  p->result_num_ = result_num_;
  p->context_ = context_;
  p->old_context_ = old_context_;
  p->need_calculate_ = need_calculate_;
  p->param_changed_ = param_changed_;
  p->optype_ = optype_;
  p->parent_ = parent_;

  p->_readyBuffer(size(), result_num_);
  for (size_t i = 0; i < result_num_; ++i) {
    if (p_buffer_[i])
      std::copy(p_buffer_[i]->begin(), p_buffer_[i]->end(),
                p->p_buffer_[i]->begin());
  }

  if (left_) {
    p->left_ = left_->clone();
    p->left_->parent_ = this;
  }
  if (right_) {
    p->right_ = right_->clone();
    p->right_->parent_ = this;
  }
  if (three_) {
    p->three_ = three_->clone();
    p->three_->parent_ = this;
  }

  for (auto iter = ind_params_.begin(); iter != ind_params_.end(); ++iter) {
    p->ind_params_[iter->first] = iter->second->clone();
  }

  if (!parent_) {
    // Rebuild the parent node of every child node
    std::forward_list<IndicatorImp *> stack;
    stack.push_front(p.get());
    while (!stack.empty()) {
      IndicatorImp *node = stack.front();
      stack.pop_front();
      if (node->three_) {
        node->three_->parent_ = node;
        stack.push_front(node->three_.get());
      }
      if (node->left_) {
        node->left_->parent_ = node;
        stack.push_front(node->left_.get());
      }
      if (node->right_) {
        node->right_->parent_ = node;
        stack.push_front(node->right_.get());
      }
    }

    p->repeatALikeNodes();
  }

  return p;
}

IndicatorImpPtr IndicatorImp::operator()(const Indicator &ind) {
  HAYAKU_INFO("This indicator not support operator()! {}", *this);
  // Guarantee the alignment
  IndicatorImpPtr result = make_shared<IndicatorImp>();
  size_t total = ind.size();
  result->_readyBuffer(total, result_num_);
  result->setDiscard(total);
  return result;
}

void IndicatorImp::setDiscard(size_t discard) noexcept {
  size_t tmp_discard = discard > size() ? size() : discard;
  if (tmp_discard > discard_) {
    value_t null_price = Null<value_t>();
    for (size_t i = 0; i < result_num_; ++i) {
      auto *dst = this->data(i);
      for (size_t j = discard_; j < tmp_discard; ++j) {
        // _set(null_price, j, i);
        dst[j] = null_price;
      }
    }
  }
  discard_ = tmp_discard;
}

string IndicatorImp::long_name() const {
  return name() + "(" + params_.getNameValueList() + ")";
}

PriceList IndicatorImp::getResultAsPriceList(size_t result_num) {
  HAYAKU_IF_RETURN(result_num >= result_num_ || p_buffer_[result_num] == NULL,
                   PriceList());
#if HAYAKU_USE_LOW_PRECISION
  size_t total = size();
  PriceList result(total);
  const auto &src = (*p_buffer_[result_num]);
  std::copy(src.begin(), src.end(), result.begin());
  return result;
#else
  // return (*m_pBuffer[result_num]);
  return PriceList(p_buffer_[result_num]->begin(),
                   p_buffer_[result_num]->end());
#endif
}

IndicatorImpPtr IndicatorImp::getResult(size_t result_num) {
  HAYAKU_IF_RETURN(result_num >= result_num_ || p_buffer_[result_num] == NULL,
                   IndicatorImpPtr());
  IndicatorImpPtr imp = make_shared<IndicatorImp>();
  size_t total = size();
  imp->_readyBuffer(total, 1);
  imp->setDiscard(discard());
  imp->name(name());
  auto const *src = this->data(result_num);
  auto *dst = imp->data(0);
  for (size_t i = imp->discard(); i < total; ++i) {
    dst[i] = src[i];
  }
  return imp;
}

IndicatorImp::value_t IndicatorImp::get(size_t pos, size_t num) const {
#if CHECK_ACCESS_BOUND
  // cppcheck-suppress [arrayIndexOutOfBoundsCond]
  HAYAKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && p_buffer_[num] && pos < p_buffer_[num]->size()),
      std::out_of_range,
      "Try to access value out of bounds! num: {}, pos: {}, name: {}", num, pos,
      name());
#endif
  return (*p_buffer_[num])[pos];
}

IndicatorImp::value_t IndicatorImp::front(size_t num) const {
#if CHECK_ACCESS_BOUND
  // cppcheck-suppress [arrayIndexOutOfBoundsCond]
  HAYAKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && p_buffer_[num] && !p_buffer_[num]->empty()),
      std::out_of_range, "Try to access value out of bounds! num: {}, name: {}",
      num, name());
#endif
  return p_buffer_[num]->front();
}

IndicatorImp::value_t IndicatorImp::back(size_t num) const {
#if CHECK_ACCESS_BOUND
  // cppcheck-suppress [arrayIndexOutOfBoundsCond]
  HAYAKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && p_buffer_[num] && !p_buffer_[num]->empty()),
      std::out_of_range, "Try to access value out of bounds! num: {}, name: {}",
      num, name());
#endif
  return p_buffer_[num]->back();
}

void IndicatorImp::_set(value_t val, size_t pos, size_t num) {
#if CHECK_ACCESS_BOUND
  // cppcheck-suppress [arrayIndexOutOfBoundsCond]
  HAYAKU_CHECK_THROW(
      (num <= MAX_RESULT_NUM && p_buffer_[num] && pos < p_buffer_[num]->size()),
      std::out_of_range,
      "Try to access value out of bounds! num: {}, pos: {}, name: {}", num, pos,
      name());
#endif
  (*p_buffer_[num])[pos] = val;
}

DatetimeList IndicatorImp::getDatetimeList() const {
  HAYAKU_IF_RETURN(haveParam("align_date_list"),
                   getParam<DatetimeList>("align_date_list"));
  return getContext().getDatetimeList();
}

Datetime IndicatorImp::getDatetime(size_t pos) const {
  if (haveParam("align_date_list")) {
    const DatetimeList &dates =
        getParam<const DatetimeList &>("align_date_list");
    return pos < dates.size() ? dates[pos] : Null<Datetime>();
  }
  const KData &k = getContext();
  return pos < k.size() ? k[pos].datetime : Null<Datetime>();
}

IndicatorImp::value_t IndicatorImp::getByDate(Datetime date, size_t num) {
  size_t pos = getPos(date);
  return (pos != Null<size_t>()) ? get(pos, num) : Null<value_t>();
}

size_t IndicatorImp::getPos(Datetime date) const {
  if (haveParam("align_date_list")) {
    const DatetimeList &dates(
        getParam<const DatetimeList &>("align_date_list"));
    auto iter = std::lower_bound(dates.begin(), dates.end(), date);
    if (iter != dates.end() && *iter == date) {
      return iter - dates.begin();
    } else {
      return Null<size_t>();
    }
  }
  return getContext().getPos(date);
}

bool IndicatorImp::existNan(size_t result_idx) const {
  HAYAKU_CHECK(result_idx < result_num_, "result_idx: {}", result_idx);
  const value_t *src = data(result_idx);
  for (size_t i = discard_, total = size(); i < total; i++) {
    if (std::isnan(src[i])) {
      return true;
    }
  }
  return false;
}

string IndicatorImp::formula() const {
  std::stringstream buf;

  switch (optype_) {
    case LEAF:
      buf << name_;
      break;

    case OP:
      buf << name_ << "(" << right_->formula() << ")";
      break;

    case ADD:
      buf << left_->formula() << " + " << right_->formula();
      break;

    case SUB:
      buf << left_->formula() << " - " << right_->formula();
      break;

    case MUL:
      buf << left_->formula() << " * " << right_->formula();
      break;

    case DIV:
      buf << left_->formula() << " / " << right_->formula();
      break;

    case MOD:
      buf << left_->formula() << " % " << right_->formula();
      break;

    case EQ:
      buf << left_->formula() << " == " << right_->formula();
      break;

    case GT:
      buf << left_->formula() << " > " << right_->formula();
      break;

    case LT:
      buf << left_->formula() << " < " << right_->formula();
      break;

    case NE:
      buf << left_->formula() << " != " << right_->formula();
      break;

    case GE:
      buf << left_->formula() << " >= " << right_->formula();
      break;

    case LE:
      buf << left_->formula() << " <= " << right_->formula();
      break;

    case AND:
      buf << left_->formula() << " & " << right_->formula();
      break;

    case OR:
      buf << left_->formula() << " | " << right_->formula();
      break;

    case WEAVE:
      buf << name_ << "(" << left_->formula() << ", " << right_->formula()
          << ")";
      break;

    case OP_IF:
      buf << "IF(" << three_->formula() << ", " << left_->formula() << ", "
          << right_->formula() << ")";
      break;

    default:
      HAYAKU_ERROR("Wrong optype! {}", int(optype_));
      break;
  }

  return buf.str();
}

void IndicatorImp::add(OPType op, IndicatorImpPtr left, IndicatorImpPtr right) {
  HAYAKU_ERROR_IF_RETURN(op == LEAF || op >= INVALID || !right, void(),
                         "Wrong used!");
  if (OP == op && !isLeaf()) {
    if (left_) {
      if (left_->isNeedContext()) {
        if (left_->isLeaf()) {
          need_calculate_ = true;
          left_ = right->clone();
        } else {
          HAYAKU_WARN(
              "Context-dependent indicator can only be at the leaf node!"
              "parent node: {}, try add node: {}",
              name(), right->name());
        }
      } else {
        left_->add(OP, left, right);
      }
    }
    if (right_) {
      if (right_->isNeedContext()) {
        if (right_->isLeaf()) {
          need_calculate_ = true;
          right_ = right->clone();
        } else {
          HAYAKU_WARN(
              "Context-dependent indicator can only be at the leaf node!"
              "parent node: {}, try add node: {}",
              name(), right->name());
        }
      } else {
        right_->add(OP, left, right);
      }
    }
    if (three_) {
      if (three_->isNeedContext()) {
        if (three_->isLeaf()) {
          need_calculate_ = true;
          three_ = right->clone();
        } else {
          HAYAKU_WARN(
              "Context-dependent indicator can only be at the leaf node!"
              "parent node: {}, try add node: {}",
              name(), right->name());
        }
      } else {
        three_->add(OP, left, right);
      }
    }
  } else {
    need_calculate_ = true;
    optype_ = op;
    left_ = left ? left->clone() : left;
    right_ = right->clone();
  }

  if (left_) {
    left_->parent_ = this;
  }

  if (right_) {
    right_->parent_ = this;
  }

  if (three_) {
    three_->parent_ = this;
  }

  if (name_ == "IndicatorImp") {
    name_ = getOPTypeName(op);
  }

  if (!parent_) {
    repeatALikeNodes();
  }
}

void IndicatorImp::add_if(IndicatorImpPtr cond, IndicatorImpPtr left,
                          IndicatorImpPtr right) {
  HAYAKU_ERROR_IF_RETURN(!cond || !left || !right, void(), "Wrong used!");
  need_calculate_ = true;
  optype_ = IndicatorImp::OP_IF;
  three_ = cond->clone();
  left_ = left->clone();
  right_ = right->clone();
  three_->parent_ = this;
  left_->parent_ = this;
  right_->parent_ = this;
  if (name_ == "IndicatorImp") {
    name_ = getOPTypeName(IndicatorImp::OP_IF);
  }
  if (!parent_) {
    repeatALikeNodes();
  }
}

bool IndicatorImp::needCalculate() {
  if (need_calculate_) {
    return true;
  }

  // Set the context for the child nodes
  if (left_) {
    need_calculate_ = left_->needCalculate();
    if (need_calculate_) {
      return true;
    }
  }

  if (right_) {
    need_calculate_ = right_->needCalculate();
    if (need_calculate_) {
      return true;
    }
  }

  if (three_) {
    need_calculate_ = three_->needCalculate();
    if (need_calculate_) {
      return true;
    }
  }

  for (auto iter = ind_params_.begin(); iter != ind_params_.end(); ++iter) {
    need_calculate_ = iter->second->needCalculate();
    if (need_calculate_) {
      return true;
    }
  }

  return false;
}

void IndicatorImp::_calculate(const Indicator &ind) {
  if (isLeaf()) {
    const auto &k = getContext();
    size_t total = k.size();
    HAYAKU_IF_RETURN(total == 0, void());
    _readyBuffer(total, 1);
    discard_ = total;
    return;
  }

  size_t total = ind.size();
  result_num_ = ind.getResultNumber();
  HAYAKU_IF_RETURN(total == 0, void());

  _readyBuffer(total, result_num_);
  discard_ = ind.discard();
  for (size_t r = 0; r < result_num_; ++r) {
    const auto *src = ind.data(r);
    auto *dst = this->data(r);
    for (size_t i = discard_; i < total; ++i) {
      dst[i] = src[i];
    }
  }
}

bool IndicatorImp::can_increment_calculate() {
  if (result_num_ == 0 || context_.empty() || old_context_.empty()) {
    return false;
  }

  if (context_.front().datetime < old_context_.front().datetime) {
    return false;
  }

  if (context_.getStock() != old_context_.getStock() ||
      old_context_.getQuery().kType() != context_.getQuery().kType() ||
      old_context_.getQuery().recoverType() !=
          context_.getQuery().recoverType()) {
    return false;
  }

  if (context_.back().datetime <= old_context_.back().datetime) {
    return false;
  }

  return true;
}

bool IndicatorImp::increment_execute_leaf_or_op(const Indicator &ind) {
  if (param_changed_ || !ms_enable_increment_calculate ||
      !supportIncrementCalculate() || !can_increment_calculate()) {
    return false;
  }

  size_t copy_start_pos = old_context_.getPos(context_.front().datetime);
  if (copy_start_pos == Null<size_t>()) {
    return false;
  }

  size_t total = context_.size();
  size_t copy_len = old_context_.size() - copy_start_pos;
  if (copy_len < discard_) {
    return false;
  }

  size_t start_pos = context_.getPos(old_context_.back().datetime);
  if (start_pos == Null<size_t>()) {
    return false;
  }

  if (start_pos < min_increment_start()) {
    return false;
  }

  if (copy_len > 0) {
    for (size_t r = 0; r < result_num_; ++r) {
      HAYAKU_ASSERT(p_buffer_[r]);
      p_buffer_[r]->resize(total, Null<value_t>());
      auto *dst = this->data(r);
      memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
    }
  }

  if (start_pos < discard_) {
    start_pos = discard_;
  }

  discard_ = 0;

  if (start_pos < total) {
    _increment_calculate(ind, start_pos);
  }

  updateDiscard();
  return true;
}

Indicator IndicatorImp::calculate() {
  IndicatorImpPtr result;
  if (!needCalculate()) {
    try {
      result = shared_from_this();
    } catch (...) {
      result = clone();
    }
    return Indicator(result);
  }

  switch (optype_) {
    case LEAF:
      if (ind_params_.empty()) {
        if (!increment_execute_leaf_or_op(Indicator())) {
          _calculate(Indicator());
        }
      } else {
        // A dynamic period leaf has no right child to drive its fixed-length
        // buffer. When the executor rebinds the context across stocks, an empty
        // input makes _dyn_calculate return early at total == 0 without
        // touching the buffer at all, leaving the data and the length of the
        // previous stock in the buffer (a dirty buffer). Here the buffer is
        // pre-sized and null filled according to the current context length
        // before the call, so that even if _dyn_calculate writes nothing,
        // size()/data() return the correct (possibly shorter) length.
        if (isNeedContext()) {
          _readyBuffer(getContext().size(), result_num_);
        }
        _dyn_calculate(Indicator());
      }
      break;

    case OP: {
      if (ind_params_.empty()) {
        if (!increment_execute_leaf_or_op(Indicator(right_))) {
          right_->calculate();
          _readyBuffer(right_->size(), result_num_);
          _calculate(Indicator(right_));
          onlySetContext(right_->getContext());
        }
      } else {
        right_->calculate();
        _readyBuffer(right_->size(), result_num_);
        _dyn_calculate(Indicator(right_));
        onlySetContext(right_->getContext());
      }
    } break;

    case ADD:
      execute_add();
      break;

    case SUB:
      execute_sub();
      break;

    case MUL:
      execute_mul();
      break;

    case DIV:
      execute_div();
      break;

    case MOD:
      execute_mod();
      break;

    case EQ:
      execute_eq();
      break;

    case NE:
      execute_ne();
      break;

    case GT:
      execute_gt();
      break;

    case LT:
      execute_lt();
      break;

    case GE:
      execute_ge();
      break;

    case LE:
      execute_le();
      break;

    case AND:
      execute_and();
      break;

    case OR:
      execute_or();
      break;

    case WEAVE:
      execute_weave();
      break;

    case OP_IF:
      execute_if();
      break;

    default:
      HAYAKU_ERROR("Unkown Indicator::OPType! {}", int(optype_));
      break;
  }

  // When the prototype way is used, the recalculation cannot happen immediately
  // without this check
  if (size() != 0) {
    need_calculate_ = false;
  }

  param_changed_ = false;
  old_context_ = KData();

  try {
    result = shared_from_this();
  } catch (const std::exception &e) {
    // Do not make the core indicator path depend on the host application
    // (Python or otherwise). Python-derived implementations should be managed
    // by shared_ptr through the binding.
    HAYAKU_ERROR("IndicatorImp::calculate() error! {}", e.what());
    throw;
  }

  return Indicator(result);
}

size_t IndicatorImp::increment_execute() {
  size_t null_pos = Null<size_t>();
  if (param_changed_ || !ms_enable_increment_calculate ||
      right_->need_calculate_ || left_->need_calculate_) {
    return null_pos;
  }

  if (!can_increment_calculate()) {
    return null_pos;
  }

  size_t copy_start_pos = old_context_.getPos(context_.front().datetime);
  if (copy_start_pos == null_pos) {
    return null_pos;
  }

  size_t total = context_.size();
  size_t copy_len = old_context_.size() - copy_start_pos;
  if (copy_len == 0) {
    return null_pos;
  }

  size_t start_pos = context_.getPos(old_context_.back().datetime);
  if (start_pos == null_pos) {
    return null_pos;
  }

  for (size_t r = 0; r < result_num_; ++r) {
    HAYAKU_ASSERT(p_buffer_[r] != nullptr);
    p_buffer_[r]->resize(total, Null<value_t>());
    auto *dst = this->data(r);
    memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
  }

  return start_pos;
}

void IndicatorImp::execute_weave() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  const IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t diff = maxp->size() - minp->size();
  size_t total = maxp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number = minp->getResultNumber() + maxp->getResultNumber();
    if (result_number > MAX_RESULT_NUM) {
      result_number = MAX_RESULT_NUM;
    }
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t const *src = nullptr;
  value_t *dst = nullptr;
  if (left_->size() >= right_->size()) {
    size_t num = left_->getResultNumber();
    for (size_t r = 0; r < num; ++r) {
      src = left_->data(r);
      dst = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = src[i];
      }
    }
    for (size_t r = num; r < result_num_; r++) {
      src = right_->data(r - num);
      dst = this->data(r);
      for (size_t i = start_pos; i < total; i++) {
        dst[i] = src[i - diff];
      }
    }
  } else {
    size_t num = left_->getResultNumber();
    for (size_t r = 0; r < num; ++r) {
      src = left_->data(r);
      dst = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = src[i - diff];
      }
    }
    for (size_t r = num; r < result_num_; r++) {
      src = right_->data(r - num);
      dst = this->data(r);
      for (size_t i = start_pos; i < total; i++) {
        dst[i] = src[i];
      }
    }
  }
}

void IndicatorImp::execute_add() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto const *data1 = maxp->data(r);
    auto const *data2 = minp->data(r);
    auto *result = this->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      result[i] = data1[i] + data2[i - diff];
    }
  }
}

void IndicatorImp::execute_sub() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      auto *data1 = left_->data(r);
      auto *data2 = right_->data(r);
      auto *result = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        result[i] = data1[i] - data2[i - diff];
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      auto *data1 = left_->data(r);
      auto *data2 = right_->data(r);
      auto *result = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        result[i] = data1[i - diff] - data2[i];
      }
    }
  }
}

void IndicatorImp::execute_mul() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto const *data1 = maxp->data(r);
    auto const *data2 = minp->data(r);
    auto *result = this->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      result[i] = data1[i] * data2[i - diff];
    }
  }
}

void IndicatorImp::execute_div() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      auto const *data1 = left_->data(r);
      auto const *data2 = right_->data(r);
      auto *result = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        result[i] = data1[i] / data2[i - diff];
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      auto const *data1 = left_->data(r);
      auto const *data2 = right_->data(r);
      auto *result = this->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        result[i] = data1[i - diff] / data2[i];
      }
    }
  }
}

void IndicatorImp::execute_mod() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t *dst = nullptr;
  value_t const *left = nullptr;
  value_t const *right = nullptr;
  value_t null_value = Null<value_t>();
  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        if (right[i - diff] == 0.0) {
          dst[i] = null_value;
        } else {
          dst[i] = int64_t(left[i]) % int64_t(right[i - diff]);
        }
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        if (right[i] == 0.0) {
          dst[i] = null_value;
        } else {
          dst[i] = int64_t(left[i - diff]) % int64_t(right[i]);
        }
      }
    }
  }
}

void IndicatorImp::execute_eq() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto *dst = this->data(r);
    auto const *maxdata = maxp->data(r);
    auto const *mindata = minp->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = (maxdata[i] == mindata[i - diff]) ? 1.0 : 0.0;
    }
  }
}

void IndicatorImp::execute_ne() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto *dst = this->data(r);
    auto const *maxdata = maxp->data(r);
    auto const *mindata = minp->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = (maxdata[i] != mindata[i - diff]) ? 1.0 : 0.0;
    }
  }
}

void IndicatorImp::execute_gt() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t *dst = nullptr;
  value_t const *left = nullptr;
  value_t const *right = nullptr;
  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = (left[i] > right[i - diff]) ? 1.0 : 0.0;
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = (left[i - diff] > right[i]) ? 1.0 : 0.0;
      }
    }
  }
}

void IndicatorImp::execute_lt() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t *dst = nullptr;
  value_t const *left = nullptr;
  value_t const *right = nullptr;
  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = (left[i] < right[i - diff]) ? 1.0 : 0.0;
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = left[i - diff] < right[i] ? 1.0 : 0.0;
      }
    }
  }
}

void IndicatorImp::execute_ge() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t *dst = nullptr;
  value_t const *left = nullptr;
  value_t const *right = nullptr;
  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = left[i] >= right[i - diff] ? 1.0 : 0.0;
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = left[i - diff] >= right[i] ? 1.0 : 0.0;
      }
    }
  }
}

void IndicatorImp::execute_le() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  value_t *dst = nullptr;
  value_t const *left = nullptr;
  value_t const *right = nullptr;
  if (left_->size() > right_->size()) {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = left[i] <= right[i - diff] ? 1.0 : 0.0;
      }
    }
  } else {
    for (size_t r = 0; r < result_num_; ++r) {
      dst = this->data(r);
      left = left_->data(r);
      right = right_->data(r);
      for (size_t i = start_pos; i < total; ++i) {
        dst[i] = left[i - diff] <= right[i] ? 1.0 : 0.0;
      }
    }
  }
}

void IndicatorImp::execute_and() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto *dst = this->data(r);
    auto const *maxdata = maxp->data(r);
    auto const *mindata = minp->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = (maxdata[i] > 0.0) && (mindata[i - diff] > 0.0) ? 1.0 : 0.0;
    }
  }
}

void IndicatorImp::execute_or() {
  size_t start_pos = increment_execute();
  if (start_pos == Null<size_t>()) {
    right_->calculate();
    left_->calculate();
  }

  IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t diff = maxp->size() - minp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  for (size_t r = 0; r < result_num_; ++r) {
    auto *dst = this->data(r);
    auto const *maxdata = maxp->data(r);
    auto const *mindata = minp->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      dst[i] = (maxdata[i] > 0.0) || (mindata[i - diff] > 0.0) ? 1.0 : 0.0;
    }
  }
}

size_t IndicatorImp::increment_execute_if() {
  size_t null_pos = Null<size_t>();
  if (param_changed_ || !ms_enable_increment_calculate ||
      three_->need_calculate_ || right_->need_calculate_ ||
      left_->need_calculate_) {
    return null_pos;
  }

  if (!can_increment_calculate()) {
    return null_pos;
  }

  size_t copy_start_pos = old_context_.getPos(context_.front().datetime);
  if (copy_start_pos == null_pos) {
    return null_pos;
  }

  size_t total = context_.size();
  size_t copy_len = old_context_.size() - copy_start_pos;
  if (copy_len == 0) {
    return null_pos;
  }

  size_t start_pos = context_.getPos(old_context_.back().datetime);
  if (start_pos == null_pos) {
    return null_pos;
  }

  for (size_t r = 0; r < result_num_; ++r) {
    HAYAKU_ASSERT(p_buffer_[r]);
    p_buffer_[r]->resize(total, Null<value_t>());
    auto *dst = this->data(r);
    memmove(dst, dst + copy_start_pos, sizeof(value_t) * (copy_len));
  }

  return start_pos;
}

void IndicatorImp::execute_if() {
  size_t start_pos = increment_execute_if();
  if (start_pos == Null<size_t>()) {
    three_->calculate();
    right_->calculate();
    left_->calculate();
  }

  const IndicatorImp *maxp, *minp;
  if (right_->size() > left_->size()) {
    maxp = right_.get();
    minp = left_.get();
  } else {
    maxp = left_.get();
    minp = right_.get();
  }

  size_t total = maxp->size();
  size_t discard = maxp->size() - minp->size() + minp->discard();
  if (discard < maxp->discard()) {
    discard = maxp->discard();
  }
  if (discard < three_->discard()) {
    discard = three_->discard();
  }

  if (three_->size() >= maxp->size()) {
    total = three_->size();
    discard = total + discard - maxp->size();
  } else {
    discard = total - three_->size();
  }

  size_t diff_right = total - right_->size();
  size_t diff_left = total - left_->size();
  size_t diff_cond = total - three_->size();

  if (start_pos == Null<size_t>()) {
    size_t result_number =
        std::min(minp->getResultNumber(), maxp->getResultNumber());
    _readyBuffer(total, result_number);
    start_pos = discard;
  } else if (start_pos < discard) {
    start_pos = discard;
  }

  setDiscard(discard);

  auto *left = left_->data(0);
  auto *right = right_->data(0);
  auto *three = three_->data(0);
  for (size_t r = 0; r < result_num_; ++r) {
    auto *dst = this->data(r);
    for (size_t i = start_pos; i < total; ++i) {
      if (three[i - diff_cond] > 0.0) {
        dst[i] = left[i - diff_left];
      } else {
        dst[i] = right[i - diff_right];
      }
    }
  }
}

void IndicatorImp::_dyn_calculate(const Indicator &ind) {
  // SPEND_TIME(IndicatorImp__dyn_calculate);
  const auto &ind_param = getIndParamImp("n");
  // CVAL or PRICELIST may be larger than ind.size()
  HAYAKU_CHECK(ind_param->size() >= ind.size(),
               "ind_param->size()={}, ind.size()={}!", ind_param->size(),
               ind.size());
  discard_ = std::max(ind.discard(), ind_param->discard());
  size_t total = ind.size();
  HAYAKU_IF_RETURN(0 == total || discard_ >= total, void());

  const value_t *param_data = ind_param->data();

  static constexpr size_t minCircleLength = 400;
  if (total < minCircleLength || isSerial()) {
    for (size_t i = ind.discard(); i < total; i++) {
      if (std::isnan(param_data[i])) {
        _set(Null<value_t>(), i);
      } else {
        size_t step = size_t(param_data[i]);
        _dyn_run_one_step(ind, i, step);
      }
    }
    updateDiscard();
    return;
  }

  global_parallel_for_index_void(
      ind.discard(), total,
      [&ind, param_data, this](size_t i) {
        if (std::isnan(param_data[i])) {
          _set(Null<value_t>(), i);
        } else {
          size_t step = size_t(param_data[i]);
          _dyn_run_one_step(ind, i, step);
        }
      },
      minCircleLength);

  updateDiscard();
}

void IndicatorImp::updateDiscard(bool force) noexcept {
  if (force) {
    discard_ = 0;
  }
  size_t total = size();
  for (size_t result_index = 0; result_index < result_num_; result_index++) {
    size_t discard = discard_;
    const auto *dst = this->data(result_index);
    for (size_t i = discard_; i < total; i++) {
      if (!std::isnan(dst[i])) {
        break;
      }
      discard++;
    }
    if (discard > discard_) {
      discard_ = discard;
    }
  }
  if (discard_ > total) {
    discard_ = total;
  }
}

bool IndicatorImp::alike(const IndicatorImp &other) const {
  HAYAKU_IF_RETURN(this == &other, true);
  HAYAKU_IF_RETURN(
      optype_ != other.optype_ || discard_ != other.discard_ ||
          result_num_ != other.result_num_ || (isLeaf() && !other.isLeaf()) ||
          (!isLeaf() && other.isLeaf()) || typeid(*this) != typeid(other) ||
          ind_params_.size() != other.ind_params_.size() ||
          params_ != other.params_,
      false);

  auto iter1 = ind_params_.cbegin();
  auto iter2 = other.ind_params_.cbegin();
  for (; iter1 != ind_params_.cend() && iter2 != other.ind_params_.cend();
       ++iter1, ++iter2) {
    HAYAKU_IF_RETURN(iter1->first != iter2->first, false);
    HAYAKU_IF_RETURN(!iter1->second->alike(*(iter2->second)), false);
  }

  if (needSelfAlikeCompare()) {
    HAYAKU_IF_RETURN(!selfAlike(other), false);
    // Special leaves use structural identity so an unevaluated operator
    // template (such as CVAL(value)) can match a calculated input without
    // comparing runtime buffers.
    HAYAKU_IF_RETURN(isLeaf(), true);
  }

  if (isLeaf() && other.isLeaf()) {
    HAYAKU_IF_RETURN(this->size() != other.size(), false);
    auto const *d1 = this->data();
    auto const *d2 = other.data();
    bool eq = true;
    for (size_t i = 0, len = this->size(); i < len; i++) {
      if (d1[i] != d2[i]) {
        eq = false;
      }
    }
    return eq;
  }

  HAYAKU_IF_RETURN(bool(three_) != bool(other.three_) ||
                       bool(left_) != bool(other.left_) ||
                       bool(right_) != bool(other.right_),
                   false);
  HAYAKU_IF_RETURN(three_ && !three_->alike(*other.three_), false);
  HAYAKU_IF_RETURN(left_ && !left_->alike(*other.left_), false);
  HAYAKU_IF_RETURN(right_ && !right_->alike(*other.right_), false);

  return true;
}

bool IndicatorImp::contains(const string &name) const {
  HAYAKU_IF_RETURN(name_ == name, true);
  vector<IndicatorImpPtr> all_nodes;
  getAllSubNodes(all_nodes);
  for (const auto &node : all_nodes) {
    if (node->name() == name) {
      return true;
    }
  }
  return false;
}

void IndicatorImp::getAllSubNodes(vector<IndicatorImpPtr> &nodes) const {
  // Use a stack to simulate the recursive calls, avoiding the stack overflow
  // caused by a deep recursion
  std::stack<IndicatorImpPtr> nodeStack;

  // Push the child nodes of the current node onto the stack (in the reverse
  // order to keep the original processing order)
  if (three_) {
    nodeStack.push(three_);
  }
  if (left_) {
    nodeStack.push(left_);
  }
  if (right_) {
    nodeStack.push(right_);
  }

  // Process the nodes in the stack
  while (!nodeStack.empty()) {
    IndicatorImpPtr current = nodeStack.top();
    nodeStack.pop();

    // Add the current node to the result list
    nodes.push_back(current);

    // Push the child nodes of the current node onto the stack (in the reverse
    // order to keep the original processing order)
    if (current->three_) {
      nodeStack.push(current->three_);
    }
    if (current->left_) {
      nodeStack.push(current->left_);
    }
    if (current->right_) {
      nodeStack.push(current->right_);
    }

    // Add the internal node of the current node (if there is one)
    current->getSelfInnerNodesWithInputConext(nodes);
  }

  // Add the internal node of the current node
  getSelfInnerNodesWithInputConext(nodes);
}

void IndicatorImp::inner_repeatALikeNodes(vector<IndicatorImpPtr> &sub_nodes) {
  vector<IndicatorImpPtr> tmp_nodes;
  size_t total = sub_nodes.size();
  for (size_t i = 0; i < total; i++) {
    const auto &cur = sub_nodes[i];
    // Detached private roots (such as Indicator2InImp::m_ref_ind) have no
    // generic parent edge and cannot participate in m_left/m_right/m_three
    // replacement.
    if (!cur || !cur->parent_) {
      continue;
    }
    for (size_t j = i + 1; j < total; j++) {
      auto &node = sub_nodes[j];
      if (!node || !node->parent_ || cur == node) {
        continue;
      }

      if (cur->alike(*node)) {
        IndicatorImp *node_parent = node->parent_;
        if (node_parent->left_ == node) {
          node_parent->left_ = cur;
        }

        if (node_parent->right_ == node) {
          node_parent->right_ = cur;
        }

        if (node_parent->three_ == node) {
          node_parent->three_ = cur;
        }

        tmp_nodes.clear();
        node->getAllSubNodes(tmp_nodes);
        for (const auto &replace_node : tmp_nodes) {
          for (size_t k = j + 1; k < total; k++) {
            if (replace_node == sub_nodes[k]) {
              sub_nodes[k].reset();
            }
          }
        }

        node = cur;
      }
    }
  }
}

void IndicatorImp::repeatALikeNodes() {
  vector<IndicatorImpPtr> sub_nodes;
  getAllSubNodes(sub_nodes);
  inner_repeatALikeNodes(sub_nodes);
  repeatSeparateKTypeLeafALikeNodes();
}

void IndicatorImp::repeatSeparateKTypeLeafALikeNodes() {
  // All the child nodes need to be fetched again after the optimization of the
  // upper layer is done
  vector<IndicatorImpPtr> all_nodes;
  getAllSubNodes(all_nodes);

  std::unordered_set<IndicatorImp *> special_set;
  std::unordered_map<string, vector<IndicatorImpPtr>> special_nodes;
  for (const auto &node : all_nodes) {
    if (node->isLeaf() && node->haveParam("ktype")) {
      if (special_set.find(node.get()) == special_set.end()) {
        special_set.insert(node.get());
        string ktype = node->getParam<string>("ktype");
        auto iter = special_nodes.find(ktype);
        if (iter == special_nodes.end()) {
          special_nodes.insert(
              std::make_pair(ktype, vector<IndicatorImpPtr>{node}));
        } else {
          iter->second.push_back(node);
        }
      }
    }
  }

  vector<IndicatorImpPtr> can_merge_nodes;
  for (const auto &item : special_nodes) {
    const auto &nodes = item.second;
    size_t total = nodes.size();
    // HAYAKU_INFO("first ktype: {}, total: {}", item.first, total);
    if (total <= 1) {
      continue;
    }

    can_merge_nodes.clear();
    for (size_t i = 0; i < total; i++) {
      nodes[i]->getSeparateKTypeLeafSubNodes(can_merge_nodes);
    }
    inner_repeatALikeNodes(can_merge_nodes);
  }
}

void IndicatorImp::_printTree(int depth, bool isLast,
                              bool show_long_name) const {
  // Print the current node
  std::string indent;
  if (depth > 0) {
    // Build the level indentation
    indent = std::string((depth - 1) * 3, ' ');
    indent += isLast ? "└─ " : "├─ ";
  }

  // Print the node name
  std::string name = (show_long_name ? long_name() : this->name());
  if (parent_) {
    if (this == parent_->three_.get()) {
      name = "[T]" + name;
    } else if (this == parent_->left_.get()) {
      name = "[L]" + name;
    } else if (this == parent_->right_.get()) {
      name = "[R]" + name;
    }
  }
  std::cout << indent << name;

  // If it is a leaf node, break the line directly
  if (isLeaf()) {
    std::cout << std::endl;
    return;
  }

  // When there are child nodes, break the line first and then print the child
  // nodes
  std::cout << std::endl;

  std::vector<IndicatorImp *> children;
  if (three_) {
    children.emplace_back(three_.get());
  }
  if (left_) {
    children.emplace_back(left_.get());
  }
  if (right_) {
    children.emplace_back(right_.get());
  }

  if (children.empty()) return;

  // Print the child nodes recursively
  for (size_t i = 0; i < children.size(); ++i) {
    bool lastChild = (i == children.size() - 1);
    std::cout << std::string(depth * 3, ' ');
    children[i]->_printTree(depth + 1, lastChild, show_long_name);
  }
}

void IndicatorImp::printTree(bool show_long_name) const {
  std::cout << "Tree structure starting from root:" << std::endl;
  _printTree(0, true, show_long_name);
}

vector<IndicatorImp *> IndicatorImp::getAllSubTrees() const {
  vector<IndicatorImpPtr> all_nodes;
  getAllSubNodes(all_nodes);
  std::unordered_set<IndicatorImpPtr> leaves;
  for (const auto &node : all_nodes) {
    if (node->isLeaf()) {
      leaves.insert(node);
    }
  }

  std::unordered_set<IndicatorImp *> tree_set;
  for (const auto &leaf : leaves) {
    const IndicatorImp *tree = leaf.get();
    while (tree->parent_) {
      tree_set.insert(tree->parent_);
      tree = tree->parent_;
    }
  }

  vector<IndicatorImp *> trees;
  trees.reserve(leaves.size() + tree_set.size());
  for (const auto &leaf : leaves) {
    trees.push_back(leaf.get());
  }
  for (const auto &tree : tree_set) {
    trees.push_back(tree);
  }
  return trees;
}

size_t IndicatorImp::treeSize(IndicatorImp *tree) {
  HAYAKU_IF_RETURN(tree == nullptr, 0);
  HAYAKU_IF_RETURN(tree->isLeaf(), 1);

  size_t ret = 1;
  if (tree->three_ != nullptr) {
    ret += treeSize(tree->three_.get());
  }
  if (tree->left_ != nullptr) {
    ret += treeSize(tree->left_.get());
  }
  if (tree->right_ != nullptr) {
    ret += treeSize(tree->right_.get());
  }
  return ret;
}

bool IndicatorImp::nodeInTree(IndicatorImp *node, IndicatorImp *tree) {
  HAYAKU_IF_RETURN(node == nullptr || tree == nullptr, false);
  HAYAKU_IF_RETURN(node == tree, true);
  if (tree->three_) {
    HAYAKU_IF_RETURN(nodeInTree(node, tree->three_.get()), true);
  }
  if (tree->left_) {
    HAYAKU_IF_RETURN(nodeInTree(node, tree->left_.get()), true);
  }
  if (tree->right_) {
    HAYAKU_IF_RETURN(nodeInTree(node, tree->right_.get()), true);
  }
  return false;
}

void IndicatorImp::printAllSubTrees(bool show_long_name) const {
  std::cout << "All sub trees:" << std::endl;
  vector<IndicatorImp *> trees = getAllSubTrees();
  std::sort(trees.begin(), trees.end(), [](IndicatorImp *a, IndicatorImp *b) {
    return treeSize(a) < treeSize(b);
  });
  for (size_t i = 0; i < trees.size(); i++) {
    std::cout << "-------------------------------------------------------------"
                 "------------"
              << std::endl;
    std::cout << "Tree " << i << " (size: " << treeSize(trees[i])
              << ") :" << std::endl;
    trees[i]->printTree(show_long_name);
  }
}

void IndicatorImp::printLeaves(bool show_long_name) const {
  vector<IndicatorImpPtr> all_nodes;
  getAllSubNodes(all_nodes);
  std::unordered_set<IndicatorImpPtr> leaves;
  for (const auto &node : all_nodes) {
    if (node->isLeaf()) {
      leaves.insert(node);
    }
  }
  size_t ix = 0;
  for (const auto &leaf : leaves) {
    std::cout << "Leaf " << ix++ << ": "
              << (show_long_name ? leaf->long_name() : leaf->name())
              << std::endl;
  }
}

} /* namespace hayaku */
