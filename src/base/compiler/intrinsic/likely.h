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
 * @file      likely.h
 * @brief     Macros to indicate if a branch is likely or unlikely to be taken
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 likely.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_INTRINSICS_LIKELY_H_                 // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSICS_LIKELY_H_                 // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/version.h"            // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_LIKELY
 * @brief  `expr` is expected to be true
 */
/**
 * @def    UNITEX_UNLIKELY
 * @brief  `expr` is expected to be false
 */
#if     defined(likely) && defined(unlikely)
#define UNITEX_LIKELY(expr)   likely(expr)
#define UNITEX_UNLIKELY(expr) unlikely(expr)
#elif   UNITEX_COMPILER_AT_LEAST(CLANG,3,2) ||\
        UNITEX_COMPILER_AT_LEAST(INTEL,9,0) ||\
        UNITEX_COMPILER_AT_LEAST(GCC,4,0)
#define UNITEX_LIKELY(expr)   __builtin_expect(!!(expr), true)
#define UNITEX_UNLIKELY(expr) __builtin_expect(!!(expr), false)
#else
#define UNITEX_LIKELY(expr)   (expr)
#define UNITEX_UNLIKELY(expr) (expr)
#endif  // defined(likely)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSICS_LIKELY_H_                // NOLINT
