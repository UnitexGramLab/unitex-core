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
 * @file      util.h
 * @brief     Preprocessor macros helpers
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
 *            `cpplint.py --linelength=120 util.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_UTIL_H_                            // NOLINT
#define UNITEX_BASE_PREPROCESSOR_UTIL_H_                            // NOLINT
/* ************************************************************************** */
// Unitex headers
/* ************************************************************************** */
/**
 * @def    UNITEX_PP_CHECK
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_CHECK(...)             UNITEX_PP_CHECK_N(__VA_ARGS__, 0,)
#define UNITEX_PP_CHECK_N(x, n, ...)     n

/**
 * @def    UNITEX_PP_PROBE
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_PROBE(x) x, 1,

/**
 * @def    UNITEX_PP_EAT
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_EAT(...)

/**
 * @def    UNITEX_PP_EMPTY
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_EMPTY()

/**
 * @def    UNITEX_PP_EXPAND
 *
 * @code{.cpp}
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_EXPAND(...)   __VA_ARGS__

/**
 * @def    UNITEX_PP_DEFER
 * @brief  A deferred expression is an expression that requires more scans
 *         to fully expand
 *
 * @code{.cpp}
 *         #define A() 123
 *         A()                                // 123
 *         UNITEX_PP_DEFER(A)()                 // A (), it requires one more scan
 *         UNITEX_PP_EXPAND(UNITEX_PP_DEFER(A)()) // 123, forces another scan
 * @endcode
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_DEFER(id)     id UNITEX_PP_EMPTY()


/**
 * @def    UNITEX_PP_OBSTRUCT
 *
 * @code{.cpp}
 * @endcode
 *
 * @see    UNITEX_PP_DEFER
 *
 * @author Paul Fultz II
 * @see    http://pfultz2.com/blog/2012/05/10/turing/
 */
#define UNITEX_PP_OBSTRUCT(id)  id UNITEX_PP_DEFER(UNITEX_PP_EMPTY)()

/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_UTIL_H_                         // NOLINT
