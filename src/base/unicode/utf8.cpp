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
 * @file      utf8.cpp
 * @brief     Functions to handle UTF-8
 *
 * @author    cristian.martinez@unitexgramlab.org (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 `utf8.cpp`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
// Unitex's .h files               (try to order the includes alphabetically)
#include "base/integer/integer.h"
/* ************************************************************************** */
// Header for this file
#include "base/unicode/utf8.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * UTF-8 is variable-Width (1 to 4 bytes)
 *
 * Length    Code Points              1st Byte            Mask  2nd Byte  3rd Byte  4th Byte  i
 *   0     0xD800..0xDFFF    80..BF   128..191  00000000  0x00  ******* ill-formed *******    0
 *   1     0x0000..0x007F    00..7F   000..127  0xxxxxxx  0x7f                                1
 *   2     0x0080..0x07FF    C2..DF   194..223  110xxxxx  0x1f  80..BF                        2
 *   3     0x0800..0x0FFF    E0..E0   224..224  1110xxxx  0x0f  A0..BF    80..BF              3
 *   3     0x1000..0xCFFF    E1..EC   225..236  1110xxxx  0x0f  80..BF    80..BF              4
 *   3     0xD000..0xD7FF    ED..ED   237..237  1110xxxx  0x0f  80..9F    80..BF              5
 *   3     0xE000..0xFFFF    EE..EF   238..239  1110xxxx  0x0f  80..BF    80..BF              6
 *   4    0x10000..0x3FFFF   F0..F0   240..240  11110xxx  0x07  90..BF    80..BF    80..BF    7
 *   4    0x40000..0xFFFFF   F1..F3   241..243  11110xxx  0x07  80..BF    80..BF    80..BF    8
 *   4   0x100000..0x10FFFF  F4..F4   244..244  11110xxx  0x07  80..8F    80..BF    80..BF    9
 *
 *   1st Byte between C0 and C1 (to try to encode ASCII characters with two bytes
 *   instead of one) or between F5 and FF (to try to encode numbers larger than
 *   the maximal unicode codepoint) are also assigned to the index 0 and thus
 *   considered invalid
 */
const u_info_utf8_t kUTF8ByteInfo[] = {
  {0,0x00,{0x0000,  0x0000  },{{0x0000,0x0000},{0x0000,0x0000},{0x0000,0x0000},{0x0000,0x0000}}},  // 0
  {1,0x7f,{0x0000,  0x007F  },{{0x0000,0x007F},{0x0000,0x0000},{0x0000,0x0000},{0x0000,0x0000}}},  // 1
  {2,0x1f,{0x0080,  0x07FF  },{{0x00C2,0x00DF},{0x0080,0x00BF},{0x0000,0x0000},{0x0000,0x0000}}},  // 2
  {3,0x0f,{0x0080,  0x0FFF  },{{0x00E0,0x00E0},{0x00A0,0x00BF},{0x0080,0x00BF},{0x0000,0x0000}}},  // 3
  {3,0x0f,{0x1000,  0xCFFF  },{{0x00E1,0x00EC},{0x0080,0x00BF},{0x0080,0x00BF},{0x0000,0x0000}}},  // 4
  {3,0x0f,{0xD000,  0xD7FF  },{{0x00ED,0x00ED},{0x0080,0x009F},{0x0080,0x00BF},{0x0000,0x0000}}},  // 5
  {3,0x0f,{0xE000,  0xFFFF  },{{0x00EE,0x00EF},{0x0080,0x00BF},{0x0080,0x00BF},{0x0000,0x0000}}},  // 6
  {4,0x07,{0x10000, 0x3FFFF },{{0x00F0,0x00F0},{0x0090,0x00BF},{0x0080,0x00BF},{0x0080,0x00BF}}},  // 7
  {4,0x07,{0x40000, 0xFFFFF },{{0x00F1,0x00F3},{0x0080,0x00BF},{0x0080,0x00BF},{0x0080,0x00BF}}},  // 8
  {4,0x07,{0x100000,0x10FFFF},{{0x00F4,0x00F4},{0x0080,0x00BF},{0x0080,0x00BF},{0x0080,0x00BF}}},  // 9
};

/**
 *
 */
const uint8_t kUTF8ByteInfoIndex[] = {
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 000-015
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 016-031
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 032-047
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 048-063
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 064-079
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 080-095
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 096-111
                     1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 112-127
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 128-143
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 144-159
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 160-175
                     0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 176-191
                     0, 0, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 192-207
                     2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2,  // 208-223
                     3, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 5, 6, 6,  // 224-239
                     7, 8, 8, 8, 9, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 240-255
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

size_t u_decode_utf8_ns(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen,
    size_t dstsize) {
  const char* it = csrc;
  int pos = 0;

  // loop till the end of string
  while (*it != '\0' && --maxlen) {
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
    udst[pos++] = u_replace_if_invalid(value);

    // advance the character pointer
    ++it;
  }

  // indicate the end of the string
  udst[pos] = '\0';

  // update read length
  if (readlen) { *readlen = (it - csrc); }

  // return the decoded length
  return pos;
}

size_t u_decode_utf8_n(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen, size_t maxlen) {
  return u_decode_utf8_ns(udst, csrc, readlen, maxlen, U_MAX_BUFFER_SIZE);
}

size_t u_decode_utf8(unichar* UNITEX_RESTRICT udst,
    const char* UNITEX_RESTRICT csrc, size_t* readlen) {
  return u_decode_utf8_ns(udst, csrc, readlen, U_MAX_BUFFER_SIZE, U_MAX_BUFFER_SIZE);
}

/* ************************************************************************** */
}  // namespace unitex
