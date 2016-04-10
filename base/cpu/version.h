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
 * @file      version.h
 * @brief     Macros to detect a CPU architecture at compile-time
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 version.h`
 *
 * @date      December 2014
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_CPU_VERSION_H_                                  // NOLINT
#define UNITEX_BASE_CPU_VERSION_H_                                  // NOLINT
/* ************************************************************************** */
// UNITEX_CPU_IS(ALPHA)
#if defined(__alpha__) || defined(__alpha)  || defined(_M_ALPHA)
# define UNITEX_CPU_ALPHA                            1
#endif

// UNITEX_CPU_IS(X64)
#if defined(__x86_64__)|| defined(__x86_64) || defined(_M_X64)    ||\
    defined(_M_AMD64)  || defined(__amd64__)|| defined(__amd64)
# define UNITEX_CPU_X64                              1
#endif

// UNITEX_CPU_IS(X86)
#if defined(i386)      || defined(__i386)   || defined(__i386__)  ||\
    defined(_M_IX86)   || defined(_X86_)    || defined(__INTEL__)
# define UNITEX_CPU_X86                              1
#endif

// UNITEX_CPU_IS(ARM64)
#if defined(__aarch64__)
# define UNITEX_CPU_ARM64                            1
#endif  //  defined(__aarch64__)

// UNITEX_CPU_IS(ARM)
// UNITEX_CPU_ARM_THUMB_MODE could be :
// not available (0) version 1 (1) or version 2 (2)
#if defined(__arm__)                 || defined(__thumb__)              ||\
    defined(_M_ARM)                  || defined(_M_ARMT)
# define UNITEX_CPU_ARM                              1
// ARM v8
# if defined(__ARM_ARCH_8__)         || (defined(_M_ARM) && _M_ARM >= 8)
#  error "Found an ARM architecture newer than v7, you need to update this code"
// UNITEX_CPU_VERSION_AT_LEAST_ARM(8)
#  define UNITEX_CPU_VERSION_AT_LEAST_ARM_8          1
# endif  // ARM v8
// ARM v7
# if defined(__ARM_ARCH_7__ )        ||  defined(__ARM_ARCH_7A__)       ||\
     defined(__ARM_ARCH_7R__)        ||  defined(__ARM_ARCH_7M__)       ||\
     defined(__ARM_ARCH_7S__)        ||                                   \
    (defined(_M_ARM) && _M_ARM >= 7) ||  defined(_M_ARMT)               ||\
     defined(UNITEX_CPU_VERSION_AT_LEAST_ARM_8)
// UNITEX_CPU_VERSION_AT_LEAST_ARM(7)
#  define UNITEX_CPU_VERSION_AT_LEAST_ARM_7          1
# endif  // ARM v7
// ARM v6
# if defined(__ARM_ARCH_6__)         ||  defined(__ARM_ARCH_6J__)       ||\
     defined(__ARM_ARCH_6K__)        ||  defined(__ARM_ARCH_6Z__)       ||\
     defined(__ARM_ARCH_6ZK__)       ||  defined(__ARM_ARCH_6T2__)      ||\
    (defined(_M_ARM) && _M_ARM >= 6) ||  defined(_M_ARMT)               ||\
     defined(UNITEX_CPU_VERSION_AT_LEAST_ARM_7)
// UNITEX_CPU_VERSION_AT_LEAST_ARM(6)
#  define UNITEX_CPU_VERSION_AT_LEAST_ARM_6          1
#  if defined(__ARM_ARCH_6T2__) || defined(_M_ARMT)
#   if !defined(UNITEX_CPU_ARM_THUMB_MODE)   ||\
       (defined(UNITEX_CPU_ARM_THUMB_MODE)   &&\
                UNITEX_CPU_ARM_THUMB_MODE < 2)
#     define UNITEX_CPU_ARM_THUMB_MODE               2
#   endif  // !defined(UNITEX_CPU_ARM_THUMB_MODE)
#  endif  // defined(__ARM_ARCH_6T2__)
# endif  // ARM v6
// ARM v5
# if defined(__ARM_ARCH_5__  )       ||  defined(__ARM_ARCH_5E__)       ||\
     defined(__ARM_ARCH_5T__)        ||  defined(__ARM_ARCH_5TE__)      ||\
     defined(__ARM_ARCH_5TEJ__)      ||\
    (defined(_M_ARM) && _M_ARM >= 5) ||  defined(_M_ARMT)               ||\
     defined(UNITEX_CPU_VERSION_AT_LEAST_ARM_6)
// UNITEX_CPU_VERSION_AT_LEAST_ARM(5)
#  define UNITEX_CPU_VERSION_AT_LEAST_ARM_5          1
#  if  defined(__ARM_ARCH_5T__)   ||  defined(__ARM_ARCH_5TE__) ||\
       defined(__ARM_ARCH_5TEJ__) ||  defined(_M_ARMT))
#   if !defined(UNITEX_CPU_ARM_THUMB_MODE)   ||\
       (defined(UNITEX_CPU_ARM_THUMB_MODE)   &&\
                UNITEX_CPU_ARM_THUMB_MODE < 1)
#     define UNITEX_CPU_ARM_THUMB_MODE               1
#   endif  // !defined(UNITEX_CPU_ARM_THUMB_MODE)
#  endif  // defined(__ARM_ARCH_6T2__)
# endif  // ARM v5
// Don't use thumb instructions
# if !defined(UNITEX_CPU_ARM_THUMB_MODE)
#  define UNITEX_CPU_ARM_THUMB_MODE                  0
# endif  // !defined(UNITEX_CPU_ARM_THUMB_MODE)
#endif  // defined(__arm__) || defined(__thumb__)

// UNITEX_CPU_IS(EPIPHANY)
#if defined(__epiphany__)
# define UNITEX_CPU_EPIPHANY                         1
#endif  // defined(__epiphany__)

// UNITEX_CPU_IS(IA64)
#if defined(__ia64__) || defined(_M_IA64)
# define UNITEX_CPU_IA64                             1
#endif  // defined(__ia64__)

// UNITEX_CPU_IS(M68K)
#if defined(__m68k__)
# define UNITEX_CPU_M68K                             1
#endif  // defined(__m68k__)

// UNITEX_CPU_IS(MIPS)
#if defined(__mips__)
# define UNITEX_CPU_MIPS                             1
#endif  // defined(__mips__)

// UNITEX_CPU_IS(PPC64)
#if defined (__powerpc64__) || defined (__ppc64__)
# define UNITEX_CPU_PPC64                            1
// !!BIG_ENDIAN
// !!64
#endif  // defined (__powerpc64__)

// UNITEX_CPU_IS(PPC)
#if defined(__powerpc) || defined(__POWERPC__)  ||\
    defined(_M_MPPC)   || defined(__PPC__) || defined(__ppc__)
# define UNITEX_CPU_PPC                              1
// !!BIG_ENDIAN
#endif  // defined(__powerpc)

// UNITEX_CPU_IS(SPARC)
#if defined(__sparc__)
# define UNITEX_CPU_SPARC                            1
#endif  // defined(__sparc__)
/* ************************************************************************** */
/**
 * @brief  Test for a CPU architecture at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_CPU_IS(ALPHA)
 *         UNITEX_CPU_IS(X86)
 *         UNITEX_CPU_IS(X64)
 *         UNITEX_CPU_IS(ARM)
 *         UNITEX_CPU_IS(ARM64)
 *         UNITEX_CPU_IS(EPIPHANY)
 *         UNITEX_CPU_IS(IA64)
 *         UNITEX_CPU_IS(M68K)
 *         UNITEX_CPU_IS(MIPS)
 *         UNITEX_CPU_IS(PPC)
 *         UNITEX_CPU_IS(PPC64)
 *         UNITEX_CPU_IS(SPARC)
 * @endcode
 */
# define UNITEX_CPU_IS(CpuName)           \
        (defined(UNITEX_CPU_##CpuName) && \
                 UNITEX_CPU_##CpuName)
/* ************************************************************************** */
/**
 * @brief  Test for an ARM CPU version at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_CPU_VERSION_AT_LEAST_ARM(8)
 *         UNITEX_CPU_VERSION_AT_LEAST_ARM(7)
 *         UNITEX_CPU_VERSION_AT_LEAST_ARM(6)
 *         UNITEX_CPU_VERSION_AT_LEAST_ARM(5)
 * @endcode
 *
 * @note   ARM Versions lower than 5 are neither identified nor supported
 */
#define UNITEX_CPU_VERSION_AT_LEAST_ARM(VersionName) \
  defined(UNITEX_CPU_VERSION_AT_LEAST_ARM_##VersionName) &&\
          UNITEX_CPU_VERSION_AT_LEAST_ARM_##VersionName
/* ************************************************************************** */
/**
 * @brief  Test for a CPU version at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_CPU_VERSION_AT_LEAST(ARM,8)
 *         UNITEX_CPU_VERSION_AT_LEAST(ARM,7)
 *         UNITEX_CPU_VERSION_AT_LEAST(ARM,6)
 *         UNITEX_CPU_VERSION_AT_LEAST(ARM,5)
 * @endcode
 *
 * @see    UNITEX_CPU_VERSION_AT_LEAST_ARM
 */
#define UNITEX_CPU_VERSION_AT_LEAST(CpuName, VersionName) \
       (UNITEX_CPU_VERSION_AT_LEAST_##CpuName(VersionName))
/* ************************************************************************** */
#endif  // UNITEX_BASE_CPU_VERSION_H_                               // NOLINT
