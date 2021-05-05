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
 * @file      static_assert.h
 * @brief     Macro to performs compile-time assertion checking
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 static_assert.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_STATIC_ASSERT_H_                                // NOLINT
#define UNITEX_BASE_STATIC_ASSERT_H_                                // NOLINT
/* ************************************************************************** */
// Unitex Macros
#include "base/compiler/compliance.h"     // COMPILER_COMPLIANT
#include "base/compiler/features.h"       // UNITEX_HAS_FEATURE
#include "base/compiler/version.h"        // COMPILER_AT_LEAST_*, COMPILER_IS
#include "base/compiler/identifiers.h"    // UNITEX_COUNTER, UNITEX_FILE_LINE
#include "base/macro/macro.h"       // UNITEX_HAS
#include "base/preprocessor/stringify.h"  // UNITEX_PP_STRINGIFY_NAME
/* ************************************************************************** */
// C++ system files                (try to order the includes alphabetically)
#include <cassert>                 // ISOC11 static_assert definition
/* ************************************************************************** */
/**
 * @def      UNITEX_STATIC_ASSERT
 * @brief    Performs compile-time assertion checking
 */
// C++11                      static_assert
// C11                       _Static_assert
// MSVC   >= 1600             static_assert
// CLANG  cxx_static_assert   static_assert
// *                          custom static_assert
#if UNITEX_HAS_FEATURE(cxx_static_asserts)                                ||\
    UNITEX_COMPILER_COMPLIANT(CXX11)
// Since C+11 When # appears before __VA_ARGS__, the entire expanded
// __VA_ARGS__ is enclosed in quotes
# define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                          \
         static_assert(bool_constexpr,   UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__))
#elif UNITEX_COMPILER_COMPLIANT(C11)
# define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                          \
         _Static_assert(bool_constexpr,  UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__))
#elif UNITEX_COMPILER_AT_LEAST_MSVC(16,0)
# define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                          \
         static_assert(bool_constexpr,   UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__))
#elif UNITEX_COMPILER_IS(CLANG)
# if __has_extension(cxx_static_assert)
#  define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                         \
          static_assert(bool_constexpr,  UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__))
# elif __has_extension(c_static_assert)
#  define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                         \
          _Static_assert(bool_constexpr, UNITEX_PP_STRINGIFY_NAME(__VA_ARGS__))
# endif   // __has_extension(cxx_static_assert)
#else    // !UNITEX_COMPILER_COMPLIANT(CXX11)
/// the code below is the static_assert update 10.0 by Pádraig Brady
/// @see http://www.pixelbeat.org/programming/gcc/static_assert.html
#define UNITEX_ASSERT_CONCAT_(a, b) a##b
#define UNITEX_ASSERT_CONCAT(a, b)  UNITEX_ASSERT_CONCAT_(a, b)
// These can't be used after statements in C89
#if UNITEX_HAS(MACRO_COUNTER)
  #define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                         \
    ;enum { UNITEX_ASSERT_CONCAT(static_assert_, UNITEX_COUNTER) = 1/(!!(bool_constexpr)) }
#else  // !defined(__COUNTER__)
   // This can't be used twice on the same line so ensure if using in headers
   // that the headers are not included twice (by wrapping in #ifndef...#endif)
   // Note it doesn't cause an issue when used on same line of separate modules
   // compiled with gcc -combine -fwhole-program
  #define UNITEX_STATIC_ASSERT(bool_constexpr, ...)                         \
    ;enum { UNITEX_ASSERT_CONCAT(assert_line_, UNITEX_FILE_LINE) = 1/(!!(bool_constexpr)) }
#endif  //  __COUNTER__
#endif  // UNITEX_COMPILER_COMPLIANT(CXX11)
/* ************************************************************************** */
#endif  // UNITEX_BASE_STATIC_ASSERT_H_                             // NOLINT
