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
#include "base/compiler/intrinsics.h"    // UNITEX_HAS_BUILTIN
#include "base/integer/types.h"          // uint32_t, uint64_t
#include "base/bits/size.h"              // size_in_bits
/* ************************************************************************** */
# if UNITEX_HAS_BUILTIN(CLZ)
# define UNITEX__ROUND__UP__POWER__OF__TWO__(SIZE, N, MIN, FIT)               \
  return (N < MIN + FIT) ? MIN :                                              \
                            (UINT##SIZE##_C(1) << (sizeof(uint##SIZE##_t) *   \
                             CHAR_BIT - unitex_builtin_clz_##SIZE(N + FIT)))
# else
// Based on Bit Twiddling Hacks, Round up to the next highest power of 2
// @see https://graphics.stanford.edu/~seander/bithacks.html
# define UNITEX__ROUND__UP__POWER__OF__TWO__(SIZE, N, MIN, FIT)              \
  if (N < MIN + FIT) return MIN;                                             \
  N += FIT;                                                                  \
  for (size_t i = 1; i <  sizeof(uint##SIZE##_t) * CHAR_BIT; i *= 2) {       \
    N |= N >> i;                                                             \
  }                                                                          \
  ++N;                                                                       \
  return N
# endif
/* ************************************************************************** */
/**
 * @brief  Round up to the next highest power of 2
 * @param  n a 32-bits number
 * @param  min value to return if n is equal to zero, default is 2
 */
UNITEX_FORCE_INLINE
uint32_t next_greater_power_of_two_32(uint32_t n,
                                      uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(32, n, min, 0);
}

/**
 * @brief  Round up to the next lowest power of 2
 * @param  n a 32-bits number
 * @param  min value to return if n is equal to zero, default is 2
 */
UNITEX_FORCE_INLINE
uint32_t next_smallest_power_of_two_32(uint32_t n,
                                       uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(32, n, min, -1);
}

/**
 * @brief  Round up to the next highest power of 2
 * @param  n a 64-bits number
 * @param  min value to return if n is equal to zero, default is 2
 */
UNITEX_FORCE_INLINE
uint32_t next_greater_power_of_two_64(uint64_t n,
                                      uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(64, n, min, 0);
}

/**
 * @brief  Round up to the next lowest power of 2
 * @param  n a 64-bits number
 * @param  min value to return if n is equal to zero, default is 2
 */
UNITEX_FORCE_INLINE
uint32_t next_smallest_power_of_two_64(uint64_t n,
                                       uint32_t min = UINT32_C(2)) {
  UNITEX__ROUND__UP__POWER__OF__TWO__(64, n, min, -1);
}
/* ************************************************************************** */
#endif  // UNITEX_BASE_INTEGER_OPERATION_ROUND_H_                   // NOLINT
