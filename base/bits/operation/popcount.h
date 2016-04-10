/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      popcount.h
 * @brief     Binary hamming weight (popcount)
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 popcount.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_BITS_OPERATION_POPCOUNT_H_                       // NOLINT
#define UNITEX_BASE_BITS_OPERATION_POPCOUNT_H_                       // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/bits/literal.h"         // mono_byte, dual_byte, ...
#include "base/compiler/intrinsics.h"  // UNITEX_HAS_BUILTIN
#include "base/integer/types.h"        // uint32_t
#include "base/bits/size.h"            // size_in_bits
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace details {  // details
/* ************************************************************************** */
// Partial specialization of function templates is not allowed, we use
// popcount_impl as a workaround
/**
 * popcount_impl
 */
template <typename T, uint32_t n>
struct popcount_impl;

/**
 * specialization to count a 0 bits data type
 */
template<typename T>
struct popcount_impl<T, 0> {
  /**
   * @brief  The number of bits in the 0-bit n that have the value 1 are
   *         counted, and the resulting sum is returned
   */
  UNITEX_FORCE_INLINE
  static uint32_t count(T n) {
    return static_cast<uint32_t>(0);
  }
};

/**
 * @brief  Count the number of bits set to one in a 1-byte data type
 */
// specialization to count a 1 byte data type
template<typename T>
struct popcount_impl<T, 8> {
  /**
   * @brief  The number of bits in the 8-bit n that have the value 1 are
   *         counted, and the resulting sum is returned
   */
  UNITEX_FORCE_INLINE
  static uint32_t count(T n) {
    T count = n;
#   if UNITEX_HAS_BUILTIN(POPCOUNT)       &&\
       UNITEX_COMPILER_BUILTIN_POPCOUNT >= 8
    count = unitex_builtin_popcount_8(n);
#   else  // !UNITEX_HAS_BUILTIN(POPCOUNT) ...
    count = (count       & mono_byte(01010101)) +
           ((count >> 1) & mono_byte(01010101));
    count = (count       & mono_byte(00110011)) +
           ((count >> 2) & mono_byte(00110011));
    count = (count       & mono_byte(00001111)) +
           ((count >> 4) & mono_byte(00001111));
#   endif  // UNITEX_HAS_BUILTIN(POPCOUNT) ...
    return static_cast<uint32_t>(count);
  }
};

/**
 * @brief  Count the number of bits set to one in a 2-byte data type
 */
// specialization to count a 2 bytes data type
template<typename T>
struct popcount_impl<T, 16> {
  /**
   * @brief  The number of bits in the 16-bit n that have the value 1 are
   *         counted, and the resulting sum is returned
   */
  UNITEX_FORCE_INLINE
  static uint32_t count(T n) {
    T count = n;
#   if UNITEX_HAS_BUILTIN(POPCOUNT)        &&\
       UNITEX_COMPILER_BUILTIN_POPCOUNT >= 16
    count = unitex_builtin_popcount_16(n);
#   else  // !UNITEX_HAS_BUILTIN(POPCOUNT) ...
    count = (count       & dual_byte(01010101,01010101)) +
           ((count >> 1) & dual_byte(01010101,01010101));
    count = (count       & dual_byte(00110011,00110011)) +
           ((count >> 2) & dual_byte(00110011,00110011));
    count = (count       & dual_byte(00001111,00001111)) +
           ((count >> 4) & dual_byte(00001111,00001111));
    count = (count       & dual_byte(00000000,11111111)) +
           ((count >> 8) & dual_byte(00000000,11111111));
#   endif  // UNITEX_HAS_BUILTIN(POPCOUNT) ...
    return static_cast<uint32_t>(count);
  }
};

/**
 * @brief  Count the number of bits set to one in a 4-byte data type
 */
// specialization to count a 4 bytes data type
template<typename T>
struct popcount_impl<T, 32> {
    /**
     * @brief  The number of bits in the 32-bit n that have the value 1 are
     *         counted, and the resulting sum is returned
     */
   UNITEX_FORCE_INLINE
   static uint32_t count(T n) {
    T count = n;
#   if UNITEX_HAS_BUILTIN(POPCOUNT)        &&\
       UNITEX_COMPILER_BUILTIN_POPCOUNT >= 32
    count = unitex_builtin_popcount_32(n);
#   else  // !UNITEX_HAS_BUILTIN(POPCOUNT) ...
    count = (count       & quad_byte(01010101,01010101,01010101,01010101)) +
           ((count >> 1) & quad_byte(01010101,01010101,01010101,01010101));
    count = (count       & quad_byte(00110011,00110011,00110011,00110011)) +
           ((count >> 2) & quad_byte(00110011,00110011,00110011,00110011));
    count = (count       & quad_byte(00001111,00001111,00001111,00001111)) +
           ((count >> 4) & quad_byte(00001111,00001111,00001111,00001111));
    count = (count       & quad_byte(00000000,11111111,00000000,11111111)) +
           ((count >> 8) & quad_byte(00000000,11111111,00000000,11111111));
    count = (count       & quad_byte(00000000,00000000,11111111,11111111)) +
           ((count >> 16)& quad_byte(00000000,00000000,11111111,11111111));
#   endif  // UNITEX_HAS_BUILTIN(POPCOUNT) ...
    return static_cast<uint32_t>(count);
  }
};

/**
 * @brief  Count the number of bits set to one in a 8-byte data type
 */
// specialization to count a 8 bytes data type
template<typename T>
struct popcount_impl<T, 64> {
  /**
   * @brief  The number of bits in the 64-bit n that have the value 1 are
   *         counted, and the resulting sum is returned
   */
  UNITEX_FORCE_INLINE
  static uint32_t count(T n) {
    T count = n;
#   if UNITEX_HAS_BUILTIN(POPCOUNT)        &&\
       UNITEX_COMPILER_BUILTIN_POPCOUNT >= 64
    count = unitex_builtin_popcount_64(n);
#   else  // !UNITEX_HAS_BUILTIN(POPCOUNT) ...
    count = (count       & octo_byte(01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101)) +
           ((count >> 1) & octo_byte(01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101));
    count = (count       & octo_byte(00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011)) +
           ((count >> 2) & octo_byte(00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011));
    count = (count       & octo_byte(00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111)) +
           ((count >> 4) & octo_byte(00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111));
    count = (count       & octo_byte(00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111)) +
           ((count >> 8) & octo_byte(00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111));
    count = (count       & octo_byte(00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111)) +
           ((count >> 16)& octo_byte(00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111));
    count = (count       & octo_byte(00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111)) +
           ((count >> 32)& octo_byte(00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111));
#   endif  // UNITEX_HAS_BUILTIN(POPCOUNT) ...
    return static_cast<uint32_t>(count);
  }
};

#if UNITEX_HAVE(UINT128_T)
/**
 * @brief  Count the number of bits set to one in a 16-byte data type
 */
// specialization to count a 16 bytes data type
template<typename T>
struct popcount_impl<T, 128> {
  /**
   * @brief  The number of bits in the 128-bit n that have the value 1 are
   *         counted, and the resulting sum is returned
   */
  UNITEX_FORCE_INLINE
  static uint32_t count(T n) {
    T count = n;
#   if UNITEX_HAS_BUILTIN(POPCOUNT)         &&\
       UNITEX_COMPILER_BUILTIN_POPCOUNT >= 128
    count = unitex_builtin_popcount_128(n);
#   else  // !UNITEX_HAS_BUILTIN(POPCOUNT) ...
    count = (count       & hexa_byte(01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101)) +
           ((count >> 1) & hexa_byte(01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101,
                                     01010101,01010101,01010101,01010101));
    count = (count       & hexa_byte(00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011)) +
           ((count >> 2) & hexa_byte(00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011,
                                     00110011,00110011,00110011,00110011));
    count = (count       & hexa_byte(00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111)) +
           ((count >> 4) & hexa_byte(00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111,
                                     00001111,00001111,00001111,00001111));
    count = (count       & hexa_byte(00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111)) +
           ((count >> 8) & hexa_byte(00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111,
                                     00000000,11111111,00000000,11111111));
    count = (count       & hexa_byte(00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111)) +
           ((count >> 16)& hexa_byte(00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111,
                                     00000000,00000000,11111111,11111111));
    count = (count       & hexa_byte(00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111,
                                     00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111)) +
           ((count >> 32)& hexa_byte(00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111,
                                     00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111));
    count = (count       & hexa_byte(00000000,00000000,00000000,00000000,
                                     00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111,
                                     11111111,11111111,11111111,11111111)) +
           ((count >> 64)& hexa_byte(00000000,00000000,00000000,00000000,
                                     00000000,00000000,00000000,00000000,
                                     11111111,11111111,11111111,11111111,
                                     11111111,11111111,11111111,11111111));
#   endif  // UNITEX_HAS_BUILTIN(POPCOUNT) ...
    return static_cast<uint32_t>(count);
  }
};
#endif  // UNITEX_HAVE(UINT128_T)
/* ************************************************************************** */
}  // namespace details
/* ************************************************************************** */
namespace util {  // util
/* ************************************************************************** */
/**
 * @brief  Count the number of bits in the n that have the value 1
 * @param  n
 *
 *
 * @code{.cpp}
 *   int32_t popcnt = unitex::util::popcount(mono_byte(10101010)); // n<=4
 *   int32_t popcnt = unitex::util::popcount(170);                 // n<=4
 * @endcode
 *
 * @return uint32_t number of bits in n
 */
template<typename T>
UNITEX_FORCE_INLINE
uint32_t popcount(T n) {
  return details::popcount_impl<T, util::size_in_bits<T>::value>::count(n);
}
/* ************************************************************************** */
}  // namespace util
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_BITS_OPERATION_POPCOUNT_H_                   // NOLINT
