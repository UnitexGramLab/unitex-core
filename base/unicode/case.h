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
UNITEX_FORCE_INLINE
unichar u_toupper(unichar c) {
  const u_info_t* u_info = u_info(c);

  int index = u_info->variant[U_CASE_UPPER];

  if (UNITEX_LIKELY(!u_has_flag_upper_expands(u_info))) {
    return c + index;
  }

  return kUSpecialVariants[index + kUSpecialVariants[index] + 1];
}

UNITEX_FORCE_INLINE
void u_toupper(unichar* s) UNITEX_PARAMS_NON_NULL;

UNITEX_FORCE_INLINE
void u_toupper(unichar* s) {
  register unichar* it = s;
  while (*it != '\0') {
    *it = u_toupper(*it);
    ++it;
  }
}


UNITEX_FORCE_INLINE
unichar u_tolower(unichar c) {
  const u_info_t* u_info = u_info(c);

  int index = u_info->variant[U_CASE_LOWER];

  if (UNITEX_LIKELY(!u_has_flag_lower_expands(u_info))) {
    return c + index;
  }

  return kUSpecialVariants[index + kUSpecialVariants[index] + 1];
}

UNITEX_FORCE_INLINE
unichar u_deaccentuate(unichar c) {
  unichar unnacent = u_info(c)->variant[U_CHAR_UNACCENT];
  return UNITEX_UNLIKELY(unnacent>0) ? unnacent : c;
}
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_CASE_H_                             // NOLINT
