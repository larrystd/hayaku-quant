#pragma once

/*
 * Performance.h
 *
 *  Created on: 2013-4-23
 *      Author: fasiondog
 */

#include "execution/ExecutionAccountPort.h"

namespace hayaku {

#if defined(_MSC_VER)
#pragma warning(disable : 4251)
#endif

/**
 * Simple performance statistics
 * @ingroup Performance
 */
class Performance {
 public:
  Performance();
  virtual ~Performance();

  Performance(const Performance& other) = default;
  Performance(Performance&& other) noexcept
      : result_(std::move(other.result_)), keys_(std::move(other.keys_)) {}

  Performance& operator=(const Performance& other) noexcept;
  Performance& operator=(Performance&& other) noexcept;

  /** Whether it is a valid statistics item
   *  @note Legacy Chinese aliases remain accepted for lookup but are
   * deprecated. */
  bool exist(const string& key);

  /** Reset, clearing the calculated results */
  void reset();

  /** Get the statistics value by the item name; it takes effect only after
   * statistics or report has been run
   *  @note Legacy Chinese aliases remain accepted for lookup but are
   * deprecated. */
  double get(const string& name) const;

  /** The same as get */
  double operator[](const string& name) const { return get(name); }

  /**
   * A simple text statistics report, used for the direct printing output.
   * @note It takes effect only after statistics has been run, or when
   * Performance itself is the result got from TM
   * @return
   */
  string report();

  /**
   * Count the system performance up to a certain moment according to the trade
   * records; datetime must be greater than or equal to lastDatetime so that it
   * can be used to calculate the current market value
   * @param tm the given trade management instance
   * @param datetime the statistics end moment
   */
  void statistics(const internal::ExecutionAccountPortPtr& account,
                  const Datetime& datetime = Datetime::now());

  /** Get the names of all the statistics items, in the same order as values */
  const StringList& names() const { return keys_; }

  /** Get the values of all the statistics items, in the same order as names */
  PriceList values() const;

  typedef std::map<string, double> map_type;
  typedef map_type::iterator iterator;
  typedef map_type::const_iterator const_iterator;

  const map_type& getAll() const { return result_; }

  /** Add a new statistics item
   *  @note Only English keys are supported; non-English keys are rejected.
   *  @param chinese optional legacy alias accepted by get/exist; report always
   * uses the English key
   */
  void addKey(const string& key, const string& chinese = string());

  /** Set the value of the given statistics item
   *  @note Only English keys are supported; non-English keys are rejected.
   */
  void setValue(const string& key, double value);

 private:
  map_type result_;
  StringList
      keys_;  // Saves the order of the statistics items; neither map nor
              // unordered_map can keep the insertion order when iterating
};

} /* namespace hayaku */
