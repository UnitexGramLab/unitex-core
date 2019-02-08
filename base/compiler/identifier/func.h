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
 * @file      func.h
 * @brief     Macro to define the name of the enclosing function as a string
 *            literal
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 func.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_IDENTIFIER_FUNC_H_                     // NOLINT
#define UNITEX_BASE_COMPILER_IDENTIFIER_FUNC_H_                     // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/compliance.h"  // COMPILER_COMPLIANT
#include "base/compiler/version.h"     // COMPILER_AT_LEAST_*, COMPILER_IS
/* ************************************************************************** */
/**
 * @brief  Valid only inside a function. Defines the  decorated or alternatively
 *         the undecorated name of the enclosing function as a string literal
 *
 * @see    https://gcc.gnu.org/onlinedocs/gcc/Function-Names.html
 * @see    http://msdn.microsoft.com/en-us/library/b0084kay.aspx
 * @see    http://www.open-std.org/jtc1/sc22/wg21/docs/papers/2007/n2340.htm
 */
// __func__            is part of the C99 and C++11 standards (it's not a macro)
//                     `static const char __func__[] = "function-name";`
// __PRETTY_FUNCTION__ is a GCC, Clang extension (it's not a macro)
// __FUNCSIG__         is a Microsoft-specific (macro)
// __FUNCTION__        is not standardized (macro or static constant variable)
#if UNITEX_COMPILER_COMPLIANT(C99)          ||\
    UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_HAS_IDENTIFIER_FUNCTION                    1
# define UNITEX_COMPILER_IDENTIFIER_FUNC                   __func__
#elif   UNITEX_COMPILER_AT_LEAST(GCC,2,6)   ||\
      UNITEX_COMPILER_AT_LEAST(CLANG,2,8)
# define UNITEX_HAS_IDENTIFIER_FUNCTION                    1
# define UNITEX_COMPILER_IDENTIFIER_FUNC                   __PRETTY_FUNCTION__
#elif UNITEX_COMPILER_IS(MSVC)              &&\
      defined(__FUNCSIG__)
# define UNITEX_HAS_IDENTIFIER_FUNCTION                    1
# define UNITEX_COMPILER_IDENTIFIER_FUNC                   __FUNCSIG__
#elif (UNITEX_COMPILER_AT_LEAST(MSVC,13,10) ||\
       UNITEX_COMPILER_AT_LEAST(INTEL,6,0)) &&\
       defined(__FUNCTION__)
# define UNITEX_HAS_IDENTIFIER_FUNCTION                    1
# define UNITEX_COMPILER_IDENTIFIER_FUNC                   __FUNCTION__
#else    // !defined(UNITEX_COMPILER_IDENTIFIER_FUNC)
# define UNITEX_COMPILER_IDENTIFIER_FUNC                   "<unknown>"
#endif  // UNITEX_COMPILER_AT_LEAST(GCC,2,6)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_IDENTIFIER_FUNC_H_                  // NOLINT
