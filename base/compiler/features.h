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
 * @file      features.h
 * @brief     Macro to query in compile-time if certain standard language
 *            features are enabled
 *
 * Use UNITEX_HAS_FEATURE(x), where x could be:
 *
 * cxx_access_control_sfinae            access-control errors are considered to
 *                                      be template argument deduction errors
 * cxx_alias_templates                  alias declarations and alias templates
 * cxx_alignas                          alignment specifiers using alignas
 * cxx_alignof                          alignment specifiers using alignof
 * cxx_attributes                       attribute parsing with square bracket
 *                                      notation
 * cxx_auto_type                        inference using the auto specifier
 * cxx_constexpr                        generalized constant expressions
 * cxx_decltype                         decltype() specifier
 * cxx_decltype_incomplete_return_types decltype() thats not require type-
 *                                      completeness of function call expression
 * cxx_default_function_template_args   default template arguments in function
 *                                      templates
 * cxx_defaulted_functions              defaulted function definitions
 * cxx_delegating_constructors          delegating constructors
 * cxx_deleted_functions                deleted function definitions
 * cxx_exceptions                       exceptions
 * cxx_explicit_conversions             explicit conversion functions
 * cxx_extern_templates                 [+] extern templates
 * cxx_generalized_initializers         generalized initializers
 * cxx_inheriting_constructors          inheriting constructors
 * cxx_inline_namespaces                inline namespaces
 * cxx_lambdas                          lambdas
 * cxx_local_type_template_args         local and unnamed types as template args
 * cxx_noexcept                         noexcept exception specifications
 * cxx_nonstatic_member_init            in-class initialization of non-static
 *                                      data members
 * cxx_nullptr                          nullptr
 * cxx_override_control                 override/final control keywords
 * cxx_range_for                        range-based for loop
 * cxx_raw_string_literals              raw string literals
 * cxx_reference_qualified_functions    reference-qualified functions
 * cxx_rtti                             RTTI
 * cxx_rvalue_references                rvalue references
 * cxx_static_asserts                   compile-time assertions
 * cxx_strong_enums                     strongly typed, scoped enumerations
 * cxx_trailing_return                  alternate function declaration syntax
 *                                      with trailing return type
 * cxx_unicode_literals                 unicode string literals
 * cxx_unrestricted_unions              unrestricted unions
 * cxx_user_literals                    user-defined literals
 * cxx_variadic_macros                  [+] variadic macros
 * cxx_variadic_templates               variadic templates
 *
 * @see       https://isocpp.org/std/standing-documents/sd-6-sg10-feature-test-recommendations
 * @see       http://clang.llvm.org/docs/LanguageExtensions.html
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the base/common.h
 *            header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 features.h`
 *
 * @date      February 2015
 *
 * This file was contributed as part of the [DataMaTex](http://www.amabis.com)
 * project developed by [Amabis SARL](http://www.amabis.fr) with the collaboration
 * of the [LIGM](http://infolingu.univ-mlv.fr/). For further information on this,
 * please contact unitex-ws@amabis.fr
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_COMPILER_FEATURES_H_                            // NOLINT
#define UNITEX_BASE_COMPILER_FEATURES_H_                            // NOLINT
/* ************************************************************************** */
#include "base/preprocessor/stringify.h"
#include "base/compiler/compliance.h"
#include "base/compiler/proposals.h"
#include "base/compiler/version.h"
/* ************************************************************************** */
/**
 * @brief  Query if certain standard language features are enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_access_control_sfinae)
 *         UNITEX_HAS_FEATURE(cxx_alias_templates)
 *         UNITEX_HAS_FEATURE(cxx_alignas)
 *         UNITEX_HAS_FEATURE(cxx_alignof)
 *         UNITEX_HAS_FEATURE(cxx_attributes)
 *         UNITEX_HAS_FEATURE(cxx_auto_type)
 *         UNITEX_HAS_FEATURE(cxx_constexpr)
 *         UNITEX_HAS_FEATURE(cxx_decltype)
 *         UNITEX_HAS_FEATURE(cxx_decltype_incomplete_return_types)
 *         UNITEX_HAS_FEATURE(cxx_default_function_template_args)
 *         UNITEX_HAS_FEATURE(cxx_defaulted_functions)
 *         UNITEX_HAS_FEATURE(cxx_delegating_constructors)
 *         UNITEX_HAS_FEATURE(cxx_deleted_functions)
 *         UNITEX_HAS_FEATURE(cxx_exceptions)
 *         UNITEX_HAS_FEATURE(cxx_explicit_conversions)
 *         UNITEX_HAS_FEATURE(cxx_extern_templates)                     // [+]
 *         UNITEX_HAS_FEATURE(cxx_generalized_initializers)
 *         UNITEX_HAS_FEATURE(cxx_inheriting_constructors)
 *         UNITEX_HAS_FEATURE(cxx_inline_namespaces)
 *         UNITEX_HAS_FEATURE(cxx_lambdas)
 *         UNITEX_HAS_FEATURE(cxx_local_type_template_args)
 *         UNITEX_HAS_FEATURE(cxx_noexcept)
 *         UNITEX_HAS_FEATURE(cxx_nonstatic_member_init)
 *         UNITEX_HAS_FEATURE(cxx_nullptr)
 *         UNITEX_HAS_FEATURE(cxx_override_control)
 *         UNITEX_HAS_FEATURE(cxx_range_for)
 *         UNITEX_HAS_FEATURE(cxx_raw_string_literals)
 *         UNITEX_HAS_FEATURE(cxx_reference_qualified_functions)
 *         UNITEX_HAS_FEATURE(cxx_rtti)
 *         UNITEX_HAS_FEATURE(cxx_rvalue_references)
 *         UNITEX_HAS_FEATURE(cxx_static_asserts)
 *         UNITEX_HAS_FEATURE(cxx_strong_enums)
 *         UNITEX_HAS_FEATURE(cxx_trailing_return)
 *         UNITEX_HAS_FEATURE(cxx_unicode_literals)
 *         UNITEX_HAS_FEATURE(cxx_unrestricted_unions)
 *         UNITEX_HAS_FEATURE(cxx_user_literals)
 *         UNITEX_HAS_FEATURE(cxx_variadic_macros)                      // [+]
 *         UNITEX_HAS_FEATURE(cxx_variadic_templates)
 * @endcode
 */
#if UNITEX_COMPILER_IS_NOT(CLANG)
# define UNITEX_HAS_FEATURE(FeatureName)                                  \
                     (defined(UNITEX_HAS_FEATURE_##FeatureName)         &&\
                              UNITEX_HAS_FEATURE_##FeatureName)
#else   // use Clang __has_feature
# define UNITEX_HAS_FEATURE(FeatureName) __has_feature(FeatureName)     ||\
                          (defined(UNITEX_HAS_FEATURE_##FeatureName)    &&\
                                   UNITEX_HAS_FEATURE_##FeatureName)
#endif  // UNITEX_COMPILER_IS_NOT(CLANG)
/* ************************************************************************** */
#if UNITEX_COMPILER_IS_NOT(CLANG)
/**
 * @brief  __has_feature macro compatibility with non-Clang compilers
 * @note   it's preferred to use UNITEX_HAS_FEATURE than Clang's __has_feature
 */
# define __has_feature(x)     UNITEX_HAS_FEATURE(x)

/**
 * @brief  __has_extension macro compatibility with non-Clang compilers
 * @note   it's preferred to use UNITEX_HAS_FEATURE than Clang's __has_feature
 */
# define __has_extension(x)   UNITEX_HAS_FEATURE(x)
#endif  //  UNITEX_COMPILER_IS_NOT(CLANG)
/* ************************************************************************** */
#if UNITEX_COMPILER_COMPLIANT(CXX98)
/**
 * @brief  C++ exceptions have been enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_exceptions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
// GCC,ICC  : __EXCEPTIONS  if -fno-exceptions is used, then this macro is
//            not defined
# if defined(__EXCEPTIONS)   || _HAS_EXCEPTIONS > 0  ||\
     __cpp_exceptions >= 199711
#  if !defined(UNITEX_HAS_FEATURE_cxx_exceptions)
#   define UNITEX_HAS_FEATURE_cxx_exceptions                     1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_exceptions)
# endif  // defined(__EXCEPTIONS)   || _HAS_EXCEPTIONS > 0  || ...

/**
 * @brief  RTTI (Run-Time Type Information) has been enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_rtti)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
// GCC  : __GXX_RTTI  if -fno-rtti is used, then this macro is not defined
// MSVC : _CPPRTTI    Defined for code compiled with /GR
# if defined(__GXX_RTTI)     ||  defined(_CPPRTTI)   || __cpp_rtti >= 199711
#  if !defined(UNITEX_HAS_FEATURE_cxx_rtti)
#   define UNITEX_HAS_FEATURE_cxx_rtti                           1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_rtti)
# endif  // defined(__GXX_RTTI)     ||  defined(_CPPRTTI)   || ...
#endif  // UNITEX_COMPILER_COMPLIANT(CXX98)
/* ************************************************************************** */
/**
 * @brief  access-control errors are considered to be template argument
 *         deduction errors
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_access_control_sfinae)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(DR339) || UNITEX_CXX_PROPOSAL(N2634)
#  if !defined(UNITEX_HAS_FEATURE_cxx_access_control_sfinae)
#   define UNITEX_HAS_FEATURE_cxx_access_control_sfinae           1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_access_control_sfinae)
#endif  // UNITEX_CXX_PROPOSAL(DR339)

/**
 * @brief  Support for C++11's alias declarations and alias templates is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_alias_templates)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2258)    || __cpp_alias_templates >= 200704
#  if !defined(UNITEX_HAS_FEATURE_cxx_alias_templates)
#   define UNITEX_HAS_FEATURE_cxx_alias_templates                 1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_alias_templates)
#endif  // UNITEX_CXX_PROPOSAL(N2258)

/**
 * @brief  Support for alignment specifiers using alignas is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_alignas)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2341)
#  if !defined(UNITEX_HAS_FEATURE_cxx_alignas)
#   define UNITEX_HAS_FEATURE_cxx_alignas                         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_alignas)
#endif  // UNITEX_CXX_PROPOSAL(N2341)

/**
 * @brief  Support for the alignof keyword is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_alignof)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2341)
#  if !defined(UNITEX_HAS_FEATURE_cxx_alignof)
#   define UNITEX_HAS_FEATURE_cxx_alignof                         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_alignof)
#endif  // UNITEX_CXX_PROPOSAL(N2341)

/**
 * @brief  Support for attribute parsing with square bracket notation
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_attributes)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2761)    ||  __cpp_attributes >=  200809
#  if !defined(UNITEX_HAS_FEATURE_cxx_attributes)
#   define UNITEX_HAS_FEATURE_cxx_attributes                      1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_attributes)
#endif  // UNITEX_CXX_PROPOSAL(N2761)

/**
 * @brief  Type inference is supported using the `auto` specifier. If this is
 *         false, auto will instead be a storage class specifier
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_auto_type)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2546)
#  if !defined(UNITEX_HAS_FEATURE_cxx_auto_type)
#   define UNITEX_HAS_FEATURE_cxx_auto_type                       1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_auto_type)
#endif  // UNITEX_CXX_PROPOSAL(N2546)

/**
 * @brief  Support for generalized constant expressions using `constexpr`
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_constexpr)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2235)    || __cpp_constexpr >= 200704
#  if !defined(UNITEX_HAS_FEATURE_cxx_constexpr)
#   define UNITEX_HAS_FEATURE_cxx_constexpr                       1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_constexpr)
#endif  // UNITEX_CXX_PROPOSAL(N2235)

/**
 * @brief  Support for the `decltype` specifier is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_decltype)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2343)    || __cpp_decltype >= 200707
#  if !defined(UNITEX_HAS_FEATURE_cxx_decltype)
#   define UNITEX_HAS_FEATURE_cxx_decltype                        1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_decltype)
#endif  // UNITEX_CXX_PROPOSAL(N2343)

/**
 * @brief  Support for `decltype` thats not require type
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_decltype_incomplete_return_types)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(TODO)
#  if !defined(UNITEX_HAS_FEATURE_cxx_decltype_incomplete_return_types)
#   define UNITEX_HAS_FEATURE_cxx_decltype_incomplete_return_types 0
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_decltype_incomplete_return_types)
#endif  // UNITEX_CXX_PROPOSAL(TODO)

/**
 * @brief  Support for defaulted function definitions (with `= default`)
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_defaulted_functions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2346)
#  if !defined(UNITEX_HAS_FEATURE_cxx_defaulted_functions)
#   define UNITEX_HAS_FEATURE_cxx_defaulted_functions             1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_defaulted_functions)
#endif  // UNITEX_CXX_PROPOSAL(N2346)

/**
 * @brief  Support for default template arguments in function templates
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_default_function_template_args)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(DR226)
#  if !defined(UNITEX_HAS_FEATURE_cxx_default_function_template_args)
#   define UNITEX_HAS_FEATURE_cxx_default_function_template_args  1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_default_function_template_args)
#endif  // UNITEX_CXX_PROPOSAL(DR226)

/**
 * @brief  Support for delegating constructors
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_delegating_constructors)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N1986)    || __cpp_delegating_constructors >= 200604
#  if !defined(UNITEX_HAS_FEATURE_cxx_delegating_constructors)
#   define UNITEX_HAS_FEATURE_cxx_delegating_constructors         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_delegating_constructors)
#endif  // UNITEX_CXX_PROPOSAL(N1986)

/**
 * @brief  Support for deleted function definitions (with `= delete`)
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_deleted_functions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2346)
#  if !defined(UNITEX_HAS_FEATURE_cxx_deleted_functions)
#   define UNITEX_HAS_FEATURE_cxx_deleted_functions               1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_deleted_functions)
#endif  // UNITEX_CXX_PROPOSAL(N2346)

/**
 * @brief  Support for `explicit` conversion functions
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_explicit_conversions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2437)
#  if !defined(UNITEX_HAS_FEATURE_cxx_explicit_conversions)
#   define UNITEX_HAS_FEATURE_cxx_explicit_conversions             1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_explicit_conversions)
#endif  // UNITEX_CXX_PROPOSAL(N2437)

/**
 * @brief  Support for extern templates
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_extern_templates)
 * @endcode
 *
 * @note   `cxx_extern_templates` isn't available using __has_feature Clang macro
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N1987) ||\
    UNITEX_COMPILER_IS(CLANG)
#  if !defined(UNITEX_HAS_FEATURE_cxx_extern_templates)
#   define UNITEX_HAS_FEATURE_cxx_extern_templates                 1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_extern_templates)
#endif  // UNITEX_CXX_PROPOSAL(N1987)

/**
 * @brief  Support for generalized initializers using braced lists and
 *         `std::initializer_list`
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_generalized_initializers)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2672)    ||  __cpp_initializer_lists >= 200806
#  if !defined(UNITEX_HAS_FEATURE_cxx_generalized_initializers)
#   define UNITEX_HAS_FEATURE_cxx_generalized_initializers        1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_generalized_initializers)
#endif  // UNITEX_CXX_PROPOSAL(N2672)

/**
 * @brief  Support for implicitly generate move constructors and move assignment
 *         operators where needed
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_implicit_moves)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
// Rvalue references v3.0 (N3053) adds new rules to implicit generate move
// constructors and move assignment operators under certain conditions
#if UNITEX_CXX_PROPOSAL(N3053)
#  if !defined(UNITEX_HAS_FEATURE_cxx_implicit_moves)
#   define UNITEX_HAS_FEATURE_cxx_implicit_moves                  1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_implicit_moves)
#endif  // UNITEX_CXX_PROPOSAL(N3053)

/**
 * @brief  Support for inheriting constructors
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_inheriting_constructors)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2540)    ||  __cpp_inheriting_constructors >= 200802
#  if !defined(UNITEX_HAS_FEATURE_cxx_inheriting_constructors)
#   define UNITEX_HAS_FEATURE_cxx_inheriting_constructors         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_inheriting_constructors)
#endif  // UNITEX_CXX_PROPOSAL(N2540)

/**
 * @brief  Support for inline namespaces
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_inline_namespaces)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2535)
#  if !defined(UNITEX_HAS_FEATURE_cxx_inline_namespaces)
#   define UNITEX_HAS_FEATURE_cxx_inline_namespaces               1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_inline_namespaces)
#endif  // UNITEX_CXX_PROPOSAL(N2535)

/**
 * @brief  Support for lambdas
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_lambdas)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2927)    ||  __cpp_lambdas >= 200907
#  if !defined(UNITEX_HAS_FEATURE_cxx_lambdas)
#   define UNITEX_HAS_FEATURE_cxx_lambdas                         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_lambdas)
#endif  // UNITEX_CXX_PROPOSAL(N2927)

/**
 * @brief  Support for local and unnamed types as template arguments
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_local_type_template_args)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2657)
#  if !defined(UNITEX_HAS_FEATURE_cxx_local_type_template_args)
#   define UNITEX_HAS_FEATURE_cxx_local_type_template_args        1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_local_type_template_args)
#endif  // UNITEX_CXX_PROPOSAL(N2657)

/**
 * @brief  Support for `noexcept` exception specifications
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_noexcept)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N3050)
#  if !defined(UNITEX_HAS_FEATURE_cxx_noexcept)
#   define UNITEX_HAS_FEATURE_cxx_noexcept                        1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_noexcept)
#endif  // UNITEX_CXX_PROPOSAL(N3050)

/**
 * @brief  Determine whether in-class initialization of non-static data members
 *         is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_nonstatic_member_init)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2756)    || __cpp_nsdmi >= 200809
#  if !defined(UNITEX_HAS_FEATURE_cxx_nonstatic_member_init)
#   define UNITEX_HAS_FEATURE_cxx_nonstatic_member_init           1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_nonstatic_member_init)
#endif  // UNITEX_CXX_PROPOSAL(N2756)

/**
 * @brief  Support for `nullptr` is enabled
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_nullptr)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2431)
#  if !defined(UNITEX_HAS_FEATURE_cxx_nullptr)
#   define UNITEX_HAS_FEATURE_cxx_nullptr                         1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_nullptr)
#endif  // UNITEX_CXX_PROPOSAL(N2431)

/**
 * @brief  Support for the override control keywords (`override` and `final`)
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_override_control)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N3272)
#  if !defined(UNITEX_HAS_FEATURE_cxx_override_control)
#   define UNITEX_HAS_FEATURE_cxx_override_control                1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_override_control)
#endif  // UNITEX_CXX_PROPOSAL(N3272)

/**
 * @brief  Support for the range-based for loop
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_range_for)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2930)    ||  __cpp_range_based_for >= 200907
#  if !defined(UNITEX_HAS_FEATURE_cxx_range_for)
#   define UNITEX_HAS_FEATURE_cxx_range_for                       1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_range_for)
#endif  // UNITEX_CXX_PROPOSAL(N2930)

/**
 * @brief  Support for raw string literals
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_raw_string_literals)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2442)    || __cpp_raw_strings >= 200710
#  if !defined(UNITEX_HAS_FEATURE_cxx_raw_string_literals)
#   define UNITEX_HAS_FEATURE_cxx_raw_string_literals             1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_raw_string_literals)
#endif  // UNITEX_CXX_PROPOSAL(N2442)

/**
 * @brief  Support for reference-qualified functions
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_reference_qualified_functions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2439)    || __cpp_ref_qualifiers >= 200710
#  if !defined(UNITEX_HAS_FEATURE_cxx_reference_qualified_functions)
#   define UNITEX_HAS_FEATURE_cxx_reference_qualified_functions   1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_reference_qualified_functions)
#endif  // UNITEX_CXX_PROPOSAL(N2439)

/**
 * @brief  Support for rvalue references
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_rvalue_references)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2118)    || __cpp_rvalue_references >= 200610
#  if !defined(UNITEX_HAS_FEATURE_cxx_rvalue_references)
#   define UNITEX_HAS_FEATURE_cxx_rvalue_references               1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_rvalue_references)
#endif  // UNITEX_CXX_PROPOSAL(N2118)

/**
 * @brief  Support for compile-time assertions using `static_assert`
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_static_asserts)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N1720)    ||  __cpp_static_assert >= 200410
#  if !defined(UNITEX_HAS_FEATURE_cxx_static_asserts)
#   define UNITEX_HAS_FEATURE_cxx_static_asserts                  1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_static_asserts)
#endif  // UNITEX_CXX_PROPOSAL(N1720)

/**
 * @brief  Support for strongly typed scoped enumerations
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_strong_enums)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2347)
#  if !defined(UNITEX_HAS_FEATURE_cxx_strong_enums)
#   define UNITEX_HAS_FEATURE_cxx_strong_enums                    1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_strong_enums)
#endif  // UNITEX_CXX_PROPOSAL(N2347)

/**
 * @brief  Support for the alternate function declaration syntax with trailing
 *         return type
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_trailing_return)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2541)
#  if !defined(UNITEX_HAS_FEATURE_cxx_trailing_return)
#   define UNITEX_HAS_FEATURE_cxx_trailing_return                 1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_trailing_return)
#endif  // UNITEX_CXX_PROPOSAL(N2541)

/**
 * @brief  Support for `thread_local` variables
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_thread_local)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2659)
#  if !defined(UNITEX_HAS_FEATURE_cxx_thread_local)
#   define UNITEX_HAS_FEATURE_cxx_thread_local                    1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_thread_local)
#endif  // UNITEX_CXX_PROPOSAL(N2659)

/**
 * @brief  Support for Unicode string literals
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_unicode_literals)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2442)    || __cpp_unicode_literals >= 200710
#  if !defined(UNITEX_HAS_FEATURE_cxx_unicode_literals)
#   define UNITEX_HAS_FEATURE_cxx_unicode_literals                1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_unicode_literals)
#endif  // UNITEX_CXX_PROPOSAL(N2442)

/**
 * @brief  Support for unrestricted unions
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_unrestricted_unions)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2544)
#  if !defined(UNITEX_HAS_FEATURE_cxx_unrestricted_unions)
#   define UNITEX_HAS_FEATURE_cxx_unrestricted_unions             1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_unrestricted_unions)
#endif  // UNITEX_CXX_PROPOSAL(N2544)

/**
 * @brief  Support for user-defined literals
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_user_literals)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2765)    || __cpp_user_defined_literals >= 200809
#  if !defined(UNITEX_HAS_FEATURE_cxx_user_literals)
#   define UNITEX_HAS_FEATURE_cxx_user_literals                   1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_user_literals)
#endif  // UNITEX_CXX_PROPOSAL(N2765)

/**
 * @brief  Support for variadic macros
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_variadic_macros)
 * @endcode
 *
 * @note   `cxx_variadic_macros` isn't available using Clang's __has_feature,
 *          this is because all versions of Clang support variadic macros
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N1653)            ||\
    UNITEX_COMPILER_IS(CLANG)             ||\
    UNITEX_COMPILER_AT_LEAST_INTEL(12, 0) ||\
    UNITEX_COMPILER_AT_LEAST_ZOSXL(10, 1) ||\
    UNITEX_COMPILER_AT_LEAST_MSVC(14, 0)  ||\
    UNITEX_COMPILER_AT_LEAST_GCC(3, 3)
#  if !defined(UNITEX_HAS_FEATURE_cxx_variadic_macros)
#   define UNITEX_HAS_FEATURE_cxx_variadic_macros                 1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_variadic_macros)
#endif  // UNITEX_CXX_PROPOSAL(N1653)

/**
 * @brief  Support for variadic templates
 *
 * @code{.cpp}
 *         UNITEX_HAS_FEATURE(cxx_variadic_templates)
 * @endcode
 *
 * @see    UNITEX_HAS_FEATURE
 */
#if UNITEX_CXX_PROPOSAL(N2242)    || __cpp_variadic_templates >= 200704
#  if !defined(UNITEX_HAS_FEATURE_cxx_variadic_templates)
#   define UNITEX_HAS_FEATURE_cxx_variadic_templates              1
#  endif  // !defined(UNITEX_HAS_FEATURE_cxx_variadic_templates)
#endif  // UNITEX_CXX_PROPOSAL(N2242)
/* ************************************************************************** */
/**
 * @brief  Stores the retrieved standard language features in a (name,status)
 *         table. This is only useful for printing and debugging information
 *         purposes. e.g.
 *
 * @code{.cpp}
 *         int32_t i = 0;
 *         while(cxx_feature_status_table[i].name) {
 *           printf("%-40s %d\n", cxx_feature_status_table[i].name,
 *                                cxx_feature_status_table[i].status);
 *           ++i;
 *         }
 * @endcode
 *
 * @note   To test if a standard language feature is available use instead the
 *         UNITEX_HAS_FEATURE macro
 *
 * @see    UNITEX_HAS_FEATURE
 *
 */
const struct /* cxx_feature_status_table_t */ {
  // at file scope this is the same that static const struct
  const char* name;       // stringize C++ feature name
  int         status;     // 0: available, 1: unavailable
} cxx_feature_status_table[] = {
  {"cxx_access_control_sfinae",            UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_access_control_sfinae)[0]            != '1' ? 0 : 1 },  // NOLINT
  {"cxx_alias_templates",                  UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_alias_templates)[0]                  != '1' ? 0 : 1 },  // NOLINT
  {"cxx_alignas",                          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_alignas)[0]                          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_alignof",                          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_alignof)[0]                          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_attributes",                       UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_attributes)[0]                       != '1' ? 0 : 1 },  // NOLINT
  {"cxx_auto_type",                        UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_auto_type)[0]                        != '1' ? 0 : 1 },  // NOLINT
  {"cxx_constexpr",                        UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_constexpr)[0]                        != '1' ? 0 : 1 },  // NOLINT
  {"cxx_decltype",                         UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_decltype)[0]                         != '1' ? 0 : 1 },  // NOLINT
  {"cxx_decltype_incomplete_return_types", UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_decltype_incomplete_return_types)[0] != '1' ? 0 : 1 },  // NOLINT
  {"cxx_defaulted_functions",              UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_defaulted_functions)[0]              != '1' ? 0 : 1 },  // NOLINT
  {"cxx_default_function_template_args",   UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_default_function_template_args)[0]   != '1' ? 0 : 1 },  // NOLINT
  {"cxx_delegating_constructors",          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_delegating_constructors)[0]          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_deleted_functions",                UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_deleted_functions)[0]                != '1' ? 0 : 1 },  // NOLINT
  {"cxx_exceptions",                       UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_exceptions)[0]                       != '1' ? 0 : 1 },  // NOLINT
  {"cxx_explicit_conversions",             UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_explicit_conversions)[0]             != '1' ? 0 : 1 },  // NOLINT
  {"cxx_extern_templates",                 UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_extern_templates)[0]                 != '1' ? 0 : 1 },  // NOLINT
  {"cxx_generalized_initializers",         UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_generalized_initializers)[0]         != '1' ? 0 : 1 },  // NOLINT
  {"cxx_inheriting_constructors",          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_inheriting_constructors)[0]          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_inline_namespaces",                UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_inline_namespaces)[0]                != '1' ? 0 : 1 },  // NOLINT
  {"cxx_lambdas",                          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_lambdas)[0]                          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_local_type_template_args",         UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_local_type_template_args)[0]         != '1' ? 0 : 1 },  // NOLINT
  {"cxx_noexcept",                         UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_noexcept)[0]                         != '1' ? 0 : 1 },  // NOLINT
  {"cxx_nonstatic_member_init",            UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_nonstatic_member_init)[0]            != '1' ? 0 : 1 },  // NOLINT
  {"cxx_nullptr",                          UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_nullptr)[0]                          != '1' ? 0 : 1 },  // NOLINT
  {"cxx_override_control",                 UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_override_control)[0]                 != '1' ? 0 : 1 },  // NOLINT
  {"cxx_range_for",                        UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_range_for)[0]                        != '1' ? 0 : 1 },  // NOLINT
  {"cxx_raw_string_literals",              UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_raw_string_literals)[0]              != '1' ? 0 : 1 },  // NOLINT
  {"cxx_reference_qualified_functions",    UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_reference_qualified_functions)[0]    != '1' ? 0 : 1 },  // NOLINT
  {"cxx_rtti",                             UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_rtti)[0]                             != '1' ? 0 : 1 },  // NOLINT
  {"cxx_rvalue_references",                UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_rvalue_references)[0]                != '1' ? 0 : 1 },  // NOLINT
  {"cxx_static_asserts",                   UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_static_asserts)[0]                   != '1' ? 0 : 1 },  // NOLINT
  {"cxx_strong_enums",                     UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_strong_enums)[0]                     != '1' ? 0 : 1 },  // NOLINT
  {"cxx_trailing_return",                  UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_trailing_return)[0]                  != '1' ? 0 : 1 },  // NOLINT
  {"cxx_unicode_literals",                 UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_unicode_literals)[0]                 != '1' ? 0 : 1 },  // NOLINT
  {"cxx_unrestricted_unions",              UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_unrestricted_unions)[0]              != '1' ? 0 : 1 },  // NOLINT
  {"cxx_user_literals",                    UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_user_literals)[0]                    != '1' ? 0 : 1 },  // NOLINT
  {"cxx_variadic_macros",                  UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_variadic_macros)[0]                  != '1' ? 0 : 1 },  // NOLINT
  {"cxx_variadic_templates",               UNITEX_PP_STRINGIFY_VALUE(UNITEX_HAS_FEATURE_cxx_variadic_templates)[0]               != '1' ? 0 : 1 },  // NOLINT
  {NULL,                                   0}
};  // compiler_version_table[]
/* ************************************************************************** */
#endif  // UNITEX_BASE_COMPILER_FEATURES_H_                        // NOLINT
