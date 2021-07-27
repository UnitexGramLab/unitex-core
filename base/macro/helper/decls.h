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
 * @file      decls.h
 * @brief     Preprocessor macro to write a multi-statement macro
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 macro_decls.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_MACRO_HELPER_DECLS_H_                           // NOLINT
#define UNITEX_BASE_MACRO_HELPER_DECLS_H_                           // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @brief  Helper macro to write a multi-statement macro
 *
 * @see    UNITEX_DO_NOTHING
 * @see    http://c-faq.com/cpp/multistmt.html
 *
 * @code{.cpp}
 * UNITEX_MACRO_DECLS_BEGIN
 * // multi-statement macro
 * UNITEX_MACRO_DECLS_END
 * @endcode
 */
// This trick was adapted from the C FAQ List
// Don't worry most C++ compilers will optimize away our do/while loop
// UNITEX_MACRO_DECLS_BEGIN
#define UNITEX_MACRO_DECLS_BEGIN do {
// Visual Studio 2008 (MSVC++ 9.0) and later
#if UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
// warning C4127: conditional expression is constant
# define UNITEX_MACRO_DECLS_END           \
__pragma(warning(push))                   \
__pragma(warning(disable:4127))           \
  } while (0)                             \
__pragma(warning(pop))
#else
// UNITEX_MACRO_DECLS_END
# define UNITEX_MACRO_DECLS_END          \
  } while (0)
#endif  //  UNITEX_COMPILER_AT_LEAST(MSVC,15,0)
/* ************************************************************************** */
#endif  // UNITEX_BASE_MACRO_HELPER_DECLS_H_                        // NOLINT
