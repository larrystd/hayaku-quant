#pragma once

/*
 *  Copyright (c) 2026 hikyuu.org
 *
 *  Created on: 2026-02-10
 *      Author: fasiondog
 */

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <initializer_list>
#include <iterator>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

#include "data/MarketTypes.h"

namespace hayaku {

/**
 * IndicatorImpBuffer class, it fully customizes the memory management and is
 * compatible with malloc and mi_malloc; it provides a std::vector-like
 * interface to the outside
 */
class IndicatorImpBuffer {
 public:
// Type definitions
#if HAYAKU_USE_LOW_PRECISION
  typedef float value_type;
#else
  typedef double value_type;
#endif
  typedef value_type& reference;
  typedef const value_type& const_reference;
  typedef value_type* pointer;
  typedef const value_type* const_pointer;
  typedef value_type* iterator;
  typedef const value_type* const_iterator;
  typedef std::reverse_iterator<iterator> reverse_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;
  typedef size_t size_type;
  typedef ptrdiff_t difference_type;

 private:
  // Internal data structure
  struct Buffer {
    pointer data;        // Data pointer
    size_type size;      // Current number of elements
    size_type capacity;  // Allocated capacity

    Buffer() : data(nullptr), size(0), capacity(0) {}
    ~Buffer() { deallocate(); }

    void allocate(size_type new_capacity);
    void deallocate();
    void reallocate(size_type new_capacity);
  };

  Buffer buffer_;

 public:
  /** Default constructor */
  IndicatorImpBuffer() = default;

  /** Constructor with the given capacity */
  explicit IndicatorImpBuffer(size_type count) {
    if (count > 0) {
      buffer_.allocate(count);
      buffer_.size = count;
      std::uninitialized_value_construct_n(buffer_.data, count);
    }
  }

  /** Constructor with an initial value */
  IndicatorImpBuffer(size_type count, const value_type& value) {
    if (count > 0) {
      buffer_.allocate(count);
      buffer_.size = count;
      std::uninitialized_fill_n(buffer_.data, count, value);
    }
  }

  /** Copy constructor */
  IndicatorImpBuffer(const IndicatorImpBuffer& other) {
    if (other.buffer_.size > 0) {
      buffer_.allocate(other.buffer_.capacity);
      buffer_.size = other.buffer_.size;
      std::uninitialized_copy(other.buffer_.data,
                              other.buffer_.data + other.buffer_.size,
                              buffer_.data);
    }
  }

  /** Move constructor */
  IndicatorImpBuffer(IndicatorImpBuffer&& other) noexcept {
    // Transfer the resources directly
    buffer_.data = other.buffer_.data;
    buffer_.size = other.buffer_.size;
    buffer_.capacity = other.buffer_.capacity;

    // Clear the source object
    other.buffer_.data = nullptr;
    other.buffer_.size = 0;
    other.buffer_.capacity = 0;
  }

  /** Construct from an iterator range */
  template <typename InputIterator>
  IndicatorImpBuffer(InputIterator first, InputIterator last) {
    // Calculate the distance
    size_type count = static_cast<size_type>(std::distance(first, last));
    if (count > 0) {
      reserve(count);
      std::uninitialized_copy(first, last, buffer_.data);
      buffer_.size = count;
    }
  }

  /** Initializer list constructor */
  IndicatorImpBuffer(std::initializer_list<value_type> init_list) {
    if (init_list.size() > 0) {
      reserve(init_list.size());
      std::uninitialized_copy(init_list.begin(), init_list.end(), buffer_.data);
      buffer_.size = init_list.size();
    }
  }

