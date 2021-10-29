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
 * @file      ctz.h
 * @brief     Count trailing zeros (ctz) routine
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 ctz.h`
 *
 * @date      October 2021
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_INTRINSICS_CTZ_H_                      // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSICS_CTZ_H_                      // NOLINT
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
 * @def    unitex_builtin_ctz_16
 * @brief  Count the number of trailing 0-bits on a 16-bits data type
 */
#if UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#pragma intrinsic(_BitScanForward)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
UNITEX_FORCE_INLINE
uint32_t unitex_builtin_ctz_16(uint16_t n) {
// GNU C/C++ compiler
// LLVM native C/C++ compiler
#if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
    UNITEX_COMPILER_AT_LEAST(CLANG,1,5)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   return UNITEX_LIKELY(n) ?
          __builtin_ctz(uint32_t(n)) :             16;
// Visual Studio 2005 and later
#elif UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   uint16_t count = 16;
   if (UNITEX_LIKELY(n)) {
     _BitScanForward((DWORD*)&count, n);
   }
   return count;
// Intel Compiler
#elif UNITEX_COMPILER_IS(INTEL)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   return UNITEX_LIKELY(n) ? _bit_scan_forward(static_cast<uint16_t>(n)) : 16;
#else
#  define UNITEX_HAS_BUILTIN_CTZ                   0
//  De Bruijn Sequence Generator
//  @see https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
   static const int8_t DeBruijnBitTable16[16] = {
      0,  1,  2, 12, 10,  3, 13,  5, 
     15, 11,  9,  4, 14,  8,  7,  6
   };
   return UNITEX_LIKELY(n) ? static_cast<uint16_t>(
           DeBruijnBitTable16[((uint16_t)(
           (n & -n) * UINT16_C(0x0BD3))) >> 12]) : 16;
#endif
}

/**
 * @def    unitex_builtin_ctz_32
 * @brief  Count the number of trailing 0-bits on a 32-bits data type
 */
#if UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#pragma intrinsic(_BitScanForward)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
UNITEX_FORCE_INLINE
uint32_t unitex_builtin_ctz_32(uint32_t n) {
// GNU C/C++ compiler
// LLVM native C/C++ compiler
#if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
    UNITEX_COMPILER_AT_LEAST(CLANG,1,5)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   return UNITEX_LIKELY(n) ? __builtin_ctz(n) :    32;
// Visual Studio 2005 and later
#elif UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   uint32_t count = 32;
   if (UNITEX_LIKELY(n)) {
     _BitScanForward((DWORD*)&count, n);
   }
   return count;
// Intel Compiler
#elif UNITEX_COMPILER_IS(INTEL)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   return UNITEX_LIKELY(n) ? _bit_scan_forward(static_cast<uint32_t>(n)) : 32;
#else
#  define UNITEX_HAS_BUILTIN_CTZ                   0
//  De Bruijn Sequence Generator
//  @see https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
   static const int8_t DeBruijnBitTable32[32] = {
      0,  1,  2, 16, 24,  3, 17, 27, 
     14, 25, 12,  4, 21, 18, 28,  6, 
     31, 15, 23, 26, 13, 11, 20,  5, 
     30, 22, 10, 19, 29,  9,  8,  7
   };
   return UNITEX_LIKELY(n) ? static_cast<uint32_t>(
           DeBruijnBitTable32[((uint32_t)(
           (n & -n) * UINT32_C(0x05F51B27))) >> 27]) : 32;
#endif
}

/**
 * @def    unitex_builtin_ctz_64
 * @brief  Count the number of trailing 0-bits on a 64-bits data type
 */
#if UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
#pragma intrinsic(_BitScanForward64)
#endif  // UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
UNITEX_FORCE_INLINE
uint32_t unitex_builtin_ctz_64(uint64_t n) {
// GNU C/C++ compiler
// LLVM native C/C++ compiler
#if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
    UNITEX_COMPILER_AT_LEAST(CLANG,1,5)
#  define UNITEX_HAS_BUILTIN_CTZ                   1
   return UNITEX_LIKELY(n) ? __builtin_ctzll(n) :  64;
// Visual Studio 2005 and later
#elif UNITEX_COMPILER_AT_LEAST(MSVC,14,0) &&\
      (UNITEX_CPU_IS(X64) || UNITEX_CPU_IS(X86))
# define UNITEX_HAS_BUILTIN_CTZ                    1
   uint32_t count = 64;
   if (UNITEX_LIKELY(n)) {
#  if UNITEX_CPU_IS(X64)
      _BitScanForward64((DWORD*)&count, n);
#  else
     if (_BitScanForward((DWORD*)&count, static_cast<uint32_t>(n >> 32))) {
       count += 32;
     }
     else {
       _BitScanForward((DWORD*)&count, static_cast<uint32_t>(n))
     }
#  endif
  }
  return count;
// Intel Compiler
#elif UNITEX_COMPILER_IS(INTEL) &&\
    (UNITEX_WORDSIZE_IS(32) || UNITEX_WORDSIZE_IS(64))
# define UNITEX_HAS_BUILTIN_CTZ                    1
 uint32_t count = 64;
 if (UNITEX_LIKELY(n)) {
#  if UNITEX_WORDSIZE_IS(64)
   _BitScanForward64(&count, n);
#  else
   if (_bit_scan_forward(&count, static_cast<uint32_t>(n >> 32))) {
     count += 32;
   } else {
     _bit_scan_forward(&count, static_cast<uint32_t>(n))
   }
#  endif
}
   return count;
#else
#  define UNITEX_HAS_BUILTIN_CTZ                   0
//  De Bruijn Sequence Generator
//  @see https://www.chessprogramming.org/De_Bruijn_Sequence_Generator
   static const int8_t DeBruijnBitTable64[64] = {
      0,  1,  2,  7,  3, 13,  8, 19,
      4, 49, 14, 35,  9, 43, 20, 52,
      5, 17, 47, 50, 15, 25, 27, 36,
     10, 32, 44, 29, 60, 21, 38, 53,
     63,  6, 12, 18, 48, 34, 42, 51,
     16, 46, 24, 26, 31, 28, 59, 37,
     62, 11, 33, 41, 45, 23, 30, 58,
     61, 40, 22, 57, 39, 56, 55, 54
   };
   return UNITEX_LIKELY(n) ? static_cast<uint64_t>(
           DeBruijnBitTable64[((uint64_t)(
           (n & -n) * UINT64_C(0x0218A3AB65E693F7))) >> 58]) : 64;
#endif
}
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSICS_CTZ_H_                   // NOLINT
