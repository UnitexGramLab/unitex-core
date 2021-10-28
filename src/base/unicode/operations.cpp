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
 * @file      operations.cpp
 * @brief     Functions to handle unitex unicode strings
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
// Unitex's .h files               (try to order the includes alphabetically)
#include "base/compiler/intrinsics.h"    // UNITEX_HAS_BUILTIN
#include "base/macro/helper/decls.h"     // UNITEX_MACRO_DECLS_*
#include "base/unicode/test.h"           // u_has_flag_fold_expands
/* ************************************************************************** */
// Header for this file
#include "base/unicode/operations.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * @brief  Finds within the first count bytes of the block of memory pointed by s
 *         for the first occurrence of unichar c and returns a pointer to it
 *
 * @param s Pointer to the block of memory where the search is performed
 * @param c The unichar to be located
 * @param count Number of bytes to be analyzed
 * @return
 */
const unichar* u_memchr(const unichar* ustr, unichar uchr, size_t maxlen) {
#if !(UNITEX_HAS_CPU_EXTENSION(SSE2) && UNITEX_HAS_BUILTIN(CTZ))
# define U__MEMCHR__UNROLL__TEST__(it, c, count, n)  \
  if (it[n] == c) return count + n < maxlen ? (it + n) : 0

  const unichar* uptr = ustr;

  // number of chars already scanned
  size_t count = 0;

  for (;count < maxlen;) {
    U__MEMCHR__UNROLL__TEST__(uptr, uchr, count, 0);
    U__MEMCHR__UNROLL__TEST__(uptr, uchr, count, 1);
    U__MEMCHR__UNROLL__TEST__(uptr, uchr, count, 2);
    U__MEMCHR__UNROLL__TEST__(uptr, uchr, count, 3);
    uptr += 4; count += 4;
  }

  // this is not supposed to happen
  return 0;

# undef U__MEMCHR__UNROLL__TEST__
#else
# define U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, pattern, mask)   \
  data = _mm_load_si128((const __m128i*) uptr);                         \
  mask = _mm_movemask_epi8(_mm_cmpeq_epi16(data, pattern))

# define U__SSE2__UNICHAR__PATTERN__RETURN__POINTER__(str, n, maxlen, mask) \
  UNITEX_MACRO_DECLS_BEGIN                                                  \
  if (mask) {                                                               \
    size_t index =  unitex_builtin_ctz_32(mask) / U_CHAR_SIZE;              \
    return  ((size_t) (n) + index < maxlen) ?                               \
                              (unichar *)(str + (size_t) (n) + index) : 0;  \
  }                                                                         \
  UNITEX_MACRO_DECLS_END

# define U__SSE2__UNICHAR__PER__BLOCK__(T) (sizeof(T) / U_CHAR_SIZE)

# define U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr, count)  \
  uptr  += (U__SSE2__UNICHAR__PER__BLOCK__(__m128i));       \
  count += (U__SSE2__UNICHAR__PER__BLOCK__(__m128i))

  // number of characters already scanned
  size_t count = 0;

  // 16-bit mask from most significant bits
  uint32_t mask;

  // find a 16-bytes alignment
  uint32_t align = ((uint32_t)(uintptr_t) (ustr) ) & 0xf;
  const unichar* uptr = ustr - (align / U_CHAR_SIZE);

  // vector of pattern
  const __m128i pattern =  _mm_set1_epi16((unichar) uchr);

  // vector of unichar data
  __m128i data;

  // compute the mask
  U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, pattern, mask);

  // make sure to have an aligned block
  mask >>= align;

  // if we already found the pattern in the first block
  U__SSE2__UNICHAR__PATTERN__RETURN__POINTER__(ustr, 0, maxlen, mask);

  // if not, we need to jump to the next block
  U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr, count);

  // and scan the remaining aligned blocks
  for(;count < maxlen;) {
    U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, pattern, mask);
    U__SSE2__UNICHAR__PATTERN__RETURN__POINTER__(ustr, (uptr - ustr), maxlen, mask);
    U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr, count);
  }
  // this is not supposed to happen
  return 0;

# undef U__SSE2__UNICHAR__BLOCK__INCREMENT__
# undef U__SSE2__UNICHAR__PER__BLOCK__
# undef U__SSE2__UNICHAR__PATTERN__RETURN__POINTER__
# undef U__SSE2__UNICHAR__MASK__COMPUTE__
#endif  //  UNITEX_HAS_CPU_EXTENSION(SSE2) && UNITEX_HAS_BUILTIN(CTZ)
}

/**
 * @brief  Finds within the first count bytes of the block of memory pointed by s
 *         for the first occurrence of unichar c and returns a pointer to it
 *
 * @param ustr The string where the search is performed
 * @param uchr The character to be located
 * @return
 */
const unichar* u_strchr(const unichar* ustr, unichar uchr) {
# define U__STRCHR__UNROLL__TEST__(it, c, n)  \
  if (it[n] == 0) return 0;                   \
  if (it[n] == c) return (it + n)

  const unichar* uptr = ustr;

  for (;;) {
    U__STRCHR__UNROLL__TEST__(uptr, uchr, 0);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, 1);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, 2);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, 3);
    uptr += 4;
  }

  // this is not supposed to happen
  return 0;