  /** Assignment operator */
  IndicatorImpBuffer& operator=(const IndicatorImpBuffer& other) {
    if (this != &other) {
      if (other.buffer_.size > buffer_.capacity) {
        // The memory needs to be reallocated
        Buffer new_buffer;
        new_buffer.allocate(other.buffer_.capacity);
        std::uninitialized_copy(other.buffer_.data,
                                other.buffer_.data + other.buffer_.size,
                                new_buffer.data);
        new_buffer.size = other.buffer_.size;

        // Clean up the old resources and replace them
        buffer_.deallocate();
        buffer_.data = new_buffer.data;
        buffer_.size = new_buffer.size;
        buffer_.capacity = new_buffer.capacity;

        // Clear new_buffer to avoid a double free
        new_buffer.data = nullptr;
        new_buffer.size = 0;
        new_buffer.capacity = 0;
      } else {
        // Destroy the existing elements
        destroy_elements(buffer_.data, buffer_.data + buffer_.size);

        // Copy the new elements
        std::copy(other.buffer_.data, other.buffer_.data + other.buffer_.size,
                  buffer_.data);
        buffer_.size = other.buffer_.size;
      }
    }
    return *this;
  }

  /** Move assignment operator */
  IndicatorImpBuffer& operator=(IndicatorImpBuffer&& other) noexcept {
    if (this != &other) {
      // Clean up the resources of the current object
      clear();
      buffer_.deallocate();

      // Transfer the resources
      buffer_.data = other.buffer_.data;
      buffer_.size = other.buffer_.size;
      buffer_.capacity = other.buffer_.capacity;

      // Clear the source object
      other.buffer_.data = nullptr;
      other.buffer_.size = 0;
      other.buffer_.capacity = 0;
    }
    return *this;
  }

  /** Destructor */
  ~IndicatorImpBuffer() { clear(); }

  /** Overloaded new operator */
  static void* operator new(size_t size);

  /** Overloaded delete operator */
  static void operator delete(void* ptr) noexcept;

  /** Overloaded new[] operator */
  static void* operator new[](size_t size);

  /** Overloaded delete[] operator */
  static void operator delete[](void* ptr) noexcept;

  // Capacity related interface
  size_type size() const noexcept { return buffer_.size; }
  size_type capacity() const noexcept { return buffer_.capacity; }
  bool empty() const noexcept { return buffer_.size == 0; }
  size_type max_size() const noexcept {
    return static_cast<size_type>(-1) / sizeof(value_type);
  }

  // Modifier interface
  void resize(size_type count) {
    if (count < buffer_.size) {
      // Shrink
      destroy_elements(buffer_.data + count, buffer_.data + buffer_.size);
      buffer_.size = count;
    } else if (count > buffer_.size) {
      // Expand
      if (count > buffer_.capacity) {
        reserve(count);
      }
      std::uninitialized_value_construct(buffer_.data + buffer_.size,
                                         buffer_.data + count);
      buffer_.size = count;
    }
  }

  void resize(size_type count, const value_type& value) {
    if (count < buffer_.size) {
      // Shrink
      destroy_elements(buffer_.data + count, buffer_.data + buffer_.size);
      buffer_.size = count;
    } else if (count > buffer_.size) {
      // Expand
      if (count > buffer_.capacity) {
        reserve(count);
      }
      std::uninitialized_fill(buffer_.data + buffer_.size, buffer_.data + count,
                              value);
      buffer_.size = count;
    }
  }

  void reserve(size_type new_cap) {
    if (new_cap > buffer_.capacity) {
      buffer_.reallocate(calculate_growth(new_cap));
    }
  }

  void shrink_to_fit() {
    if (buffer_.capacity > buffer_.size) {
      buffer_.reallocate(buffer_.size);
    }
  }

  void clear() noexcept {
    destroy_elements(buffer_.data, buffer_.data + buffer_.size);
    buffer_.size = 0;
  }

  // Element access interface
  reference at(size_type pos) {
    if (pos >= buffer_.size) {
      throw std::out_of_range("IndicatorImpBuffer::at: position out of range");
    }
    return buffer_.data[pos];
  }

  const_reference at(size_type pos) const {
    if (pos >= buffer_.size) {
      throw std::out_of_range("IndicatorImpBuffer::at: position out of range");
    }
    return buffer_.data[pos];
  }

  reference operator[](size_type pos) { return buffer_.data[pos]; }
  const_reference operator[](size_type pos) const { return buffer_.data[pos]; }
  reference front() { return buffer_.data[0]; }
  const_reference front() const { return buffer_.data[0]; }
  reference back() { return buffer_.data[buffer_.size - 1]; }
  const_reference back() const { return buffer_.data[buffer_.size - 1]; }
  pointer data() noexcept { return buffer_.data; }
  const_pointer data() const noexcept { return buffer_.data; }

