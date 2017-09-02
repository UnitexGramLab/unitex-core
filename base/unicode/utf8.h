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
 * @file      utf8.h
 * @brief     Functions to handle UTF-8
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @warning   These functions don't handle code points greater than 0xFFFF
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 utf8.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_UTF8_H_                                 // NOLINT
#define UNITEX_BASE_UNICODE_UTF8_H_                                 // NOLINT
/* ************************************************************************** */
#include "base/unicode/table.h"
#include "base/unicode/test.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * UTF-8 info
 */
typedef struct {
  uint8_t    width;   ///<
  unichar    mask;    ///<

  struct {
    uint32_t min;     ///<
    uint32_t max;     ///<
  } range;

  struct {
    unichar  min;     ///<
    unichar  max;     ///<
  } byte[4];

} u_info_utf8_t;

/**
 *
 */
extern const u_info_utf8_t kUTF8ByteInfo[];

/**
 *
 */
extern const uint8_t kUTF8ByteInfoIndex[];

/**
 *
 *
 * @param source
 * @param destination
 * @return the length of the destination string
 */
int u_encode_utf8(const unichar* source, char* destination) UNITEX_PARAMS_NON_NULL;

/**
 *
 *
 * @param source
 * @param destination
 * @return
 */
int u_decode_utf8(const char* source, unichar* destination) UNITEX_PARAMS_NON_NULL;

size_t u_strlen(const unichar* s);

int u_strcmp(const unichar* s1, const unichar* s2);

int u_strncmp(const unichar* UNITEX_RESTRICT s1, const unichar* UNITEX_RESTRICT s2, size_t n);

int u_strnicmp(const unichar* s1, const unichar* s2, size_t n);

int u_stricmp(const unichar* s1,const unichar* s2);
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
/* ************************************************************************** */

/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_UTF8_H_                              // NOLINT
