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
 * @file      encode.h
 * @brief     Functions to encode/decode unitex unicode strings
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors
 *
 * @date      September 2016, October 2021
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_ENCODE_H_                                // NOLINT
#define UNITEX_BASE_UNICODE_ENCODE_H_                                // NOLINT
/* ************************************************************************** */
#include "base/unicode/utf8.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * Flags for determining encoding/decoding types
 */
typedef enum {
  U_ENCODE_AUTO  = 0,  ///<
  U_ENCODE_BINARY   ,  ///<
  U_ENCODE_ASCII    ,  ///<
  U_ENCODE_UNICODE  ,  ///<
  U_ENCODE_UTF8     ,  ///<
  U_ENCODE_UTF16    ,  ///<
  U_ENCODE_UTF16_SE ,  ///<
  U_ENCODE_UTF16_LE ,  ///<
  U_ENCODE_UTF16_BE ,  ///<
  U_ENCODE_UTF32    ,  ///<
  U_ENCODE_UTF32_SE ,  ///<
  U_ENCODE_UTF32_LE ,  ///<
  U_ENCODE_UTF32_BE
} u_encode_t;
/* ************************************************************************** */
/**
 *
 * @param encoding
 * @param source
 * @param destination
 * @return length of the destination string
 */
size_t u_encode(u_encode_t encoding, const unichar* source, char* destination)
                UNITEX_PARAMS_NON_NULL;

/**
 *
 * @param encoding
 * @param source
 * @param destination
 * @return length of the destination string
 */
size_t u_decode_ns(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen,
    size_t dstsize)
    UNITEX_PARAMS_NON_NULL_(2, 3);

/**
 *
 * @param encoding
 * @param source
 * @param destination
 * @return length of the destination string
 */
size_t u_decode_n(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen)
    UNITEX_PARAMS_NON_NULL_(2, 3);

/**
 *
 * @param encoding
 * @param source
 * @param destination
 * @return length of the destination string
 */
size_t u_decode(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc)
    UNITEX_PARAMS_NON_NULL_(2, 3);
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_ENCODE_H_                              // NOLINT
