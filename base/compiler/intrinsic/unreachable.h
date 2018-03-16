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
 * @file      unreachable.h
 * @brief     Macros to indicate that a specific point in the program cannot be reached
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 unreachable.h`
 *
 * @date      August 2017
 *
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_INTRINSICS_UNREACHABLE_H_              // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSICS_UNREACHABLE_H_              // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/compiler/version.h"            // UNITEX_COMPILER_AT_LEAST
/* ************************************************************************** */
/**
 * @def    UNITEX_UNREACHABLE
 * @brief  indicates that a specific point in the program cannot be reached
 */
#if     UNITEX_COMPILER_AT_LEAST(CLANG,3,2) ||\
        UNITEX_COMPILER_AT_LEAST(GCC,4,5)
#define UNITEX_UNREACHABLE   __builtin_unreachable()
#else
#define UNITEX_UNREACHABLE   ((void)0)
#endif  // defined(unreachable)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSICS_UNREACHABLE_H_           // NOLINT
