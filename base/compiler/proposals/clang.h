/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      clang.h
 * @brief     C++ standard language proposals enabled by the Clang compiler
 *
 * @see       http://clang.llvm.org/cxx_status.html
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 clang.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_PROPOSALS_CLANG_H_                     // NOLINT
#define UNITEX_BASE_COMPILER_PROPOSALS_CLANG_H_                     // NOLINT
/* ************************************************************************** */
#include "base/compiler/version.h"     // UNITEX_COMPILER_AT_LEAST_CLANG
#include "base/compiler/compliance.h"  // UNITEX_COMPILER_COMPLIANT
/* ************************************************************************** */
// C++11

// Clang 2.9 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(2,9) &&\
   (UNITEX_COMPILER_COMPLIANT(CXX11)  || defined(__GXX_EXPERIMENTAL_CXX0X__))
# define UNITEX_CXX_PROPOSAL_N1610               1
# define UNITEX_CXX_PROPOSAL_N1627               1
# define UNITEX_CXX_PROPOSAL_N1653               1
# define UNITEX_CXX_PROPOSAL_N1720               1
# define UNITEX_CXX_PROPOSAL_N1727               1
# define UNITEX_CXX_PROPOSAL_N1737               1
# define UNITEX_CXX_PROPOSAL_N1757               1
# define UNITEX_CXX_PROPOSAL_N1791               1
# define UNITEX_CXX_PROPOSAL_N1811               1
# define UNITEX_CXX_PROPOSAL_N1984               1
# define UNITEX_CXX_PROPOSAL_N1987               1
# define UNITEX_CXX_PROPOSAL_N2118               1
# define UNITEX_CXX_PROPOSAL_N2179               1
# define UNITEX_CXX_PROPOSAL_N2242               1
# define UNITEX_CXX_PROPOSAL_N2249               1
# define UNITEX_CXX_PROPOSAL_N2340               1
# define UNITEX_CXX_PROPOSAL_N2343               1
# define UNITEX_CXX_PROPOSAL_N2346               1
# define UNITEX_CXX_PROPOSAL_N2347               1
# define UNITEX_CXX_PROPOSAL_N2439               1
# define UNITEX_CXX_PROPOSAL_N2535               1
# define UNITEX_CXX_PROPOSAL_N2541               1
# define UNITEX_CXX_PROPOSAL_N2546               1
# define UNITEX_CXX_PROPOSAL_N2555               1
# define UNITEX_CXX_PROPOSAL_N2657               1
# define UNITEX_CXX_PROPOSAL_N2660               1
# define UNITEX_CXX_PROPOSAL_NR226               1
# define UNITEX_CXX_PROPOSAL_NR339               1
#endif  // Clang 2.9 or newer

// Clang 3.0 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,0) &&\
   (UNITEX_COMPILER_COMPLIANT(CXX11)  || defined(__GXX_EXPERIMENTAL_CXX0X__))
# define UNITEX_CXX_PROPOSAL_N1986               1
# define UNITEX_CXX_PROPOSAL_N2258               1
# define UNITEX_CXX_PROPOSAL_N2342               1
# define UNITEX_CXX_PROPOSAL_N2346               1
# define UNITEX_CXX_PROPOSAL_N2431               1
# define UNITEX_CXX_PROPOSAL_N2437               1
# define UNITEX_CXX_PROPOSAL_N2442               1
# define UNITEX_CXX_PROPOSAL_N2442               1
# define UNITEX_CXX_PROPOSAL_N2756               1
# define UNITEX_CXX_PROPOSAL_N2928               1
# define UNITEX_CXX_PROPOSAL_N2930               1
# define UNITEX_CXX_PROPOSAL_N3050               1
# define UNITEX_CXX_PROPOSAL_N3053               1
# define UNITEX_CXX_PROPOSAL_N3206               1
# define UNITEX_CXX_PROPOSAL_N3272               1
#endif  // Clang 3.0 or newer

// Clang 3.1 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,1) &&\
   (UNITEX_COMPILER_COMPLIANT(CXX11)  || defined(__GXX_EXPERIMENTAL_CXX0X__))
# define UNITEX_CXX_PROPOSAL_N2170               1
# define UNITEX_CXX_PROPOSAL_N2235               1
# define UNITEX_CXX_PROPOSAL_N2253               1
# define UNITEX_CXX_PROPOSAL_N2427               1
# define UNITEX_CXX_PROPOSAL_N2544               1
# define UNITEX_CXX_PROPOSAL_N2547               1
# define UNITEX_CXX_PROPOSAL_N2672               1
# define UNITEX_CXX_PROPOSAL_N2748               1
# define UNITEX_CXX_PROPOSAL_N2752               1
# define UNITEX_CXX_PROPOSAL_N2764               1
# define UNITEX_CXX_PROPOSAL_N2765               1
# define UNITEX_CXX_PROPOSAL_N2927               1
# define UNITEX_CXX_PROPOSAL_N3276               1
# define UNITEX_CXX_PROPOSAL_NR850               1
#endif  // Clang 3.1 or newer

// Clang 3.2 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,2) &&\
   (UNITEX_COMPILER_COMPLIANT(CXX11)  || defined(__GXX_EXPERIMENTAL_CXX0X__))
# define UNITEX_CXX_PROPOSAL_N2429               1
# define UNITEX_CXX_PROPOSAL_N2664               1
#endif  // Clang 3.2 or newer

// Clang 3.3 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,3) &&\
   (UNITEX_COMPILER_COMPLIANT(CXX11)  || defined(__GXX_EXPERIMENTAL_CXX0X__))
# define UNITEX_CXX_PROPOSAL_N2341               1
# define UNITEX_CXX_PROPOSAL_N2540               1
# define UNITEX_CXX_PROPOSAL_N2239               1
# define UNITEX_CXX_PROPOSAL_N2659               1
# define UNITEX_CXX_PROPOSAL_N2761               1
#endif  // Clang 3.3 or newer

// Clang 3.4 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,4) &&\
    UNITEX_COMPILER_COMPLIANT(CXX14)
#endif  // Clang 3.4 or newer

// Clang 3.5 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,5) &&\
    UNITEX_COMPILER_COMPLIANT(CXX14)
#endif  // Clang 3.5 or newer

// Clang 3.6 or newer
#if UNITEX_COMPILER_AT_LEAST_CLANG(3,6) &&\
    UNITEX_COMPILER_COMPLIANT(CXX14)
#endif  // Clang 3.6 or newer

// C++14
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_PROPOSALS_CLANG_H_                  // NOLINT
