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
 * @file      support.h
 * @brief     Intrinsics compiler support
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 support.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_INTRINSIC_SUPPORT_H_                   // NOLINT
#define UNITEX_BASE_COMPILER_INTRINSIC_SUPPORT_H_                   // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/cpu/extensions.h"          // UNITEX_HAS_CPU_EXTENSION
#include "base/os/os.h"                   // UNITEX_OS_IS
/* ************************************************************************** */
#if UNITEX_OS_IS(WINDOWS)
# include <intrin.h>                      // common intrinsic functions
#endif // ! UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
#if UNITEX_HAS_CPU_EXTENSION(MMX)
# include <mmintrin.h>                    // MultiMedia eXtensions
#endif  // UNITEX_HAS_CPU_EXTENSION(MMX)

#if UNITEX_HAS_CPU_EXTENSION(3DNOW)
# include <mm3dnow.h>                     // 3dNow! eXtensions
#endif  // UNITEX_HAS_CPU_EXTENSION(3DNOW)

#if UNITEX_HAS_CPU_EXTENSION(SSE)
# include <xmmintrin.h>                   // Streaming SIMD Extensions
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE)

#if UNITEX_HAS_CPU_EXTENSION(SSE2)
#include <emmintrin.h>                    // Streaming SIMD Extensions 2
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE3)

#if UNITEX_HAS_CPU_EXTENSION(SSE3)
#include <pmmintrin.h>                    // Streaming SIMD Extensions 3
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE3)

#if UNITEX_HAS_CPU_EXTENSION(SSSE3)
#include <tmmintrin.h>                    // Supplemental Streaming SIMD Ext 3
#endif  // UNITEX_HAS_CPU_EXTENSION(SSSE3)

#if UNITEX_HAS_CPU_EXTENSION(SSE4A)
#include <ammintrin.h>                    // Streaming SIMD Extensions 4a
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE4A)

#if UNITEX_HAS_CPU_EXTENSION(SSE41)
#include <smmintrin.h>                    // Streaming SIMD Extensions 4.1
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE41)

#if UNITEX_HAS_CPU_EXTENSION(SSE42)
#include <nmmintrin.h>                    // Streaming SIMD Extensions 4.2
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE42)

#if UNITEX_HAS_CPU_EXTENSION(SSE5)
#include <bmmintrin.h>                    // Streaming SIMD Extensions 5
#endif  // UNITEX_HAS_CPU_EXTENSION(SSE42)

#if UNITEX_HAS_CPU_EXTENSION(AES)
#include <wmmintrin.h>                    // Advanced Encryption Standard
#endif  // UNITEX_HAS_CPU_EXTENSION(AES)
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_INTRINSIC_SUPPORT_H_                // NOLINT
