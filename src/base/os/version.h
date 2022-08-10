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
 * @file      version.h
 * @brief     Macros to detect operating systems types at compile-time
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @attention Please do note, not to change the `UNITEX_OS_*` macros
 *            definition order
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
#ifndef UNITEX_BASE_OS_VERSION_H_                                   // NOLINT
#define UNITEX_BASE_OS_VERSION_H_                                   // NOLINT
/* ************************************************************************** */
#include "base/config.h"            // HAVE_LINUX_VERSION_H
#include "base/macro/helper/test.h" // UNITEX_HAVE
/* ************************************************************************** */
/**
 * @def    UNITEX_OS_CYGWIN
 * @brief  Cygwin environment detection at compile-time
 * @return 1 if we are under a Cygwin environment, otherwise it is undefined
 * @note   Do not use this directly, rather use UNITEX_OS_IS(CYGWIN)
 * @see    UNITEX_OS_IS
 */
#if defined(__CYGWIN__)
// Cygwin OS
# define UNITEX_OS_CYGWIN                            1
# include <cygwin/version.h>  // CYGWIN_VERSION_* // NOLINT
#if defined(_WIN32) || defined(WIN32) || defined (__WIN32__) ||\
    defined(__WINDOWS__)
// - UNITEX_OS_CYGWIN_API_IS(WINDOWS)
# define UNITEX_OS_CYGWIN_API_WINDOWS                1
#else  // Not windows target detected
// - UNITEX_OS_CYGWIN_API_IS(POSIX)
# define UNITEX_OS_CYGWIN_API_POSIX                  1
#endif   // defined(_WIN32) || defined(WIN32) || defined (__WIN32__)
#endif  // defined(__CYGWIN__)
/* ************************************************************************** */
/**
 * @def    UNITEX_OS_UNIX
 * @brief  Unix-like system detection at compile-time
 * @return 1 if we are under a Unix-like system, otherwise it is undefined
 * @note   Do not use this directly, rather use UNITEX_OS_IS(UNIX)
 * @see    UNITEX_OS_IS
 */
#if defined(__unix__)      || defined(__unix)       || defined(unix)          ||\
    defined(BSD)           || defined(__bsdi__)     || defined(__SVR4)        ||\
    defined(_SYSTYPE_SVR4) || defined(__sysv__)     || defined(__svr4__)      ||\
    defined(DARWIN)        || defined(darwin)       || defined(__APPLE__)     ||\
    defined(__MACH__)      || defined(__linux__)    || defined(linux)         ||\
    defined(__linux)       || defined(__gnu_linux__)|| defined(__ANDROID__)   ||\
    defined(_AIX)          || defined(__AIX__)      || defined(__AIX)         ||\
    defined(__hpux)        || defined(hpux)         || defined(_hpux)         ||\
    defined(__FreeBSD__)   || defined(__NetBSD__)   || defined(__OpenBSD__)   ||\
    defined(__DragonFly__) || defined(__sun__)      || defined(sun)           ||\
    defined(__sun)         || defined(__gnu_hurd__) || defined(__gnu_linux__) ||\
    defined(__HAIKU__)     || defined(__MVS__)      || defined(__HOS_MVS__)   ||\
    defined(__TOS_MVS__)   || defined(_MVS)         || defined(ZOS)           ||\
    defined(OS390)         || defined(__pnacl__)    || defined(__native_client__)

// Unix-like OS
# define UNITEX_OS_UNIX                              1

// Test for a Fully/Mostly POSIX-compliant OS
// POSIX compliance isn't trivially detectable by the compiler
// however, if unistd.h is present, this test is harmless
// @see http://en.wikipedia.org/wiki/POSIX
# include <unistd.h>        // _POSIX_VERSION, getpid()
# if defined(_POSIX_VERSION)
#  define UNITEX_OS_COMPLIANT_POSIX                  1
# endif   // defined(_POSIX_VERSION)

// Environment macros below are mainly for diagnostic/printing
// purposes. Note also that these definitions are not mutually
// exclusive

# include <sys/param.h>     // BSD macro