#undef U__STRCHR__UNROLL__TEST__
}

/**
 * @brief  Finds within the first count bytes of the block of memory pointed by s
 *         for the first occurrence of unichar c and returns a pointer to it
 *
 * @param s Pointer to the block of memory where the search is performed
 * @param c The unichar to be located
 * @param count Number of bytes to be analyzed
 * @return
 */
const unichar* u_strnchr(const unichar* ustr, unichar uchr, size_t maxlen) {
# define U__STRCHR__UNROLL__TEST__(it, c, count, n, maxlen)  \
  if (it[n] == 0) return 0;                                  \
  if (it[n] == c) return count + n < maxlen ? (it + n) : 0

  const unichar* uptr = ustr;

  // number of chars already scanned
  size_t count = 0;

  for (;count < maxlen;) {
    U__STRCHR__UNROLL__TEST__(uptr, uchr, count, 0, maxlen);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, count, 1, maxlen);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, count, 2, maxlen);
    U__STRCHR__UNROLL__TEST__(uptr, uchr, count, 3, maxlen);
    uptr += 4; count += 4;
  }

  // this is not supposed to happen
  return 0;

#undef U__STRCHR__UNROLL__TEST__
}

/**
 * @brief  Copies characters from a C-string pointed by source to
 *         a unicode string pointed to by destination
 *
 * @param  udst Pointer to the unicode string to copy to
 * @param  usrc Pointer to the C-string to copy from
 * @param  uchr Terminating character
 * @param  maxlen Number of characters to copy
 *
 * @return If the character (unichar)c was found u_memccpy returns a pointer
 *         to the next character in dest after uchr,
 *         otherwise returns null pointer.
 */
unichar* u_memccpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT usrc,
                   unichar uchr, size_t maxlen) {
  const char* it_src = usrc;
  for (unichar* it_dst = udst; maxlen; ++it_dst, ++it_src, --maxlen) {
    *it_dst = (unichar)((unsigned char)*it_src);
    if (*it_dst == uchr) {
      return it_dst + 1;
    }
  }
  return 0;
}

/**
 * @brief  Returns the length of the string s, if that is less than
 *         maxlen, or maxlen if there is no null terminating
 */
size_t u_strnlen(const unichar* str, size_t maxlen) {
  const unichar* uptr = u_memchr(str, '\0', maxlen);
  return UNITEX_LIKELY(uptr != NULL) ? (size_t) (uptr - str) : maxlen;
}

/**
 * @brief  Returns the length of the string s
 */
size_t u_strlen(const unichar* ustr) {
#if !(UNITEX_HAS_CPU_EXTENSION(SSE2) && UNITEX_HAS_BUILTIN(CTZ))
  const unichar* uptr;
  for (uptr = ustr; *uptr; ++uptr) {}
  return (uptr - ustr);
#else // UNITEX_HAS_CPU_EXTENSION(SSE2) && UNITEX_HAS_BUILTIN(CTZ)
# define U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, pattern, mask)   \
  data = _mm_load_si128((const __m128i*) uptr);                         \
  mask = _mm_movemask_epi8(_mm_cmpeq_epi16(data, pattern))

# define U__SSE2__UNICHAR__PATTERN__RETURN__INDEX__(n, mask)            \
  UNITEX_MACRO_DECLS_BEGIN                                              \
  if (mask) {                                                           \
    return (size_t) (n) + unitex_builtin_ctz_32(mask) / U_CHAR_SIZE;    \
  }                                                                     \
  UNITEX_MACRO_DECLS_END

# define U__SSE2__UNICHAR__PER__BLOCK__(T) (sizeof(T) / U_CHAR_SIZE)

# define U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr)  \
  uptr += (U__SSE2__UNICHAR__PER__BLOCK__(__m128i))

  // 16-bit mask from most significant bits
  uint32_t mask;

  // find a 16-bytes alignment
  uint32_t align = ((uint32_t)(uintptr_t) (ustr) ) & 0xf;
  const unichar* uptr = ustr - (align / U_CHAR_SIZE);

  // vector of zeros
  const __m128i zero = _mm_setzero_si128();

  // vector of unichar data
  __m128i data;

  // compute the mask
  U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, zero, mask);

  // make sure to have an aligned block
  mask >>= align;

  // if we already found the pattern in the first block
  U__SSE2__UNICHAR__PATTERN__RETURN__INDEX__(0, mask);

  // if not, we need to jump to the next block
  U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr);

  // and scan the remaining aligned blocks
  for(;;) {
    U__SSE2__UNICHAR__MASK__COMPUTE__(data, uptr, zero, mask);
    U__SSE2__UNICHAR__PATTERN__RETURN__INDEX__((uptr - ustr), mask);
    U__SSE2__UNICHAR__BLOCK__INCREMENT__(uptr);
  }
  // this is not supposed to happen
  return 0;

