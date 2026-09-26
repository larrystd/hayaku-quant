/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-07-15
 *      Author: woleigegg
 *
 *  Internal compiled execution plan for stock-local factor formulas.
 */

#include "CompiledFactorPlan.h"

#include <algorithm>

#include "operators/IndicatorImp.h"

namespace hayaku {
namespace detail {

IndicatorImpPtr CompiledFactorPlan::cloneNode(const IndicatorImpPtr& src,
                                              CloneMap& clones) {
  if (!src) {
    return {};
  }

  auto iter = clones.find(src.get());
  if (iter != clones.end()) {
    return iter->second;
  }

  // Indicator2InImp::_clone() currently clones its private reference graph
  // independently. The executor still rebinds that graph correctly, but sharing
  // inside it is not preserved yet.
  IndicatorImpPtr dst = src->_clone();
  clones.emplace(src.get(), dst);

  dst->params_ = src->params_;
  dst->name_ = src->name_;
  dst->origin_id_ =
      src->origin_id_;  // Copy the origin id, as in IndicatorImp::clone()
  dst->is_python_object_ = src->is_python_object_;
  dst->need_self_alike_compare_ = src->need_self_alike_compare_;
  dst->is_serial_ = src->is_serial_;
  dst->discard_ = src->discard_;
  dst->result_num_ = src->result_num_;
  dst->context_ = src->context_;
  dst->old_context_ = src->old_context_;
  dst->need_calculate_ = src->need_calculate_;
  dst->param_changed_ = src->param_changed_;
  dst->optype_ = src->optype_;
  dst->parent_ = nullptr;

  dst->_readyBuffer(src->size(), src->result_num_);
  for (size_t i = 0; i < src->result_num_; ++i) {
    if (src->p_buffer_[i]) {
      std::copy(src->p_buffer_[i]->begin(), src->p_buffer_[i]->end(),
                dst->p_buffer_[i]->begin());
    }
  }

  dst->left_ = cloneNode(src->left_, clones);
  dst->right_ = cloneNode(src->right_, clones);
  dst->three_ = cloneNode(src->three_, clones);
  if (dst->left_) {
    dst->left_->parent_ = dst.get();
  }
  if (dst->right_) {
    dst->right_->parent_ = dst.get();
  }
  if (dst->three_) {
    dst->three_->parent_ = dst.get();
  }

  dst->ind_params_.clear();
  for (const auto& [name, value] : src->ind_params_) {
    dst->ind_params_[name] = cloneNode(value, clones);
  }

  return dst;
}

IndicatorImpPtr CompiledFactorPlan::canonicalizeNode(
    const IndicatorImpPtr& node, CanonicalMap& canonical,
    vector<IndicatorImpPtr>& unique_nodes) {
  if (!node) {
    return {};
  }

  auto iter = canonical.find(node.get());
  if (iter != canonical.end()) {
    return iter->second;
  }

  node->left_ = canonicalizeNode(node->left_, canonical, unique_nodes);
  node->right_ = canonicalizeNode(node->right_, canonical, unique_nodes);
  node->three_ = canonicalizeNode(node->three_, canonical, unique_nodes);
  for (auto& [_, value] : node->ind_params_) {
    value = canonicalizeNode(value, canonical, unique_nodes);
  }

  for (const auto& candidate : unique_nodes) {
    if (candidate->alike(*node)) {
      canonical.emplace(node.get(), candidate);
      return candidate;
    }
  }

  unique_nodes.emplace_back(node);
  canonical.emplace(node.get(), node);
  return node;
}

void CompiledFactorPlan::prepareNode(
    const IndicatorImpPtr& node, const KData& kdata,
    std::unordered_set<IndicatorImp*>& visited) {
  if (!node || !visited.emplace(node.get()).second) {
    return;
  }

  node->onlySetContext(kdata);
  node->old_context_ = KData();
  node->need_calculate_ = true;
  node->param_changed_ = true;
  node->discard_ = 0;

  // CONTEXT owns a deliberately independent input context. Its implementation
  // is responsible for rebinding that private subgraph when the outer context
  // changes.
  if (node->name() == "CONTEXT") {
    return;
  }

  prepareNode(node->left_, kdata, visited);
  prepareNode(node->right_, kdata, visited);
  prepareNode(node->three_, kdata, visited);
  for (const auto& [_, value] : node->ind_params_) {
    prepareNode(value, kdata, visited);
    value->calculate();
  }

  vector<IndicatorImpPtr> inner_nodes;
  node->getSelfInnerNodesWithInputConext(inner_nodes);
  for (const auto& inner : inner_nodes) {
    prepareNode(inner, kdata, visited);
  }
}

bool CompiledFactorPlan::isEligibleNode(
    const IndicatorImpPtr& node,
    std::unordered_set<const IndicatorImp*>& visited) {
  if (!node || !visited.emplace(node.get()).second) {
    return true;
  }

  // Python implementations can retain arbitrary state that the C++ executor
  // cannot reset. CONTEXT contains a private independently-bound graph and
  // stays on the proven legacy path.
  if (!node->supportBatchReuse() || node->name() == "CONTEXT") {
    return false;
  }

  if (!isEligibleNode(node->left_, visited) ||
      !isEligibleNode(node->right_, visited) ||
      !isEligibleNode(node->three_, visited)) {
    return false;
  }
  for (const auto& [_, value] : node->ind_params_) {
    if (!isEligibleNode(value, visited)) {
      return false;
    }
  }

  vector<IndicatorImpPtr> inner_nodes;
  node->getSelfInnerNodesWithInputConext(inner_nodes);
  for (const auto& inner : inner_nodes) {
    if (!isEligibleNode(inner, visited)) {
      return false;
    }
  }
  return true;
}

void CompiledFactorPlan::scrubTemplateNode(
    const IndicatorImpPtr& node, std::unordered_set<IndicatorImp*>& visited) {
  if (!node || !visited.emplace(node.get()).second) {
    return;
  }

  node->context_ = KData();
  node->old_context_ = KData();
  node->need_calculate_ = true;
  node->param_changed_ = true;
  if (!node->isLeaf() || node->isNeedContext()) {
    node->_clearBuffer();
    node->discard_ = 0;
  }

  scrubTemplateNode(node->left_, visited);
  scrubTemplateNode(node->right_, visited);
  scrubTemplateNode(node->three_, visited);
  for (const auto& [_, value] : node->ind_params_) {
    scrubTemplateNode(value, visited);
  }

  vector<IndicatorImpPtr> inner_nodes;
  node->getSelfInnerNodesWithInputConext(inner_nodes);
  for (const auto& inner : inner_nodes) {
    scrubTemplateNode(inner, visited);
  }
}

void CompiledFactorPlan::normalizeParents(const IndicatorList& roots) {
  using ParentInfo = std::pair<size_t, IndicatorImp*>;
  std::unordered_map<IndicatorImp*, ParentInfo> parents;
  std::unordered_set<IndicatorImp*> visited;
  vector<IndicatorImpPtr> stack;

  for (const auto& root : roots) {
    if (root.getImp()) {
      auto [iter, inserted] =
          parents.emplace(root.getImp().get(), ParentInfo{1, nullptr});
      if (!inserted) {
        ++iter->second.first;
      }
      root.getImp()->parent_ = nullptr;
      stack.emplace_back(root.getImp());
    }
  }

  while (!stack.empty()) {
    IndicatorImpPtr node = std::move(stack.back());
    stack.pop_back();
    if (!node || !visited.emplace(node.get()).second) {
      continue;
    }

    auto record_child = [&](const IndicatorImpPtr& child) {
      if (!child) {
        return;
      }
      auto [iter, inserted] =
          parents.emplace(child.get(), ParentInfo{1, node.get()});
      if (!inserted) {
        ++iter->second.first;
      }
      stack.emplace_back(child);
    };
    record_child(node->left_);
    record_child(node->right_);
    record_child(node->three_);
    for (const auto& [_, value] : node->ind_params_) {
      record_child(value);
    }
  }

  for (const auto& [node, info] : parents) {
    node->parent_ = info.first == 1 ? info.second : nullptr;
  }
}

CompiledFactorPlan::CompiledFactorPlan(const IndicatorList& formulas) {
  std::unordered_set<const IndicatorImp*> visited;
  for (const auto& formula : formulas) {
    if (!isEligibleNode(formula.getImp(), visited)) {
      reusable_ = false;
      return;
    }
  }

  roots_.reserve(formulas.size());
  CloneMap clones;
  for (const auto& formula : formulas) {
    roots_.emplace_back(cloneNode(formula.getImp(), clones));
  }
  normalizeParents(roots_);

  std::unordered_set<IndicatorImp*> scrubbed;
  for (const auto& root : roots_) {
    scrubTemplateNode(root.getImp(), scrubbed);
  }

  CanonicalMap canonical;
  vector<IndicatorImpPtr> unique_nodes;
  for (auto& root : roots_) {
    root = Indicator(canonicalizeNode(root.getImp(), canonical, unique_nodes));
  }
  normalizeParents(roots_);

  // Private derived-class graphs are intentionally left to the existing
  // indicator hook. They remain correct but cannot participate in plan-wide CSE
  // until their ownership is exposed.
  for (const auto& root : roots_) {
    if (root.getImp()) {
      root.getImp()->repeatSeparateKTypeLeafALikeNodes();
    }
  }
  normalizeParents(roots_);
}

IndicatorList CompiledFactorPlan::cloneRoots() const {
  IndicatorList roots;
  roots.reserve(roots_.size());

  CloneMap clones;
  for (const auto& root : roots_) {
    roots.emplace_back(cloneNode(root.getImp(), clones));
  }

  normalizeParents(roots);

  return roots;
}

FactorPlanExecutor CompiledFactorPlan::createExecutor() const {
  return FactorPlanExecutor(cloneRoots());
}

IndicatorList FactorPlanExecutor::executeValues(const KData& kdata) {
  if (kdata.empty()) {
    return IndicatorList(roots_.size());
  }

  std::unordered_set<IndicatorImp*> visited;
  for (const auto& root : roots_) {
    CompiledFactorPlan::prepareNode(root.getImp(), kdata, visited);
  }

  std::unordered_set<IndicatorImp*> executed_roots;
  for (auto& root : roots_) {
    if (root.getImp() && executed_roots.emplace(root.getImp().get()).second) {
      root.getImp()->calculate();
    }
  }

  IndicatorList result;
  result.reserve(roots_.size());
  for (const auto& root : roots_) {
    result.emplace_back(root.getResult(0));
  }
  return result;
}

}  // namespace detail
}  // namespace hayaku
