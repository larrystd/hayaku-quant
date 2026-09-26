#pragma once

/*
 *  Copyright (c) 2024 hikyuu.org
 *
 *  Created on: 2024-03-12
 *      Author: fasiondog
 */

#include <atomic>

#include "Normalizers.h"
#include "ScoresFilterBase.h"
#include "data/KData.h"
#include "operators/FactorSet.h"

namespace hayaku {

/**
 * Multi-factor synthesis; when there is only one factor it is equivalent to a
 * simple score board
 * @ingroup MultiFactor
 */
class HAYAKU_API MultiFactorBase
    : public enable_shared_from_this<MultiFactorBase> {
  PARAMETER_SUPPORT_WITH_CHECK

 public:
  typedef Indicator::value_t value_t;
  friend HAYAKU_API std::ostream& operator<<(std::ostream&,
                                             const MultiFactorBase&);

 public:
  MultiFactorBase();
  explicit MultiFactorBase(const string& name);
  MultiFactorBase(const StockList& stks, const KQuery& query,
                  const Stock& ref_stk, const string& name, int ic_n,
                  bool spearman, int mode, bool save_all_factors);
  MultiFactorBase(const MultiFactorBase&);
  virtual ~MultiFactorBase() = default;

  /** Get the name */
  const string& name() const { return name_; }

  /** Set the name */
  void name(const string& name) { name_ = name; }

  /** Get the reference date list */
  const DatetimeList& getDatetimeList();

  /** Get the query range */
  const KQuery& getQuery() const { return query_; }

  /** Set the query range */
  void setQuery(const KQuery& query);

  /** Get the reference security */
  const Stock& getRefStock() const { return ref_stk_; }

  /** Set the reference security */
  void setRefStock(const Stock& stk);

  /** Get the security list */
  const StockList& getStockList() const { return stks_; }

  /** Set the security list of the calculation range */
  void setStockList(const StockList& stks);

  /** Get the current number of the securities in the security list */
  size_t getStockListNumber() const { return stks_.size(); }

  /** Get the original factor set */
  const FactorSet& getRefFactorSet() const { return factorset_; }

  /** Set the original factor set */
  void setRefFactorSet(const FactorSet& factorset);

  /** Get the synthesized factor of the given security */
  const Indicator& getFactor(const Stock&);

  /**
   * Get the new factors synthesized from all the securities, in the same order
   * as the passed security portfolio
   */
  const IndicatorList& getAllFactors();

  /** Get all the factor values of the given date cross-section, they are
   * already in the descending order */
  ScoreRecordList getScores(const Datetime&);

  ScoreRecordList getScores(const Datetime& date, size_t start,
                            size_t end = Null<size_t>());

  /**
   * Get the factor values (scores) within the given date cross-section range
   * [start, end], and filter them through filter
   * @param date the given date
   * @param start the sorting start point
   * @param end the sorting end point (excluded)
   * @param filter the filter function
   */
  ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                            std::function<bool(const ScoreRecord&)>&& filter);

  ScoreRecordList getScores(
      const Datetime& date, size_t start, size_t end,
      std::function<bool(const Datetime&, const ScoreRecord&)>&& filter);

  ScoreRecordList getScores(const Datetime& date, size_t start, size_t end,
                            const ScoresFilterPtr& filter);

  /** Get all the cross-section data, they are already in the descending order
   */
  const vector<ScoreRecordList>& getAllScores();

  /**
   * Get the IC of the synthesized factor, its length is the same as the
   * reference dates (the non-strict IC mode)
   * @note For a new factor using the IC/ICIR weighting, ndays had better stay
   * consistent with ic_n; but for a new factor calculated with the equal
   * weight, ic_n does not have to be used. Therefore a special value 0 is added
   * to ndays, meaning the IC is calculated directly with the ic_n parameter
   * @param ndays calculate the IC value relative to the ndays day return
   */
  Indicator getIC(int ndays = 0);

  /**
   * Get the ICIR of the synthesized factor
   * @param ir_n the n window for calculating the IR
   * @param ic_n the n window for calculating the IC
   */
  Indicator getICIR(int ir_n, int ic_n = 0);

  /**
   * Get all the processed original factor values (normalized and standardized).
   * It is calculated every time.
   * @note Considering the memory usage, this data is not cached; it is
   * generally used for the testing or when you want to view the processed
   * original factor values
   * @return vector<IndicatorList>  stks x inds
   */
  vector<IndicatorList> getAllSrcFactors();