  // Iterator interface
  iterator begin() noexcept { return buffer_.data; }
  const_iterator begin() const noexcept { return buffer_.data; }
  const_iterator cbegin() const noexcept { return buffer_.data; }
  iterator end() noexcept { return buffer_.data + buffer_.size; }
  const_iterator end() const noexcept { return buffer_.data + buffer_.size; }
  const_iterator cend() const noexcept { return buffer_.data + buffer_.size; }
  reverse_iterator rbegin() noexcept { return reverse_iterator(end()); }
  const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }
  const_reverse_iterator crbegin() const noexcept { return rbegin(); }
  reverse_iterator rend() noexcept { return reverse_iterator(begin()); }
  const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }
  const_reverse_iterator crend() const noexcept { return rend(); }

  // Modifier interface
  void push_back(const value_type& value) {
    if (buffer_.size >= buffer_.capacity) {
      reserve(buffer_.size + 1);
    }
    new (buffer_.data + buffer_.size) value_type(value);
    ++buffer_.size;
  }

  void push_back(value_type&& value) {
    if (buffer_.size >= buffer_.capacity) {
      reserve(buffer_.size + 1);
    }
    new (buffer_.data + buffer_.size) value_type(std::move(value));
    ++buffer_.size;
  }

  template <class... Args>
  reference emplace_back(Args&&... args) {
    if (buffer_.size >= buffer_.capacity) {
      reserve(calculate_growth(buffer_.size + 1));
    }

    pointer pos = buffer_.data + buffer_.size;
    new (pos) value_type(std::forward<Args>(args)...);
    ++buffer_.size;
    return *pos;
  }

  void pop_back() {
    if (buffer_.size > 0) {
      --buffer_.size;
      std::destroy_at(buffer_.data + buffer_.size);
    }
  }

  iterator insert(const_iterator pos, const value_type& value) {
    return emplace(pos, value);
  }

  iterator insert(const_iterator pos, value_type&& value) {
    return emplace(pos, std::move(value));
  }

  iterator insert(const_iterator pos, size_type count,
                  const value_type& value) {
    difference_type pos_offset = pos - buffer_.data;

    if (count == 0) return const_cast<iterator>(pos);

    if (buffer_.size + count > buffer_.capacity) {
      reserve(calculate_growth(buffer_.size + count));
      // Recalculate the position because reserve may cause the memory to be
      // reallocated
      pos = buffer_.data + pos_offset;
    }

    iterator pos_it = const_cast<iterator>(pos);
    if (pos_offset < static_cast<difference_type>(buffer_.size)) {
      move_elements_backward(pos_it, buffer_.data + buffer_.size,
                             pos_it + count);
    }

    std::uninitialized_fill_n(pos_it, count, value);
    buffer_.size += count;

    return pos_it;
  }

  template <class InputIt>
  iterator insert(const_iterator pos, InputIt first, InputIt last) {
    difference_type pos_offset = pos - buffer_.data;
    difference_type count = std::distance(first, last);

    if (count == 0) return const_cast<iterator>(pos);

    if (buffer_.size + count > buffer_.capacity) {
      reserve(calculate_growth(buffer_.size + count));
      // Recalculate the position because reserve may cause the memory to be
      // reallocated
      pos = buffer_.data + pos_offset;
    }

    iterator pos_it = const_cast<iterator>(pos);
    if (pos_offset < static_cast<difference_type>(buffer_.size)) {
      move_elements_backward(pos_it, buffer_.data + buffer_.size,
                             pos_it + count);
    }

    std::uninitialized_copy(first, last, pos_it);
    buffer_.size += count;

    return pos_it;
  }

  template <class... Args>
  iterator emplace(const_iterator pos, Args&&... args) {
    difference_type pos_offset = pos - buffer_.data;

    if (buffer_.size >= buffer_.capacity) {
      reserve(calculate_growth(buffer_.size + 1));
      // Recalculate the position because reserve may cause the memory to be
      // reallocated
      pos = buffer_.data + pos_offset;
    }

    iterator pos_it = const_cast<iterator>(pos);
    if (pos_offset < static_cast<difference_type>(buffer_.size)) {
      move_elements_backward(pos_it, buffer_.data + buffer_.size, pos_it + 1);
    }

    new (pos_it) value_type(std::forward<Args>(args)...);
    ++buffer_.size;

    return pos_it;
  }

  iterator erase(const_iterator pos) {
    iterator pos_it = const_cast<iterator>(pos);
    // Destroy the elements to be erased first
    std::destroy_at(pos_it);
    // Then move the following elements
    std::move(pos_it + 1, buffer_.data + buffer_.size, pos_it);
    --buffer_.size;
    return pos_it;
  }

  iterator erase(const_iterator first, const_iterator last) {
    if (first == last) return const_cast<iterator>(first);

    iterator first_it = const_cast<iterator>(first);
    iterator last_it = const_cast<iterator>(last);

    // Destroy the elements within the erased range
    destroy_elements(first_it, last_it);

    // Move the following elements
    std::move(last_it, buffer_.data + buffer_.size, first_it);
    size_type count = last - first;
    buffer_.size -= count;

    return first_it;
  }

  void swap(IndicatorImpBuffer& other) noexcept {
    std::swap(buffer_.data, other.buffer_.data);
    std::swap(buffer_.size, other.buffer_.size);
    std::swap(buffer_.capacity, other.buffer_.capacity);
  }

  // STL algorithm compatible interface
  template <typename UnaryPredicate>
  iterator erase_if(UnaryPredicate p) {
    iterator it = std::remove_if(begin(), end(), p);
    size_type new_size = it - begin();
    destroy_elements(buffer_.data + new_size, buffer_.data + buffer_.size);
    buffer_.size = new_size;
    return it;
  }

  template <typename Compare>
  void sort(Compare comp) {
    std::sort(begin(), end(), comp);
  }

  void sort() { sort(std::less<value_type>()); }

 private:
  void construct_elements(pointer first, pointer last,
                          const value_type& value) {
    std::uninitialized_fill(first, last, value);
  }

  void destroy_elements(pointer first, pointer last) {
    std::destroy(first, last);
  }

  void move_elements_backward(pointer first, pointer last, pointer dest) {
    // Move the elements backward manually to avoid the complexity of
    // std::move_backward
    difference_type count = last - first;
    if (count <= 0) return;

    // Move the elements from the back to the front
    for (difference_type i = count - 1; i >= 0; --i) {
      *(dest + i) = std::move(*(first + i));
    }
  }

  size_type calculate_growth(size_type new_size) const {
    size_type current_capacity = buffer_.capacity;
    if (current_capacity == 0) {
      return std::max(new_size, static_cast<size_type>(16));
    }
    // return std::max(new_size, current_capacity * 2);
    // The Indicator cache always requests the space or resizes in advance, it
    // does not grow by 2x; it increases by the actual minute-line increment
    return std::max(new_size, current_capacity) + 240;
  }
};

// Non-member functions
inline void swap(IndicatorImpBuffer& lhs, IndicatorImpBuffer& rhs) noexcept {
  lhs.swap(rhs);
}

inline bool operator==(const IndicatorImpBuffer& lhs,
                       const IndicatorImpBuffer& rhs) {
  if (lhs.size() != rhs.size()) {
    return false;
  }
  return std::equal(lhs.begin(), lhs.end(), rhs.begin());
}

inline bool operator!=(const IndicatorImpBuffer& lhs,
                       const IndicatorImpBuffer& rhs) {
  return !(lhs == rhs);
}

inline bool operator<(const IndicatorImpBuffer& lhs,
                      const IndicatorImpBuffer& rhs) {
  return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                      rhs.end());
}

inline bool operator<=(const IndicatorImpBuffer& lhs,
                       const IndicatorImpBuffer& rhs) {
  return !(rhs < lhs);
}

inline bool operator>(const IndicatorImpBuffer& lhs,
                      const IndicatorImpBuffer& rhs) {
  return rhs < lhs;
}

inline bool operator>=(const IndicatorImpBuffer& lhs,
                       const IndicatorImpBuffer& rhs) {
  return !(lhs < rhs);
}

}  // namespace hayaku
