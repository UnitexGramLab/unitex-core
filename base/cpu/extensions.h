/*
 * Unitex
 *
 * Copyright (C) 2001-2020 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
#ifndef UNITEX_BASE_CPU_EXTENSIONS_H_                               // NOLINT
#define UNITEX_BASE_CPU_EXTENSIONS_H_                               // NOLINT
/* ************************************************************************** */
#include "base/cpu/x86_64/extensions.h"
/* ************************************************************************** */
/**
 * @brief  Test if an instruction set extension to the CPU architecture will
 *         be available in the target system
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_HAS_CPU_EXTENSION(MMX)    \\ MultiMedia eXtensions
 *         UNITEX_HAS_CPU_EXTENSION(3DNOW)  \\ 3dNow! Extensions
 *         UNITEX_HAS_CPU_EXTENSION(SSE)    \\ Streaming SIMD Extensions
 *         UNITEX_HAS_CPU_EXTENSION(SSE2)   \\ Streaming SIMD Extensions 2
 *         UNITEX_HAS_CPU_EXTENSION(SSE3)   \\ Streaming SIMD Extensions 3
 *         UNITEX_HAS_CPU_EXTENSION(SSSE3)  \\ Suppl. Streaming SIMD Ext. 3
 *         UNITEX_HAS_CPU_EXTENSION(SSE4A)  \\ AMD Streaming SIMD Extensions
 *         UNITEX_HAS_CPU_EXTENSION(SSE41)  \\ Streaming SIMD Extensions 4.1
 *         UNITEX_HAS_CPU_EXTENSION(SSE42)  \\ Streaming SIMD Extensions 4.2
 *         UNITEX_HAS_CPU_EXTENSION(SSE5)   \\ Streaming SIMD Extensions 5
 *         UNITEX_HAS_CPU_EXTENSION(AES)    \\ AES Instruction Set
 * @endcode
 *
 * @see    UNITEX_HAS_CPU_EXTENSION_MMX
 * @see    UNITEX_HAS_CPU_EXTENSION_3DNOW
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE2
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE3
 * @see    UNITEX_HAS_CPU_EXTENSION_SSSE3
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE4A
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE41
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE42
 * @see    UNITEX_HAS_CPU_EXTENSION_SSE5
 * @see    UNITEX_HAS_CPU_EXTENSION_AES
 */
#define UNITEX_HAS_CPU_EXTENSION(ExtensionName)\
        (defined(UNITEX_HAS_CPU_EXTENSION_##ExtensionName) &&\
                 UNITEX_HAS_CPU_EXTENSION_##ExtensionName)

/* ************************************************************************** */
#endif  // UNITEX_BASE_CPU_EXTENSIONS_H_                            // NOLINT
