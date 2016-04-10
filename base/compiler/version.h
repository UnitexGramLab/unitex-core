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
 * @brief     Macros to detect, normalize and test compilers versions
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/base.h
 *            header file to gain this file's functionality
 *
 * @attention Please do note, not to change the macros definition order
 *
 * @note      Compiler versions are normalized to the integer form VVRRPPPPP
 *            where VV is the major version, RR is a 2-digit minor version, and
 *            PPPPP represents a 5-digit build or patch level
 *
 * @note      COMPILER_AT_LEAST macros have two versions, one finishing by
 *            underscore (_) that requires three arguments (major,minor,patch),
 *            another, without underscore, needing only two args (mayor,minor)
 *            these alternative version uses 0 as patch or build number.
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
#ifndef UNITEX_BASE_COMPILER_VERSION_H_                             // NOLINT
#define UNITEX_BASE_COMPILER_VERSION_H_                             // NOLINT
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <stdlib.h>                // NULL
/* ************************************************************************** */
// Project's .h files              (try to order the includes alphabetically)
#include "base/os/os.h"            // WINDOWS_API_AT_LEAST_*
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_MINGW
 * @brief    MinGW compiler : Minimalist GNU C/C++ compiler version

 * @details  Set UNITEX_COMPILER_MINGW as VVRR00000 (Version, Release, 00000)
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if defined(__MINGW32__) || defined(__MINGW64__)
# define UNITEX_COMPILER_IS_MINGW   1
// When compiling with MinGW request at least Windows XP or Windows Server 2003
# if !UNITEX_OS_WINDOWS_API_AT_LEAST(WS03)
#  undef  WINVER        /* undef */
#  define WINVER        UNITEX_OS_WINDOWS_API_WS03
#  undef  _WIN32_WINNT  /* undef */
#  define _WIN32_WINNT  UNITEX_OS_WINDOWS_API_WS03
#  undef  NTDDI_VERSION /* undef */
#  define NTDDI_VERSION UNITEX_OS_WINDOWS_API_WS03 * 0x00010000
# endif  // !UNITEX_OS_WINDOWS_API_AT_LEAST(WS03)
# include <_mingw.h>  // __MINGW32_*_VERSION macros
# if   defined(__MINGW32__) &&\
       defined(__MINGW32_MAJOR_VERSION) &&\
       defined(__MINGW32_MINOR_VERSION)
#  define UNITEX_COMPILER_IS_MINGW32   1
#  undef  UNITEX_COMPILER_MINGW /* undef */
#  define UNITEX_COMPILER_MINGW (__MINGW32_MAJOR_VERSION * 10000000\
                               + __MINGW32_MINOR_VERSION * 100000)
# endif  // defined(__MINGW32__)
// Note that MinGW-w64 defines both __MINGW32__ and __MINGW64__, if needed,
// we also set UNITEX_COMPILER_IS_MINGW32 and UNITEX_COMPILER_IS_MINGW64
# if   defined(__MINGW64__) &&\
       defined(__MINGW64_MAJOR_VERSION) &&\
       defined(__MINGW64_MINOR_VERSION)
#  define UNITEX_COMPILER_IS_MINGW64   1
#  undef  UNITEX_COMPILER_MINGW /* undef */
#  define UNITEX_COMPILER_MINGW (__MINGW64_MAJOR_VERSION * 10000000\
                               + __MINGW64_MINOR_VERSION * 100000)
# endif
#endif  // defined (__MINGW32__) || defined (__MINGW64__)
/* ************************************************************************** */
/**
 * @brief  Test MinGW version in the format (version, release, patch)
 * @return true if MinGW version is at least (version, release, patch)
 */
// UNITEX_COMPILER_AT_LEAST_MINGW(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_MINGW_(v, r, p) \
         (defined(UNITEX_COMPILER_MINGW) &&\
             UNITEX_COMPILER_MINGW >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test MinGW version in the format (version, release)
 * @return true if MinGW version is at least (version, release, 0)
 * @see    UNITEX_COMPILER_AT_LEAST_MINGW_
 */
// UNITEX_COMPILER_AT_LEAST_MINGW(version, release)
# define UNITEX_COMPILER_AT_LEAST_MINGW(v, r) \
         UNITEX_COMPILER_AT_LEAST_MINGW_(v, r, 0)
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_CLANG
 * @brief    Clang : LLVM native C/C++ compiler

 * @details  Set UNITEX_COMPILER_CLANG as VVRR000PP (Version, Release, Patch)
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if defined(__clang__)
#  define UNITEX_COMPILER_IS_CLANG        1
#  define UNITEX_COMPILER_CLANG   (__clang_major__       * 10000000 \
                                 + __clang_minor__       * 100000   \
                                 + __clang_patchlevel__  * 1)
