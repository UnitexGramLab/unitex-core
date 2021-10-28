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
 * @file      encoded.cpp
 * @brief     Functions to encode/decode unitex unicode strings
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors
 *
 * @date      October 2021
 */
/* ************************************************************************** */
// Unitex's .h files               (try to order the includes alphabetically)
#include "base/unicode/operations.h"
/* ************************************************************************** */
// Header for this file
#include "base/unicode/encode.h"
/* ************************************************************************** */
namespace unitex {
/**
 * @brief  Encode the unicode string into a C-string
 *
 * @param  An already allocated buffer destination (C-string)
 *
 * @return length of the destination string
 */
size_t u_encode(u_encode_t encoding, const unichar* source, char* destination) {
  size_t length = 0;
  switch (encoding) {
    case U_ENCODE_UTF8:
      length = u_encode_utf8(source, destination);
      break;
    default:
      break;
  }
  return length;
}

/**
 * @brief  Decode the C-string into a unicode string
 *
 * @param  An already allocated buffer destination (unichar)
 *
 * @return length of the destination string
 */
size_t u_decode_ns(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen,
    size_t dstsize) {
  size_t length = 0;
  switch (encoding) {
    case U_ENCODE_BINARY:
    case U_ENCODE_ASCII:
      //length = u_strlncpy(udst, csrc, readlen, maxlen, dstsize);
      break;
    case U_ENCODE_UTF8:
      // decode from UTF-8
      length = u_decode_utf8_ns(udst, csrc, readlen, maxlen, dstsize);
      break;
    default:
      break;
  }
  return length;
}

size_t u_decode_n(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen) {
  return u_decode_ns(encoding, udst, csrc, readlen, maxlen, U_MAX_BUFFER_SIZE);
}

/**
 *
 * @param encoding
 * @param source
 * @param destination
 * @param maxlen
 * @param count
 * @return
 */
size_t u_decode(u_encode_t encoding, unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc) {
  return u_decode_ns(encoding, udst, csrc, NULL, U_MAX_BUFFER_SIZE, U_MAX_BUFFER_SIZE);
}
/* ************************************************************************** */
}  // namespace unitex
