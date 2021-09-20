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
 * @file      zosxl.h
 * @brief     C++ standard language proposals enabled by the IBM z/OS XL C/C++ compiler
 *
 * @see       https://goo.gl/VZ6H2q
 * @see       https://goo.gl/a6TU1V
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 cpu.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_PROPOSALS_ZOSXL_H_                     // NOLINT
#define UNITEX_BASE_COMPILER_PROPOSALS_ZOSXL_H_                     // NOLINT
/* ************************************************************************** */
#include "base/compiler/version.h"     // UNITEX_COMPILER_AT_LEAST_ZOSXL
#include "base/compiler/compliance.h"  // UNITEX_COMPILER_COMPLIANT
/* ************************************************************************** */
// The macro __C99_LLONG is defined to 1 with -qlanglvl=extended0x
// and is otherwise undefined. We test defined(__C99_LLONG) to know
// if C++11 experimental support has been enabled, i.e. when compiling
// with -qlanglvl=extended0x

// C++11

// IBM z/OS XL C/C++ 7.0 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(7,0) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_N2170      1
#endif  // IBM z/OS XL C/C++ 7.0 or newer

// IBM z/OS XL C/C++ 8.0 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(8,0) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_DR339      1
# define UNITEX_CXX_PROPOSAL_N2340      1
#endif  // IBM z/OS XL C/C++ 8.0 or newer

// IBM z/OS XL C/C++ 10.1 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(10,1) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_N1653      1
# define UNITEX_CXX_PROPOSAL_N1811      1
#endif  // IBM z/OS XL C/C++ 10.1 or newer

// IBM z/OS XL C/C++ 11.1 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(11,1) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_N2242      1
# define UNITEX_CXX_PROPOSAL_N1720      1
# define UNITEX_CXX_PROPOSAL_N1984      1
# define UNITEX_CXX_PROPOSAL_N1737      1
# define UNITEX_CXX_PROPOSAL_N2546      1
# define UNITEX_CXX_PROPOSAL_N2541      1
# define UNITEX_CXX_PROPOSAL_N2343      1
# define UNITEX_CXX_PROPOSAL_N1986      1
# define UNITEX_CXX_PROPOSAL_N1791      1
# define UNITEX_CXX_PROPOSAL_N2253      1
# define UNITEX_CXX_PROPOSAL_DR850      1
# define UNITEX_CXX_PROPOSAL_N2535      1
#endif  // IBM z/OS XL C/C++ 11.1 or newer

// IBM z/OS XL C/C++ 12.1 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(12,1) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_N2118      1
# define UNITEX_CXX_PROPOSAL_N1757      1
# define UNITEX_CXX_PROPOSAL_DR226      1
# define UNITEX_CXX_PROPOSAL_N1987      1
# define UNITEX_CXX_PROPOSAL_N2347      1
# define UNITEX_CXX_PROPOSAL_N2764      1
# define UNITEX_CXX_PROPOSAL_N2437      1
#endif  // IBM z/OS XL C/C++ 12.1 or newer

// IBM z/OS XL C/C++ 13.1 or newer
#if UNITEX_COMPILER_AT_LEAST_ZOSXL(13,1) &&\
    (UNITEX_COMPILER_COMPLIANT(CXX11) || defined(__C99_LLONG))
# define UNITEX_CXX_PROPOSAL_N2672      1
# define UNITEX_CXX_PROPOSAL_N2431      1
# define UNITEX_CXX_PROPOSAL_N2346      1
#endif  // IBM z/OS XL C/C++ 13.1 or newer

// C++14
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_PROPOSALS_ZOSXL_H_                  // NOLINT
