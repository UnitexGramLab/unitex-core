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
 * @file      utf8.h
 * @brief     Functions to handle UTF-8
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
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
 * @def      U_MAX_WIDTH_UTF8
 * @brief    Maximal numbers of UTF8 bytes that a unichar can encode
 * @see      U_MAX_VALUE
 */
#define U_MAX_WIDTH_UTF8 3

/**
 * Test if the code point is valid
 */
#define u_has_valid_utf8_width(n) UNITEX_LIKELY(n > 0 && n <= U_MAX_WIDTH_UTF8)

/**
 * @def      kUTF8ReplacementChar
 * @brief    The unicode replacement character used when decoding byte sequences
 *           that cannot be successfully converted
 * @see      U_REPLACEMENT_CHAR
 */
const uint8_t kUTF8ReplacementChar[] = {0xEF, 0xBF, 0xBD};
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
 * @return length of the destination string
 */
size_t u_encode_utf8(const unichar* source, char* destination)
  ;

/**
 *
 * @param source
 * @param destination
 * @param n
 * @return length of decoded destination string
 */
size_t u_decode_utf8_ns(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen,
    size_t maxlen, size_t dstsize)
    UNITEX_PARAMS_NON_NULL_(1, 2);

/**
 *
 * @param source
 * @param destination
 * @param n
 * @return length of decoded destination string
 */
size_t u_decode_utf8_n(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen)
    UNITEX_PARAMS_NON_NULL_(1, 2);

/**
 *
 * @param source
 * @param destination
 * @param n
 * @return length of decoded destination string
 */
size_t u_decode_utf8(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen)
    UNITEX_PARAMS_NON_NULL_(1, 2);

/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_UTF8_H_                              // NOLINT
