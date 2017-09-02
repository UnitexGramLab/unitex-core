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
 * @file      restrict.h
 * @brief     C99 `restrict` keyword
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler/common.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 cpu.h`
 *
 * @date      September 2017
 *
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_KEYWORD_RESTRICT_H_                    // NOLINT
#define UNITEX_BASE_COMPILER_KEYWORD_RESTRICT_H_                    // NOLINT
/* ************************************************************************** */
#include "base/compiler/compliance.h"     // COMPILER_COMPLIANT
#include "base/compiler/version.h"        // COMPILER_AT_LEAST_*, COMPILER_IS
/* ************************************************************************** */
/**
 * @brief C99 `restrict` keyword
 */
// C99       restrict
// GCC     __restrict
// MSVC    __restrict
// CLANG   __restrict__
#if   UNITEX_COMPILER_COMPLIANT(C99)  // C99 compliant compiler
# define UNITEX_RESTRICT              restrict
#elif defined(restrict)               // restrict defined elsewhere
# define UNITEX_RESTRICT              restrict
#elif UNITEX_COMPILER_IS(CLANG)       // Clang Compiler
# define UNITEX_RESTRICT              __restrict__
#elif UNITEX_COMPILER_IS(MSVC)        // Microsoft Visual Studio Compiler
# define UNITEX_RESTRICT              __restrict
#elif UNITEX_COMPILER_IS(GCC)         // GNU Compiler
# define UNITEX_RESTRICT              __restrict
#else  // no restrict keyword support
# define UNITEX_RESTRICT              /* nothing */
#endif  // UNITEX_COMPILER_COMPLIANT(C99)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_KEYWORD_RESTRICT_H_                 // NOLINT
