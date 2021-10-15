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
#include "base/compiler/intrinsic/assume_aligned.h"
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
/**
 * @brief  Returns the length of the string s
 */
size_t u_strlen(const unichar* s) {
  const unichar *it;
  for (it = s; *it; ++it) {}
  return (it - s);
}

// all U__* macros must be undefined at the end of this file
#define U__STRCMP__NULL__(s1, s2)                        \
    if (UNITEX_UNLIKELY((s1 == NULL) || (s2 == NULL))) { \
      if ((s1 == NULL) && (s2 == NULL)) return  0;       \
      if  (s1 == NULL)                  return  1;       \
      else                              return -1;       \
    }

#define U__STRCMP__(s1_t, s1, s2_t, s2, init, cond)                                        \
  const s1_t* it1 = s1;                                                                    \
  const s2_t* it2 = s2;                                                                    \
  s1_t c1 = '\0';                                                                          \
  s2_t c2 = '\0';                                                                          \
  size_t pos = init;                                                                       \
  for (; cond ; pos += 4) {                                                                \
    c1 = *(it1+pos);   c2= *(it2+pos)   ; if (c1=='\0'){ break; } if ((c1-c2)) { break; }  \
    c1 = *(it1+pos+1); c2= *(it2+pos+1) ; if (c1=='\0'){ break; } if ((c1-c2)) { break; }  \
    c1 = *(it1+pos+2); c2= *(it2+pos+2) ; if (c1=='\0'){ break; } if ((c1-c2)) { break; }  \
    c1 = *(it1+pos+3); c2= *(it2+pos+3) ; if (c1=='\0'){ break; } if ((c1-c2)) { break; }  \
  }


#define U__BLOCKSTRCMP__(s1, s2, n)                                                          \
  size_t block = 0;                                                                          \
  const size_t elements_per_block = sizeof(uintptr_t)   / sizeof(unichar);                   \
  const size_t number_of_blocks   = sizeof(unichar) * n / sizeof(uintptr_t);                 \
  size_t repeat = number_of_blocks / 2;                                                      \
  if(repeat) {                                                                               \
    const uintptr_t* r0 = reinterpret_cast<const uintptr_t*>(UNITEX_ASSUME_ALIGNED(s1, 16)); \
    const uintptr_t* r1 = reinterpret_cast<const uintptr_t*>(UNITEX_ASSUME_ALIGNED(s2, 16)); \
    while (repeat--) {                                                                       \
      if ((*(r0+block)   - *(r1+block))   != 0) {           break; }                         \
      if ((*(r0+block+1) - *(r1+block+1)) != 0) { block+=1; break; }                         \
      block += 2;                                                                            \
    }                                                                                        \
  }

/**
 * @brief  Compares two strings
 *
 * @param  s1 A null-terminated character sequence (unichar-string)
 * @param  s2 A null-terminated character sequence (unichar-string)
 * @return Returns a signed integral indicating the relation between
 *         the strings:
 *         -  0 : They compare equal
 *         - <0 : Either the value of the first character that does not match
 *                is lower in the compared string, or all compared characters
 *                match but the compared string is shorter
 *         - >0 : Either the value of the first character that does not match
 *                is greater in the compared string, or all compared characters
 *                match but the compared string is longer.
 *
 * @note   Null strings are allowed
 */
int u_strcmp(const unichar* s1, const unichar* s2) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);
  // compare the two non-null strings
  U__STRCMP__(unichar, s1, unichar, s2, 0,);
  // return a signed integral indicating the relation between the strings
  return (c1 == '\0') ? -(unsigned int)c2 :
                        ((unsigned int)c1 - (unsigned int)c2);
}