  /**
   * Set the factor standardization / normalization operation
   * @param norm the standardization operation
   */
  void setNormalize(NormPtr norm);

  /**
   * Apply the given standardization / normalization, industry neutralization
   * and style factor neutralization operations to the indicator with the given
   * name.
   * @note The standardization, the industry neutralization and the style factor
   * neutralization are independent of each other; they can be given together or
   * separately.
   * @param name indicator name
   * @param norm the standardization operation
   * @param category the block category the indicator belongs to (it needs to be
   * given for the neutralization)
   * @param style_inds the style list of the indicator
   */
  void addSpecialNormalize(const string& name, NormalizePtr norm,
                           const string& category = "",
                           const IndicatorList& style_inds = IndicatorList());

  void reset();

  typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
  MultiFactorPtr clone();

  virtual void _reset() {}
  virtual MultiFactorPtr _clone() = 0;
  virtual IndicatorList _calculate(const vector<IndicatorList>&) = 0;

  bool isPythonObject() const noexcept { return is_python_object_; }

  /**
   * Execute the calculation. It is calculated automatically when the result is
   * got by default.
   *
   * Concurrency semantics (PR1):
   *   - Multiple threads are supported to trigger calculate / getter for the
   * first time on the same uncalculated instance at the same time;
   *   - After a successful calculation, multiple threads are allowed to read
   * concurrently;
   *   - The getter is not supported to be concurrent with
   *     reset/setQuery/setStockList/setRefFactorSet/setParam;
   *   - It is not supported that another thread modifies the instance while the
   * caller holds the internal reference returned by the getter.
   *
   * Failure semantics:
   *   - When the calculation throws an exception, the derived cache of the base
   * class is cleaned up and m_calculated stays false;
   *   - The exception propagates upward, the next caller can recalculate (DCLP,
   * not single-flight).
   */
  void calculate();

 private:
  void initParam();

  // Clear all the derived data generated after the calculation, keeping the
  // configuration members. It is called before the construction in calculate()
  // and after an exception, ensuring that the retry is based on a clean state.
  void clearCalculatedData();

  // Build the industry attribution labels (integer block indexes) and the block
  // count of every indicator, so that the industry neutralization can be
  // performed. It returns {factor_name -> (labels, blk_count)}:
  //   labels[i] = the index of the block the stock i belongs to in blks
  //   (0..blk_count-1), and it is blk_count if there is no attribution.
  unordered_map<string, std::pair<PriceList, size_t>> _buildDummyIndex();

  void _buildIndex();  // Create the cross-section index after the calculation
                       // is finished

  void _checkData();

 protected:
  IndicatorList _getAllReturns(int ndays) const;

 protected:
  bool is_python_object_{false};
  string name_;
  FactorSet factorset_;  // The input original factor set
  StockList stks_;       // Security portfolio
  Stock ref_stk_;  // The given reference security, it is used to align the
                    // dates only
  KQuery query_;   // The date range condition of the calculation

  NormPtr norm_;  // Global standardization / normalization
                   // operation
  unordered_map<string, NormPtr>
      special_norms_;  // The specific standardization operation
                        // performed on a specific indicator
  unordered_map<string, string>
      special_category_;  // The block category given when the industry
                           // neutralization is performed on a specific
                           // indicator
  unordered_map<string, IndicatorList>
      special_style_inds_;  // The style factors given when the style factor
                             // neutralization is performed on a specific
                             // indicator

  // The following variables are generated after the calculation
  DatetimeList ref_dates_;  // The reference dates calculated from the
                             // reference security and the query, the
                             // synthesized factor is aligned to these dates
  unordered_map<Stock, size_t> stk_map_;  // Security -> the position index of
                                           // the synthesized factor
  IndicatorList all_factors_;  // Saves the new factors synthesized from all
                                // the securities
  unordered_map<Datetime, size_t> date_index_;
  vector<ScoreRecordList> stk_factor_by_date_;
  Indicator ic_;