// Take notice that Apple's Clang distribution bundled with XCode may rewrite
// __clang_major__ and __clang_minor__ open source Clang distribution' values
#if defined(__apple_build_version__)
// UNITEX_COMPILER_IS_CLANG_APPLE
#  define UNITEX_COMPILER_IS_CLANG_APPLE  1
// __APPLE_CC__   // distinguish  compilers based on the same version of GCC
// __APPLE_CPP__  //
// UNITEX_COMPILER_CLANG_APPLE [VVRRPPPPP] (Version, Release, Patch)
// e.g. if __apple_build_version__ is 4250028 [VRPPPPP]
// UNITEX_COMPILER_CLANG_APPLE will be 040250028 [VVRRPPPPP]
#  define UNITEX_COMPILER_CLANG_APPLE\
                      (__apple_build_version__            / 1000000) * 10000000\
                   + ((__apple_build_version__ % 1000000) /  100000) *   100000\
                   + ((__apple_build_version__ %  100000) /       1) *        1
#endif   // defined(__apple_build_version__)
#endif  // defined(__clang__)
/* ************************************************************************** */
/**
 * @brief  Test Clang version in the format (version, release, patch)
 * @return true if the Clang version is at least (version, release, patch)
 */
// UNITEX_COMPILER_AT_LEAST_CLANG_(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_CLANG_(v, r, p) \
         (defined(UNITEX_COMPILER_CLANG) &&\
                  UNITEX_COMPILER_CLANG >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test Clang version in the format (version, release)
 * @return true if Clang version is at least (version, release, 0)
 *
 * @code{.cpp}
 *         UNITEX_COMPILER_AT_LEAST_CLANG(2,9)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,0)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,1)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,2)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,3)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,4)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,5)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,6)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,7)
 *         UNITEX_COMPILER_AT_LEAST_CLANG(3,8)
 * @endcode
 *
 * @see    UNITEX_COMPILER_AT_LEAST_CLANG_
 */
// UNITEX_COMPILER_AT_LEAST_CLANG(version, release)
# define UNITEX_COMPILER_AT_LEAST_CLANG(v, r) \
         UNITEX_COMPILER_AT_LEAST_CLANG_(v, r, 0)
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_GCC
 * @brief    GNU C/C++ conformant compiler

 * @details  Set UNITEX_COMPILER_GCC as VVRR000PP (Version, Release, Patch)
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if defined(__GNUC__)
// Notice that _GNUC__ could be also defined by CLANG, INTEL, MINGW and by
// others compilers
# define UNITEX_COMPILER_IS_GCC     1
# if defined(__GNUC_PATCHLEVEL__)
#  define UNITEX_COMPILER_GCC    (__GNUC__            * 10000000  \
                                + __GNUC_MINOR__      * 100000    \
                                + __GNUC_PATCHLEVEL__ * 1)
# else  // !defined(__GNUC_PATCHLEVEL__)
#  define UNITEX_COMPILER_GCC    (__GNUC__            * 10000000  \
                                + __GNUC_MINOR__      * 100000)
# endif  // defined(__GNUC_PATCHLEVEL__)
#endif  // defined(__GNUC__)
/* ************************************************************************** */
/**
 * @brief  Test GNU C/C++ version in the format (version, release, patch)
 * @return true if the GNU C/C++ version is at least (version, release, patch)
 */
// UNITEX_COMPILER_AT_LEAST_GCC_(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_GCC_(v, r, p) \
         (defined(UNITEX_COMPILER_GCC) &&\
                  UNITEX_COMPILER_GCC >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test GNU C/C++ version in the format (version, release)
 * @return true if the GNU C/C++ version is at least (version, release, 0)
 * @see    UNITEX_COMPILER_AT_LEAST_MINGW_
 */
// UNITEX_COMPILER_AT_LEAST_GCC(version, release)
# define UNITEX_COMPILER_AT_LEAST_GCC(v, r) \
         UNITEX_COMPILER_AT_LEAST_GCC_(v, r, 0)
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_MSVC
 * @brief    Microsoft Visual C++ compiler

 * @details  Set UNITEX_COMPILER_MSVC as VVRRPPPPP (Version, Release, Patch)
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if defined(_MSC_VER)
# define UNITEX_COMPILER_IS_MSVC    1
# if defined(_MSC_FULL_VER)
#  if   _MSC_VER < 1400
// _MSC_FULL_VER is VVRRPPPP (4-digits build number), transform to VVRR0PPPP
#     define UNITEX_COMPILER_MSVC  ((_MSC_FULL_VER / 10000)  * 100000 \
                                  + (_MSC_FULL_VER % 10000))