/**
 * @brief  Compares the specified number of characters of two strings
 *
 * @param  s1 A null-terminated character sequence (unichar-string)
 * @param  s2 A null-terminated character sequence (unichar-string)
 * @param  n  Number of characters to compare
 * @return Returns a signed integral indicating the relation between
 *         the strings:
 *         -  0 : They compare equal
 *         - <0 : Either the value of the first character that does not match
 *                is lower in the compared string, or all compared characters
 *                match but the compared string is shorter
 *         - >0 : Either the value of the first character that does not match
 *                is greater in the compared string, or all compared characters
 *                match but the compared string is longer.
 *
 * @note   Null strings are allowed
 */
int u_strncmp(const unichar* UNITEX_RESTRICT s1, const unichar* UNITEX_RESTRICT s2, size_t n) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);
  // find on which block of bytes there are a difference
  U__BLOCKSTRCMP__(s1, s2, n);
  // find the first character that is different inside the detected block
  U__STRCMP__(unichar, s1, unichar, s2, block * elements_per_block, pos < n);
  // return a signed integral indicating the relation between the strings
  return (c1 == '\0') ? -(unsigned int)c2 :
                        ((unsigned int)c1 - (unsigned int)c2);
}

/**
 * @brief  Compares the specified number of characters of two strings without
 *         regard to case
 *
 * @param  s1 A null-terminated character sequence (unichar-string)
 * @param  s2 A null-terminated character sequence (unichar-string)
 * @param  n  Number of characters to compare
 * @return Returns a signed integral indicating the relation between
 *         the strings:
 *         -  0 : They compare equal
 *         - <0 : Either the value of the first character that does not match
 *                is lower in the compared string, or all compared characters
 *                match but the compared string is shorter
 *         - >0 : Either the value of the first character that does not match
 *                is greater in the compared string, or all compared characters
 *                match but the compared string is longer.
 *
 * @note   Null strings are allowed
 */
int u_strnicmp(const unichar* s1, const unichar* s2, size_t n) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);

  const unichar*  it1 = s1;
  const unichar*  it2 = s2;
  const u_info_t* info1 = (const u_info_t *) '\0';
  const u_info_t* info2 = (const u_info_t *) '\0';

  while(n) {
    if(*it1 == '\0' || *it2 == '\0') {
      break;
    }

    info1 = u_info(*it1);
    info2 = u_info(*it2);

    // 0: s1 and s2 have simple case folding
    // 1: s2 have full case folding
    // 2: s1 have full case folding
    // 3: s1 and s2 have both full case folding
    int fold_expands = 2 * u_has_flag_fold_expands(info1) +
                           u_has_flag_fold_expands(info2);

    // simple case folding compare (where string lengths don't change)
    if(UNITEX_LIKELY(fold_expands == 0)) {
      if(*it1 + info1->variant[U_CASE_FOLD] !=
         *it2 + info2->variant[U_CASE_FOLD]) {
        break;
      }
    // s1 or s2 have full case folding
    } else if (fold_expands == 1 || fold_expands == 2) {
      int index  = fold_expands == 1 ? info2->variant[U_CASE_FOLD] :
                                       info1->variant[U_CASE_FOLD];
      const unichar** it_unfold = fold_expands == 1 ? &it1 : &it2;
      const unichar*  it_fold   = &kUSpecialVariants[index + 1];
      size_t length = kUSpecialVariants[index];
      int strings_not_equal = u_strnicmp(*it_unfold,it_fold,length);
      if (strings_not_equal) return strings_not_equal;
      (*it_unfold) += (length-1);
    // both of strings have full case folding
    } else {
      if (info1->variant[U_CASE_FOLD] !=
          info2->variant[U_CASE_FOLD]) {
        break;
      }
    }

    ++it1;
    ++it2;
    --n;
  }

  return n == 0 ? 0 : (*(const unsigned int *)it1 - *(const unsigned int *)it2);
}

