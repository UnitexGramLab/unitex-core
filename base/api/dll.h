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
 * @file      api.h
 * @brief     C++ API library support macros
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      This code was adapted from the "Step-by-step guide" of the gcc's
 *            visibility wiki originally written by Niall Douglas
 *
 * @see       https://gcc.gnu.org/wiki/Visibility
 * @see       http://www.cmake.org/Wiki/BuildingWinDLL
 *
 * @note      For every non-templated non-static function/class definition in your
 *            library (both headers and source files), decide if it is publicly
 *            used or internally used: If it is publicly used, mark with
 *            UNITEX_API, If it is only internally used, mark with UNITEX_LOCAL
 *
 *            Finally, to compile you have several options:
 *
 * @code{.cpp}
 *   // Use as static library
 *   gcc -c foo.cpp                                                      // 1
 *
 *   // Compile as static library
 *   gcc -c foo.cpp -DUNITEX_BUILD_LIBRARY  -lfoolib                     // 2
 *
 *   // Use as shared library
 *   gcc -c foo.cpp -DUNITEX_SHARED_LIB -lfoolib                         // 3
 *
 *   // Compile as shared library
 *   gcc -c foo.cpp -DUNITEX_BUILD_LIBRARY -DUNITEX_SHARED_LIB           // 4
 * @endcode
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 api.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_API_DLL_H_                                      // NOLINT
#define UNITEX_BASE_API_DLL_H_                                      // NOLINT
/* ************************************************************************** */
#include "base/compiler/attribute/dll_visibility.h"  // UNITEX_API_*
/* ************************************************************************** */
/**
 * @def      UNITEX_ABI_IMPORT
 * @brief    Function or object is imported. Does nothing for static build
 * @see      UNITEX_API_IMPORT
 */
/**
 * @def      UNITEX_ABI_EXPORT
 * @brief    Function or object is exported. Does nothing for static build
 * @see      UNITEX_API_EXPORT
 */
/**
 * @def      UNITEX_LOCAL
 * @brief    Used for internally used (non-api) symbols. Does nothing for
 *           static build
 * @see      UNITEX_API_LOCAL
 *
 * @code{.cpp}
 *  UNITEX_LOCAL int foo();    // foo() function is only internally used
 *  class UNITEX_LOCAL bar();  // bar() class is only internally used
 * @endcode
 *
 * @note     Individual member functions of an exported class that are not part
 *           of the interface, in particular ones which are private, and are not
 *           used by friend code, should be marked individually with UNITEX_LOCAL
 *
 * @see      https://gcc.gnu.org/wiki/Visibility
 */
// Test if we compiling as a shared library
#if defined(UNITEX_SHARED_LIB)
# define UNITEX_ABI_IMPORT UNITEX_API_IMPORT
# define UNITEX_ABI_EXPORT UNITEX_API_EXPORT
# define UNITEX_LOCAL      UNITEX_API_LOCAL
#else  //  !defined(UNITEX_SHARED_LIB)
// Does nothing for static build
# define UNITEX_ABI_IMPORT  /* nothing */
# define UNITEX_ABI_EXPORT  /* nothing */
# define UNITEX_LOCAL       /* nothing */
#endif  // defined(UNITEX_SHARED_LIB)
/* ************************************************************************** */
/**
 * @def      UNITEX_API
 * @brief    Used for the public API symbols (every non-templated non-static
 *           function definition and every non-templated class definition in
 *           your library). It either DLL imports or DLL exports, or does
 *           nothing for a static build
 *
 * @code{.cpp}
 *  UNITEX_API int foo();    // foo() function is publicly used
 *  class UNITEX_API bar();  // bar() class is publicly used
 * @endcode
 *
 * @see      https://gcc.gnu.org/wiki/Visibility
 */
// Test if we are building a library (export), instead of using it (import)
#if defined(UNITEX_BUILD_LIBRARY)
# define UNITEX_API UNITEX_ABI_EXPORT
#else  // !defined(UNITEX_BUILD_LIBRARY)
# define UNITEX_API UNITEX_ABI_IMPORT
#endif  //  defined(UNITEX_BUILD_LIBRARY)
/* ************************************************************************** */
#endif  // UNITEX_BASE_API_DLL_H_                                   // NOLINT