#  else  // _MSC_VER >= 1400
// _MSC_FULL_VER is already VVRRPPPPP (5-digits build number)
#     define UNITEX_COMPILER_MSVC   _MSC_FULL_VER
#  endif
# else   // !defined(_MSC_FULL_VER)
// _MSC_VER is VVRR  (0-digits build number), transform to VVRR00000
# define UNITEX_COMPILER_MSVC      ((_MSC_VER / 100) * 10000000 \
                                  + (_MSC_VER % 100) * 100000)
# endif
#endif  // defined(_MSC_VER)
/* ************************************************************************** */
/**
 * @brief  Test MSVC version in the format (version, release, patch)
 * @return true if the MSVC version is at least (version, release, patch)
 * @note   You need to pass a valid _MSC_VER (version, release,patch) value and
 *         not the commercial MSVC++ version. e.g. if you need to test against
 *         a MSVC++ 8.0 or later version you must pass (14,0) not (8,0).
 *         See below for a list of valid version identifiers
 *
 * @code{.cpp}
 *  UNITEX_COMPILER_AT_LEAST_MSVC(6,0)   //                         (C      6.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(7,0)   //                         (C/C++  7.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(8,0)   //                         (MSVC++ 1.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(9,0)   //                         (MSVC++ 2.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(10,0)  //                         (MSVC++ 4.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(11,0)  //                         (MSVC++ 5.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(12,0)  //                         (MSVC++ 6.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(13,0)  //                         (MSVC++ 7.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(13,10) // Visual Studio .NET 2003 (MSVC++ 7.1)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(14,0)  // Visual Studio      2005 (MSVC++ 8.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(15,0)  // Visual Studio      2008 (MSVC++ 9.0)  and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(16,0)  // Visual Studio      2010 (MSVC++ 10.0) and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(17,0)  // Visual Studio      2012 (MSVC++ 11.0) and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(18,0)  // Visual Studio      2013 (MSVC++ 12.0) and later
 *  UNITEX_COMPILER_AT_LEAST_MSVC(19,0)  // Visual Studio      2015 (MSVC++ 14.0) and later
 * @endcode
 *
 * @see    UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO
 */
