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
 * @file      breakpoint.h
 * @brief     Macro to force a debug trap
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 stringify.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_DEBUG_BREAKPOINT_H_                            // NOLINT
#define UNITEX_BASE_DEBUG_BREAKPOINT_H_                            // NOLINT
/* ************************************************************************** */
#include "base/debug/build_mode.h"    // UNITEX_BUILD_MODE
#include "base/compiler/compliance.h" // UNITEX_COMPILER_COMPLIANT
#include "base/compiler/version.h"    // UNITEX_COMPILER_AT_LEAST
#include "base/cpu/version.h"         // UNITEX_CPU_*
#include "base/macro/helper/decls.h"  // UNITEX_MACRO_DECLS_*
#include "base/os/os.h"               // UNITEX_OS_*
/* ************************************************************************** */
/**
 * @def      UNITEX_DEBUG_BREAKPOINT
 * @brief    Force a debug trap but only in debug mode
 *
 * @code{.cpp}
 *         UNITEX_DEBUG_BREAKPOINT();
 * @endcode
 */
#if UNITEX_BUILD_MODE(DEBUG)
/* ************************************************************************** */
// Microsoft Visual C++
# if    UNITEX_COMPILER_IS(MSVC)
// >= Visual Studio .NET 2003
#  if UNITEX_COMPILER_AT_LEAST(MSVC,13,10)
#   define UNITEX_DEBUG_BREAKPOINT()                \
           UNITEX_MACRO_DECLS_BEGIN                 \
           __debugbreak();                          \
           UNITEX_MACRO_DECLS_END
#  else    // !UNITEX_COMPILER_AT_LEAST(MSVC,13,10)
#   define UNITEX_DEBUG_BREAKPOINT()                \
           UNITEX_MACRO_DECLS_BEGIN                 \
           __emit__(0x90CC);                        \
           UNITEX_MACRO_DECLS_END
#  endif  // UNITEX_COMPILER_AT_LEAST(MSVC,13,10)
/* ************************************************************************** */
// Apple environment
# elif  UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
#  if  UNITEX_OS_UNIX_APPLE_IOS_MODE(SIMULATOR)
// iOS Simulator
#   define UNITEX_DEBUG_BREAKPOINT()                \
           UNITEX_MACRO_DECLS_BEGIN                 \
           __asm__ __volatile__("int 3");           \
           UNITEX_MACRO_DECLS_END
#  elif  UNITEX_OS_UNIX_APPLE_IOS_MODE(DEVICE)
// iOS Device
#   define UNITEX_DEBUG_BREAKPOINT()                \
           UNITEX_MACRO_DECLS_BEGIN                 \
           __asm__("trap");                         \
           UNITEX_MACRO_DECLS_END
#  else
// (Mac) OS X
#   define UNITEX_DEBUG_BREAKPOINT()                \
           UNITEX_MACRO_DECLS_BEGIN                 \
           __asm__("int $3\n" : : );                \
           UNITEX_MACRO_DECLS_END
#  endif  // UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
/* ************************************************************************** */
// Arch: X86, X64, IA64
# elif  UNITEX_CPU_IS(X86)     ||                   \
        UNITEX_CPU_IS(X64)     ||                   \
        UNITEX_CPU_IS(IA64)
#  define UNITEX_DEBUG_BREAKPOINT()                  \
          UNITEX_MACRO_DECLS_BEGIN                   \
          __asm__ __volatile__("int $0x03\n\tnop");  \
          UNITEX_MACRO_DECLS_END
# elif  UNITEX_CPU_IS(PPC64)
// Arch: PPC64
#  define UNITEX_DEBUG_BREAKPOINT()                 \
          UNITEX_MACRO_DECLS_BEGIN                  \
          __asm__ __volatile__("tw 31,1,1");        \
          UNITEX_MACRO_DECLS_END
# elif  UNITEX_CPU_IS(PPC)
// Arch: PPC
#  define UNITEX_DEBUG_BREAKPOINT()                 \
          UNITEX_MACRO_DECLS_BEGIN                  \
          asm("trap");                              \
          UNITEX_MACRO_DECLS_END
# elif  UNITEX_CPU_IS(MIPS)
// Arch: MIPS
#  define UNITEX_DEBUG_BREAKPOINT()                 \
          UNITEX_MACRO_DECLS_BEGIN                  \
          asm("break");                             \
          UNITEX_MACRO_DECLS_END
# elif  UNITEX_CPU_VERSION_AT_LEAST(ARM,5)
// Arch: ARM
#  define UNITEX_DEBUG_BREAKPOINT()                 \
          UNITEX_MACRO_DECLS_BEGIN                  \
          asm("bkpt 0");                            \
          UNITEX_MACRO_DECLS_END
/* ************************************************************************** */
// SIGNAL_H
# elif  UNITEX_OS_IS(UNIX) &&\
        UNITEX_HAVE(SIGNAL_H)
// if single threaded, raise is equivalent to calling
// kill(getpid(), SIGTRAP), otherwise raise is equivalent
// to calling pthread_kill(pthread_self(), SIGTRAP);
# include <signal.h>
#  define UNITEX_DEBUG_BREAKPOINT()                 \
          UNITEX_MACRO_DECLS_BEGIN                  \
          raise(SIGTRAP);                           \
          UNITEX_MACRO_DECLS_END
/* ************************************************************************** */
# else    // UNITEX_COMPILER_IS(MSVC)
# define UNITEX_DEBUG_BREAKPOINT()                  \
         UNITEX_MACRO_DECLS_BEGIN                   \
         UNITEX_MACRO_DECLS_END
#endif   // UNITEX_COMPILER_IS(MSVC)
/* ************************************************************************** */
#else    // UNITEX_BUILD_MODE(RELEASE)
# define UNITEX_DEBUG_BREAKPOINT()        /* nothing */
#endif  // UNITEX_BUILD_MODE(DEBUG)
//  __asm__ __volatile__(".byte 0x90; .byte 0xCC");
/* ************************************************************************** */
#endif  // UNITEX_BASE_DEBUG_BREAKPOINT_H_                         // NOLINT