 private:
  std::mutex mutex_;
  std::atomic<bool> calculated_{false};

//============================================
// Serialization support
//============================================
#if HAYAKU_SUPPORT_SERIALIZATION
 private:
  friend class boost::serialization::access;
  template <class Archive>
  void save(Archive& ar, const unsigned int version) const {
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_factorset", factorset_);
    ar& boost::serialization::make_nvp("m_stks", stks_);
    ar& boost::serialization::make_nvp("m_ref_stk", ref_stk_);
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_norm", norm_);
    ar& boost::serialization::make_nvp("m_special_norms", special_norms_);
    ar& boost::serialization::make_nvp("m_special_category", special_category_);
    ar& boost::serialization::make_nvp("m_special_style_inds", special_style_inds_);
    // The following do not need to be saved, they are recalculated after
    // loading ar& boost::serialization::make_nvp("m_stk_map", stk_map_); ar&
    // boost::serialization::make_nvp("m_all_factors", all_factors_); ar&
    // boost::serialization::make_nvp("m_date_index", date_index_); ar& boost::serialization::make_nvp("m_ic", ic_);
    // ar& boost::serialization::make_nvp("m_calculated", calculated_);
    // ar& boost::serialization::make_nvp("m_stk_factor_by_date", stk_factor_by_date_);
  }

  template <class Archive>
  void load(Archive& ar, const unsigned int version) {
    ar& boost::serialization::make_nvp("m_is_python_object", is_python_object_);
    ar& boost::serialization::make_nvp("m_name", name_);
    ar& boost::serialization::make_nvp("m_params", params_);
    ar& boost::serialization::make_nvp("m_factorset", factorset_);
    ar& boost::serialization::make_nvp("m_stks", stks_);
    ar& boost::serialization::make_nvp("m_ref_stk", ref_stk_);
    ar& boost::serialization::make_nvp("m_query", query_);
    ar& boost::serialization::make_nvp("m_norm", norm_);
    ar& boost::serialization::make_nvp("m_special_norms", special_norms_);
    ar& boost::serialization::make_nvp("m_special_category", special_category_);
    ar& boost::serialization::make_nvp("m_special_style_inds", special_style_inds_);
    // ar& boost::serialization::make_nvp("m_stk_map", stk_map_);
    // ar& boost::serialization::make_nvp("m_all_factors", all_factors_);
    // ar& boost::serialization::make_nvp("m_date_index", date_index_);
    // ar& boost::serialization::make_nvp("m_ic", ic_);
    // ar& boost::serialization::make_nvp("m_calculated", calculated_);
    // ar& boost::serialization::make_nvp("m_stk_factor_by_date", stk_factor_by_date_);
    calculated_.store(false, std::memory_order_relaxed);
  }

  BOOST_SERIALIZATION_SPLIT_MEMBER()
#endif /* HAYAKU_SUPPORT_SERIALIZATION */
};

#if HAYAKU_SUPPORT_SERIALIZATION
BOOST_SERIALIZATION_ASSUME_ABSTRACT(MultiFactorBase)
#endif

#if HAYAKU_SUPPORT_SERIALIZATION
/**
 * For an inheriting subclass without private variables, this macro can be used
 * directly for the serialization
 * @code
 * class Drived: public MultiFactorBase {
 *     MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
 *
 * public:
 *     Drived();
 *     ...
 * };
 * @endcode
 * @ingroup MultiFactor
 */
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION           \
 private:                                                     \
  friend class boost::serialization::access;                  \
  template <class Archive>                                    \
  void serialize(Archive& ar, const unsigned int version) {   \
    ar& BOOST_SERIALIZATION_BASE_OBJECT_NVP(MultiFactorBase); \
  }
#else
#define MULTIFACTOR_NO_PRIVATE_MEMBER_SERIALIZATION
#endif

typedef std::shared_ptr<MultiFactorBase> FactorPtr;
typedef std::shared_ptr<MultiFactorBase> MultiFactorPtr;
typedef std::shared_ptr<MultiFactorBase> MFPtr;

#define MULTIFACTOR_IMP(classname)           \
 public:                                     \
  virtual MultiFactorPtr _clone() override { \
    return std::make_shared<classname>();    \
  }                                          \
  virtual IndicatorList _calculate(const vector<IndicatorList>&) override;

HAYAKU_API std::ostream& operator<<(std::ostream&, const MultiFactorBase&);
HAYAKU_API std::ostream& operator<<(std::ostream&, const MultiFactorPtr&);

}  // namespace hayaku

#if FMT_VERSION >= 90000
template <>
struct fmt::formatter<hayaku::MultiFactorBase> : ostream_formatter {};

template <>
struct fmt::formatter<hayaku::MultiFactorPtr> : ostream_formatter {};
#endif
