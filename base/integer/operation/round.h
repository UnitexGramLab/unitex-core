/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA.
 *
 */
/**
 * @file      round.h
 * @brief     Integer round operations
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 round.h`
 *
 * @date      September 2017
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_INTEGER_OPERATION_ROUND_H_                       // NOLINT
#define UNITEX_BASE_INTEGER_OPERATION_ROUND_H_                       // NOLINT
/* ************************************************************************** */
// Unitex headers
//#include "base/compiler/intrinsics.h"  // UNITEX_HAS_BUILTIN
#include "base/integer/types.h"        // uint32_t, uint64_t
#include "base/bits/size.h"            // size_in_bits
/* ************************************************************************** */
# if 0
# define UNITEX__ROUND__UP__POWER__OF__TWO__(SIZE, N, MIN, DECAY)            \
  return (N != 0) ? (UINT##SIZE##_C(1) << (sizeof(uint##SIZE##_t) *          \
                     CHAR_BIT - unitex_builtin_clz_##SIZE(N + DECAY))) : MIN
# else
// Based on Bit Twiddling Hacks, Round up to the next highest power of 2
// @see https://graphics.stanford.edu/~seander/bithacks.html
# define UNITEX__ROUND__UP__POWER__OF__TWO__(SIZE, N, MIN, DECAY)            \
  if (N == 0) return MIN;                                                    \
  N += DECAY;                                                                \
  for (size_t i = 1; i <  sizeof(uint##SIZE##_t) * CHAR_BIT; i *= 2) {       \
    N |= N >> i;                                                             \
  }                                                                          \
  ++N;                                                                       \
  return N
# endif

/* ************************************************************************** */

UNITEX_FORCE_INLINE
static uint32_t unitex_round_up_greater_power_of_two_32(uint32_t n,
                                                 uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(32, n, min, 0);
}

UNITEX_FORCE_INLINE
static uint32_t unitex_round_up_smallest_power_of_two_32(uint32_t n,
                                                  uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(32, n, min, -1);
}

UNITEX_FORCE_INLINE
static uint32_t unitex_round_up_greater_power_of_two_64(uint64_t n,
                                                 uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(64, n, min, 0);
}

UNITEX_FORCE_INLINE
static uint32_t unitex_round_up_smallest_power_of_two_64(uint64_t n,
                                                  uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(64, n, min, -1);
}

//namespace unitex {
///* ************************************************************************** */
//namespace details {  // details
///* ************************************************************************** */
//// Partial specialization of function templates is not allowed, we use
//// round_up_power_of_two_impl as a workaround
///**
// * round_up_power_of_two_impl
// */
//template <typename T, uint32_t n>
//struct round_up_power_of_two_impl;
//
///**
// * specialization to round up to power of two a 0 bits data type
// */
//template<typename T>
//struct round_up_power_of_two_impl<T, 0> {
//  /**
//   * @brief  The number of bits in the 0-bit n that have the value 1 are
//   *         counted, and the resulting sum is returned
//   */
//  UNITEX_FORCE_INLINE
//  static uint32_t round_up_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//    return static_cast<uint32_t>(0);
//  }
//};
//
///**
// * @brief  Count the number of bits set to one in a 1-byte data type
// */
//// specialization to round up to power of two a 1 byte data type
//template<typename T>
//struct round_up_power_of_two_impl<T, 8> {
//  /**
//   * @brief  The number of bits in the 8-bit n that have the value 1 are
//   *         counted, and the resulting sum is returned
//   */
//  UNITEX_FORCE_INLINE
//  static uint32_t round_up_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//    UNITEX__ROUND__UP__POWER__OF__TWO__(32, static_cast<uint32_t>(n), min, 0);
//  }
//};
//
///**
// * @brief  Count the number of bits set to one in a 2-byte data type
// */
//// specialization to round up to power of two a 2 bytes data type
//template<typename T>
//struct round_up_power_of_two_impl<T, 16> {
//  /**
//   * @brief  The number of bits in the 16-bit n that have the value 1 are
//   *         counted, and the resulting sum is returned
//   */
//  UNITEX_FORCE_INLINE
//  static uint32_t round_up_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//    UNITEX__ROUND__UP__POWER__OF__TWO__(32, static_cast<uint32_t>(n), min, 0);
//  }
//};
//
///**
// * @brief  Count the number of bits set to one in a 4-byte data type
// */
//// specialization to round up to power of two a 4 bytes data type
//template<typename T>
//struct round_up_power_of_two_impl<T, 32> {
//    /**
//     * @brief  The number of bits in the 32-bit n that have the value 1 are
//     *         counted, and the resulting sum is returned
//     */
//   UNITEX_FORCE_INLINE
//   static uint32_t round_up_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//    UNITEX__ROUND__UP__POWER__OF__TWO__(32, static_cast<uint32_t>(n), min, 0);
//  }
//};
//
///**
// * @brief  Count the number of bits set to one in a 8-byte data type
// */
//// specialization to round up to power of two a 8 bytes data type
//template<typename T>
//struct round_up_power_of_two_impl<T, 64> {
//  /**
//   * @brief  The number of bits in the 64-bit n that have the value 1 are
//   *         counted, and the resulting sum is returned
//   */
//  UNITEX_FORCE_INLINE
//  static uint32_t round_up_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//    UNITEX__ROUND__UP__POWER__OF__TWO__(64, n, min, 0);
//  }
//};
///* ************************************************************************** */
//}  // namespace details
///* ************************************************************************** */
//namespace util {  // util
///* ************************************************************************** */
///**
// * @brief  Count the number of bits in the n that have the value 1
// * @param  n
// *
// *
// * @code{.cpp}
// *   int32_t pow2 = unitex::util::round_up_power_of_two(); // n<=4
// *   int32_t pow2 = unitex::util::round_up_power_of_two(); // n<=4
// * @endcode
// *
// * @return uint32_t number of bits in n
// */
//template<typename T>
//UNITEX_FORCE_INLINE
//uint32_t round_up_greater_power_of_two(T n, uint32_t min = UINT32_C(2)) {
//  return details::round_up_power_of_two_impl<T, util::size_in_bits<T>::value>::round_up_power_of_two(n, min);
//}
///* ************************************************************************** */
//}  // namespace util
///* ************************************************************************** */
//}  // namespace unitex
/* ************************************************************************** */
#undef UNITEX__ROUND__UP__POWER__OF__TWO__
/* ************************************************************************** */
#endif  // UNITEX_BASE_INTEGER_OPERATION_ROUND_H_                   // NOLINT
