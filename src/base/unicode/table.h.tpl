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
##header
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_TABLE_H_                                // NOLINT
#define UNITEX_BASE_UNICODE_TABLE_H_                                // NOLINT
/* ************************************************************************** */
#include "base/api/unmangle.h"
#include "base/integer/types.h"
#include "base/compiler/attribute/inline.h"
#include "base/compiler/attribute/nonnull.h"
#include "base/compiler/intrinsic/likely.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
/**
 * @def      UNITEX_UNICODE
 * @brief    The unicode version implemented

 * @details  Set UNITEX_UNICODE as VVRRPPPPP (Version, Release, Patch)
 *
 * @note     Do not use this directly, rather use UNITEX_UNICODE_AT_LEAST
 *
 * @see      UNITEX_UNICODE_AT_LEAST
 */
#define UNITEX_UNICODE {v:unicodeVersion}

/**
 * @brief  Test the implemented unicode version in the format (version, release, patch)
 * @return true if the implemented unicode version is at least (version, release, patch)
 */
# define UNITEX_UNICODE_AT_LEAST_(v, r, p) \
         (defined(UNITEX_UNICODE) &&\
                  UNITEX_UNICODE >= (v * 10000000 + r * 100000 + p))
/**
 * @brief  Test the implemented unicode version in the format (version, release)
 * @return true if the implemented unicode version is at least (version, release, 0)
 */
# define UNITEX_UNICODE_AT_LEAST(v, r) \
         UNITEX_UNICODE_AT_LEAST_(v, r, 0)

/**
 * @def      {c:MaxValue}
 * @brief    Maximum allowed unicode value
 */
#define {c:MaxValue} {v:maxValue}

/**
 * @def      U_SIZE_MAX
 * @brief    Maximum allowed unicode buffer size
 */
#if defined(SIZE_MAX)
#define U_SIZE_MAX SIZE_MAX
#elif defined(__SIZE_MAX__)
#define U_SIZE_MAX __SIZE_MAX__
#else
#define U_SIZE_MAX (static_cast<size_t>(-1))
#endif  // defined(SIZE_MAX)

/**
 * @def      {c:ReplacementChar}
 * @brief    The unicode replacement character used when decoding byte sequences
 *           that cannot be successfully converted
 */
#define {c:ReplacementChar} 0xFFFD

/**
 * @def      {c:Size}
 * @brief    This is the size in bits of a unicode character
 */
#define {c:Size} {v:charSize}

/**
 * @def      {v:charName}
 * @brief    This is the type of a unicode character. Note that it is a
 *           {v:charSize}-bits type and it cannot handle characters beyond {v:maxValue}
 */
typedef {v:charType} {v:charName};

/**
 * `{n:InfoT}` integer member types
 */
{if:addFlags}
#define {n:FlagsIntT}      uint32_t
{endif:}
{if:addBlocks}
#define {n:BlockIntT}      uint16_t
{endif:}
{if:addCategories}
#define {n:CategoryIntT}   uint8_t
{endif:}
{if:addScripts}
#define {n:ScriptIntT}     uint8_t
{endif:}
{if:addBidiClasses}
#define {n:BidiIntT}       uint8_t
{endif:}
{if:addNormalization}
#define {n:NfqcIntT}       uint8_t
{endif:}
{if:addDecomposition}
#define {n:CccIntT}        uint8_t
#define {n:TagIntT}        uint8_t
#define {n:MapIntT}        int32_t
{endif:}
{if:addVariants}
#define {n:VariantsIntT}   int32_t
{endif:}
{if:addNumbers}
#define {n:NumbersIntT}    int64_t
{endif:}

{if:addFlags}
/**
 * Flags for determining character type
 */
typedef enum {
  ##flags
} {n:FlagT};

{endif:}
{if:addBlocks}
/**
 * Unicode blocks
 * Every Unicode block is discrete, and cannot overlap with any other block
 * @see http://unicode.org/faq/blocks_ranges.html#4
 */
typedef enum {
##blocks
} {n:BlockT};

{endif:}
{if:addCategories}
/**
 * Unicode categories
 */
typedef enum {
##categories
} {n:CategoryT};

{endif:}
{if:addScripts}
/**
 * Unicode scripts
 * @see http://www.unicode.org/reports/tr24
 */
typedef enum {
##scripts
} {n:ScriptT};

{endif:}
{if:addBidiClasses}
/**
 * Unicode bidirectional class values
 * @see http://unicode.org/reports/tr44/#Bidi_Class_Values
 */
typedef enum {
##bidiClasses
} {n:BidiT};

{endif:}
{if:addNormalization}
/**
 * Canonical and Compatibility Equivalence masks
 * @see http://www.unicode.org/reports/tr15/#Canon_Compat_Equivalence
 */
typedef enum {
##normalizationMasks
} {n:NormalizationMask};

/**
 * Unicode Normalization Forms
 * @see http://www.unicode.org/reports/tr15
 */
 typedef enum {
##normalizationForms
} {n:NormalizationForm};

/**
 * Quick_Check property values
 * @see http://www.unicode.org/reports/tr15/#Quick_Check_Table
 */
 typedef enum {
##normalizationFormQuickCheckValues
} {n:NormalizationFormQuickCheckValue};

/**
 * Quick_Check property flags
 * @see http://unicode.org/reports/tr44/#QC_Values_Table
 */
 typedef enum {
##normalizationFormQuickCheckFlags
} {n:NormalizationFormQuickCheckFlag};

{endif:}
{if:addDecomposition}
/**
 * Unicode Canonical Combining Class values
 * @see http://unicode.org/reports/tr44/#Canonical_Combining_Class_Values
 */
typedef enum {
##decompositionCccs
} {n:DecompositionCccT};

