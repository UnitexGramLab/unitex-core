/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * @file      variant.h
 * @brief     Functions to get Unicode v9.0.0 code points variants
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @warning   These functions don't handle code points greater than 0xFFFF
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 case.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_CASE_H_                                // NOLINT
#define UNITEX_BASE_UNICODE_CASE_H_                                // NOLINT
/* ************************************************************************** */
#include "base/unicode/table.h"
#include "base/unicode/test.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
/* ************************************************************************** */
// all U__* macros must be undefined at the end of this file
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__CASE__ITERATE__FIRST__(_name, _true, _false)  \
UNITEX_FORCE_INLINE                                                         \
void u_to##_name##_first(unichar* s) UNITEX_PARAMS_NON_NULL;                \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
void u_to##_name##_first(unichar* s) {                                      \
  register unichar* it = s;                                                 \
  const u_info_t* info =  u_info(*it);                                      \
  if (*it != '\0') { _true; ++it; } else { return; }                        \
  while (*it != '\0') {                                                     \
    info = u_info(*(it-1));                                                 \
    if (u_has_flag_space(info)) {                                           \
      _true;                                                                \
    } else {                                                                \
      _false;                                                               \
    }                                                                       \
    ++it;                                                                   \
  }                                                                         \
}
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__CASE__ITERATE__ALL__(_name)                   \
UNITEX_FORCE_INLINE                                                         \
void _name(unichar* s) UNITEX_PARAMS_NON_NULL;                              \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
void _name(unichar* s) {                                                    \
  register unichar* it = s;                                                 \
  while (*it != '\0') {                                                     \
    *it = _name(*it);                                                       \
    ++it;                                                                   \
  }                                                                         \
}                                                                           \
                                                                            \
size_t _name(const unichar* s, unichar* d) UNITEX_PARAMS_NON_NULL;          \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
size_t _name(const unichar* s, unichar* d) {                                \
  const register unichar* it = s;                                           \
  while (*it != '\0') {                                                     \
    *d++ = _name(*it);                                                      \
    ++it;                                                                   \
  }                                                                         \
  return (it - s);                                                          \
}
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__CASE__ITERATE__N__(_name)                     \
UNITEX_FORCE_INLINE                                                         \
void _name##_n(unichar* s, size_t n) UNITEX_PARAMS_NON_NULL;                \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
void _name##_n(unichar* s, size_t n) {                                      \
  register unichar* it = s;                                                 \
  while(n) {                                                                \
    if (*it == '\0') {                                                      \
      break;                                                                \
    }                                                                       \
    *it = _name(*it);                                                       \
    ++it;                                                                   \
    --n;                                                                    \
  }                                                                         \
}
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__CASE__(_name, _u_variant_t)                   \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
unichar u_to##_name(unichar c);                                             \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
unichar u_to##_name(unichar c, const u_info_t* u_info);                     \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
unichar u_to##_name(unichar c, const u_info_t* u_info) {                    \
  int index = u_info->variant[_u_variant_t];                                \
  if (UNITEX_LIKELY(!u_has_flag_##_name##_expands(u_info))) {               \
    return c + index;                                                       \
  }                                                                         \
  return kUSpecialVariants[index + kUSpecialVariants[index] + 1];           \
}                                                                           \
                                                                            \
UNITEX_FORCE_INLINE                                                         \
unichar u_to##_name(unichar c) {                                            \
  const u_info_t* u_info = u_info(c);                                       \
  int index = u_info->variant[_u_variant_t];                                \
  if (UNITEX_LIKELY(!u_has_flag_##_name##_expands(u_info))) {               \
    return c + index;                                                       \
  }                                                                         \
  return kUSpecialVariants[index + kUSpecialVariants[index] + 1];           \
}                                                                           \
U__DECLARE__FUNCTION__CASE__ITERATE__ALL__(u_to##_name)                     \
U__DECLARE__FUNCTION__CASE__ITERATE__N__(u_to##_name)
/* ************************************************************************** */
U__DECLARE__FUNCTION__CASE__(upper, U_CASE_UPPER)
U__DECLARE__FUNCTION__CASE__(lower, U_CASE_LOWER)
U__DECLARE__FUNCTION__CASE__(title, U_CASE_TITLE)
U__DECLARE__FUNCTION__CASE__(fold,  U_CASE_FOLD)
/* ************************************************************************** */
U__DECLARE__FUNCTION__CASE__ITERATE__FIRST__(title,
                                             *it = u_totitle(*it),
                                             *it = u_tolower(*it))
/* ************************************************************************** */
UNITEX_FORCE_INLINE
unichar u_deaccentuate(unichar c) {
  unichar unnacent = u_info(c)->variant[U_CHAR_UNACCENT];
  return UNITEX_UNLIKELY(unnacent>0) ? unnacent : c;
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__CASE__ITERATE__ALL__(u_deaccentuate)
U__DECLARE__FUNCTION__CASE__ITERATE__N__(u_deaccentuate)
/* ************************************************************************** */
/* ************************************************************************** */
#undef U__DECLARE__FUNCTION__CASE__
#undef U__DECLARE__FUNCTION__CASE__ITERATE__N__
#undef U__DECLARE__FUNCTION__CASE__ITERATE__ALL__
#undef U__DECLARE__FUNCTION__CASE__ITERATE__FIRST__
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_CASE_H_                              // NOLINT
