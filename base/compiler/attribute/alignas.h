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
 * @file      alignas.h
 * @brief     Alignment specifiers using `alignas`
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 alignas.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_ATTRIBUTE_ALIGNAS_H_                   // NOLINT
#define UNITEX_BASE_COMPILER_ATTRIBUTE_ALIGNAS_H_                   // NOLINT
/* ************************************************************************** */
#include "base/compiler/features.h"  // UNITEX_HAS_FEATURE
#include "base/compiler/version.h"   // UNITEX_COMPILER_IS
/* ************************************************************************** */
/**
 * @brief  C++11 alignment specifiers using `alignas`
 * @see    UNITEX_CXX_PROPOSAL_N2341
 */
#if UNITEX_HAS_FEATURE(cxx_alignas)
# define UNITEX_ALIGNAS(alignment)    alignas(alignment)
#elif UNITEX_COMPILER_IS(MSVC)        // Microsoft Visual Studio Compiler
# define UNITEX_ALIGNAS(alignment)    __declspec(align(alignment))
#elif UNITEX_COMPILER_IS(GCC)         // GNU Compiler
# define UNITEX_ALIGNAS(alignment)    __attribute__((aligned(alignment)))
#endif  //  UNITEX_HAS_FEATURE(cxx_alignas)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_ATTRIBUTE_ALIGNAS_H_                // NOLINT
