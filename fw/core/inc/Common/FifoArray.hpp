/**
 * @file     FifoArray.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     28.05.2025
 */

#ifndef FIFOARRAY_HPP
#define FIFOARRAY_HPP

#include <array>
#include <algorithm>

template <typename T, std::size_t N>
class FifoArray {
  static_assert((N & (N - 1)) == 0, "N must be a power of 2");

 public:
  using value_type = T;
  using size_type = std::size_t;
  using reference = value_type&;
  using const_reference = const value_type&;
  using pointer = value_type*;
  using const_pointer = const value_type*;

  FifoArray() : _wr(), _rd() {}

  /* !! Linearize prior to traverisng */

  pointer begin() {
    return _data.data() + (_rd & DataMask());
  }
  pointer end() {
    return begin() + size();
  }
  const_pointer begin() const {
    return _data.data() + (_rd & DataMask());
  }
  const_pointer end() const {
    return begin() + size();
  }
  const_pointer cbegin() const {
    return begin();
  }
  const_pointer cend() const {
    return end();
  }

  bool linearize() {
    if (is_linearized())
      return false;

    const auto wr_hops = _wr & DataMask();
    const auto rd_hops = N - (_rd & DataMask());

    if (wr_hops <= rd_hops) {
      std::rotate(_data.begin(), _data.begin() + wr_hops, _data.end());
      _wr = Next(_wr, -wr_hops);
      _rd = Next(_rd, -wr_hops);
    } else {
      std::rotate(_data.rbegin(), _data.rbegin() + rd_hops, _data.rend());
      _wr = Next(_wr, rd_hops);
      _rd = Next(_rd, rd_hops);
    }

    return true;
  }
  bool is_linearized() const {
    return empty() || (Next(_wr, -1) & DataMask()) >= (_rd & DataMask());
  }

  reference front() { return _data[_rd & DataMask()]; }
  const_reference front() const { return _data[_rd & DataMask()]; }

  bool empty() const { return _wr == _rd; }
  bool full() const { return (_wr ^ N) == _rd; }
  size_type size() const { return (_wr - _rd) & PtrMask(); }

  bool push(const value_type& value) {
    if (full()) return false;

    _data[_wr & DataMask()] = value;
    _wr = Next(_wr);
    return true;
  }
  bool push(value_type&& value) {
    if (full()) return false;

    _data[_wr & DataMask()] = std::move(value);
    _wr = Next(_wr);
    return true;
  }
  bool pop() {
    if (empty()) return false;
    _rd = Next(_rd);
    return true;
  }

  void clear() { _rd = _wr; }

 private:
  static constexpr auto PtrMask() { return (N << 1) - 1; }
  static constexpr auto DataMask() { return N - 1; }

  static constexpr auto Next(std::size_t ptr, std::ptrdiff_t n = 1) {
    return (ptr + n) & PtrMask();
  }

  std::size_t _wr;
  std::size_t _rd;
  std::array<T, N> _data;
};

#endif  // FIFOARRAY_HPP
