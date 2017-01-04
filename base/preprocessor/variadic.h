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
 * @file      variadic.h
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
 *            `cpplint.py --linelength=120 variadic.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_VARIADIC_H_                        // NOLINT
#define UNITEX_BASE_PREPROCESSOR_VARIADIC_H_                        // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/util.h"
#include "base/preprocessor/paste.h"
#include "base/preprocessor/control.h"
#include "base/preprocessor/logic_unary.h"
#include "base/preprocessor/is.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_HAS_ARGS
 * @brief  Return 1 if the number of arguments is > 0
 *
 * @code{.cpp}
 *         #define FOO UNITEX_PP_HAS_ARGS()     // Expands to 0
 *         #define BAR UNITEX_PP_HAS_ARGS(BAZ)  // Expands to 1
 * @endcode
 */
#define UNITEX_PP_HAS_ARGS(...)      UNITEX_PP_COMPL(UNITEX_PP_HAS_ARGS_(__VA_ARGS__))
#define UNITEX_PP_HAS_ARGS_(x, ...)  UNITEX_PP_CHECK(UNITEX_PP_TOKEN_CAT_(UNITEX_PP_HAS_ARGS__, x))
#define UNITEX_PP_HAS_ARGS__         ~, 1


/**
 * @def    UNITEX_PP_VA_NARGS
 * @brief  Return the number of arguments contained in __VA_ARGS__ before its
 *         expansion,
 *
 * @code{.cpp}
 *         #define FOO UNITEX_PP_VA_NARGS()     // Expands to 0
 *         #define FOO UNITEX_PP_VA_NARGS(X)    // Expands to 1
 *         #define BAR UNITEX_PP_VA_NARGS(X,Y)  // Expands to 2
 * @endcode
 *
 * @note   We added the `UNITEX_PP_HAS_ARGS` to test if no arguments were passed
 *
 * @author Laurent Deniau
 * @see    https://groups.google.com/d/msg/comp.std.c/d-6Mj5Lko_s/5R6bMWTEbzQJ
 */
#define UNITEX_PP_VA_NARGS(...)                                              \
        UNITEX_PP_TEST(UNITEX_PP_HAS_ARGS(__VA_ARGS__))                      \
       (UNITEX_PP_VA_NARGS_(__VA_ARGS__,UNITEX_PP_VA_NARGS_RSEQ()),0)
#define UNITEX_PP_VA_NARGS_(...) UNITEX_PP_VA_NARGS_N(__VA_ARGS__)
#define UNITEX_PP_VA_NARGS_N(                                                \
     _1, _2, _3, _4, _5, _6, _7, _8, _9,_10,_11,_12,_13,_14,_15,_16,         \
    _17,_18,_19,_20,_21,_22,_23,_24,_25,_26,_27,_28,_29,_30,_31,_32,         \
    _33,_34,_35,_36,_37,_38,_39,_40,_41,_42,_43,_44,_45,_46,_47,_48,         \
    _49,_50,_51,_52,_53,_54,_55,_56,_57,_58,_59,_60,_61,_62,_63,_64, N, ...) N
#define UNITEX_PP_VA_NARGS_RSEQ()                                            \
     64, 63, 62, 61, 60, 59, 58, 57, 56, 55, 54, 53, 52, 51, 50, 49,         \
     48, 47, 46, 45, 44, 43, 42, 41, 40, 39, 38, 37, 36, 35, 34, 33,         \
     32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22, 21, 20, 19, 18, 17,         \
     16, 15, 14, 13, 12, 11, 10,  9,  8,  7,  6,  5,  4,  3,  2,  1, 0

/**
 * @def    UNITEX_PP_VA_ARGS_CAT
 * @brief  Concatenate a list of arguments,
 *
 * @code{.cpp}
 *         #define FOO UNITEX_PP_VA_ARGS_CAT(X)    // Expands to X
 *         #define BAR UNITEX_PP_VA_ARGS_CAT(X,Y)  // Expands to XY
 * @endcode
 */
#define UNITEX_PP_VA_ARGS_CAT(...)             UNITEX_PP_VA_ARGS_CAT_(UNITEX_PP_VA_NARGS(__VA_ARGS__),__VA_ARGS__)
#define UNITEX_PP_VA_ARGS_CAT_(n, ...)         UNITEX_PP_TOKEN_CAT_(UNITEX_PP_VA_ARGS_CAT__, n)(__VA_ARGS__)
#define UNITEX_PP_VA_ARGS_CAT__0()             UNITEX_PP_EMPTY()
#define UNITEX_PP_VA_ARGS_CAT__1(_1)           _1
#define UNITEX_PP_VA_ARGS_CAT__2(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,__VA_ARGS__)
/*
 * Code snippet to generate the following definitions
 * awk 'BEGIN { count=2 ; while (count++<64) \
 * printf "#define UNITEX_PP_VA_ARGS_CAT__%d(_1,...) \
 * UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__%d(__VA_ARGS__))\n",\
 * count,count-1 }'
 */
#define UNITEX_PP_VA_ARGS_CAT__3(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__2(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__4(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__3(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__5(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__4(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__6(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__5(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__7(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__6(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__8(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__7(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__9(_1,...)       UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__8(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__10(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__9(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__11(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__10(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__12(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__11(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__13(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__12(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__14(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__13(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__15(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__14(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__16(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__15(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__17(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__16(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__18(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__17(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__19(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__18(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__20(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__19(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__21(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__20(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__22(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__21(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__23(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__22(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__24(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__23(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__25(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__24(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__26(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__25(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__27(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__26(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__28(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__27(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__29(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__28(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__30(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__29(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__31(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__30(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__32(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__31(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__33(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__32(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__34(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__33(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__35(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__34(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__36(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__35(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__37(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__36(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__38(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__37(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__39(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__38(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__40(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__39(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__41(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__40(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__42(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__41(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__43(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__42(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__44(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__43(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__45(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__44(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__46(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__45(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__47(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__46(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__48(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__47(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__49(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__48(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__50(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__49(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__51(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__50(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__52(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__51(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__53(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__52(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__54(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__53(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__55(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__54(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__56(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__55(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__57(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__56(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__58(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__57(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__59(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__58(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__60(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__59(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__61(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__60(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__62(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__61(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__63(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__62(__VA_ARGS__))
#define UNITEX_PP_VA_ARGS_CAT__64(_1,...)      UNITEX_PP_TOKEN_PASTE(_1,UNITEX_PP_VA_ARGS_CAT__63(__VA_ARGS__))
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_VARIADIC_H_                     // NOLINT
