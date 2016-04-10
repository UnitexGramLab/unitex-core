/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      test.h
 * @brief     Preprocessor macros to test definitions
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 test.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_PREPROCESSOR_TEST_H_                            // NOLINT
#define UNITEX_BASE_PREPROCESSOR_TEST_H_                            // NOLINT
/* ************************************************************************** */
/**
 * @brief  Tests if a macro is defined and set
 *
 * @code{.cpp}
 *  // true if kFoo is defined and true
 *  UNITEX_DEFINED_SET(kFoo)
 * @endcode
 */
#define UNITEX_DEFINED_SET(definition)        \
        (defined(definition) && (definition))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro is defined and unset
 *
 * @code{.cpp}
 *  // true if kFoo is defined and false
 *  UNITEX_DEFINED_UNSET(kFoo)
 * @endcode
 */
#define UNITEX_DEFINED_UNSET(definition)      \
        (defined(definition) && !(definition))

/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAVE_ or UNITEX_HAVE_ is defined
 *
 * @code{.cpp}
 *  // true if HAVE_BOOST_THREAD or UNITEX_HAVE_BOOST_THREAD are defined
 *  UNITEX_HAVE(BOOST_THREAD)
 * @endcode
 */
#define UNITEX_HAVE(X)                        \
            (defined(HAVE_##X)              ||\
             defined(UNITEX_HAVE_##X))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAVE_ or UNITEX_HAVE_ is defined and set
 *
 * @code{.cpp}
 *  // true if HAVE_kFoo or UNITEX_HAVE_kFoo are defined and set
 *  UNITEX_HAVE_SET(kFoo)
 * @endcode
 */
#define UNITEX_HAVE_SET(X)                                     \
            ((defined(HAVE_##X)        && (HAVE_##X))        ||\
             (defined(UNITEX_HAVE_##X) && (UNITEX_HAVE_##X)))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAVE_ or UNITEX_HAVE_ is defined and
 *         unset
 *
 * @code{.cpp}
 *  // true if HAVE_kFoo or UNITEX_HAVE_kFoo are defined and unset
 *  UNITEX_HAVE_UNSET(kFoo)
 * @endcode
 */
#define UNITEX_HAVE_UNSET(X)                                    \
            ((defined(HAVE_##X)        && !(HAVE_##X))        ||\
             (defined(UNITEX_HAVE_##X) && !(UNITEX_HAVE_##X)))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAS_ or UNITEX_HAS_ is defined
 *
 * @code{.cpp}
 *  // true if HAS_kFoo or UNITEX_HAS_kFoo are defined
 *  UNITEX_HAS(kFoo)
 * @endcode
 */
#define UNITEX_HAS(X)                        \
            (defined(HAS_##X)              ||\
             defined(UNITEX_HAS_##X))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAS_ or UNITEX_HAS_ is defined and set
 *
 * @code{.cpp}
 *  // true if HAS_kFoo or UNITEX_HAS_kFoo are defined and set
 *  UNITEX_HAS_SET(kFoo)
 * @endcode
 */
#define UNITEX_HAS_SET(X)                                    \
            ((defined(HAS_##X)        && (HAS_##X))        ||\
             (defined(UNITEX_HAS_##X) && (UNITEX_HAS_##X)))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by HAS_ or UNITEX_HAS_ is defined and
 *         unset
 *
 * @code{.cpp}
 *  // true if HAS_kFoo or UNITEX_HAS_kFoo are defined and unset
 *  UNITEX_HAS_UNSET(kFoo)
 * @endcode
 */
#define UNITEX_HAS_UNSET(X)                                   \
            ((defined(HAS_##X)        && !(HAS_##X))        ||\
             (defined(UNITEX_HAS_##X) && !(UNITEX_HAS_##X)))
/* ************************************************************************** */
/**
 * @brief  Tests if a macro prefixed by USE_ or UNITEX_USE_ is defined and
 *         true
 *
 * @code{.cpp}
 *  // true if USE_BOOST_THREAD or UNITEX_USE_BOOST_THREAD are defined and true
 *  UNITEX_USE(BOOST_THREADS)
 * @endcode
 */
#define UNITEX_USE(X)                                        \
            ((defined(USE_##X)        && (USE_##X))        ||\
             (defined(UNITEX_USE_##X) && (UNITEX_USE_##X)))
/* ************************************************************************** */
#endif  // UNITEX_BASE_PREPROCESSOR_TEST_H_                         // NOLINT