/**
 * Unicode compatibility formatting tags
 * @see http://www.unicode.org/reports/tr44/#Character_Decomposition_Mappings
 */
typedef enum {
##decompositionTags
} {n:DecompositionTagT};

{endif:}
{if:addVariants}
/**
 * Index usable for `variant` in `{n:InfoT}`
 *
 * If `{n:InfoT}.variant[c]` is `0`, that specific variant does not exist.
 * If one of the EXPANDS flags is set in `{n:flag}`,  `c` will expands into
 * multiple characters, in this case `{n:InfoT}.variant[c]` is an index usable
 * within the array `{k:SpecialVariants}`. The value that is pointed by
 * {k:SpecialVariants}[index] indicates the length of the expanded sequence,
 * i.e. if {k:SpecialVariants}[index] is equal to n, the expanded sequence will
 * be located between {k:SpecialVariants}[index+1] and {k:SpecialVariants}[index+n]
 */
typedef enum {
  {c:CaseUpper}    = 0,  ///< value to be added to `c` to get its uppercase variant
  {c:CaseLower}    = 1,  ///< value to be added to `c` to get its lowercase variant
  {c:CaseTitle}    = 2,  ///< value to be added to `c` to get its titlecase variant
  {c:CaseFold}     = 3,  ///< value to be added to `c` to get its foldcase variant
{endif:}
{if:addVariantUnnacent}
  {c:CharUnaccent} = 4,  ///< unaccent variant of  `c` (no arithmetic performed)
{endif:}
{if:addVariantAsciify}
  {c:CharAsciify}  = 5,  ///< value to be added to `c` to get its asciify variant
{endif:}
{if:addVariants}
} {n:VariantT};

{endif:}
/**
 * Character info
 */
typedef struct {
{if:addFlags}
  {n:FlagsIntT}    flags;       ///< flags: 0 or more flags listed in `{n:flag}`
{endif:}
{if:addBlocks}
  {n:BlockIntT}    block;       ///< block: one of the Unicode blocks listed in `{n:BlockT}`
{endif:}
{if:addCategories}
  {n:CategoryIntT} category;    ///< category: One of the categories listed in `{n:CategoryT}`
{endif:}
{if:addScripts}
  {n:ScriptIntT}   script;      ///< script: One of the scripts listed in `{n:ScriptT}`
{endif:}
{if:addBidiClasses}
  {n:BidiIntT}     bidi;        ///< bidi class: one of the classes listed in `{n:BidiT}`
{endif:}
{if:addDecomposition}
  struct {
{endif:}
{if:addNormalization}
    {n:NfqcIntT}   nfqc;        ///< normalization form quick check: 0 or more flags listed in `{n:NormalizationFormQuickCheckFlag}`
{endif:}
{if:addDecomposition}
    {n:CccIntT}    ccc;         ///< canonical combining class: number used in the canonical ordering algorithm
    {n:TagIntT}    tag;         ///< decomposition tag: one of the tags listed in `{n:DecompositionT}`
    {n:MapIntT}    map[3];      ///< decomposition mapping, `{c:FlagWideMap}` set when consist of 4 or more
  } decomposition;
{endif:}
{if:addVariants}
  {n:VariantsIntT} variant[{v:variantsSize}];  ///< indexable with `{n:VariantT}`
{endif:}
{if:addNumbers}
  union {
    {n:NumbersIntT} num;  ///< number value if `flags & {c:FlagNumber}`
    const char* frac;    ///< fraction string if `flags & {c:FlagFraction}`
  };
{endif:}
} {n:InfoT};

/**
 * Unicode code points lookup table
 */
extern const {n:InfoT} {k:Info}[];

/**
 * All Unicode pages
 * Points to `{k:InfoIndex}`
 */
extern const {v:pagesType} {k:PageIndex}[];

/**
 * Indexes of {k:Info}
 * {k:InfoIndex}[page index][index in page]
 */
extern const {v:infoType} {k:InfoIndex}[][256];
{if:addVariants}

/**
 * Special variants characters
 */
extern const {v:charName} {k:SpecialVariants}[];
{endif:}
{if:addDecomposition}

/**
 * Decomposition mappings constituted by 4 or more characters
 */
extern const {v:charName} {k:SpecialMappings}[];
{endif:}

/**
 * Test if the code point is valid
 */
#define {n:IsValid}(c) UNITEX_LIKELY(c < 0xD800 || (c > 0xDFFF &&  c <= U_MAX_VALUE))

/**
 * Test if the code point is invalid
 */
#define {n:IsInvalid}(c) UNITEX_UNLIKELY(c >= 0xD800 && (c  <= 0xDFFF || c > U_MAX_VALUE))

/**
 * If the code point is invalid replace it by {c:ReplacementChar}
 */
#define {n:ReplaceIfInvalid}(c) {n:IsValid}(c) ? c : {c:ReplacementChar}

/**
 * Lookup character
 */
#define {n:Info}(c) (&{k:Info}[{k:InfoIndex}[{k:PageIndex}[c >> 8u]][c & 0xFF]])
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 * Lookup the character info by its code point
 *
 * @param  c The character to lookup
 * @return If a character with the given code point is found, return the
 *         corresponding {n:InfoT} entry, If not found {k:Info}[0] is returned
 * @see    `{v:charName}_info`
 */
UNITEX_FORCE_INLINE
const {n:InfoT}* {n:Lookup}(const {v:charName} c) {
  {v:pagesType} page;
  {v:infoType} offset;

  if ({n:IsInvalid}(c)) {
    return &{k:Info}[0];
  }

  page = {k:PageIndex}[c >> 8u];
  offset = {k:InfoIndex}[page][c & 0xFF];

  return &{k:Info}[offset];
}
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_TABLE_H_                             // NOLINT
