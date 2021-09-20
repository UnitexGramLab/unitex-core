/*
 * Unitex
 *
 * Copyright (C) 2001-2021 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * @file      test.h
 * @brief     Functions to test Unicode v9.0.0 code points
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @warning   These functions don't handle code points greater than 0xFFFF
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 test.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_TEST_H_                                 // NOLINT
#define UNITEX_BASE_UNICODE_TEST_H_                                 // NOLINT
/* ************************************************************************** */
#include "base/unicode/table.h"
#include "base/preprocessor/punctuation.h"
#include "base/preprocessor/util.h"
#include "base/preprocessor/variadic.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
/* ************************************************************************** */
// all U__* macros must be undefined at the end of this file
/* ************************************************************************** */
#define U__PARAMS(...)        U__PARAMS_(UNITEX_PP_VA_NARGS(__VA_ARGS__),__VA_ARGS__)
#define U__PARAMS_(n, ...)    UNITEX_PP_TOKEN_CAT_(U__PARAMS__, n)(__VA_ARGS__)
#define U__PARAMS__0()        UNITEX_PP_EMPTY()
#define U__PARAMS__1(_1)      UNITEX_PP_COMMA() u_\#\#_1\#\#_int_t _1
#define U__PARAMS__2(_1,...)  U__PARAMS__1(_1) U__PARAMS__1(__VA_ARGS__)
#define U__PARAMS__3(_1,...)  U__PARAMS__1(_1) U__PARAMS__2(__VA_ARGS__)
#define U__PARAMS__4(_1,...)  U__PARAMS__1(_1) U__PARAMS__3(__VA_ARGS__)
#define U__PARAMS__5(_1,...)  U__PARAMS__1(_1) U__PARAMS__4(__VA_ARGS__)
#define U__PARAMS__6(_1,...)  U__PARAMS__1(_1) U__PARAMS__5(__VA_ARGS__)
#define U__PARAMS__7(_1,...)  U__PARAMS__1(_1) U__PARAMS__6(__VA_ARGS__)
#define U__PARAMS__8(_1,...)  U__PARAMS__1(_1) U__PARAMS__7(__VA_ARGS__)
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__TEST__(_name,_prepare,_test,...)                   \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar c  U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL;       \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const u_info_t* u_info U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL; \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar* s U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL;       \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar c U__PARAMS(__VA_ARGS__)) {                              \
  if(u_is_invalid(c)) return 0;                                                  \
  const u_info_t* u_info = u_info(c);                                            \
  _prepare                                                                       \
  return _test;                                                                  \
}                                                                                \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const u_info_t* u_info U__PARAMS(__VA_ARGS__)) {                       \
  _prepare                                                                       \
  return _test;                                                                  \
}                                                                                \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar* s U__PARAMS(__VA_ARGS__))  {                            \
  const unichar* it = s;                                                         \
  while (*it != '\0') {                                                          \
    if (!_name(*it UNITEX_PP_IF(UNITEX_PP_VA_NARGS(__VA_ARGS__))(                \
                UNITEX_PP_COMMA() UNITEX_PP_EXPAND(__VA_ARGS__),                 \
                UNITEX_PP_EMPTY())))  {                                          \
      return -(it - s);                                                          \
    }                                                                            \
    ++it;                                                                        \
  }                                                                              \
  return (it - s);                                                               \
}
/* ************************************************************************** */
{if:addFlags}
U__DECLARE__FUNCTION__TEST__(u_test_flag,,
                            ((u_info->flags & flags) != 0),
                            flags)
{endif:}
{if:addBlocks}
U__DECLARE__FUNCTION__TEST__(u_test_block,,
                            (u_info->block == block),
                            block)
{endif:}
{if:addCategories}
U__DECLARE__FUNCTION__TEST__(u_test_category,,
                            (u_info->category == category),
                            category)
{endif:}
{if:addScripts}
U__DECLARE__FUNCTION__TEST__(u_test_script,,
                            (u_info->script == script),
                            script)
{endif:}
{if:addBidiClasses}
U__DECLARE__FUNCTION__TEST__(u_test_bidi,,
                            (u_info->bidi == bidi),
                            bidi)
{endif:}
{if:addNormalization}
U__DECLARE__FUNCTION__TEST__(u_test_nf_quick_check,,
                            ((u_info->decomposition.nfqc & nfqc) != 0),
                            nfqc)
{endif:}
{if:addDecomposition}
U__DECLARE__FUNCTION__TEST__(u_test_decomposition_tag,,
                            ((u_info->decomposition.tag & tag) != 0),
                            tag)
{endif:}
/* *********************************************************************************************************************** */
#define u_is_digit(c)               u_test_flag(c,U_FLAG_DIGIT)       // #
#define u_is_letter(c)              u_test_flag(c,U_FLAG_LETTER)      // #
#define u_is_identifier(c)          u_test_flag(c,U_FLAG_IDENTIFIER)  // #
{if:addFlags}
/* *********************************************************************************************************************** */
##test_has_flag
{endif:}
{if:addBlocks}
/* *********************************************************************************************************************** */
##test_in_block
{endif:}
{if:addCategories}
/* *********************************************************************************************************************** */
##test_in_category
{endif:}
{if:addScripts}
/* *********************************************************************************************************************** */
##test_in_script
{endif:}
{if:addBidiClasses}
/* *********************************************************************************************************************** */
##test_has_bidi
{endif:}
{if:addNormalization}
/* *********************************************************************************************************************** */
##test_has_normalization_form_quick_check
{endif:}
{if:addDecomposition}
/* *********************************************************************************************************************** */
##test_has_decomposition_tag
{endif:}
/* *********************************************************************************************************************** */
#undef U__DECLARE__FUNCTION__TEST__
#undef U__PARAMS__8
#undef U__PARAMS__7
#undef U__PARAMS__6
#undef U__PARAMS__5
#undef U__PARAMS__4
#undef U__PARAMS__3
#undef U__PARAMS__2
#undef U__PARAMS__1
#undef U__PARAMS__0
#undef U__PARAMS_
#undef U__PARAMS
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_TEST_H_                              // NOLINT
