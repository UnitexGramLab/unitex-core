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
 * @file      intel.h
 * @brief     C++ standard language proposals enabled by the Intel Compiler
 *
 * @see       https://software.intel.com/en-us/articles/c0x-features-supported-by-intel-c-compiler
 * @see       https://software.intel.com/en-us/articles/c14-features-supported-by-intel-c-compiler
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 intel.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_PROPOSALS_INTEL_H_                     // NOLINT
#define UNITEX_BASE_COMPILER_PROPOSALS_INTEL_H_                     // NOLINT
/* ************************************************************************** */
#include "base/compiler/version.h"     // UNITEX_COMPILER_AT_LEAST_INTEL
#include "base/compiler/compliance.h"  // UNITEX_COMPILER_COMPLIANT
#include "base/os/type.h"              // UNITEX_OS_UNIX_ENVIRONMENT_IS
/* ************************************************************************** */
// C++11

// Intel C/C++ compiler 11.1 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(11,1) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_N1610      1
# define UNITEX_CXX_PROPOSAL_N1653      1
# define UNITEX_CXX_PROPOSAL_N1720      1
# define UNITEX_CXX_PROPOSAL_N1757      1
# define UNITEX_CXX_PROPOSAL_N1811      1
# define UNITEX_CXX_PROPOSAL_N1987      1
# define UNITEX_CXX_PROPOSAL_N2340      1
# if UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX) ||\
     UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
#   define UNITEX_CXX_PROPOSAL_N2660    1
# endif  // Linux and OS X
#endif  // Intel C/C++ compiler 11.1 or newer

// Intel C/C++ compiler 12.0 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(12,0) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_N1737      1
# define UNITEX_CXX_PROPOSAL_N1791      1
# define UNITEX_CXX_PROPOSAL_N1984      1
# define UNITEX_CXX_PROPOSAL_N2118      1
# define UNITEX_CXX_PROPOSAL_N2179      1
# define UNITEX_CXX_PROPOSAL_N2343      1
# define UNITEX_CXX_PROPOSAL_N2346      1
# define UNITEX_CXX_PROPOSAL_N2546      1
# define UNITEX_CXX_PROPOSAL_N2550      1
# define UNITEX_CXX_PROPOSAL_N2657      1
# define UNITEX_CXX_PROPOSAL_N2658      1
# define UNITEX_CXX_PROPOSAL_N2844      1
# define UNITEX_CXX_PROPOSAL_N2927      1
# define UNITEX_CXX_PROPOSAL_N3276      1
#endif  // Intel C/C++ compiler 12.0 or newer

// Intel C/C++ compiler 12.1 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(12,1) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_DR226      1
# define UNITEX_CXX_PROPOSAL_N2170      1
# define UNITEX_CXX_PROPOSAL_N2242      1
# if UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX) ||\
     UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
#   define UNITEX_CXX_PROPOSAL_N2249    1
# endif  // Linux and OS X
# define UNITEX_CXX_PROPOSAL_N2258      1
# define UNITEX_CXX_PROPOSAL_N2431      1
# define UNITEX_CXX_PROPOSAL_N2541      1
# define UNITEX_CXX_PROPOSAL_N2555      1
# define UNITEX_CXX_PROPOSAL_N2634      1
# define UNITEX_CXX_PROPOSAL_N2761      1
#endif  // Intel C/C++ compiler 12.1 or newer

// Intel C/C++ compiler 13.0 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(13,0) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_N2427      1
# define UNITEX_CXX_PROPOSAL_N2437      1
# define UNITEX_CXX_PROPOSAL_N2748      1
# define UNITEX_CXX_PROPOSAL_N2752      1
# define UNITEX_CXX_PROPOSAL_N2855      1
# define UNITEX_CXX_PROPOSAL_N2930      1
# define UNITEX_CXX_PROPOSAL_N2947      1
# define UNITEX_CXX_PROPOSAL_N3052      1
#endif  // Intel C/C++ compiler 13.0 or newer

// Intel C/C++ compiler 14.0 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(14,0) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_N1986      1
# define UNITEX_CXX_PROPOSAL_N2235      1
# undef  UNITEX_CXX_PROPOSAL_N2249         // First defined in 12.1 or newer
# define UNITEX_CXX_PROPOSAL_N2249      1
# define UNITEX_CXX_PROPOSAL_N2253      1
# define UNITEX_CXX_PROPOSAL_N2342      1
# define UNITEX_CXX_PROPOSAL_N2347      1
# define UNITEX_CXX_PROPOSAL_N2439      1
# define UNITEX_CXX_PROPOSAL_N2442      1
# define UNITEX_CXX_PROPOSAL_N2535      1
# if UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX) ||\
     UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
# define UNITEX_CXX_PROPOSAL_N2544      1
# endif  // Linux and OS X
# define UNITEX_CXX_PROPOSAL_N2672      1
# define UNITEX_CXX_PROPOSAL_N2756      1
# define UNITEX_CXX_PROPOSAL_N2928      1
# define UNITEX_CXX_PROPOSAL_N3050      1
# define UNITEX_CXX_PROPOSAL_N3053      1
# define UNITEX_CXX_PROPOSAL_N3206      1
# define UNITEX_CXX_PROPOSAL_N3272      1
#endif  // Intel C/C++ compiler 14.0 or newer

// Intel C/C++ compiler 15.0 or newer
#if UNITEX_COMPILER_AT_LEAST_INTEL(15,0) && UNITEX_COMPILER_COMPLIANT(CXX11)
# define UNITEX_CXX_PROPOSAL_N2239      1
# define UNITEX_CXX_PROPOSAL_N2341      1
# if UNITEX_OS_UNIX_ENVIRONMENT_IS(LINUX) ||\
     UNITEX_OS_UNIX_ENVIRONMENT_IS(APPLE)
#  define UNITEX_CXX_PROPOSAL_N1988     1
#  define UNITEX_CXX_PROPOSAL_N2429     1
#  define UNITEX_CXX_PROPOSAL_N2440     1
#  define UNITEX_CXX_PROPOSAL_N2547     1
#  define UNITEX_CXX_PROPOSAL_N2659     1
# endif  // Linux and OS X
# define UNITEX_CXX_PROPOSAL_N2540      1
# if UNITEX_OS_IS(WINDOWS)
#   define UNITEX_CXX_PROPOSAL_N2670    1
# endif  // Windows
# define UNITEX_CXX_PROPOSAL_N2765      1
# define UNITEX_CXX_PROPOSAL_N2782      1
#endif  // Intel C/C++ compiler 15.0 or newer

// C++14
/* ************************************************************************** */
#endif /* UNITEX_BASE_COMPILER_PROPOSALS_INTEL_H_ */                // NOLINT
