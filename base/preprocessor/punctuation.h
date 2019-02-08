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
 * @file      punctuation.h
 * @brief     Punctuation helpers
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
 *            `cpplint.py --linelength=120 punctuation.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_PUNCTUATION_H_                     // NOLINT
#define UNITEX_BASE_PREPROCESSOR_PUNCTUATION_H_                     // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/util.h"
#include "base/preprocessor/control.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_COMMA
 */
#define UNITEX_PP_COMMA()       ,


/**
 * @def    UNITEX_PP_LPAREN
 */
#define UNITEX_PP_LPAREN()      (


/**
 * @def    UNITEX_PP_RPAREN
 */
#define UNITEX_PP_RPAREN()      )

/**
 * @def    UNITEX_PERIOD
 */
#define UNITEX_PP_PERIOD()      .

/**
 * @def    UNITEX_PP_COMMA_IF
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_COMMA_IF(n)   UNITEX_PP_IF(n)(UNITEX_PP_COMMA, UNITEX_PP_EAT)()
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_PUNCTUATION_H_                  // NOLINT
