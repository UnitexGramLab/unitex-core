/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      printf_format.h
 * @brief     Get compile-time type-checked warnings in functions that
 *            takes printf style arguments
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 cpu.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_PRINTF_FORMAT_H_            // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_PRINTF_FORMAT_H_            // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_PRINTF_LIKE_FORMAT_CHECK
 * @brief  Get compile-time type-checked warnings in functions that
 *         takes printf style arguments
 * @see    http://gcc.gnu.org/onlinedocs/gcc/Function-Attributes.html
 * @note   Specific to GCC >= 2.4, Clang >= 1.0, MinGW/GCC >= 2.4
 *
 * @param  m      The number of the "format string" parameter
 * @param  n      The number of the first variadic parameter
 */
// TODO(martinec) What needs to be done to work with unitex' u_printf ?
#if  UNITEX_COMPILER_AT_LEAST(GCC,2,4)   ||\
     UNITEX_COMPILER_AT_LEAST(CLANG,1,0)
# if  UNITEX_COMPILER_IS_NOT(MINGW)
# define UNITEX_PRINTF_LIKE_FORMAT_CHECK(m,n) \
         __attribute__ ((format(__printf__, m, n)))
# else    // UNITEX_COMPILER_IS_(MINGW)
# define UNITEX_PRINTF_LIKE_FORMAT_CHECK(m,n) \
         __attribute__ ((format(ms_printf, m, n)))
# endif  // UNITEX_COMPILER_IS_NOT(MINGW)
#else   // !UNITEX_COMPILER_AT_LEAST(GCC,2,4)
#define UNITEX_PRINTF_LIKE_FORMAT_CHECK(m,n) /* nothing */
#endif  // UNITEX_COMPILER_AT_LEAST_GCC(2,4,0)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_PRINTF_FORMAT_H_          // NOLINT
