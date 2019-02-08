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
 * @file      compliance.h
 * @brief     Macros to test compilers standards compliance
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @attention Please do note, not to change the macros definition order
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 compliance.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_COMPLIANCE_H_                          // NOLINT
#define UNITEX_BASE_COMPILER_COMPLIANCE_H_                          // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/config.h"             // HAVE_STD_TR1
#include "base/compiler/version.h"   // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_MODE_C
 * @brief  C compiler mode
 * @return 1 if not defined(__cplusplus)
 * @note   Do not use this directly, rather use UNITEX_COMPILER_MODE(C)
 * @see    UNITEX_COMPILER_MODE
 */
// g++ -x c -E -dM - < /dev/null | grep cplusplus
#if !defined(__cplusplus)
# define UNITEX_COMPILER_MODE_C              1
#endif  // defined(__cplusplus)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_MODE_CXX
 * @brief  C++ compiler mode
 * @return 1 if defined(__cplusplus)
 * @note   Do not use this directly, rather use UNITEX_COMPILER_MODE(CXX)
 * @see    UNITEX_COMPILER_MODE
 */
// g++ -x c++ -E -dM - < /dev/null | grep cplusplus
#if defined(__cplusplus)
# define UNITEX_COMPILER_MODE_CXX            1
#endif  // defined(__cplusplus)
/* ************************************************************************** */
/**
 * @brief  Test whenever the C++ compiler is in use or not
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_MODE(C)    \\ C
 *         UNITEX_COMPILER_MODE(CXX)  \\ C++
 * @endcode
 *
 * @see    UNITEX_COMPILER_MODE_C
 * @see    UNITEX_COMPILER_MODE_CXX
 */
