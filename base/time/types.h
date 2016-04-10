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
 * @file      types.h
 * @brief     Simple portable time-related types
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 time.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_TIME_TYPES_H_                                   // NOLINT
#define UNITEX_BASE_TIME_TYPES_H_                                   // NOLINT
/* ************************************************************************** */
// Unitex base
#include "base/config.h"            // HAVE_CLOCK_GETTIME
#include "base/compiler/compiler.h" // _FORCE_INLINE, _COMPLIANT, _AT_LEAST
#include "base/integer/integer.h"   // UNITEX_WORDSIZE_IS
#include "base/os/os.h"             // UNITEX_OS_*
#include "base/macro/macro.h"       // UNITEX_UNITEX_DISCARD_UNUSED_PARAMETER
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <assert.h>                // assert
#include <time.h>                  // strftime
#if UNITEX_OS_IS(UNIX)
# include <sys/time.h>
# if UNITEX_OS_UNIX_ENVIRONMENT_IS(MACH)
#  include <mach/clock.h>
#  include <mach/mach.h>
#  define UNITEX_UNIX_FEATURE_MACH_CLOCK_GETTIME
# endif   // UNITEX_OS_UNIX_ENVIRONMENT_IS(MACH)
# elif UNITEX_OS_IS(WINDOWS)
# include <windows.h>
#endif  // UNITEX_OS_IS(UNIX)
/* ************************************************************************** */
// Other libraries' .h files       (try to order the includes alphabetically)
#include "Unicode.h"               // u_* functions            // NOLINT
#include "Ustring.h"               // u_* functions            // NOLINT
#include "UnitexString.h"          // UnitexString class       // NOLINT
/* ************************************************************************** */
#if   UNITEX_OS_IS(UNIX)
// time_unit_t is a signed 64-bit value representing an elapsed time
// we need 64 bits to avoid the Y2K38BUG
typedef int64_t time_unit_t;

#elif UNITEX_OS_IS(WINDOWS)
// time_unit_t is a signed 64-bit value representing an elapsed time
// we need 64 bits to avoid the Y2K38BUG
typedef __time64_t time_unit_t;

// timeval structure is normally declared in winsock2.h, but if you wrap
// windows.h with WIN32_LEAN_AND_MEAN macro then winsock2.h will be not included
#if defined(WIN32_LEAN_AND_MEAN)
#ifndef _TIMEVAL_DEFINED  // prevent double definition (e.g. defined by MinGW64)
struct timeval {
  time_unit_t tv_sec;     // seconds
  time_unit_t tv_usec;    // microseconds, always less than one million
};
#define _TIMEVAL_DEFINED
#endif  // !_TIMEVAL_DEFINED
#endif  // defined(WIN32_LEAN_AND_MEAN)
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
// timeval type
typedef struct timeval  timeval_t;

// timeinfo type
typedef struct ::tm     timeinfo_t;

// strtime type
typedef unitex::unichar strtime_t[26];

// strhour type
typedef unitex::unichar strhour_t[9];
/* ************************************************************************** */
// Literal constant for time_unit_t
#ifndef TIME_UNIT_C
# if UNITEX_WORDSIZE_IS(64)
#   define TIME_UNIT_C(c) c ## L
# else
#   define TIME_UNIT_C(c) c ## LL
# endif
#endif  // !defined(TIME_UNIT_C)
/* ************************************************************************** */
// time_t limits
// TIME_T_MAX is defined as a preprocessor macro in linux/time.h, line 20
#ifndef TIME_T_MAX
// FIXME(martinec) Normally, we can't use sizeof in a preprocessor expression
// GCC/Clang and others accepts this but it's an extension
# define TIME_T_MAX  (time_t)((1UL << ((sizeof(time_t) << 3) - 1)) - 1)
#endif
// Custom TIME_T_MIN based in TIME_T_MAX
#ifndef TIME_T_MIN
# define TIME_T_MIN (0 < (time_t) -1   ?\
                    (time_t) 0 : (time_t) -TIME_T_MAX - (time_t) 1)
#endif
/* ************************************************************************** */
// WIN32 GetSystemTimePreciseAsFileTime function was introduced with
// Windows 8 and Windows Server 2012 (both 0x0602)
#if UNITEX_OS_IS(WINDOWS)
# if UNITEX_OS_WINDOWS_API_AT_LEAST(WIN8)
#  define UNITEX_WIN32_FEATURE_TIME_PRECISE
# endif  // UNITEX_OS_WINDOWS_API_AT_LEAST(WIN8)
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
#endif  // UNITEX_BASE_TIME_TYPES_H_                                // NOLINT
