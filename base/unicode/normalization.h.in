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
#ifndef UNITEX_BASE_UNICODE_NORMALIZATION_H_                        // NOLINT
#define UNITEX_BASE_UNICODE_NORMALIZATION_H_                        // NOLINT
/* ************************************************************************** */
#include "base/unicode/table.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
#define {n:NfqcIntT}       uint8_t
#define {n:CccIntT}        uint8_t
#define {n:TagIntT}        uint8_t
#define {n:MapIntT}        int32_t

/**
 * Flags for determining ***
 */
typedef enum {
  ##normalizationFlags
} {n:NormFlagT};

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

/**
 * Character decomposition and normalization info
 */
typedef struct {
    {n:NfqcIntT} nfqc;    ///< normalization form quick check: 0 or more flags listed in `{n:NormalizationFormQuickCheckFlag}`
    {n:CccIntT}  ccc;     ///< canonical combining class: number used in the canonical ordering algorithm
    {n:TagIntT}  tag;     ///< decomposition tag: one of the tags listed in `{n:DecompositionT}`
    {n:MapIntT}  map[3];  ///< decomposition mapping, `{c:FlagWideMap}` set when consist of 4 or more
} {n:NormInfo};

/**
 * Decomposition and normalization info lookup table
 */
extern const {n:NormInfoT} {k:NormInfo}[];

/**
 * All Unicode pages
 * Points to `{k:NormInfoIndex}`
 */
extern const {v:pagesType} {k:NormPageIndex}[];

/**
 * Indexes of {k:NormInfo}
 * {k:NormInfoIndex}[page index][index in page]
 */
extern const {v:infoType} {k:NormInfoIndex}[][256];

/**
 * Decomposition mappings constituted by 4 or more characters
 */
extern const {v:charName} {k:SpecialMappings}[];

/**
 * Lookup decomposition and normalization information
 */
#define {n:NormInfo}(c) (&{k:NormInfo}[{k:NormInfoIndex}[{k:NormPageIndex}[c >> 8u]][c & 0xFF]])
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_NORMALIZATION_H_                     // NOLINT
