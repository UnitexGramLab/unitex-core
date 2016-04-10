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
 * @file      noreturn.h
 * @brief     Tells the compiler that a function does not return
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler_attributes.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 noreturn.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_NORETURN_H_                  // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_NORETURN_H_                  // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_NORETURN
 * @brief  Tells the compiler that a function does not return
 */
// Tell the compiler that a method does not return
#if    UNITEX_COMPILER_COMPLIANT(CXX11)
#      define UNITEX_NORETURN               [[noreturn]]
#elif  UNITEX_COMPILER_AT_LEAST(MSVC,13,10)  ||\
       UNITEX_COMPILER_AT_LEAST(INTEL,9,0)
#      define UNITEX_NORETURN               __declspec(noreturn)
#elif  UNITEX_COMPILER_AT_LEAST(GCC,2,5)     ||\
       UNITEX_COMPILER_AT_LEAST(CLANG,1,4)
#      define UNITEX_NORETURN               __attribute__((noreturn))
#else   // !CX11 && ! MSC_VER(>=1310) && !GNUC(>=25)
#      define UNITEX_NORETURN               /* nothing */
#endif  // UNITEX_COMPILER_COMPLIANT(CXX11)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_NORETURN_H_               // NOLINT
