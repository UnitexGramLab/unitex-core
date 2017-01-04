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
 * @file      inline.h
 * @brief     Tells the compiler that a method must be or not inlined
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler_attributes.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 inline.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_INLINE_H_                    // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_INLINE_H_                    // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/debug/build_mode.h"    // UNITEX_BUILD_MODE(DEBUG)
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_FORCE_INLINE
 * @brief  Tells the compiler that a method must be inlined
 * @see    UNITEX_NEVER_INLINE
 */
/**
 * @def    UNITEX_NEVER_INLINE
 * @brief  Tells the compiler that a method must not be inlined
 * @see    UNITEX_FORCE_INLINE
 */
#if UNITEX_BUILD_MODE(RELEASE)
# if UNITEX_COMPILER_AT_LEAST(MSVC,13,10) ||\
     UNITEX_COMPILER_AT_LEAST(INTEL,9,0)
#    define UNITEX_FORCE_INLINE        __forceinline
#    define UNITEX_NEVER_INLINE        __declspec(noinline)
# elif UNITEX_COMPILER_AT_LEAST(GCC,3,2)  ||\
       UNITEX_COMPILER_AT_LEAST(CLANG,2,1)
#    define UNITEX_FORCE_INLINE inline __attribute__((always_inline))
#    define UNITEX_NEVER_INLINE        __attribute__((noinline))
# else  // !MSC_VER(>=1310) && !GNUC(>=25)
#    define UNITEX_FORCE_INLINE inline
#    define UNITEX_NEVER_INLINE /* nothing */
# endif    // MSC_VER(>=1310) && !GNUC(>=25)
#else    // UNITEX_BUILD_MODE(DEBUG)
# define UNITEX_FORCE_INLINE inline
# define UNITEX_NEVER_INLINE /* nothing */
#endif  // !UNITEX_BUILD_MODE(DEBUG)
/* ************************************************************************** */
/**
 * @def    UNITEX_EXTERN_INLINE
 * @brief  Tells the compiler that a method is extern and must be inlined
 * @see    UNITEX_FORCE_INLINE
 */
# if UNITEX_COMPILER_IS(GCC) && !defined(__GNUC_STDC_INLINE__)
#  define UNITEX_EXTERN_INLINE extern UNITEX_FORCE_INLINE
# else
#  define UNITEX_EXTERN_INLINE UNITEX_FORCE_INLINE
# endif
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_INLINE_H_                 // NOLINT