# undef U__SSE2__UNICHAR__BLOCK__INCREMENT__
# undef U__SSE2__UNICHAR__PER__BLOCK__
# undef U__SSE2__UNICHAR__PATTERN__RETURN__INDEX__
# undef U__SSE2__UNICHAR__MASK__COMPUTE__
#endif  //  UNITEX_HAS_CPU_EXTENSION(SSE2) && UNITEX_HAS_BUILTIN(CTZ)
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
int u_strncmp(const unichar* UNITEX_RESTRICT s1, const unichar* UNITEX_RESTRICT s2, size_t maxlen) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);
  // find on which block of bytes there are a difference
  U__BLOCKSTRCMP__(s1, s2, maxlen);
  // find the first character that is different inside the detected block
  U__STRCMP__(unichar, s1, unichar, s2, block * elements_per_block, pos < maxlen);
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
int u_strnicmp(const unichar* s1, const unichar* s2, size_t maxlen) {
  // if any of the two strings is equal to null
  U__STRCMP__NULL__(s1, s2);

  const unichar*  it1 = s1;
  const unichar*  it2 = s2;
  const u_info_t* info1 = (const u_info_t *) '\0';
  const u_info_t* info2 = (const u_info_t *) '\0';

  while(maxlen) {
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
    --maxlen;
  }

  return maxlen == 0 ? 0 : (*(const unsigned int *)it1 - *(const unsigned int *)it2);
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
size_t u_reverse(unichar* str) {
  const size_t len = u_strlen(str);
  U__REVERSE__(unichar, str, len);
  return len;
}

/**
 * @brief  Reverse a string
 */
size_t u_reverse(unichar* ustr, size_t maxlen) {
  U__REVERSE__(unichar, ustr, maxlen);
  return maxlen;
}

#define U__REVERSE__DEST__(s_t, s, d_t, d, l)   \
  if (s == NULL || !(*s)) return 0;             \
  const s_t*  it_end = s + l - 1;               \
  while (it_end >= s) {                         \
    *d = *it_end;                               \
    d++;                                        \
    it_end--;                                   \
  }                                             \

size_t u_reverse(const unichar* UNITEX_RESTRICT usrc, unichar* UNITEX_RESTRICT udst) {
  const size_t len = u_strlen(usrc);
  U__REVERSE__DEST__(unichar, usrc, unichar, udst, len);
  return len;
}

size_t u_reverse(const unichar* UNITEX_RESTRICT usrc, unichar* UNITEX_RESTRICT udst, size_t maxlen) {
  U__REVERSE__DEST__(unichar, usrc, unichar, udst, maxlen);
  return maxlen;
}

#undef U__REVERSE__
#undef U__REVERSE__DEST__

/**
 * @brief  Copy at most (maxlen-strlen(destination)-1) characters of the source C-string
 *         to the destination unicode string
 *
 * @param  destination
 * @param  source
 * @param  maxlen
 * @return The length of source, in other words the number of characters that would have been copied
 *
 * @note   As this function always writes a single NULL byte to the destination, the resulting
 *         string is guaranteed to be NULL-terminated even if truncated
 */
size_t u_strlncpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT usrc,
                  size_t* readlen, size_t maxlen, size_t dstsize) {
  // we need to known the length of the source string
  const size_t srclen = strlen(usrc);

  // continue only if maxlen is positive
  if (maxlen > 0) {
    // compute the max number of characters that we can copy
    const size_t max_count = srclen + 1 < maxlen ? srclen + 1 : maxlen - 1;
    // copy max_count characters from source to the destination
    unichar* result = u_memccpy(udst, usrc, '\0', max_count);
    // always writes a single NULL byte at the end
    *result = '\0';
  }

  // return the length of the source C-string
  return srclen;
}

size_t u_strlcpy(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT usrc,
                 size_t* readlen, size_t maxlen) {
  return u_strlncpy(udst, usrc, readlen, maxlen, U_MAX_STRING_LENGTH);
}

/**
 * @brief  Append at most (maxlen-strlen(destination)-1) characters of the source C-string
 *         to the destination unicode string
 *
 * @param  dst
 * @param  src
 * @param  maxlen
 * @return The length of source plus destination, in other words the number of characters that
 *         would have the resulting string
 *
 * @note   As this function always writes a single NULL byte to the destination, the resulting
 *         string is guaranteed to be NULL-terminated even if truncated
 */
size_t u_strlncat(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT usrc,
                  size_t* readlen, size_t maxlen, size_t dstsize) {
  // the initial length of the destination string
  const size_t dstlen = u_strnlen(udst, maxlen);
  // copy at least count characters at end of the destination string
  const size_t srclen = u_strlncpy(udst + dstlen, usrc, readlen, maxlen, dstsize);

  // return the length of both strings
  // FIXME(martinec) Avoid MIN Macro
  // @see https://dustri.org/b/min-and-max-macro-considered-harmful.html
  return srclen + MIN(maxlen, dstlen);
}

size_t u_strlcat(unichar* UNITEX_RESTRICT udst, const char* UNITEX_RESTRICT usrc,
    size_t* readlen, size_t maxlen) {
  return u_strlncat(udst, usrc, readlen, maxlen, U_MAX_STRING_LENGTH);
}
/* ************************************************************************** */
}  // namespace unitex
