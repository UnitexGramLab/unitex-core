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
 * @brief     Binary hamming weight (popcount) routine
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
#ifndef UNITEX_BASE_COMPILER_INTRINSICS_POPCOUNT_H_                 // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSICS_POPCOUNT_H_                 // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/version.h"            // UNITEX_COMPILER_AT_LEAST
#include "base/cpu/version.h"                 // UNITEX_CPU_IS
#include "base/integer/types.h"               // uint32_t, uint64_t
/* ************************************************************************** */
// Some macros to use compiler's builtin popcount function
// POPCNT on x86 compiles in a single instruction
// use g++/mingw -mpopcnt or clang++ -march=corei7 to compile
// TODO(martinec) add more architectures
#if UNITEX_CPU_IS(X86) || UNITEX_CPU_IS(X64)
/**
 * @def    unitex_builtin_popcount_8
 * @brief  Count the number of bits set to one in a 1-byte data type
 */
/**
 * @def    unitex_builtin_popcount_16
 * @brief  Count the number of bits set to one in a 2-byte data type
 */
/**
 * @def    unitex_builtin_popcount_32
 * @brief  Count the number of bits set to one in a 4-byte data type
 */
/**
 * @def    unitex_builtin_popcount_64
 * @brief  Count the number of bits set to one in a 8-byte data type
 */
# if UNITEX_COMPILER_AT_LEAST(GCC,3,4)                                  ||\
     UNITEX_COMPILER_AT_LEAST(CLANG,1,7)
#  define unitex_builtin_popcount_8(n)                                  \
      __builtin_popcount(static_cast<uint32_t>(n & 0xFF))
#  define unitex_builtin_popcount_16(n)                                 \
      __builtin_popcount(static_cast<uint32_t>(n & 0xFFFF))
#  define unitex_builtin_popcount_32(n)                                 \
      __builtin_popcount(static_cast<uint32_t>(n))
#  if   UNITEX_WORDSIZE_IS(32)
#       define unitex_builtin_popcount_64(n)                            \
            unitex_builtin_popcount_32(n)       +                       \
            unitex_builtin_popcount_32(n >> 32)
#  elif UNITEX_WORDSIZE_IS(64)
#       define unitex_builtin_popcount_64(n)                            \
            __builtin_popcountll(static_cast<uint64_t>(n))
#  endif  // UNITEX_WORDSIZE_IS(32)
#  define UNITEX_HAS_BUILTIN_POPCOUNT                 1
#  define UNITEX_COMPILER_BUILTIN_POPCOUNT            64
// Visual Studio 2008 and later
# elif UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
#  define unitex_builtin_popcount_8(n)                                  \
          __popcnt16(static_cast<__uint16>(n & 0xFF))
#  define unitex_builtin_popcount_16(n)                                 \
          __popcnt16(static_cast<__uint16>(n))
#  define unitex_builtin_popcount_32(n)                                 \
          __popcnt  (static_cast<__uint32>(n))
#  if   UNITEX_WORDSIZE_IS(32)
#       define unitex_builtin_popcount_64(n)                            \
               unitex_builtin_popcount_32(n) +                          \
               unitex_builtin_popcount_32(n >> 32)
#  elif UNITEX_WORDSIZE_IS(64)
#       define unitex_builtin_popcount_64(n)                            \
               __popcnt64(static_cast<__uint64>(n))
#  endif  // UNITEX_WORDSIZE_IS(32)
#  define UNITEX_HAS_BUILTIN_POPCOUNT                 1
#  define UNITEX_COMPILER_BUILTIN_POPCOUNT            64
# endif   // UNITEX_COMPILER_AT_LEAST_GCC(3,4)
#else  // No built-in popcount support
# define UNITEX_COMPILER_BUILTIN_POPCOUNT             0
#endif //UNITEX_CPU_IS(X86) || UNITEX_CPU_IS(X64)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSICS_POPCOUNT_H_              // NOLINT