// UNITEX_COMPILER_AT_LEAST_MSVC_(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_MSVC_(v, r, p) \
         (defined(UNITEX_COMPILER_MSVC) &&\
                  UNITEX_COMPILER_MSVC  >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test MSVC version in the format (version, release)
 * @return true if the MSVC version is at least (version, release, 0)
 * @see    UNITEX_COMPILER_AT_LEAST_MSVC_
 */
// UNITEX_COMPILER_AT_LEAST_MSVC(version, release)
# define UNITEX_COMPILER_AT_LEAST_MSVC(v, r)     \
         UNITEX_COMPILER_AT_LEAST_MSVC_(v, r, 0)
/* ************************************************************************** */
// Commercial MSVC++ Visual Studio names shortcuts
// To test against UNITEX_COMPILER_MSVC
#if UNITEX_COMPILER_IS_MSVC
// Visual Studio .NET 2003 (MSVC++ 7.1)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2003 13 * 10000000 + 10 * 100000 + 0
// Visual Studio      2005 (MSVC++ 8.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2005 14 * 10000000 +  0 * 100000 + 0
// Visual Studio      2008 (MSVC++ 9.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2008 15 * 10000000 +  0 * 100000 + 0
// Visual Studio      2010 (MSVC++ 10.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2010 16 * 10000000 +  0 * 100000 + 0
// Visual Studio      2012 (MSVC++ 11.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2012 17 * 10000000 +  0 * 100000 + 0
// Visual Studio      2013 (MSVC++ 12.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2013 18 * 10000000 +  0 * 100000 + 0
// Visual Studio      2015 (MSVC++ 14.0)
#define UNITEX_COMPILER_MSVC_VISUAL_STUDIO_2015 19 * 10000000 +  0 * 100000 + 0
#endif  // UNITEX_COMPILER_IS_MSVC
/* ************************************************************************** */
/**
 * @brief  Test for a Microsoft Visual Studio commercial version name
 * @return true if the version is at least UNITEX_COMPILER_MSVC
 *
 * @code{.cpp}
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2003)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2005)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2008)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2010)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2012)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2013)
 *  UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(2015)
 * @endcode
 *
 * @see    UNITEX_COMPILER_AT_LEAST_MSVC_
 */
// UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(CommercialVersion)
# define UNITEX_COMPILER_AT_LEAST_MSVC_VISUAL_STUDIO(CommercialVersion)    \
        (defined(UNITEX_COMPILER_MSVC_VISUAL_STUDIO_##CommercialVersion) &&\
                 UNITEX_COMPILER_MSVC  >= \
                 UNITEX_COMPILER_MSVC_VISUAL_STUDIO_##CommercialVersion)
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_INTEL
 * @brief    Intel C/C++ compiler

 * @details  Set UNITEX_COMPILER_INTEL as VVRR000PP (Version, Release, Patch)
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if defined(__INTEL_COMPILER)
# define UNITEX_COMPILER_IS_INTEL   1
// __INTEL_COMPILER is VRP, transform to VV0R0000P
# define UNITEX_COMPILER_INTEL\
                                    ((__INTEL_COMPILER / 100)       * 10000000 \
                                  + ((__INTEL_COMPILER % 100) / 10) * 100000   \
                                  +  (__INTEL_COMPILER % 10))
#endif  // defined(__INTEL_COMPILER)
/* ************************************************************************** */
/**
 * @brief  Test Intel C/C++ version in the format (version, release, patch)
 * @return true if the Intel C/C++ version is at least (version, release, patch)
 */
// UNITEX_COMPILER_AT_LEAST_INTEL_(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_INTEL_(v, r, p) \
         (defined(UNITEX_COMPILER_INTEL) &&\
             UNITEX_COMPILER_INTEL >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test Intel C/C++ version in the format (version, release)
 * @return true if the Intel C/C++ version is at least (version, release, 0)
 *
 * @code{.cpp}
 *         UNITEX_COMPILER_AT_LEAST_INTEL(11,1)
 *         UNITEX_COMPILER_AT_LEAST_INTEL(12,0)
 *         UNITEX_COMPILER_AT_LEAST_INTEL(12,1)
 *         UNITEX_COMPILER_AT_LEAST_INTEL(13,0)
 *         UNITEX_COMPILER_AT_LEAST_INTEL(14,0)
 *         UNITEX_COMPILER_AT_LEAST_INTEL(15,0)
 * @endcode
 *
 * @see    UNITEX_COMPILER_AT_LEAST_INTEL_
 */
// UNITEX_COMPILER_AT_LEAST_INTEL(version, release)
# define UNITEX_COMPILER_AT_LEAST_INTEL(v, r) \
         UNITEX_COMPILER_AT_LEAST_INTEL_(v, r, 0)
/* ************************************************************************** */
/**
 * @def      UNITEX_COMPILER_ZOSXL
 * @brief    IBM z/OS XL C/C++ compiler

 * @details  Set UNITEX_COMPILER_ZOSXL as 0VRR0PPPP (Version, Release, Patch)
 *
 * @note     The greatest decimals numbers allowed are: 9 for version, 99
 *           for revision and 9999 for patch
 *
 * @note     Do not use this directly, rather use UNITEX_COMPILER_AT_LEAST or
 *           UNITEX_COMPILER_IS macro variants
 *
 * @see      UNITEX_COMPILER_AT_LEAST
 * @see      UNITEX_COMPILER_IS
 */
#if ((defined(__IBMC__) || defined(__IBMCPP__)) &&\
      defined(__COMPILER_VER__))
# define UNITEX_COMPILER_IS_ZOSXL   1
// __COMPILER_VER__ is OxNVRRPPPP, N = 4 = z/OS, V = Version, RR = Revision, and
// P = 4-digits Patch number). Transform into 0VRR0PPPP.
# define UNITEX_COMPILER_ZOSXL  ((__COMPILER_VER__ & 0x0F000000) / 0x1000000) \
                                  * 10000000                                  \
                              + ((__COMPILER_VER__ & 0x00FF0000) / 0x10000)   \
                                  * 100000                                    \
                              +  (__COMPILER_VER__ & 0x0000FFFF)
#endif  // defined(__IBMC__) || defined(__IBMCPP__) && defined(__COMPILER_VER__)
/* ************************************************************************** */
/**
 * @brief  Test IBM z/OS XL C/C++ version in the format (version, release, patch)
 * @return true if the XL C/C++ version is at least (version, release, patch)
 */
// UNITEX_COMPILER_AT_LEAST_ZOSXL_(version, release, patch)
# define UNITEX_COMPILER_AT_LEAST_ZOSXL_(v, r, p) \
         (defined(UNITEX_COMPILER_ZOSXL) &&\
                  UNITEX_COMPILER_ZOSXL >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test IBM z/OS XL C/C++ version in the format (version, release)
 * @return true if the IBM z/OS XL C/C++ version is at least (version, release, 0)
 * @see    UNITEX_COMPILER_AT_LEAST_ZOSXL_
 */
// UNITEX_COMPILER_AT_LEAST_ZOSXL(version, release)
# define UNITEX_COMPILER_AT_LEAST_ZOSXL(v, r) \
         UNITEX_COMPILER_AT_LEAST_ZOSXL_(v, r, 0)
/* ************************************************************************** */
/**
 * @brief  Test for a compiler type at compile-time
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_IS(CLANG)    // LLVM native C/C++ compiler
 *         UNITEX_COMPILER_IS(GCC)      // GNU C/C++ compiler
 *         UNITEX_COMPILER_IS(INTEL)    // Intel C/C++ compiler
 *         UNITEX_COMPILER_IS(MINGW)    // Minimalist GNU C/C++ compiler (32/64)
 *         UNITEX_COMPILER_IS(MINGW64)  // Minimalist GNU C/C++ compiler (64)
 *         UNITEX_COMPILER_IS(MSVC)     // Microsoft Visual C++ compiler
 *         UNITEX_COMPILER_IS(ZOSXL)    // IBM z/OS XL C/C++ compiler
 * @endcode
 *
 * @see    UNITEX_COMPILER_CLANG
 * @see    UNITEX_COMPILER_GCC
 * @see    UNITEX_COMPILER_INTEL
 * @see    UNITEX_COMPILER_MINGW
 * @see    UNITEX_COMPILER_MSVC
 * @see    UNITEX_COMPILER_ZOSXL
 */
#define UNITEX_COMPILER_IS(CompilerName)              \
        (defined(UNITEX_COMPILER_IS_##CompilerName) &&\
                 UNITEX_COMPILER_IS_##CompilerName)
/* ************************************************************************** */
/**
 * @brief  Test for a compiler type at compile-time  (NOT)
 *
 * The supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_IS_NOT(CLANG) // LLVM native C/C++ compiler
 *         UNITEX_COMPILER_IS_NOT(GCC)   // GNU C/C++ compiler
 *         UNITEX_COMPILER_IS_NOT(INTEL) // Intel C/C++ compiler
 *         UNITEX_COMPILER_IS_NOT(MINGW) // Minimalist GNU C/C++ compiler
 *         UNITEX_COMPILER_IS_NOT(MSVC)  // Microsoft Visual C++ compiler
 *         UNITEX_COMPILER_IS_NOT(ZOSXL) // IBM z/OS XL C/C++ compiler
 * @endcode
 *
 * @see    UNITEX_COMPILER_IS
 */
#define UNITEX_COMPILER_IS_NOT(CompilerName) \
        (!UNITEX_COMPILER_IS(CompilerName))
/* ************************************************************************** */
/**
 * @brief  Test compiler name and version in the format (version, release, patch)
 *         at compile-time
 *
 * Some examples of supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_AT_LEAST_(GCC,2,0,0)
 * @endcode
 */
// UNITEX_COMPILER_AT_LEAST_
#define UNITEX_COMPILER_AT_LEAST_(CompilerName, v, r, p) \
       (UNITEX_COMPILER_AT_LEAST_##CompilerName##_(v, r, p))
/**
 * @brief  Test compiler name and version in the format (version, release)
 *         at compile-time
 *
 * Some examples of supported expressions are:
 * @code{.cpp}
 *         UNITEX_COMPILER_AT_LEAST(GCC,2,0)
 * @endcode
 *
 * @see    UNITEX_COMPILER_AT_LEAST_
 */
// UNITEX_COMPILER_AT_LEAST
#define UNITEX_COMPILER_AT_LEAST(CompilerName, v, r) \
       (UNITEX_COMPILER_AT_LEAST_##CompilerName(v, r))
/* ************************************************************************** */
// Attention! Please do note, not to change array definition order
// mainly because the GNU C version macros are also defined by
// others compilers, consider also that Apple's Clang distribution
// bundled with XCode may rewrite __clang_major__ and  __clang_minor__
// originally found in the open source Clang distribution.
// In others words, put all compilers that are impersonating another
// before the genuine compiler itself.
const struct /* compiler_name_version_table_t */ {
  // At file scope this is the same that static const struct
  const char* name;       // stringize compiler name
  int         version;    // VVRRPPPPP compiler integer version
} compiler_name_version_table[] = {
// MINGW
#if UNITEX_COMPILER_IS(MINGW)
  {"mingw",         UNITEX_COMPILER_MINGW},
#endif  // UNITEX_COMPILER_IS(MINGW)
// CLANG_APPLE
#if UNITEX_COMPILER_IS(CLANG_APPLE)
  {"clang.apple",   UNITEX_COMPILER_CLANG_APPLE},
#endif  // UNITEX_COMPILER_IS(CLANG_APPLE)
// CLANG
#if UNITEX_COMPILER_IS(CLANG)
  {"clang",         UNITEX_COMPILER_CLANG},
#endif  // UNITEX_COMPILER_IS(CLANG)
// INTEL
#if UNITEX_COMPILER_IS(INTEL)
  {"intel",         UNITEX_COMPILER_INTEL},
#endif  // UNITEX_COMPILER_IS(INTEL)
// ZOSXL
#if UNITEX_COMPILER_IS(ZOSXL)
  {"zosxl",         UNITEX_COMPILER_ZOSXL},
#endif  // UNITEX_COMPILER_IS(ZOSXL)
// MSVC
#if UNITEX_COMPILER_IS(MSVC)
  {"msvc",         UNITEX_COMPILER_MSVC},
#endif  // UNITEX_COMPILER_IS(MSVC)
// GCC
#if UNITEX_COMPILER_IS(GCC)
  {"gcc",         UNITEX_COMPILER_GCC},
#endif  // UNITEX_COMPILER_IS(GCC)
//  Unknown compiler, sorted alphabetically
#if UNITEX_COMPILER_IS_NOT(CLANG) &&\
    UNITEX_COMPILER_IS_NOT(GCC)   &&\
    UNITEX_COMPILER_IS_NOT(INTEL) &&\
    UNITEX_COMPILER_IS_NOT(MINGW) &&\
    UNITEX_COMPILER_IS_NOT(MSVC)  &&\
    UNITEX_COMPILER_IS_NOT(ZOSXL)
  {"<unknown>",   0},
#endif
  {NULL,          0}
};  // compiler_name_version_table[]
/* ************************************************************************** */
/**
 * @def    UNITEX_COMPILER_NAME
 * @brief  Store the main compiler name
 * @see    config::build::compiler_name
 */
/**
 * @def    UNITEX_COMPILER_VERSION
 * @brief  Store the main compiler version
 *         in the integer format VVRRPPPPP
 * @see    config::build::compiler_version
 */
// Attention! Please do note, not to change array definition order
// mainly because the GNU C version macros are also defined by
// others compilers, consider also that Apple's Clang distribution
// bundled with XCode may rewrite __clang_major__ and  __clang_minor__
// originally found in the open source Clang distribution.
// In others words, put all compilers that are impersonating another
// before the genuine compiler itself.
#if UNITEX_COMPILER_IS(MINGW)
// MINGW
# define UNITEX_COMPILER_NAME         "mingw"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_MINGW
#elif UNITEX_COMPILER_IS(CLANG_APPLE)
// CLANG_APPLE
# define UNITEX_COMPILER_NAME         "clang.apple"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_CLANG_APPLE
#elif UNITEX_COMPILER_IS(CLANG)
// CLANG
# define UNITEX_COMPILER_NAME         "clang"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_CLANG
#elif UNITEX_COMPILER_IS(INTEL)
// INTEL
# define UNITEX_COMPILER_NAME         "intel"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_INTEL
#elif UNITEX_COMPILER_IS(ZOSXL)
// ZOSXL
# define UNITEX_COMPILER_NAME         "zosxl"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_ZOSXL
#elif UNITEX_COMPILER_IS(MSVC)
// MSVC
# define UNITEX_COMPILER_NAME         "msvc"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_MSVC
#elif UNITEX_COMPILER_IS(GCC)
// GCC
# define UNITEX_COMPILER_NAME         "gcc"
# define UNITEX_COMPILER_VERSION      UNITEX_COMPILER_GCC
#else
//  Unknown compiler
# define UNITEX_COMPILER_NAME         "<unknown>"
# define UNITEX_COMPILER_VERSION      0
#endif
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_VERSION_H_                         // NOLINT