// BSD environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(BSD)
// sys/param.h defines BSD macro in BSD-like systems
# if defined(BSD) || defined(__bsdi__)
#  define UNITEX_OS_UNIX_ENVIRONMENT_BSD             1
# endif   // defined(_POSIX_VERSION)

// System V environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(SYSTEM_V)
# if defined(__SVR4)   || defined(__svr4__)      ||\
     defined(__sysv__) || defined(_SYSTYPE_SVR4)
#  define UNITEX_OS_UNIX_ENVIRONMENT_SYSTEM_V        1
# endif  // defined(__SVR4)   || defined(__svr4__)

// Darwin-based environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(DARWIN)
#if defined(DARWIN) || defined(darwin)
# define UNITEX_OS_UNIX_ENVIRONMENT_DARWIN           1
#endif  //  defined(DARWIN) || defined(darwin)

// Mach kernel based environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(MACH)
// __MACH__  macro is defined whether Mach system calls are supported
#if defined(__MACH__)
# define UNITEX_OS_UNIX_ENVIRONMENT_MACH             1
#endif  // defined(__MACH__)

// Apple's iOS and (Mac) OS X environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
// For a fine-grained control, use :
// - UNITEX_OS_UNIX_IS(APPLE_IOS)
// - UNITEX_OS_UNIX_IS(APPLE_OSX)
// For an extra control level, use :
// - UNITEX_OS_UNIX_APPLE_IOS_MODE(DEVICE)
// - UNITEX_OS_UNIX_APPLE_IOS_MODE(SIMULATOR)
// __APPLE__ macro is defined in any Apple computer
// __MACH__  macro is defined whether Mach system calls are supported
#if defined(__APPLE__) && defined(__MACH__)
# define UNITEX_OS_UNIX_ENVIRONMENT_APPLE            1
# include <TargetConditionals.h>  // TARGET_* macros
// AvailabilityMacros.h is for targeting (Mac) OS X from 10.1
# include <AvailabilityMacros.h>  // MAC_OS_X_VERSION_* macros
# if defined(__IPHONE_OS_MIN_REQUIRED)
// Availability.h (new header) is for targeting iOS and Mac OS X
#  include <Availability.h>       // __OS_X_VERSION_* macros
# endif  // defined(__MAC_OS_X_MIN_REQUIRED)
#endif

// Deployment Target :
// MACOSX_DEPLOYMENT_TARGET
// Environment variable used for (Mac) OS X builds. It was introduced
// in Mac OS X 10.2. If it's not set, Xcode assumes a value of 10.1
// Environment variable used for iOS builds.
// IPHONEOS_DEPLOYMENT_TARGET
// Base SDK :
// Environment variable
// SDKROOT

// Linux kernel based environment
// - UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX)
// For a fine-grained control, use :
// - UNITEX_OS_UNIX_IS(ANDROID)
// - UNITEX_OS_UNIX_IS(LINUX_GNU)
// - UNITEX_OS_UNIX_IS(LINUX_GENERIC)
// __linux__ macro is POSIX compliant, linux and __linux aren't
#if defined(__linux__)   || defined(linux)         ||\
    defined(__linux)     || defined(__gnu_linux__) ||\
    defined(__ANDROID__)
#if  UNITEX_HAVE(LINUX_VERSION_H)
# include <linux/version.h>       // LINUX_VERSION_CODE macro
#endif  //  HAVE_LINUX_VERSION_H
# include <sys/sysinfo.h>         // sysinfo()
# define UNITEX_OS_UNIX_ENVIRONMENT_LINUX            1
#endif  // defined(__linux__)   || defined(linux)

// Unix-like OS compile-time identification
// Macros below are marginally used for conditional compilation purposes.
// Attention! Please do note, not to change the definition order.
// Note also that all definitions are mutually exclusive

// Native Client sandbox for running compiled C and C++ in the browser
// Native Client provides a cross-platform POSIX like environment
# if     defined(__native_client__) ||\
         defined(__pnacl__)
