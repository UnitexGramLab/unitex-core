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
 * @file      logic_unary.h
 * @brief     Preprocessor macros helpers
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For a full preprocessor library, use instead the Boost Preprocessor
 *            standalone library. A basic introduction is available from:
 *            http://www.boost.org/doc/libs/1_57_0/libs/preprocessor/doc/index.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 logic_unary.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_LOGIC_UNARY_H_                     // NOLINT
#define UNITEX_BASE_PREPROCESSOR_LOGIC_UNARY_H_                     // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/util.h"
#include "base/preprocessor/paste.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_NOT
 * @brief  0=>1, ?=>0
 *
 * @code{.cpp}
 *         UNITEX_PP_NOT(FOO)   // Expands to 0
 *         UNITEX_PP_NOT(0)     // Expands to 1
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_NOT(x)        UNITEX_PP_CHECK(UNITEX_PP_TOKEN_CAT_(UNITEX_PP_NOT_, x))
#define UNITEX_PP_NOT_0         ~, 1,

/**
 * @def    UNITEX_PP_COMPL
 * @brief  0=>1, 0=>1
 *
 * @code{.cpp}
 *         UNITEX_PP_COMPL(1)   // Expands to 0
 *         UNITEX_PP_COMPL(0)   // Expands to 1
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_COMPL(b)      UNITEX_PP_TOKEN_CAT_(UNITEX_PP_COMPL_, b)
#define UNITEX_PP_COMPL_0       1
#define UNITEX_PP_COMPL_1       0

/**
 * @def    UNITEX_PP_BOOL
 * @brief  x
 *
 * @code{.cpp}
 *         UNITEX_PP_BOOL(FOO)  // Expands to 1, even if FOO is undefined
 *         UNITEX_PP_BOOL(1)    // Expands to 1
 *         UNITEX_PP_BOOL(0)    // Expands to 0
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_BOOL(x)       UNITEX_PP_COMPL(UNITEX_PP_NOT(x))
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_LOGIC_UNARY_H_                  // NOLINT
