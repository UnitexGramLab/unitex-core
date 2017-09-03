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
 * @file      types.h
 * @brief     Portable integer types using C++11 or pstdint.h as fallback.
 *            pstdint.h is the Paul Hsieh's cross platform stdint.h
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @see       http://en.cppreference.com/w/cpp/types/integer
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @see       pstdint.h
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 integer_types.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_INTEGER_LITERALS_H_                             // NOLINT
#define UNITEX_BASE_INTEGER_LITERALS_H_                             // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/integer/types.h"    // portable integer types
/* ************************************************************************** */
// C99-compatible 64 bits integer literals
// Acts as a fallback if they are not already defined in stdint.h
// i.e. when __STDC_LIMIT_MACROS or __STDC_CONSTANT_MACROS aren't explicitly
// requested
#if UNITEX_WORDSIZE_IS(64)
# ifndef INT64_C
#  define INT64_C(c) c  ## L
# endif  // INT64_C
# ifndef UINT64_C
#  define UINT64_C(c) c ## UL
# endif  // UINT64_C
#else  // UNITEX_WORDSIZE_IS(32)
# ifndef INT64_C
#  define INT64_C(c) c  ## LL
# endif  // INT64_C
# ifndef UINT64_C
#  define UINT64_C(c) c ## ULL
# endif  // UINT64_C
#endif  // UNITEX_WORDSIZE_IS(64)
/* ************************************************************************** */
#endif  // UNITEX_BASE_INTEGER_LITERALS_H_                          // NOLINT
