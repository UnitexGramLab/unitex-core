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
 * @file      wordsize.h
 * @brief     Macro to test target architecture (32- or 64–bit mode) at compile-time
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For more details about the stringification process in C
 *            visit http://gcc.gnu.org/onlinedocs/cpp/Stringification.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 wordsize.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_INTEGER_WORDSIZE_H_                             // NOLINT
#define UNITEX_BASE_INTEGER_WORDSIZE_H_                             // NOLINT
/* ************************************************************************** */
/**
 * @def    UNITEX_WORDSIZE
 * @return 64   if ints are 32 bit quantities and longs and pointers are 64 bit
 * @return 32   if ints, longs and pointers are 32-bit quantities
 * @see    UNITEX_FORCE_INLINE
 */
// _LP64              defined by the compiler (GCC)
// __LP64__           defined in (Mac) OS X
// _WIN64             defined by the compiler (MSVC)
// __WORDSIZE         defined in limits.h
// __SIZEOF_POINTER__ defined by the compiler (GCC)
#if defined(_LP64)     || defined(__LP64__) || defined(_WIN64) ||\
    (__WORDSIZE == 64) || (__SIZEOF_POINTER__ == 8)
/**
 * @def    UNITEX_WORDSIZE_IS_64
 * @brief  true if ints are 32 bit quantities and longs and pointers are 64 bit
 *         quantities
 * @see    UNITEX_NEVER_INLINE
 */
# define UNITEX_WORDSIZE_IS_64   1
# define UNITEX_WORDSIZE         64
#else  // WORDSIZE = 32
/**
 * @def    UNITEX_WORDSIZE_IS_32
 * @brief  true if ints, longs and pointers are 32-bit quantities
 * @see    UNITEX_FORCE_INLINE
 */
# define UNITEX_WORDSIZE_IS_32   1
# define UNITEX_WORDSIZE         32
#endif  // defined(_LP64) || defined(_WIN64) || (__WORDSIZE == 64)
/* ************************************************************************** */
/**
 * @brief  Test how many bits per long will be used by the target system
 *
 * @code{.cpp}
 *  UNITEX_WORDSIZE_IS(32)   // ints: 32, longs and pointers: 64 bit quantities
 *  UNITEX_WORDSIZE_IS(64)   // ints, longs and pointers: 32-bit quantities
 * @endcode
 */
#define UNITEX_WORDSIZE_IS(Size)              \
        (defined(UNITEX_WORDSIZE_IS_##Size) &&\
                 UNITEX_WORDSIZE_IS_##Size)
/* ************************************************************************** */
/**
 * @brief  Test if the target system wordsize is at least of n bits
 *
 * @code{.cpp}
 *  UNITEX_WORDSIZE_IS_AT_LEAST(32)   // word size at least 32 bits
 *  UNITEX_WORDSIZE_IS_AT_LEAST(64)   // word size at least 64 bits
 * @endcode
 */
#define UNITEX_WORDSIZE_IS_AT_LEAST(n)        \
        (defined(UNITEX_WORDSIZE)           &&\
                 UNITEX_WORDSIZE >= n)
/* ************************************************************************** */
#endif  // UNITEX_BASE_INTEGER_WORDSIZE_H_                          // NOLINT
