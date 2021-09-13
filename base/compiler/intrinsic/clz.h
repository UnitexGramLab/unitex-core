/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      clz.h
 * @brief     Count leading zeros (clz) routine
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *             header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 clz.h`
 *
 * @date      September 2017
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_INTRINSICS_CLZ_H_                      // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSICS_CLZ_H_                      // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/attribute/inline.h"   // UNITEX_FORCE_INLINE
#include "base/compiler/intrinsic/likely.h"   // UNITEX_LIKELY
#include "base/compiler/intrinsic/support.h"  // common intrinsic functions
#include "base/compiler/version.h"            // UNITEX_COMPILER_AT_LEAST
#include "base/cpu/version.h"                 // UNITEX_CPU_IS
#include "base/integer/types.h"               // uint32_t, uint64_t
#include "base/integer/wordsize.h"            // UNITEX_WORDSIZE_IS()
/* ************************************************************************** */
/**
 * @def    unitex_builtin_clz_32
 * @brief  Count the number of leading 0-bits on a 32-bits data type
 */
#if UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#pragma intrinsic(_BitScanReverse)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
UNITEX_FORCE_INLINE
uint32_t unitex_builtin_clz_32(uint32_t n) {
// GNU C/C++ compiler
// LLVM native C/C++ compiler
#if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
    UNITEX_COMPILER_AT_LEAST(CLANG,1,5)
#  define UNITEX_HAS_BUILTIN_CLZ                   1
   return UNITEX_LIKELY(n) ? __builtin_clz(n)      : 32;
// Visual Studio 2005 and later
#elif UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#  define UNITEX_HAS_BUILTIN_CLZ                   1
   uint32_t count = 32;
   if (UNITEX_LIKELY(n)) {
     _BitScanReverse((DWORD*)&count, n);
     count ^= 31;
   }
   return count;
// Intel Compiler
#elif UNITEX_COMPILER_IS(INTEL)
#  define UNITEX_HAS_BUILTIN_CLZ                   1
   return UNITEX_LIKELY(n) ? _bit_scan_reverse(static_cast<uint32_t>(n)) ^ 31 : 32;
#else
#  define UNITEX_HAS_BUILTIN_CLZ                   0
//  Bit Twiddling Hacks
//  @see https://graphics.stanford.edu/~seander/bithacks.html
   static const int8_t LogTable256[256] =  {
#  define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
     0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
     LT(5), LT(6), LT(6), LT(7), LT(7), LT(7), LT(7),
     LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8)
#  undef LT
   };
   int8_t count = 0;
   uint32_t t = 0;
   if      ((t = n >> UINT32_C(24)))  count =  8 - LogTable256[t];
   else if ((t = n >> UINT32_C(16)))  count = 16 - LogTable256[t];
   else if ((t = n >> UINT32_C(8)))   count = 24 - LogTable256[t];
   else                               count = 32 - LogTable256[n];
   return static_cast<uint32_t>(count);
#endif
}

/**
 * @def    unitex_builtin_clz_64
 * @brief  Count the number of leading 0-bits on a 64-bits data type
 */
#if UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#pragma intrinsic(_BitScanReverse64)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
UNITEX_FORCE_INLINE
uint32_t unitex_builtin_clz_64(uint64_t n) {
// GNU C/C++ compiler
// LLVM native C/C++ compiler
#if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
    UNITEX_COMPILER_AT_LEAST(CLANG,1,5)
#  define UNITEX_HAS_BUILTIN_CLZ                   1
   return UNITEX_LIKELY(n) ? __builtin_clzll(n) :  64;
// Visual Studio 2005 and later
#elif UNITEX_COMPILER_AT_LEAST(MSVC,14,0) &&\
      (UNITEX_CPU_IS(X64) || UNITEX_CPU_IS(X86))
# define UNITEX_HAS_BUILTIN_CLZ                    1
   uint32_t count = 64;
   if (UNITEX_LIKELY(n)) {
#  if UNITEX_CPU_IS(X64)
      _BitScanReverse64((DWORD *)&count, n);
      count ^= 63;
#  else
     if (_BitScanReverse((DWORD*)&count, static_cast<uint32_t>(n >> 32))) {
       count += 32;
       count ^= 63;
     }
     else if (_BitScanReverse((DWORD*)&count, static_cast<uint32_t>(n))) {
       count ^= 63;
     }
#  endif
  }
  return count;
// Intel Compiler
#elif UNITEX_COMPILER_IS(INTEL) &&\
    (UNITEX_WORDSIZE_IS(32) || UNITEX_WORDSIZE_IS(64))
# define UNITEX_HAS_BUILTIN_CLZ                    1
 uint32_t count = 64;
 if (UNITEX_LIKELY(n)) {
#  if UNITEX_WORDSIZE_IS(64)
   _BitScanReverse64(&count, n);
   count ^= 63;
#  else
   if (_bit_scan_reverse(&count, static_cast<uint32_t>(n >> 32))) {
     count += 32;
     count ^= 63;
   } else if (_bit_scan_reverse(&count, static_cast<uint32_t>(n))) {
     count ^= 63;
   }
#  endif
}
   return count;
#else
#  define UNITEX_HAS_BUILTIN_CLZ                   0
//  Bit Twiddling Hacks
//  @see https://graphics.stanford.edu/~seander/bithacks.html
   static const int8_t LogTable256[256] =  {
#  define LT(n) n, n, n, n, n, n, n, n, n, n, n, n, n, n, n, n
     0, 1, 2, 2, 3, 3, 3, 3, 4, 4, 4, 4, 4, 4, 4, 4,
     LT(5), LT(6), LT(6), LT(7), LT(7), LT(7), LT(7),
     LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8), LT(8)
#  undef LT
   };
   int8_t count = 0;
   uint64_t t = 0;
   if      ((t = n >> UINT64_C(56)))  count =  8 - LogTable256[t];
   else if ((t = n >> UINT64_C(48)))  count = 16 - LogTable256[t];
   else if ((t = n >> UINT64_C(40)))  count = 24 - LogTable256[t];
   else if ((t = n >> UINT64_C(32)))  count = 32 - LogTable256[t];
   else if ((t = n >> UINT64_C(24)))  count = 40 - LogTable256[t];
   else if ((t = n >> UINT64_C(16)))  count = 48 - LogTable256[t];
   else if ((t = n >> UINT64_C(8)))   count = 56 - LogTable256[t];
   else                               count = 64 - LogTable256[n];
   return static_cast<uint32_t>(count);
#endif
}
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSICS_CLZ_H_                   // NOLINT
