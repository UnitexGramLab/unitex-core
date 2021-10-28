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
 * @file      operations.h
 * @brief     Unicode library
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 unicode.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_OPERATIONS_H_                           // NOLINT
#define UNITEX_BASE_UNICODE_OPERATIONS_H_                           // NOLINT
/* ************************************************************************** */
// C system files                  (try to order the includes alphabetically)
#include <string.h>
/* ************************************************************************** */
// Unitex's .h files               (try to order the includes alphabetically)
#include "base/compiler/attributes.h"  // UNITEX_PARAMS_NON_NULL
#include "base/unicode/table.h"        // unichar
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/// cchr      C-char
/// cdst      Pointer to the destination C-string
/// csrc      Pointer to the source C-string
/// cstr      C-string
/// maxlen    The maximum number of characters to scan
/// uchr      Unitex char
/// udst      Pointer to the destination Unitex string
/// usrc      Pointer to the source Unitex string
/// ustr      Unitex string
/// ustr1     Unitex string
/// ustr2     Unitex string
/// udstsize  Size in characters of the destination buffer
/* ************************************************************************** */
const unichar* u_memchr(const unichar* ustr, unichar uchr, size_t maxlen)
UNITEX_PARAMS_NON_NULL_(1);
unichar* u_memccpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT csrc, unichar uchr, size_t maxlen)
UNITEX_PARAMS_NON_NULL_(1, 2);
/* ************************************************************************** */
const unichar* u_strchr(const unichar* ustr, unichar uchr)
UNITEX_PARAMS_NON_NULL_(1);
const unichar* u_strnchr(const unichar* ustr, unichar uchr, size_t maxlen)
UNITEX_PARAMS_NON_NULL_(1);
/* ************************************************************************** */
size_t u_strlen(const unichar* ustr)
UNITEX_PARAMS_NON_NULL_(1);
size_t u_strnlen(const unichar* ustr, size_t maxlen)
UNITEX_PARAMS_NON_NULL_(1);
/* ************************************************************************** */
//size_t u_strlncpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen, size_t udstsize)
//UNITEX_PARAMS_NON_NULL_(1, 2);
//size_t u_strlncat(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen, size_t udstsize)
//UNITEX_PARAMS_NON_NULL_(1, 2);
//size_t u_strlcpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen)
//UNITEX_PARAMS_NON_NULL_(1, 2);
//size_t u_strlcat(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen)
//UNITEX_PARAMS_NON_NULL_(1, 2);
/* ************************************************************************** */
int u_strcmp(const unichar* ustr1, const unichar* ustr2);
int u_strncmp(const unichar* ustr1, const unichar*  ustr2, size_t maxlen);
int u_strnicmp(const unichar* ustr1, const unichar* ustr2, size_t maxlen);
int u_stricmp(const unichar* ustr1,const unichar* ustr2);
/* ************************************************************************** */
size_t u_reverse(unichar* ustr);
size_t u_reverse(unichar* ustr, size_t maxlen);
size_t u_reverse(const unichar* UNITEX_RESTRICT usrc, unichar* UNITEX_RESTRICT udst)
UNITEX_PARAMS_NON_NULL_(2);
size_t u_reverse(const unichar* UNITEX_RESTRICT usrc, unichar* UNITEX_RESTRICT udst, size_t maxlen)
UNITEX_PARAMS_NON_NULL_(2);
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_OPERATIONS_H_                        // NOLINT
