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
 * @file      dll_visibility.h
 * @brief     Macro helpers definitions for shared library support
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler_attributes.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 dll_visibility.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_DLL_VISIBILITY_H_            // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_DLL_VISIBILITY_H_            // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/version.h"  // UNITEX_COMPILER_AT_LEAST_GCC
#include "base/os/os.h"             // UNITEX_OS_*
/* ************************************************************************** */
/**
 * @def    UNITEX_DLL_IMPORT
 * @brief  Indicate to the compiler that a function or object is imported from
 *         a DLL. This is an optimization that removes a layer of indirection
 * @see    UNITEX_API
 */
/**
 * @def    UNITEX_DLL_EXPORT
 * @brief  Indicate to the compiler that a function or object is exported from
 *         a DLL. This is an optimization that removes a layer of indirection
 * @see    UNITEX_API
 */
/**
 * @def    UNITEX_DLL_LOCAL
 * @brief  Indicate to the compiler that a function or object is part of a
 *         non-api symbol
 * @see    UNITEX_LOCAL
 */
// This was adapted from the "Step-by-step guide" of the gcc's visibility wiki,
// originally written by Niall Douglas.
// @see https://gcc.gnu.org/wiki/Visibility
#if UNITEX_OS_IS(WINDOWS)
# define UNITEX_DLL_IMPORT __declspec(dllimport)
# define UNITEX_DLL_EXPORT __declspec(dllexport)
# define UNITEX_DLL_LOCAL  /* nothing */
#else  // !UNITEX_OS_IS(WINDOWS)
# if UNITEX_COMPILER_AT_LEAST_GCC(4,0)
#  define UNITEX_DLL_IMPORT __attribute__ ((visibility ("default")))
#  define UNITEX_DLL_EXPORT __attribute__ ((visibility ("default")))
#  define UNITEX_DLL_LOCAL  __attribute__ ((visibility ("hidden")))
# else  // !UNITEX_COMPILER_AT_LEAST_GCC(4,0)
#  define UNITEX_DLL_IMPORT  /* nothing */
#  define UNITEX_DLL_EXPORT  /* nothing */
#  define UNITEX_DLL_LOCAL   /* nothing */
# endif  // UNITEX_COMPILER_AT_LEAST_GCC(4,0)
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_DLL_VISIBILITY_H_         // NOLINT
