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
 * @file      is.h
 * @brief     Conditional macros
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For a full preprocessor library, use instead the Boost Preprocessor
 *            standalone library. A basic introduction is available from:
 *            http://www.boost.org/doc/libs/1_57_0/libs/preprocessor/doc/index.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 is.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_IS_H_                              // NOLINT
#define UNITEX_BASE_PREPROCESSOR_IS_H_                              // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/util.h"
#include "base/preprocessor/paste.h"
#include "base/preprocessor/logic_unary.h"
#include "base/preprocessor/logic_binary.h"
#include "base/preprocessor/control.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_IS_PAREN
 * @brief  x
 *
 * @code{.cpp}
 *         UNITEX_PP_IS_PAREN(())  // Expands to 1
 *         UNITEX_PP_IS_PAREN(FOO) // Expands to 0
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_IS_PAREN(x)            UNITEX_PP_CHECK(UNITEX_PP_IS_PAREN_PROBE x)
#define UNITEX_PP_IS_PAREN_PROBE(...)    UNITEX_PROBE(~)

/**
 * @def    UNITEX_PP_IS_EMPTY
 *
 * @code{.cpp}
 *         UNITEX_PP_IS_EMPTY(UNITEX_PP_EMPTY())  // Expands to 1
 *         UNITEX_PP_IS_EMPTY(FOO)                // Expands to 0
 * @endcode
 */
#define UNITEX_PP_IS_EMPTY(x)                        UNITEX_PP_IS_EMPTY_(x UNITEX_PP_IS_EMPTY_PROBE)
#define UNITEX_PP_IS_EMPTY_(l)                       UNITEX_PP_CHECK(UNITEX_PP_TOKEN_CAT_(UNITEX_PP_IS_EMPTY_, l()))
#define UNITEX_PP_IS_EMPTY_UNITEX_PP_IS_EMPTY_PROBE  ~, 1 UNITEX_PP_EAT
#define UNITEX_PP_IS_EMPTY_PROBE(...)                ~, 0

/**
 * @def    UNITEX_PP_IS_0
 * @brief  Alias for `UNITEX_PP_NOT`
 *
 * @code{.cpp}
 *         UNITEX_PP_IS_0(FOO)  // Expands to 0
 *         UNITEX_PP_IS_0(0)    // Expands to 1
 * @endcode
 */
#define UNITEX_PP_IS_0(x)       UNITEX_PP_NOT(x)

/**
 * @def    UNITEX_PP_IS_1
 * @brief  Alias for `UNITEX_PP_NOT`
 *
 * @code{.cpp}
 *         UNITEX_PP_IS_1(FOO)   // Expands to 0
 *         UNITEX_PP_IS_1(0)     // Expands to 0
 *         UNITEX_PP_IS_1(1)     // Expands to 1
 * @endcode
 */
#define UNITEX_PP_IS_1(x)       UNITEX_PP_NOT(UNITEX_PP_COMPL(x))

/**
 * @def    UNITEX_PP_IS_BIT
 * @brief  Test if 1 or 0
 *
 * @code{.cpp}
 *         UNITEX_PP_IS_BIT(FOO)  // Expands to 0, FOO is undefined
 *         UNITEX_PP_IS_BIT(0)    // Expands to 1
 *         UNITEX_PP_IS_BIT(1)    // Expands to 1
 * @endcode
 */
#define UNITEX_PP_IS_BIT(x)     UNITEX_PP_OR(UNITEX_PP_IS_1(x),UNITEX_PP_IS_0(x))
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_IS_H_                           // NOLINT
