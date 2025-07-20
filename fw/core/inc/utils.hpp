/**
 * @file     utils.hpp
 * @author   Fabio Scatozza <s315216@studenti.polito.it>
 * @date     20.07.2025
 */

#ifndef UTILS_HPP
#define UTILS_HPP

#include <concepts>
#include <type_traits>

template <std::integral T>
constexpr T iround(T x, T y) {
  T sgn_x = (x < 0) ? -1 : 1;
  T sgn_y = (y < 0) ? -1 : 1;
  T sgn_q = (x < 0) == (y < 0) ? 1 : -1;

  T abs_r = (x % y) * sgn_x;
  T abs_yhalf = (y / 2) * sgn_y;
  return (x / y) + static_cast<T>(abs_r >= abs_yhalf) * sgn_q;
}

#endif //UTILS_HPP