#  define UNITEX_OS_UNIX_NACL                        1
// Native Client sandbox  without Software Fault Isolation (SFI)
// @see http://goo.gl/KOmnPB
#  if   !defined(__native_client_nonsfi__)
#   define UNITEX_OS_UNIX_NACL_SFI                   1
#  else  // defined(__native_client_nonsfi__)
#   define UNITEX_OS_UNIX_NACL_NONSFI                1
#  endif  // !defined(__native_client_nonsfi__)
// Portable Native Client (PNaCl)
#  if    defined(__pnacl__)
#   define UNITEX_OS_UNIX_NACL_PORTABLE              1
#  endif  // !defined(__native_client_nonsfi__)
// Emscripten behaves like a variant of Unix, so the preprocessor defines unix,
// _unix and __unix__ are always present when compiling code with Emscripten
// Note that Native Client sandbox technologies were deprecated on Q4 2019 in
// favor of WebAssembly. WebAssembly (abbreviated Wasm) is a binary instruction
// format for a stack-based virtual machine
# elif   defined(__EMSCRIPTEN__)
# include <emscripten/version.h>    // __EMSCRIPTEN_major__
#   define UNITEX_OS_UNIX_EMSCRIPTEN                 1
// Android
// - UNITEX_OS_UNIX_IS(ANDROID)
# elif   defined(__ANDROID__)
# include <sys/cdefs.h>            // __BIONIC__
# include <android/api-level.h>    // __ANDROID_API__
// # include <android/log.h>
#  define UNITEX_OS_UNIX_ANDROID                     1
// AIX
// - UNITEX_OS_UNIX_IS(AIX)
# elif defined(_AIX)  || defined(__AIX__) ||\
       defined(__AIX)
#  define UNITEX_OS_UNIX_IBM_AIX                     1
// HP-UX
// - UNITEX_OS_UNIX_IS(HP_UX)
# elif defined(__hpux) || defined(hpux) ||\
       defined(_hpux)
#  define  UNITEX_OS_UNIX_HP_UX                      1
// Haiku
// - UNITEX_OS_UNIX_IS(HAIKU)
# elif defined(__HAIKU__)
#  define  UNITEX_OS_UNIX_HAIKU                      1
// z/OS
// - UNITEX_OS_UNIX_IS(ZOS)
# elif defined(__MVS__)     || defined(__HOS_MVS__)  ||\
       defined(__TOS_MVS__) || defined(_MVS)         ||\
       defined(ZOS)
#  define  UNITEX_OS_UNIX_ZOS                        1
// iOS, (Mac) OS X
// - UNITEX_OS_UNIX_IS(APPLE_IOS)
// - UNITEX_OS_UNIX_IS(APPLE_OSX)
# elif defined(UNITEX_OS_UNIX_ENVIRONMENT_APPLE)
// iOS simulator
// - UNITEX_OS_UNIX_APPLE_IOS_MODE(SIMULATOR)
#  if defined(TARGET_IPHONE_SIMULATOR) && \
              TARGET_IPHONE_SIMULATOR
#   define UNITEX_OS_UNIX_APPLE_IOS                  1
#   define UNITEX_OS_UNIX_APPLE_IOS_MODE_SIMULATOR   1
// iOS device
// - UNITEX_OS_UNIX_APPLE_IOS_MODE(DEVICE)
#  elif defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE
#   define UNITEX_OS_UNIX_APPLE_IOS                  1
#   define UNITEX_OS_UNIX_APPLE_IOS_MODE_DEVICE      1
// OS X
// - UNITEX_OS_UNIX_IS(APPLE_OSX)
#  elif defined(TARGET_OS_MAC) && TARGET_OS_MAC
#   define UNITEX_OS_UNIX_APPLE_OSX                  1
#  endif  // defined(TARGET_IPHONE_SIMULATOR)
// GNU/KFreeBSD
// - UNITEX_OS_UNIX_IS(GNU_KFREEBSD)
# elif defined(__FreeBSD_kernel__) && __GLIBC__
#  define UNITEX_OS_UNIX_GNU_KFREEBSD                1
// FreeBSD
// - UNITEX_OS_UNIX_IS(FREEBSD)
# elif defined(__FreeBSD__)
#  define UNITEX_OS_UNIX_FREEBSD                     1
// NetBSD
// - UNITEX_OS_UNIX_IS(NETBSD)
# elif defined(__NetBSD__)
#  define UNITEX_OS_UNIX_NETBSD                      1
// OpenBSD
// - UNITEX_OS_UNIX_IS(OPENBSD)
# elif defined(__OpenBSD__)
#  define UNITEX_OS_UNIX_OPENBSD                     1
// DragonFly
// - UNITEX_OS_UNIX_IS(DRAGONFLY)
# elif defined(__DragonFly__)
#  define UNITEX_OS_UNIX_DRAGONFLY                   1
// Solaris, SunOS
# elif defined(__sun__) || defined(sun) ||\
       defined(__sun)
