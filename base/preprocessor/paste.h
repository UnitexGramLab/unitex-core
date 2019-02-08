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
 * @file      paste.h
 * @brief     Token- and String- pasting macros
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      For more details about the Token- and String- pasting process
 *            visit http://isocpp.org/wiki/faq/misc-technical-issues#macros-with-token-pasting
 *
 * @note      For a full preprocessor library, use instead the Boost Preprocessor
 *            standalone library. A basic introduction is available from:
 *            http://www.boost.org/doc/libs/1_57_0/libs/preprocessor/doc/index.html
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 paste.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_PASTE_H_                           // NOLINT
#define UNITEX_BASE_PREPROCESSOR_PASTE_H_                           // NOLINT
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_TOKEN_PASTE
 * @brief  Token-Pasting macro
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PP_TOKEN_PASTE(__TI,ME__)
 *         // After be expanded FOO will be `__TIME__`
 * @endcode
 */
#define UNITEX_PP_TOKEN_PASTE(a,b)        UNITEX_PP_TOKEN_PASTE_(a, b)
#define UNITEX_PP_TOKEN_PASTE_(a,b)       a##b

/**
 * @def    UNITEX_PP_STRING_CONCAT
 * @brief  String-Pasting macro
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PP_STRING_CONCAT(UNI,TEX)
 *         #define BAR UNITEX_PP_STRINGIFY_VALUE(FOO)
 *         // BAR will be the string "UNITEX"
 * @endcode
 */
#define UNITEX_PP_STRING_EXPAND(a)        a
#define UNITEX_PP_STRING_CONCAT(a,b)      UNITEX_PP_STRING_EXPAND(a)b

/**
 * @def    UNITEX_PP_TOKEN_CONCAT
 * @brief  Token-Concatenation macro without inhibition
 *
 * e.g.
 * @code{.cpp}
 *         #define FOO UNITEX_PP_TOKEN_CAT(__TI,ME__)
 *         // Expanded FOO will be the value of `__TIME__`
 * @endcode
 *
 * @see    UNITEX_PP_TOKEN_PASTE
 */
#define UNITEX_PP_TOKEN_CAT(a, ...)       UNITEX_PP_TOKEN_CAT_(a, __VA_ARGS__)
#define UNITEX_PP_TOKEN_CAT_(a, ...)      a ## __VA_ARGS__
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_PASTE_H_                        // NOLINT
