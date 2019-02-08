/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      types.h
 * @brief     Portable integer types using C++11 or pstdint.h as fallback.
 *            pstdint.h is the Paul Hsieh's cross platform stdint.h
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @see       http://en.cppreference.com/w/cpp/types/integer
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @see       pstdint.h
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 types.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_INTEGER_TYPES_H_                                // NOLINT
#define UNITEX_BASE_INTEGER_TYPES_H_                                // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/macro/macro.h"         // UNITEX_HAVE
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
#include "base/integer/wordsize.h"    // UNITEX_WORDSIZE_IS
#include "base/compiler/keywords.h"   // UNITEX_STATIC_ASSERT
#include "base/compiler/intrinsics.h" // __uint128_t, __m128
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <stdlib.h>                // size_t
/* ************************************************************************** */
// Portable integer types (for uint64_t, uint32_t types and literal macros)
#if   UNITEX_COMPILER_COMPLIANT(CXX11)    ||\
      UNITEX_COMPILER_AT_LEAST(MSVC,16,0)
#      include <cstdint>                   // C++11 standard integer types
#elif ((UNITEX_COMPILER_IS(GCC)           ||\
        UNITEX_COMPILER_IS(CLANG)         ||\
        UNITEX_HAVE(STDINT_H)))
// use stdint.h macros specified in the C99
// standard that aren't in the C++ standard
# ifndef __STDC_LIMIT_MACROS
#  define __STDC_LIMIT_MACROS
# endif  //  __STDC_LIMIT_MACROS
# ifndef __STDC_CONSTANT_MACROS
#  define __STDC_CONSTANT_MACROS
# endif  // __STDC_CONSTANT_MACROS
#      include <stdint.h>               // standard integer types
#else    // !UNITEX_COMPILER_IS(GCC)...
#      include "base/vendor/pstdint.h"  //  Paul Hsieh's portable stdint.h
#endif  // UNITEX_COMPILER_COMPLIANT(CXX11)
/* ************************************************************************** */
// Portable inttypes.h implementing features such as printf and scanf format
// specifiers. Note that the <inttypes.h> header shall include the <stdint.h>
#if    UNITEX_COMPILER_IS(MSVC)
#      include "base/vendor/inttypes.h"  // Format conversion of integer types
#else     // !UNITEX_COMPILER_IS(MSVC)
#      include <inttypes.h>              // Format conversion of integer types
#endif  // UNITEX_COMPILER_IS(MSVC)
/* ************************************************************************** */
#endif  // UNITEX_BASE_INTEGER_TYPES_H_                             // NOLINT
