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
 * @file      extensions.h
 * @brief     Target CPU multimedia extensions support
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 extensions.h`
 *
 * @date      March 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_CPU_X86_64_EXTENSIONS_H_                        // NOLINT
#define UNITEX_BASE_CPU_X86_64_EXTENSIONS_H_                        // NOLINT
/* ************************************************************************** */
// Unitex headers
#include "base/macro/helper/test.h"
#include "base/cpu/version.h"
/* ************************************************************************** */
#if UNITEX_CPU_IS(X86) || UNITEX_CPU_IS(X64)
// MMX:    MultiMedia eXtensions instructions
// 3DNOW:  3DNow! extension instruction set
// SSE:    Streaming SIMD Extensions instructions
// SSE2:   Streaming SIMD Extensions 2 instructions
// SSE3:   Streaming SIMD Extensions 3 instructions
// SSSE3:  Supplemental Streaming SIMD Extensions 3 instructions
// SSE41:  Streaming SIMD Extensions 4.1 instructions
// SSE42:  Streaming SIMD Extensions 4.2 instructions
// SSE5:   Streaming SIMD Extensions 5 instructions
// AES:    Advanced Encryption Standard Instruction Set
// PCLMUL: Carryless multiply instruction
// gcc -dM -E -x c /dev/null -march=native
#if UNITEX_HAVE(MMX)     || __MMX__     ||\
    UNITEX_HAVE(3DNOW)   || __3dNOW__   ||\
    UNITEX_HAVE(SSE)     || __SSE__     ||\
    UNITEX_HAVE(SSE2)    || __SSE2__    ||\
    UNITEX_HAVE(SSE3)    || __SSE3__    ||\
    UNITEX_HAVE(SSSE3)   || __SSSE3__   ||\
    UNITEX_HAVE(SSE4A)   || __SSE4A__   ||\
    UNITEX_HAVE(SSE41)   || __SSE4_1__  ||\
    UNITEX_HAVE(SSE42)   || __SSE4_2__  ||\
    UNITEX_HAVE(SSE5)    || __SSSE5__   ||\
    UNITEX_HAVE(AES)     || __AES__     ||\
    UNITEX_HAVE(PCLMUL)  || __PCLMUL__
/* ************************************************************************** */
#  if UNITEX_HAVE(MMX)   || __MMX__
#  define UNITEX_HAS_CPU_EXTENSION_MMX     1   // MultiMedia eXtensions
#  endif  // UNITEX_HAVE(SSE)

#  if UNITEX_HAVE(3DNOW) || __3dNOW__
#  define UNITEX_HAS_CPU_EXTENSION_3DNOW   1   // 3dNow! Extensions
#  endif  // UNITEX_HAVE(SSE)

#  if UNITEX_HAVE(SSE)   || __SSE__
#  define UNITEX_HAS_CPU_EXTENSION_SSE     1   // Streaming SIMD Extensions
#  endif  // UNITEX_HAVE(SSE)

#  if UNITEX_HAVE(SSE2)  || __SSE2__
#  define UNITEX_HAS_CPU_EXTENSION_SSE2    1   // Streaming SIMD Extensions 2
#  endif  // UNITEX_HAVE(SSE3)

#  if UNITEX_HAVE(SSE3)  || __SSE3__
#  define UNITEX_HAS_CPU_EXTENSION_SSE3    1   // Streaming SIMD Extensions 3
#  endif  // UNITEX_HAVE(SSE3)

#  if UNITEX_HAVE(SSSE3) || __SSSE3__
#  define UNITEX_HAS_CPU_EXTENSION_SSSE3   1   // Suppl. Streaming SIMD Ext 3
#  endif  // UNITEX_HAVE(SSSE3)

#  if UNITEX_HAVE(SSE4A) || __SSE4A__
#  define UNITEX_HAS_CPU_EXTENSION_SSE4A   1   // AMD Streaming SIMD Extensions
#  endif  // UNITEX_HAVE(SSE4A)

#  if UNITEX_HAVE(SSE41) || __SSE4_1__
#  define UNITEX_HAS_CPU_EXTENSION_SSE41   1   // Streaming SIMD Extensions 4.1
#  endif  // UNITEX_HAVE(SSE41)

#  if UNITEX_HAVE(SSE42) || __SSE4_2__
#  define UNITEX_HAS_CPU_EXTENSION_SSE42   1   // Streaming SIMD Extensions 4.2
#  endif  // UNITEX_HAVE(SSE42)

#  if UNITEX_HAVE(SSE5) ||  __SSSE5__
#  define UNITEX_HAS_CPU_EXTENSION_SSE5    1   // Streaming SIMD Extensions 5
#  endif  // UNITEX_HAVE(SSE42)

#  if UNITEX_HAVE(AES)     ||  __AES__     ||\
      UNITEX_HAVE(PCLMUL)  ||  __PCLMUL__
#  define UNITEX_HAS_CPU_EXTENSION_AES     1   // AES Instruction Set
#  endif  // UNITEX_HAVE(AES)

# endif  // UNITEX_HAVE(MMX) ...
/* ************************************************************************** */
#endif  //  UNITEX_CPU_IS(X86) || UNITEX_CPU_IS(X64)
/* ************************************************************************** */
#endif  // UNITEX_BASE_CPU_X86_64_EXTENSIONS_H_                     // NOLINT