/**
 * @brief  Compares two strings without regard to case
 *
 * @param  s1 A null-terminated character sequence (unichar-string)
 * @param  s2 A null-terminated character sequence (unichar-string)
 * @return Returns a signed integral indicating the relation between
 *         the strings:
 *         -  0 : They compare equal
 *         - <0 : Either the value of the first character that does not match
 *                is lower in the compared string, or all compared characters
 *                match but the compared string is shorter
 *         - >0 : Either the value of the first character that does not match
 *                is greater in the compared string, or all compared characters
 *                match but the compared string is longer.
 *
 * @note   Null strings are allowed
 */
int u_stricmp(const unichar* s1, const unichar* s2) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);

  const unichar*  it1 = s1;
  const unichar*  it2 = s2;
  const u_info_t* info1 = (const u_info_t *) '\0';
  const u_info_t* info2 = (const u_info_t *) '\0';

  for(;;) {
    if(*it1 == '\0' || *it2 == '\0') {
      break;
    }

    info1 = u_info(*it1);
    info2 = u_info(*it2);

    // 0: s1 and s2 have simple case folding
    // 1: s2 have full case folding
    // 2: s1 have full case folding
    // 3: s1 and s2 have both full case folding
    int fold_expands = 2 * u_has_flag_fold_expands(info1) +
                           u_has_flag_fold_expands(info2);

    // simple case folding compare (where string lengths don't change)
    if(UNITEX_LIKELY(fold_expands == 0)) {
      if(*it1 + info1->variant[U_CASE_FOLD] !=
         *it2 + info2->variant[U_CASE_FOLD]) {
        break;
      }
    // s1 or s2 have full case folding
    } else if (fold_expands == 1 || fold_expands == 2) {
      int index  = fold_expands == 1 ? info2->variant[U_CASE_FOLD] :
                                       info1->variant[U_CASE_FOLD];
      const unichar** it_unfold = fold_expands == 1 ? &it1 : &it2;
      const unichar*  it_fold   = &kUSpecialVariants[index + 1];
      size_t length = kUSpecialVariants[index];
      int strings_not_equal = u_strnicmp(*it_unfold,it_fold,length);
      if (strings_not_equal) return strings_not_equal;
      (*it_unfold) += (length-1);
    // both of strings have full case folding
    } else {
      if (info1->variant[U_CASE_FOLD] !=
          info2->variant[U_CASE_FOLD]) {
        break;
      }
    }

    ++it1;
    ++it2;
  }

  return (*(const unsigned int *)it1 - *(const unsigned int *)it2);
}

#define U__REVERSE__(s_t, s, l)     \
  if (s == NULL || !(*s)) return 0; \
  s_t tmp = '\0';                   \
  s_t*  it_end = s + l - 1;         \
  while (it_end > s) {              \
    tmp = *s;                       \
    *s = *it_end;                   \
    *it_end = tmp;                  \
    s++;                            \
    it_end--;                       \
  }

/**
 * @brief  Reverse a string
 */
size_t u_reverse(unichar* s) {
  size_t len = u_strlen(s);
  U__REVERSE__(unichar, s, len);
  return len;
}

/**
 * @brief  Reverse a string
 */
size_t u_reverse(unichar* s, size_t n) {
  U__REVERSE__(unichar, s, n);
  return n;
}

#define U__REVERSE__DEST__(s_t, s, d_t, d, l)   \
  if (s == NULL || !(*s)) return 0;             \
  const s_t*  it_end = s + l - 1;               \
  while (it_end >= s) {                         \
    *d = *it_end;                               \
    d++;                                        \
    it_end--;                                   \
  }                                             \

size_t u_reverse(const unichar* UNITEX_RESTRICT s, unichar* UNITEX_RESTRICT d) {
  size_t len = u_strlen(s);
  U__REVERSE__DEST__(unichar, s, unichar, d, len);
  return len;
}

size_t u_reverse(const unichar* UNITEX_RESTRICT s, unichar* UNITEX_RESTRICT d, size_t n) {
  U__REVERSE__DEST__(unichar, s, unichar, d, n);
  return n;
}


#undef U__REVERSE__
#undef U__REVERSE__DEST__
/* ************************************************************************** */
}  // namespace unitex
