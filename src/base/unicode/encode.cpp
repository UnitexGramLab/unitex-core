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
 * @date      September 2016, October 2021
 */
/* ************************************************************************** */
// Header for this file
#include "base/unicode/encode.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
size_t u_encode_utf8(const unichar* source, char* destination) {
  const unichar* it = source;
  int pos = 0;

  // loop till the end of string
  while (*it != '\0') {
    // ASCII characters lower than 0x80
    // 1-byte unicode codepoint
    if (*it <= 0x7F) {
      destination[pos++] = (char) *it;
    }
    // 2-bytes unicode codepoints
    else if (*it <= 0x7FF) {
      destination[pos++] = (char) (0xC0 | (*it >> 6));
      destination[pos++] = (char) (0x80 | (*it & 0x3F));
    }
    // 3-bytes unicode codepoints
    // Even if such behavior should be treated as an encoding error,
    // note that this allows the encoding of the code points between
    // 0xD800 and 0xDFFF, i.e. the code points for UTF-16 surrogates
    else if (*it <= 0xFFFF) {
      destination[pos++] = (char) (0xE0 | (*it >> 12));
      destination[pos++] = (char) (0x80 | ((*it >> 6) & 0x3F));
      destination[pos++] = (char) (0x80 | (*it & 0x3F));
    }
    // 4-bytes unicode codepoints
    else if (*it <= 0x10FFFF) {
      destination[pos++] = (char) (0xF0 | (*it  >> 18));
      destination[pos++] = (char) (0x80 | ((*it >> 12) & 0x3F));
      destination[pos++] = (char) (0x80 | ((*it >> 6)  & 0x3F));
      destination[pos++] = (char) (0x80 | (*it & 0x3F));
    }
    // if the codepoint is invalid encode it as the replacement char
    else {
      destination[pos++] = (char) kUTF8ReplacementChar[0];
      destination[pos++] = (char) kUTF8ReplacementChar[1];
      destination[pos++] = (char) kUTF8ReplacementChar[2];
    }
    // advance the character pointer
    ++it;
  }

  // indicate the end of the string
  destination[pos] = '\0';

  // return the length of the destination string
  return pos;
}

size_t u_decode_utf8(const char* source, unichar* destination) {
  const char* it = source;
  int pos = 0;

  // loop till the end of string
  while (*it != '\0') {
    // 1-byte encodes UTF-8 information
    uint8_t encoded_index  = kUTF8ByteInfoIndex[static_cast<uint8_t>(*it)];
    //
    uint8_t encoded_width  = kUTF8ByteInfo[encoded_index].width;
    //
    uint8_t encoded_offset = encoded_width - 1u;
    // set the replacement char (0xFFFD) as default value
    unichar value = U_REPLACEMENT_CHAR;

    if (u_has_valid_utf8_width(encoded_width)) {
      // 1-byte unicode codepoint
      value = *it & kUTF8ByteInfo[encoded_index].mask;

      // n-byte following unicode codepoints
      for (uint8_t i=0u; i < encoded_offset && *(it+1) != '\0'; ++i, ++it) {
         value = (value<<6) | ((*(it+1)) & 0x3F);
      }
    } else {
      it += encoded_offset;
    }

    // unichar = decoded value
    destination[pos++] = u_replace_if_invalid(value);

    // advance the character pointer
    ++it;
  }

  // indicate the end of the string
  destination[pos] = '\0';

  // return the length of the destination string
  return pos;
}

size_t u_decode_utf8_n(const char* source, unichar* destination, size_t n) {
  const char* it = source;
  int pos = 0;

  // loop till the end of string
  while (*it != '\0' && n--) {
    // 1-byte encodes UTF-8 information
    uint8_t encoded_index  = kUTF8ByteInfoIndex[static_cast<uint8_t>(*it)];
    //
    uint8_t encoded_width  = kUTF8ByteInfo[encoded_index].width;
    //
    uint8_t encoded_offset = encoded_width - 1u;
    // set the replacement char (0xFFFD) as default value
    unichar value = U_REPLACEMENT_CHAR;

    if (u_has_valid_utf8_width(encoded_width)) {
      // 1-byte unicode codepoint
      value = *it & kUTF8ByteInfo[encoded_index].mask;

      // n-byte following unicode codepoints
      for (uint8_t i=0u; i < encoded_offset && *(it+1) != '\0'; ++i, ++it) {
         value = (value<<6) | ((*(it+1)) & 0x3F);
      }
    } else {
      it += encoded_offset;
    }

    // unichar = decoded value
    destination[pos++] = u_replace_if_invalid(value);

    // advance the character pointer
    ++it;
  }

  // indicate the end of the string
  destination[pos] = '\0';

  // return the number of bytes that were read from source
  return (it - source);
}
/* ************************************************************************** */
}  // namespace unitex
