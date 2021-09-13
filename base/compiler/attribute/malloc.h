/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      malloc.h
 * @brief     Tells the compiler that a function is malloc-like
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the
 *            base/compiler_attributes.h header file to gain this file's
 *            functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 cpu.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_MALLOC_H_                    // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_MALLOC_H_                    // NOLINT
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_FUNCTION_MALLOC
 * @brief  Tells the compiler that a function is malloc-like
 */
#if  UNITEX_COMPILER_AT_LEAST(MSVC,14,0)
# define UNITEX_FUNCTION_MALLOC                __declspec(noalias)
#elif UNITEX_COMPILER_AT_LEAST_(GCC,2,9,6)
# define UNITEX_FUNCTION_MALLOC                __attribute__((__malloc__))
#else    // Not compiler support
# define UNITEX_FUNCTION_MALLOC                /* nothing */
#endif  // UNITEX_COMPILER_IS(CLANG)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_MALLOC_H_                 // NOLINT
