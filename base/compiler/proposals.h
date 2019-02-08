/*
 * Unitex
 *
 * Copyright (C) 2001-2019 Université Paris-Est Marne-la-Vallée <unitex-devel@univ-mlv.fr>
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
 * @file      proposals.h
 * @brief     Macros to query if certain standard language proposals are enabled
 *
 * Use UNITEX_CXX_PROPOSAL(x), where x could be:
 *
 * C++11 Core Language Proposals
 * N1610      Rvalue refs (init objects by rvalues)                       v0.1
 * N2118      Rvalue refs and move constructors      std::move            v1.0
 * N2844      Rvalue refs and move constructors      std::move            v2.0
 * DR910      Rvalue refs and move constructors      std::move            v2.1
 * N3053      Rvalue refs and move constructors      std::move            v3.0
 * N2439      Rvalue references for *this
 * N2756      Non-static data member initializers
 * N2242      Variadic Templates                                          v0.9
 * N2555      Variadic Templates                                          v1.0
 * N2672      Initializer Lists
 * N1720      Static Assertions                      static_assert
 * N1984      auto type                              auto                 v0.9
 * N2546      auto type                              auto                 v1.0
 * N2541      Trailing return types                  auto
 * N1737      Multi-declarator auto                  auto
 * N2550      Lambda expressions and closures                             v0.9
 * N2658      Lambda expressions and closures                             v1.0
 * N2927      Lambda expressions and closures                             v1.1
 * N2343      Declared type of an Expression         decltype             v1.0
 * N3276      Declared type of an Expression         decltype             v1.1
 * N1757      Right Angle Brackets
 * DR226      Def template args func templates
 * N2634      Solving SFINAE expressions problem                          DR339
 * N2258      Template aliases
 * N1987      Extern Templates
 * N2431      Null Pointer                           nullptr
 * N2347      Strongly-typed enums
 * N2764      Forward declarations for enums
 * N2761      Generalized Attributes
 * N2235      Generalized constant expressions       constexpr
 * N2341      Alignment support                      alignas, alignof
 * N1986      Delegating Constructors
 * N2540      Inheriting Constructors
 * N2437      Explicit conversion operators          explicit
 * N2249      New character types                    char16_t, char32_t
 * N2442      Unicode String Literals                u8", u", U"
 * N2442      Raw String Literals                    R", u8R", uR", UR"
 * N2170      Universal character name literals
 * N2765      User-defined Literals
 * N2342      Standard Layout Types
 * N2346      Defaulted and Deleted Functions        =default, =delete
 * N1791      Extended friend Declarations
 * N2253      Extending sizeof
 * N2535      Inline namespaces
 * N2544      Unrestricted unions
 * N2657      Local and unnamed types
 * N2930      Range-based for-loop
 * N2928      Explicit overrides and final           override, final      v0.8
 * N3206      Explicit overrides and final           override, final      v0.9
 * N3272      Explicit overrides and final           override, final      v1.0
 * N2670      Minimal garbage collection (GC)
 * N3050      Allow move constructors throw          noexcept
 *
 * C++11 Core Language Proposals: Standard library changes
 * N1836      type traits for metaprogramming
 *
 * C++11 Core Language Proposals: Concurrency
 * N2239      Reworded sequence points
 * N2427      Atomic Operations
 * N2748      Strong Compare and Exchange
 * N2752      Bidirectional Fences
 * N2429      Memory model
 * N2664      Data-dependency ordering
 * N2782      Data-dependency ordering: function annotation
 * N2179      Propagating exceptions                 exception_ptr
 * N2440      Abandoning process, at_quick_exit      at_quick_exit
 * N2547      Allow atomics in signal handlers
 * N2659      Thread-Local Storage
 * N2660      Dynamic init/destr concurrency
 *
 * C++11 Core Language Proposals: C99
 * N2340      Function-local Predefined Variable     __func__
 * N1653      C99 preprocessor
 * N1811      long long type                         long long
 * N1988      Extended integral types
 *
 * @note      The previous list was borrowed from "C++0x/C++11 Support in GCC"
 * @see       http://gcc.gnu.org/projects/cxx0x.html
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 proposals.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_PROPOSALS_H_                           // NOLINT
#define UNITEX_BASE_COMPILER_PROPOSALS_H_                           // NOLINT
/* ************************************************************************** */
#include "base/compiler/version.h"  // UNITEX_COMPILER_IS
/* ************************************************************************** */
/**
 * Test if certain standard language proposal is enabled
 */
#define UNITEX_CXX_PROPOSAL(Proposal)               \
        (defined(UNITEX_CXX_PROPOSAL_##Proposal)  &&\
                 UNITEX_CXX_PROPOSAL_##Proposal)
/* ************************************************************************** */
// include compiler specific headers
// Attention, do not change declaration order
#if UNITEX_COMPILER_IS(CLANG)    // Clang Compiler
# include "base/compiler/proposals/clang.h"
#elif UNITEX_COMPILER_IS(INTEL)  // Intel Compiler
# include "base/compiler/proposals/intel.h"
#elif UNITEX_COMPILER_IS(MSVC)   // Microsoft Visual Studio Compiler
# include "base/compiler/proposals/msvc.h"
#elif UNITEX_COMPILER_IS(ZOSXL)  // IBM z/OS XL C/C++ Compiler
# include "base/compiler/proposals/zosxl.h"
#elif UNITEX_COMPILER_IS(GCC)    // GNU Compiler
# include "base/compiler/proposals/gcc.h"
#endif
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_PROPOSALS_H_                        // NOLINT