// Solaris
// - UNITEX_OS_UNIX_IS(SUN_SOLARIS)
#  if    defined(__SVR4) || defined(__svr4__)
#   define UNITEX_OS_UNIX_SUN_SOLARIS                1
#  else    // !(defined(__SVR4) || defined(__svr4__))
// SunOS
// - UNITEX_OS_UNIX_IS(SUN_SUNOS)
#   define UNITEX_OS_UNIX_SUN_SUNOS                  1
#  endif  // defined(__SVR4) || defined(__svr4__)
// GNU Hurd
// - UNITEX_OS_UNIX_IS(GNU_HURD)
# elif defined(__gnu_hurd__)
#  define UNITEX_OS_UNIX_GNU_HURD                    1
// GNU Linux
// - UNITEX_OS_UNIX_IS(LINUX_GNU)
# elif defined(__gnu_linux__)
#   define UNITEX_OS_UNIX_LINUX_GNU                  1
// Linux
// - UNITEX_OS_UNIX_IS(LINUX_GENERIC)
# elif defined(__linux__) || defined(linux) ||\
       defined(__linux)
#  define UNITEX_OS_UNIX_LINUX_GENERIC               1
# else  // Unknown Unix-like system
# error "Please add your Unix-like system identification"
#endif  // defined(__ANDROID__)
#endif  // defined(__unix__) || defined(__unix) || defined(unix)
/* ************************************************************************** */
/**
 * @def    UNITEX_OS_WINDOWS
 * @brief  Windows-based system detection at compile-time
 * @return 1 if we are under a Windows-based system, otherwise it is undefined
 * @note   Do not use this directly, rather use UNITEX_OS_IS(WINDOWS)
 * @see    UNITEX_OS_IS
 */
//  _WIN32     Defined for both 32-bit and 64-bit environments
//   WIN32     Defined by MinGW
// __WIN32__   Defined by Borland C++
// __WINDOWS__ Defined by Watcom C/C++
#if defined(_WIN32)        || defined(WIN32)        || defined (__WIN32__)   ||\
    defined(__WINDOWS__)
# define UNITEX_OS_WINDOWS                           1
// Exclude some of the less frequently used APIs in Windows.h
# ifndef WIN32_LEAN_AND_MEAN
#  define WIN32_LEAN_AND_MEAN
# endif  // !defined(WIN32_LEAN_AND_MEAN)
# include <windows.h>    // Windows-specific header
# include <sdkddkver.h>  // NTDDI_VERSION
#endif  //  defined(_WIN32) || defined(WIN32) || defined (__WIN32__)
/* ************************************************************************** */
/**
 * @brief  Test for an OS name at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_IS(CYGWIN)
 *         UNITEX_OS_IS(UNIX)
 *         UNITEX_OS_IS(WINDOWS)
 * @endcode
 *
 * @see    UNITEX_OS_CYGWIN
 * @see    UNITEX_OS_UNIX
 * @see    UNITEX_OS_WINDOWS
 */
