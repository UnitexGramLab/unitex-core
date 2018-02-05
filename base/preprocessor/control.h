/*
 * Unitex
 *
 * Copyright (C) 2001-2018 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      control.h
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
 *            `cpplint.py --linelength=120 control.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_CONTROL_H_                         // NOLINT
#define UNITEX_BASE_PREPROCESSOR_CONTROL_H_                         // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/preprocessor/util.h"
#include "base/preprocessor/paste.h"
#include "base/preprocessor/logic_unary.h"
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_IFF
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_IFF(c)                 UNITEX_PP_TOKEN_CAT_(UNITEX_PP_IFF_, c)
#define UNITEX_PP_IFF_0(t, ...)          __VA_ARGS__
#define UNITEX_PP_IFF_1(t, ...)          t

/**
 * @def    UNITEX_PP_IF
 *
 * @code{.cpp}
 *         UNITEX_PP_IF(1)("YES","NO")      // Expands to "YES"
 *         UNITEX_PP_IF(0)("YES","NO")      // Expands to "NO"
 *         #define FOO  0                   // FOO is 0
 *         UNITEX_PP_IF(FOO)("YES","NO")    // Expands to "NO"
 *         #define BAR  1                   // BAR is 1
 *         UNITEX_PP_IF(BAR)("YES","NO")    // Expands to "YES"
 *         #undef  FOO                      // BAR isn't defined
 *         UNITEX_PP_IF(FOO)("YES","NO")    // Expands to "YES"
 *         #define BAZ  2                   // BAZ is 2
 *         UNITEX_PP_TEST(BAZ)("YES","NO")  // Expands to "YES"
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_IF(c)         UNITEX_PP_IFF(UNITEX_PP_BOOL(c))

/**
 * @def    UNITEX_PP_TEST
 *
 * @code{.cpp}
 *         UNITEX_PP_TEST(1)("YES","NO")    // Expands to "YES"
 *         UNITEX_PP_TEST(0)("YES","NO")    // Expands to "NO"
 *         #define FOO  0                // FOO is 0
 *         UNITEX_PP_TEST(FOO)("YES","NO")  // Expands to "NO"
 *         #define BAR  1                // BAR is 1
 *         UNITEX_PP_TEST(BAR)("YES","NO")  // Expands to "YES"
 *         #undef  FOO                   // BAR isn't defined
 *         UNITEX_PP_TEST(FOO)("YES","NO")  // Expands to "NO"
 *         #define BAZ  2                // BAZ is 1
 *         UNITEX_PP_TEST(BAZ)("YES","NO")  // Expands to "NO"
 * @endcode
 */
#define UNITEX_PP_TEST(c)       UNITEX_PP_IFF(UNITEX_PP_NOT(UNITEX_PP_COMPL(c)))

/**
 * @def    UNITEX_PP_WHEN
 *
 * @code{.cpp}
 * @endcode
 */
#define UNITEX_PP_WHEN(c)       UNITEX_PP_IF(c)(UNITEX_PP_EXPAND, UNITEX_PP_EAT)

/**
 * @def    UNITEX_PP_WHEN_1
 *
 * @code{.cpp}
 * @endcode
 *
 * @see    UNITEX_PP_IS_1
 */
#define UNITEX_PP_WHEN_1(c)     UNITEX_PP_IFF(UNITEX_PP_NOT(UNITEX_PP_COMPL(c)))(UNITEX_PP_EXPAND, UNITEX_PP_EAT)


/**
 * @def    UNITEX_PP_WHEN_0
 *
 * @code{.cpp}
 * @endcode
 *
 * @see    UNITEX_PP_IS_0
 */
#define UNITEX_PP_WHEN_0(c)     UNITEX_PP_IFF(UNITEX_PP_NOT(c))(UNITEX_PP_EXPAND, UNITEX_PP_EAT)


/**
 * @def    UNITEX_PP_EXPAND_IF
 *
 * @code{.cpp}
 * @endcode
 */
#define UNITEX_PP_EXPAND_IF(c,e) UNITEX_PP_IF(c)(UNITEX_PP_EXPAND(e), UNITEX_PP_EMPTY())

/**
 * @def    UNITEX_PP_EXPAND_TEST
 *
 * @code{.cpp}
 * @endcode
 */
#define UNITEX_PP_EXPAND_TEST(c,e) UNITEX_PP_TEST(c)(UNITEX_PP_EXPAND(e), UNITEX_PP_EMPTY())
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_CONTROL_H_                      // NOLINT