#define UNITEX_COMPILER_MODE(Mode)\
        (defined(UNITEX_COMPILER_MODE_##Mode) &&\
                 UNITEX_COMPILER_MODE_##Mode)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_C89
 * @brief  C89-conformant (not experimental) compiler   [ANSI X3.159-1989]
 * @return 1 if defined __STDC__
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(C89)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
#if defined(__STDC__)
# define UNITEX_COMPILER_COMPLIANT_C89       1
#endif  // defined(__STDC__)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_C90
 * @brief  C90-conformant (not experimental) compiler   [ISO/IEC 9899:1990]
 * @return 1 if defined __STDC_VERSION__
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(C90)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
//
#if defined(__STDC_VERSION__)
# define UNITEX_COMPILER_COMPLIANT_C90       1
#endif  // defined(__STDC_VERSION__)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_C94
 * @brief  C94-conformant (not experimental) compiler   [ISO/IEC 9899-1:1994]
 * @return 1 if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(C94)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
# define UNITEX_COMPILER_COMPLIANT_C94       1
#endif  // defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199409L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_C99
 * @brief  C99-conformant (not experimental) compiler   [ISO/IEC 9899:1999]
 * @return 1 if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(C99)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// gcc -x c -std=c99  -E -dM - < /dev/null | grep __STDC_VERSION__
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
# define UNITEX_COMPILER_COMPLIANT_C99       1
#endif  // defined(__STDC_VERSION__) && __STDC_VERSION__ >= 199901L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_C11
 * @brief  C11-conformant (not experimental) compiler   [ISO/IEC 9899:2011]
 * @return 1 if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(C11)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// gcc -x c -std=c11  -E -dM - < /dev/null | grep __STDC_VERSION__
#if defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
# define UNITEX_COMPILER_COMPLIANT_C11       1
#endif  // defined(__STDC_VERSION__) && __STDC_VERSION__ >= 201112L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_CXX98
 * @brief  C++98-conformant (not experimental) compiler [ISO/IEC 14882:1998]
 * @return 1 if defined(__cplusplus) && __cplusplus >= 199711L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(CXX98)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// g++ -x c++ -std=c++98  -E -dM - < /dev/null | grep cplusplus
#if defined(__cplusplus) && __cplusplus >= 199711L
# define UNITEX_COMPILER_COMPLIANT_CXX98     1
#endif  // defined(__cplusplus) && __cplusplus >= 199711L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_CXX03
 * @brief  C++03-conformant (not experimental) compiler [ISO/IEC 14882:2003]
 * @return 1 if UNITEX_COMPILER_COMPLIANT_CXX98 && UNITEX_HAVE(STD_TR1)
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(CXX03)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// g++ -x c++ -std=c++03  -E -dM - < /dev/null | grep cplusplus
#if defined(__cplusplus) && __cplusplus >= 199711L
# if UNITEX_COMPILER_AT_LEAST(GCC, 4, 1) || UNITEX_HAVE(STD_TR1)
#  define UNITEX_COMPILER_COMPLIANT_CXX03    1
# endif  // GCC >= 4.1 || HAVE_STD_TR1
#endif  // defined(__cplusplus) && __cplusplus >= 199711L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_CXX11
 * @brief  C++11-conformant (not experimental) compiler [ISO/IEC 14882:2011]
 * @return 1 if defined(__cplusplus) && __cplusplus >= 201103L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(CXX11)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// g++ -x c++ -std=c++11  -E -dM - < /dev/null | grep cplusplus
#if defined(__cplusplus) && __cplusplus >= 201103L
# define UNITEX_COMPILER_COMPLIANT_CXX11     1
#endif  // defined(__cplusplus) && __cplusplus >= 201103L
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_COMPLIANT_CXX14
 * @brief  C++14-conformant (not experimental) compiler [ISO/IEC 14882:2014]
 * @return 1 if defined(__cplusplus) && __cplusplus >= 201402L
 * @note   Do not use this directly, rather use UNITEX_COMPILER_COMPLIANT(CXX14)
 * @see    UNITEX_COMPILER_COMPLIANT
 */
// g++ -x c++ -std=c++14  -E -dM - < /dev/null | grep cplusplus
#if defined(__cplusplus) && __cplusplus >= 201402L
# define UNITEX_COMPILER_COMPLIANT_CXX14     1
#endif  // defined(__cplusplus) && __cplusplus >= 201402L
/* ************************************************************************** */
/**
 * @brief  Test for a compiler compliance at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_COMPLIANT(C89)    \\ ANSI X3.159-1989
 *         UNITEX_COMPILER_COMPLIANT(C90)    \\ ISO/IEC 9899:1990
 *         UNITEX_COMPILER_COMPLIANT(C94)    \\ ISO/IEC 9899-1:1994
 *         UNITEX_COMPILER_COMPLIANT(C99)    \\ ISO/IEC 9899:1999
 *         UNITEX_COMPILER_COMPLIANT(C11)    \\ ISO/IEC 9899:2011
 *         UNITEX_COMPILER_COMPLIANT(CXX98)  \\ ISO/IEC 14882:1998
 *         UNITEX_COMPILER_COMPLIANT(CXX03)  \\ ISO/IEC 14882:2003
 *         UNITEX_COMPILER_COMPLIANT(CXX11)  \\ ISO/IEC 14882:2011
 *         UNITEX_COMPILER_COMPLIANT(CXX14)  \\ ISO/IEC 14882:2014
 * @endcode
 *
 * @see    UNITEX_COMPILER_COMPLIANT_C89
 * @see    UNITEX_COMPILER_COMPLIANT_C90
 * @see    UNITEX_COMPILER_COMPLIANT_C94
 * @see    UNITEX_COMPILER_COMPLIANT_C99
 * @see    UNITEX_COMPILER_COMPLIANT_CXX98
 * @see    UNITEX_COMPILER_COMPLIANT_CXX03
 * @see    UNITEX_COMPILER_COMPLIANT_CXX11
 * @see    UNITEX_COMPILER_COMPLIANT_CXX14
 */
#define UNITEX_COMPILER_COMPLIANT(VersionName)\
        (defined(UNITEX_COMPILER_COMPLIANT_##VersionName) &&\
                 UNITEX_COMPILER_COMPLIANT_##VersionName)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_STRING_STANDARD_C
 * @brief  C standard compliance string constant
 * @note   The valid declaration order is from more to less recent
 * @see    UNITEX_COMPILER_COMPLIANT
 */
#if      UNITEX_COMPILER_COMPLIANT(C11)
# define UNITEX_COMPILER_STRING_STANDARD_C   "C11"
#elif    UNITEX_COMPILER_COMPLIANT(C99)
# define UNITEX_COMPILER_STRING_STANDARD_C   "C99"
#elif    UNITEX_COMPILER_COMPLIANT(C94)
# define UNITEX_COMPILER_STRING_STANDARD_C   "C94"
#elif    UNITEX_COMPILER_COMPLIANT(C90)
# define UNITEX_COMPILER_STRING_STANDARD_C   "C90"
#elif    UNITEX_COMPILER_COMPLIANT(C89)
# define UNITEX_COMPILER_STRING_STANDARD_C   "C89"
#else  // Non strict conformance to the ISO Standard C
# define UNITEX_COMPILER_STRING_STANDARD_C   "C??"
#endif  //  UNITEX_COMPILER_COMPLIANT(C11)
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_STRING_STANDARD_CXX
 * @brief  C++ standard compliance string constant
 * @note   The valid declaration order is from more to less recent
 * @see    UNITEX_COMPILER_COMPLIANT
 */
#if      UNITEX_COMPILER_COMPLIANT(CXX14)
# define UNITEX_COMPILER_STRING_STANDARD_CXX "CXX14"
#elif    UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_COMPILER_STRING_STANDARD_CXX "CXX11"
#elif    UNITEX_COMPILER_COMPLIANT(CXX03)
# define UNITEX_COMPILER_STRING_STANDARD_CXX "CXX03"
#elif    UNITEX_COMPILER_COMPLIANT(CXX98)
# define UNITEX_COMPILER_STRING_STANDARD_CXX "CXX98"
#elif    UNITEX_COMPILER_MODE(CXX)
// Non-conforming C++ Compiler or buggy __cplusplus definition
// @see https://gcc.gnu.org/bugzilla/show_bug.cgi?id=1773
# define UNITEX_COMPILER_STRING_STANDARD_CXX "CXX??"
#else  // Not in C++ mode, UNITEX_COMPILER_MODE(C) is 1
# define UNITEX_COMPILER_STRING_STANDARD_CXX "none"
#endif  // UNITEX_COMPILER_COMPLIANT(CXX11)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_COMPLIANCE_H_                       // NOLINT