#define UNITEX_OS_IS(OSName)            \
        (UNITEX_OS_##OSName == 1)
/* ************************************************************************** */
/**
 * @brief  Test for an OS name at compile-time (NOT)
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_IS_NOT(CYGWIN)
 *         UNITEX_OS_IS_NOT(UNIX)
 *         UNITEX_OS_IS_NOT(UNIX)
 * @endcode
 *
 * @see    UNITEX_OS_IS
 */
#define UNITEX_OS_IS_NOT(OSName) \
        (!UNITEX_OS_IS(OSName))
/* ************************************************************************** */
/**
 * @brief  Test for a Unix-like system at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_UNIX_IS(ANDROID)
 *         UNITEX_OS_UNIX_IS(APPLE_IOS)
 *         UNITEX_OS_UNIX_IS(APPLE_OSX)
 *         UNITEX_OS_UNIX_IS(DRAGONFLY)
 *         UNITEX_OS_UNIX_IS(FREEBSD)
 *         UNITEX_OS_UNIX_IS(GNU_HURD)
 *         UNITEX_OS_UNIX_IS(GNU_KFREEBSD)
 *         UNITEX_OS_UNIX_IS(HAIKU)
 *         UNITEX_OS_UNIX_IS(HP_UX)
 *         UNITEX_OS_UNIX_IS(IBM_AIX)
 *         UNITEX_OS_UNIX_IS(LINUX_GENERIC)
 *         UNITEX_OS_UNIX_IS(LINUX_GNU)
  *        UNITEX_OS_UNIX_IS(NACL)
 *         UNITEX_OS_UNIX_IS(NETBSD)
 *         UNITEX_OS_UNIX_IS(OPENBSD)
 *         UNITEX_OS_UNIX_IS(SUN_SOLARIS)
 *         UNITEX_OS_UNIX_IS(SUN_SUNOS)
 *         UNITEX_OS_UNIX_IS(ZOS)
 * @endcode
 *
 * @note   Definitions are mutually exclusive
 *
 * @see    UNITEX_OS_UNIX
 */
# define UNITEX_OS_UNIX_IS(UnixName)           \
        (UNITEX_OS_UNIX_##UnixName == 1)
/* ************************************************************************** */
/**
 * @brief  Test for a Unix-like system at compile-time (NOT)
 *
 * @see    UNITEX_OS_UNIX_IS
 */
#define UNITEX_OS_UNIX_IS_NOT(UnixName)\
               (!UNITEX_OS_UNIX_IS(UnixName))
/* ************************************************************************** */
/**
 * @brief  Test Apple's IOS mode at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_UNIX_APPLE_IOS_MODE(DEVICE)
 *         UNITEX_OS_UNIX_APPLE_IOS_MODE(SIMULATOR)
 * @endcode
 *
 * @see    UNITEX_OS_UNIX_APPLE_IOS_MODE_DEVICE
 * @see    UNITEX_OS_UNIX_APPLE_IOS_MODE_SIMULATOR
 */
# define UNITEX_OS_UNIX_APPLE_IOS_MODE(ModeName)\
        (UNITEX_OS_UNIX_APPLE_IOS_MODE_##ModeName == 1)
/* ************************************************************************** */
/**
 * @brief  Test Cygwin's target API type at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_CYGWIN_API_IS(WINDOWS)
 *         UNITEX_OS_CYGWIN_API_IS(POSIX)
 * @endcode
 *
 * @see    UNITEX_OS_CYGWIN_API_WINDOWS
 * @see    UNITEX_OS_CYGWIN_API_POSIX
 */
#if UNITEX_OS_IS(CYGWIN)
#define UNITEX_OS_CYGWIN_API_IS(ApiName)
        (UNITEX_OS_CYGWIN_API_##ApiName == 1)
#else     // Expression syntax error in non-Windows target systems
#define UNITEX_OS_CYGWIN_API_IS(ApiName)             /* nothing */
#endif  //  UNITEX_OS_IS(CYGWIN)
/* ************************************************************************** */
/**
 * @brief  Test for a Unix-like environment at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(BSD)
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(DARWIN)
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX)
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(MACH)
 *         UNITEX_OS_UNIX_ENVIRONMENT_IS(SYSTEM_V)
 * @endcode
 *
 * @note   Tests are not mutually exclusive
 */
#define UNITEX_OS_UNIX_ENVIRONMENT_IS(EnvironmentName)            \
        (UNITEX_OS_UNIX_ENVIRONMENT_##EnvironmentName == 1)
/* ************************************************************************** */
/**
 * @brief  Test for a Unix-like environment at compile-time
 *
 * @see    UNITEX_OS_UNIX_ENVIRONMENT_IS
 */
// UNITEX_OS_UNIX_ENVIRONMENT_IS_NOT
#define UNITEX_OS_UNIX_ENVIRONMENT_IS_NOT(EnvironmentName)       \
        (!UNITEX_OS_UNIX_ENVIRONMENT_IS(EnvironmentName))
/* ************************************************************************** */
// Windows OS target API version constants
// To check the various version numbers of Windows
// @see http://msdn.microsoft.com/en-us/library/ms724833.aspx
#if UNITEX_OS_IS(WINDOWS)
# define UNITEX_OS_WINDOWS_API_NT4                   0x0400
# define UNITEX_OS_WINDOWS_API_NT4E                  0x0401
# define UNITEX_OS_WINDOWS_API_WIN95                 0x0400
# define UNITEX_OS_WINDOWS_API_WIN98                 0x0410
# define UNITEX_OS_WINDOWS_API_WINME                 0x0490
# define UNITEX_OS_WINDOWS_API_WIN2K                 0x0500
# define UNITEX_OS_WINDOWS_API_WINXP                 0x0501
# define UNITEX_OS_WINDOWS_API_WS03                  0x0502
# define UNITEX_OS_WINDOWS_API_WIN6                  0x0600
# define UNITEX_OS_WINDOWS_API_VISTA                 0x0600
# define UNITEX_OS_WINDOWS_API_LONGORN               0x0600
# define UNITEX_OS_WINDOWS_API_WS08                  0x0600
# define UNITEX_OS_WINDOWS_API_WIN7                  0x0601
# define UNITEX_OS_WINDOWS_API_WIN8                  0x0602
# define UNITEX_OS_WINDOWS_API_WS12                  0x0602
# define UNITEX_OS_WINDOWS_API_WIN81                 0x0603
# define UNITEX_OS_WINDOWS_API_WINBLUE               0x0603
# define UNITEX_OS_WINDOWS_API_WS12R2                0x0603
# define UNITEX_OS_WINDOWS_API_WIN10                 0x0604
#endif  // define UNITEX_OS_WINDOWS
/* ************************************************************************** */
/**
 * @brief  Test Windows' target API type at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_WINDOWS_API_IS(LONGORN)
 *         UNITEX_OS_WINDOWS_API_IS(NT4E)
 *         UNITEX_OS_WINDOWS_API_IS(VISTA)
 *         UNITEX_OS_WINDOWS_API_IS(WIN2K)
 *         UNITEX_OS_WINDOWS_API_IS(WIN6)
 *         UNITEX_OS_WINDOWS_API_IS(WIN7)
 *         UNITEX_OS_WINDOWS_API_IS(WIN8)
 *         UNITEX_OS_WINDOWS_API_IS(WIN81)
 *         UNITEX_OS_WINDOWS_API_IS(WIN10)
 *         UNITEX_OS_WINDOWS_API_IS(WIN95)
 *         UNITEX_OS_WINDOWS_API_IS(WIN98)
 *         UNITEX_OS_WINDOWS_API_IS(WINBLUE)
 *         UNITEX_OS_WINDOWS_API_IS(WINME)
 *         UNITEX_OS_WINDOWS_API_IS(WINXP)
 *         UNITEX_OS_WINDOWS_API_IS(WS03)
 *         UNITEX_OS_WINDOWS_API_IS(WS08)
 *         UNITEX_OS_WINDOWS_API_IS(WS12)
 * @endcode
 */
#if UNITEX_OS_IS(WINDOWS)
#define UNITEX_OS_WINDOWS_API_IS(VersionName) \
     (_WIN32_WINNT == (UNITEX_OS_WINDOWS_API_##VersionName))
#else     // Expression syntax error in non-Windows target systems
#define UNITEX_OS_WINDOWS_API_IS(VersionName)        /* nothing */
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
/**
 * @brief  Test Windows' target API version and service pack number at
 *         compile-time
 *
 * Some examples of supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_WINDOWS_API_VERSION_IS(WINXP,1)
 *         UNITEX_OS_WINDOWS_API_VERSION_IS(VISTA,2)
 * @endcode
 */
#if UNITEX_OS_IS(WINDOWS)
#define UNITEX_OS_WINDOWS_API_VERSION_IS(VersionName, ServicePackNumber) \
(NTDDI_VERSION == ((UNITEX_OS_WINDOWS_API_##VersionName * 0x00010000)\
                 + (0x##ServicePackNumber * 0x100)))
#else     // Expression syntax error in non-Windows target systems
#define UNITEX_OS_WINDOWS_API_VERSION_IS(VersionName, \
                                            ServicePackNumber) /* nothing */
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
/**
 * @brief  Test for a Windows' target API at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(LONGORN)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(NT4E)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(VISTA)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN2K)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN6)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN7)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN8)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN81)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN10)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN95)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WIN98)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WINBLUE)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WINME)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WINXP)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WS03)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WS08)
 *         UNITEX_OS_WINDOWS_API_AT_LEAST(WS12)
 * @endcode
 *
 * @see    UNITEX_OS_WINDOWS_API_IS
 */
#if UNITEX_OS_IS(WINDOWS)
#define UNITEX_OS_WINDOWS_API_AT_LEAST(VersionName) \
     (_WIN32_WINNT >= (UNITEX_OS_WINDOWS_API_##VersionName))
#else     // Expression syntax error in non-Windows target systems
# define UNITEX_OS_WINDOWS_API_AT_LEAST(VersionName) /* nothing */
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
/**
 * @brief  Test Windows' target API version and service pack number at
 *         compile-time
 *
 * Some examples of supported expressions are:
 * @code{.cpp}
 *         UNITEX_OS_WINDOWS_API_VERSION_AT_LEAST(WINXP,1)
 *         UNITEX_OS_WINDOWS_API_VERSION_AT_LEAST(VISTA,2)
 * @endcode
 */
#if UNITEX_OS_IS(WINDOWS)
# define UNITEX_OS_WINDOWS_API_VERSION_AT_LEAST(VersionName, \
                                                   ServicePackNumber) \
(NTDDI_VERSION >= ((UNITEX_OS_WINDOWS_API_##VersionName * 0x00010000)\
                 + (0x##ServicePackNumber * 0x100)))
#else     // Expression syntax error in non-Windows target systems
# define UNITEX_OS_WINDOWS_API_VERSION_AT_LEAST(VersionName, \
                                                ServicePackNumber)
                                                               /* nothing */
#endif  // UNITEX_OS_IS(WINDOWS)
/* ************************************************************************** */
#if UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
// Mac OS X 10.0 to 10.7, OS X 10.8 and further
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_0            1000
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_1            1010
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_2            1020
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_3            1030
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_4            1040
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_5            1050
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_6            1060
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_7            1070
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_8            1080
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_9            1090
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_10           1010
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_10_11           1011

// Mac OS X 10.0 to 10.7, OS X 10.8 to 10.10 aliases
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_CHEETAH         1000
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_PUMA            1010
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_JAGUAR          1020
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_PANTHER         1030
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_TIGER           1040
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_LEOPARD         1050
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_SNOW_LEOPARD    1060
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_LION            1070
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_MOUNTAIN_LION   1080
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_MAVERICKS       1090
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_YOSEMITE        1010
#define UNITEX_OS_UNIX_APPLE_SDK_OSX_ELCAPITAN       1010

// iPhone OS 1.x, 2.x, 3.x and iOS 4.x to 9.x
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_2_0             20000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_2_1             20100
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_2_2             20200
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_3_0             30000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_3_1             30100
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_3_2             30200
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_4_0             40000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_4_1             40100
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_4_2             40200
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_4_3             40300
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_5_0             50000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_5_1             50100
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_6_0             60000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_6_1             60100
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_7_0             70000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_8_0             80000
#define UNITEX_OS_UNIX_APPLE_SDK_IOS_9_0             90000
#endif  // UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
/* ************************************************************************** */
/**
 * @def    UNITEX_OS_NAME
 * @brief  Set a target OS name string constant at compile-time
 * @note   For a run-time version use instead config::host::operating_system_name
 * @see    config::target::operating_system_name
 */
#if   UNITEX_OS_IS(CYGWIN)
# if    UNITEX_OS_CYGWIN_API_IS(WINDOWS)
#  define UNITEX_OS_NAME  "cygwin.windows"
# elif UNITEX_OS_CYGWIN_API_IS(POSIX)
#  define UNITEX_OS_NAME  "cygwin.posix"
# else  // This isn't supposed to happen
#  define UNITEX_OS_NAME  "cygwin.<unknown>"
# endif  // UNITEX_OS_UNIX_ENVIRONMENT_IS(CYGWIN_WINDOWS)
#elif UNITEX_OS_IS(UNIX)
# if   UNITEX_OS_UNIX_IS(ANDROID)
#  define UNITEX_OS_NAME  "unix.android"
# elif UNITEX_OS_UNIX_IS(APPLE_IOS_DEVICE)
#  define UNITEX_OS_NAME  "unix.apple.ios.device"
# elif UNITEX_OS_UNIX_IS(APPLE_IOS_SIMULATOR)
#  define UNITEX_OS_NAME  "unix.apple.ios.simulator"
# elif UNITEX_OS_UNIX_IS(APPLE_OSX)
#  define UNITEX_OS_NAME  "unix.apple.osx"
# elif UNITEX_OS_UNIX_IS(DRAGONFLY)
#  define UNITEX_OS_NAME  "unix.dragonfly"
# elif UNITEX_OS_UNIX_IS(EMSCRIPTEN)
#  define UNITEX_OS_NAME  "unix.emscripten"
# elif UNITEX_OS_UNIX_IS(FREEBSD)
#  define UNITEX_OS_NAME  "unix.freebsd"
# elif UNITEX_OS_UNIX_IS(GNU_HURD)
#  define UNITEX_OS_NAME  "unix.gnu_hurd"
# elif UNITEX_OS_UNIX_IS(GNU_KFREEBSD)
#  define UNITEX_OS_NAME  "unix.gnu_kfreebsd"
# elif UNITEX_OS_UNIX_IS(HAIKU)
#  define UNITEX_OS_NAME  "unix.haiku"
# elif UNITEX_OS_UNIX_IS(HP_UX)
#  define UNITEX_OS_NAME  "unix.hpux"
# elif UNITEX_OS_UNIX_IS(IBM_AIX)
#  define UNITEX_OS_NAME  "unix.ibm.aix"
# elif UNITEX_OS_UNIX_IS(LINUX_GENERIC)
#  define UNITEX_OS_NAME  "unix.linux.generic"
# elif UNITEX_OS_UNIX_IS(LINUX_GNU)
#  define UNITEX_OS_NAME  "unix.gnu_linux"
# elif UNITEX_OS_UNIX_IS(NACL)
#  define UNITEX_OS_NAME  "unix.native_client"
# elif UNITEX_OS_UNIX_IS(NETBSD)
#  define UNITEX_OS_NAME  "unix.netbsd"
# elif UNITEX_OS_UNIX_IS(OPENBSD)
#  define UNITEX_OS_NAME  "unix.openbsd"
# elif UNITEX_OS_UNIX_IS(SUN_SOLARIS)
#  define UNITEX_OS_NAME  "unix.sun.solaris"
# elif UNITEX_OS_UNIX_IS(SUN_SUNOS)
#  define UNITEX_OS_NAME  "unix.sun.sunos"
# elif UNITEX_OS_UNIX_IS(ZOS)
#  define UNITEX_OS_NAME  "unix.zos"
# else  // Unknown Unix-like OS
#  define UNITEX_OS_NAME  "unix.<unknown>"
#endif  // UNITEX_OS_UNIX_IS(ANDROID)
#elif UNITEX_OS_IS(WINDOWS)
# define UNITEX_OS_NAME   "windows"
#else    // Unknown OS
# define UNITEX_OS_NAME   "<unknown>"
#endif  // UNITEX_OS_IS(CYGWIN)
/* ************************************************************************** */
#endif  // UNITEX_BASE_OS_VERSION_H_                                // NOLINT
