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
 * @file      table
 * @brief     Functions to lookup Unicode v9.0.0 code points
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @warning   unichar is the 16-bits type used to represent a unicode code point.
 *            Internally, UTF16-LE is used to code each unichar. However,
 *            for now, Unitex doesn't handle code points greater than 0xFFFF
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 `table`
 *
 * @date      septembre 2017
 *
 * This file was automatically generated using an enhanced version of unicode-table 0.3.2
 * @see https://github.com/UnitexGramLab/unitex-core/tree/master/base/unicode
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
#define UNITEX_UNICODE 90000000

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
 * @def      U_MAX_VALUE
 * @brief    Maximum allowed unicode value
 */
#define U_MAX_VALUE 0xFFFF

/**
 * @def      U_REPLACEMENT_CHAR
 * @brief    The unicode replacement character used when decoding byte sequences
 *           that cannot be successfully converted
 */
#define U_REPLACEMENT_CHAR 0xFFFD

/**
 * @def      U_SIZE
 * @brief    This is the size in bits of a unicode character
 */
#define U_SIZE 16

/**
 * @def      unichar
 * @brief    This is the type of a unicode character. Note that it is a
 *           16-bits type and it cannot handle characters beyond 0xFFFF
 */
typedef uint16_t unichar;

/**
 * `u_info_t` integer member types
 */
#define u_flags_int_t      uint32_t
#define u_category_int_t   uint8_t
#define u_script_int_t     uint8_t
#define u_bidi_int_t       uint8_t
#define u_variants_int_t   int32_t

/**
 * Flags for determining character type
 */
typedef enum {
  U_FLAG_LETTER          = 1 << 0 ,  ///< Lu, Ll, Lt, Lm, Lo
  U_FLAG_UPPERCASE       = 1 << 1 ,  ///< Lu
  U_FLAG_LOWERCASE       = 1 << 2 ,  ///< Ll
  U_FLAG_TITLECASE       = 1 << 3 ,  ///< Lt
  U_FLAG_SPACE           = 1 << 4 ,  ///< Zs, Zl, Zp
  U_FLAG_LINEBREAK       = 1 << 5 ,  ///< Zl, Zp
  U_FLAG_PUNCTUATION     = 1 << 6 ,  ///< Pc, Pd, Ps, Pe, Pi, Pf, Po
  U_FLAG_DIGIT           = 1 << 7 ,  ///< Nd
  U_FLAG_NUMBER          = 1 << 8 ,  ///< Nd, Nl, No
  U_FLAG_FRACTION        = 1 << 9 ,  ///< -
  U_FLAG_CONTROL         = 1 << 10,  ///< Cc
  U_FLAG_SYMBOL          = 1 << 11,  ///< Sm, Sc, Sk, So
  U_FLAG_IDENTIFIER      = 1 << 12,  ///< 0-9, A-Z, a-z, _
  U_FLAG_IGNORABLE       = 1 << 13,  ///< character should be ignored in processing
  U_FLAG_OTHER           = 1 << 14,  ///< Mn, Mc, Me, Cf, Cs, Co, Cn
  U_FLAG_UPPER_EXPANDS   = 1 << 16,  ///< uppercase expands to multiple characters
  U_FLAG_LOWER_EXPANDS   = 1 << 17,  ///< lowercase expands to multiple characters
  U_FLAG_TITLE_EXPANDS   = 1 << 18,  ///< titlecase expands to multiple characters
  U_FLAG_FOLD_EXPANDS    = 1 << 19   ///< foldcase expands to multiple characters
} u_flag_t;

/**
 * Unicode categories
 */
typedef enum {
  U_CATEGORY_INVALID,                     ///< Invalid
  U_CATEGORY_LETTER_UPPERCASE,            ///< Lu: Letter Uppercase
  U_CATEGORY_LETTER_LOWERCASE,            ///< Ll: Letter Lowercase
  U_CATEGORY_LETTER_TITLECASE,            ///< Lt: Letter Titlecase
  U_CATEGORY_LETTER_MODIFIER,             ///< Lm: Letter Modifier
  U_CATEGORY_LETTER_OTHER,                ///< Lo: Letter Other
  U_CATEGORY_MARK_NONSPACING,             ///< Mn: Mark Nonspacing
  U_CATEGORY_MARK_SPACING_COMBINING,      ///< Mc: Mark Spacing Combining
  U_CATEGORY_MARK_ENCLOSING,              ///< Me: Mark Enclosing
  U_CATEGORY_NUMBER_DECIMAL_DIGIT,        ///< Nd: Number Decimal Digit
  U_CATEGORY_NUMBER_LETTER,               ///< Nl: Number Letter
  U_CATEGORY_NUMBER_OTHER,                ///< No: Number Other
  U_CATEGORY_PUNCTUATION_CONNECTOR,       ///< Pc: Punctuation Connector
  U_CATEGORY_PUNCTUATION_DASH,            ///< Pd: Punctuation Dash
  U_CATEGORY_PUNCTUATION_OPEN,            ///< Ps: Punctuation Open
  U_CATEGORY_PUNCTUATION_CLOSE,           ///< Pe: Punctuation Close
  U_CATEGORY_PUNCTUATION_INITIAL_QUOTE,   ///< Pi: Punctuation Initial Quote
  U_CATEGORY_PUNCTUATION_FINAL_QUOTE,     ///< Pf: Punctuation Final Quote
  U_CATEGORY_PUNCTUATION_OTHER,           ///< Po: Punctuation Other
  U_CATEGORY_SYMBOL_MATH,                 ///< Sm: Symbol Math
  U_CATEGORY_SYMBOL_CURRENCY,             ///< Sc: Symbol Currency
  U_CATEGORY_SYMBOL_MODIFIER,             ///< Sk: Symbol Modifier
  U_CATEGORY_SYMBOL_OTHER,                ///< So: Symbol Other
  U_CATEGORY_SEPARATOR_SPACE,             ///< Zs: Separator Space
  U_CATEGORY_SEPARATOR_LINE,              ///< Zl: Separator Line
  U_CATEGORY_SEPARATOR_PARAGRAPH,         ///< Zp: Separator Paragraph
  U_CATEGORY_OTHER_CONTROL,               ///< Cc: Other Control
  U_CATEGORY_OTHER_FORMAT,                ///< Cf: Other Format
  U_CATEGORY_OTHER_SURROGATE,             ///< Cs: Other Surrogate
  U_CATEGORY_OTHER_PRIVATE_USE,           ///< Co: Other Private Use
  U_CATEGORY_OTHER_NOT_ASSIGNED           ///< Cn: Other Not Assigned
} u_category_t;

/**
 * Unicode scripts
 * @see http://www.unicode.org/reports/tr24
 */
typedef enum {
  U_SCRIPT_UNKNOWN,                 ///< Unknown
  U_SCRIPT_COMMON,                  ///< Common
  U_SCRIPT_LATIN,                   ///< Latin
  U_SCRIPT_GREEK,                   ///< Greek
  U_SCRIPT_CYRILLIC,                ///< Cyrillic
  U_SCRIPT_ARMENIAN,                ///< Armenian
  U_SCRIPT_HEBREW,                  ///< Hebrew
  U_SCRIPT_ARABIC,                  ///< Arabic
  U_SCRIPT_SYRIAC,                  ///< Syriac
  U_SCRIPT_THAANA,                  ///< Thaana
  U_SCRIPT_DEVANAGARI,              ///< Devanagari
  U_SCRIPT_BENGALI,                 ///< Bengali
  U_SCRIPT_GURMUKHI,                ///< Gurmukhi
  U_SCRIPT_GUJARATI,                ///< Gujarati
  U_SCRIPT_ORIYA,                   ///< Oriya
  U_SCRIPT_TAMIL,                   ///< Tamil
  U_SCRIPT_TELUGU,                  ///< Telugu
  U_SCRIPT_KANNADA,                 ///< Kannada
  U_SCRIPT_MALAYALAM,               ///< Malayalam
  U_SCRIPT_SINHALA,                 ///< Sinhala
  U_SCRIPT_THAI,                    ///< Thai
  U_SCRIPT_LAO,                     ///< Lao
  U_SCRIPT_TIBETAN,                 ///< Tibetan
  U_SCRIPT_MYANMAR,                 ///< Myanmar
  U_SCRIPT_GEORGIAN,                ///< Georgian
  U_SCRIPT_HANGUL,                  ///< Hangul
  U_SCRIPT_ETHIOPIC,                ///< Ethiopic
  U_SCRIPT_CHEROKEE,                ///< Cherokee
  U_SCRIPT_CANADIAN_ABORIGINAL,     ///< Canadian_Aboriginal
  U_SCRIPT_OGHAM,                   ///< Ogham
  U_SCRIPT_RUNIC,                   ///< Runic
  U_SCRIPT_KHMER,                   ///< Khmer
  U_SCRIPT_MONGOLIAN,               ///< Mongolian
  U_SCRIPT_HIRAGANA,                ///< Hiragana
  U_SCRIPT_KATAKANA,                ///< Katakana
  U_SCRIPT_BOPOMOFO,                ///< Bopomofo
  U_SCRIPT_HAN,                     ///< Han
  U_SCRIPT_YI,                      ///< Yi
  U_SCRIPT_OLD_ITALIC,              ///< Old_Italic
  U_SCRIPT_GOTHIC,                  ///< Gothic
  U_SCRIPT_DESERET,                 ///< Deseret
  U_SCRIPT_INHERITED,               ///< Inherited
  U_SCRIPT_TAGALOG,                 ///< Tagalog
  U_SCRIPT_HANUNOO,                 ///< Hanunoo
  U_SCRIPT_BUHID,                   ///< Buhid
  U_SCRIPT_TAGBANWA,                ///< Tagbanwa
  U_SCRIPT_LIMBU,                   ///< Limbu
  U_SCRIPT_TAI_LE,                  ///< Tai_Le
  U_SCRIPT_LINEAR_B,                ///< Linear_B
  U_SCRIPT_UGARITIC,                ///< Ugaritic
  U_SCRIPT_SHAVIAN,                 ///< Shavian
  U_SCRIPT_OSMANYA,                 ///< Osmanya
  U_SCRIPT_CYPRIOT,                 ///< Cypriot
  U_SCRIPT_BRAILLE,                 ///< Braille
  U_SCRIPT_BUGINESE,                ///< Buginese
  U_SCRIPT_COPTIC,                  ///< Coptic
  U_SCRIPT_NEW_TAI_LUE,             ///< New_Tai_Lue
  U_SCRIPT_GLAGOLITIC,              ///< Glagolitic
  U_SCRIPT_TIFINAGH,                ///< Tifinagh
  U_SCRIPT_SYLOTI_NAGRI,            ///< Syloti_Nagri
  U_SCRIPT_OLD_PERSIAN,             ///< Old_Persian
  U_SCRIPT_KHAROSHTHI,              ///< Kharoshthi
  U_SCRIPT_BALINESE,                ///< Balinese
  U_SCRIPT_CUNEIFORM,               ///< Cuneiform
  U_SCRIPT_PHOENICIAN,              ///< Phoenician
  U_SCRIPT_PHAGS_PA,                ///< Phags_Pa
  U_SCRIPT_NKO,                     ///< Nko
  U_SCRIPT_SUNDANESE,               ///< Sundanese
  U_SCRIPT_LEPCHA,                  ///< Lepcha
  U_SCRIPT_OL_CHIKI,                ///< Ol_Chiki
  U_SCRIPT_VAI,                     ///< Vai
  U_SCRIPT_SAURASHTRA,              ///< Saurashtra
  U_SCRIPT_KAYAH_LI,                ///< Kayah_Li
  U_SCRIPT_REJANG,                  ///< Rejang
  U_SCRIPT_LYCIAN,                  ///< Lycian
  U_SCRIPT_CARIAN,                  ///< Carian
  U_SCRIPT_LYDIAN,                  ///< Lydian
  U_SCRIPT_CHAM,                    ///< Cham
  U_SCRIPT_TAI_THAM,                ///< Tai_Tham
  U_SCRIPT_TAI_VIET,                ///< Tai_Viet
  U_SCRIPT_AVESTAN,                 ///< Avestan
  U_SCRIPT_EGYPTIAN_HIEROGLYPHS,    ///< Egyptian_Hieroglyphs
  U_SCRIPT_SAMARITAN,               ///< Samaritan
  U_SCRIPT_LISU,                    ///< Lisu
  U_SCRIPT_BAMUM,                   ///< Bamum
  U_SCRIPT_JAVANESE,                ///< Javanese
  U_SCRIPT_MEETEI_MAYEK,            ///< Meetei_Mayek
  U_SCRIPT_IMPERIAL_ARAMAIC,        ///< Imperial_Aramaic
  U_SCRIPT_OLD_SOUTH_ARABIAN,       ///< Old_South_Arabian
  U_SCRIPT_INSCRIPTIONAL_PARTHIAN,  ///< Inscriptional_Parthian
  U_SCRIPT_INSCRIPTIONAL_PAHLAVI,   ///< Inscriptional_Pahlavi
  U_SCRIPT_OLD_TURKIC,              ///< Old_Turkic
  U_SCRIPT_KAITHI,                  ///< Kaithi
  U_SCRIPT_BATAK,                   ///< Batak
  U_SCRIPT_BRAHMI,                  ///< Brahmi
  U_SCRIPT_MANDAIC,                 ///< Mandaic
  U_SCRIPT_CHAKMA,                  ///< Chakma
  U_SCRIPT_MEROITIC_CURSIVE,        ///< Meroitic_Cursive
  U_SCRIPT_MEROITIC_HIEROGLYPHS,    ///< Meroitic_Hieroglyphs
  U_SCRIPT_MIAO,                    ///< Miao
  U_SCRIPT_SHARADA,                 ///< Sharada
  U_SCRIPT_SORA_SOMPENG,            ///< Sora_Sompeng
  U_SCRIPT_TAKRI,                   ///< Takri
  U_SCRIPT_CAUCASIAN_ALBANIAN,      ///< Caucasian_Albanian
  U_SCRIPT_BASSA_VAH,               ///< Bassa_Vah
  U_SCRIPT_DUPLOYAN,                ///< Duployan
  U_SCRIPT_ELBASAN,                 ///< Elbasan
  U_SCRIPT_GRANTHA,                 ///< Grantha
  U_SCRIPT_PAHAWH_HMONG,            ///< Pahawh_Hmong
  U_SCRIPT_KHOJKI,                  ///< Khojki
  U_SCRIPT_LINEAR_A,                ///< Linear_A
  U_SCRIPT_MAHAJANI,                ///< Mahajani
  U_SCRIPT_MANICHAEAN,              ///< Manichaean
  U_SCRIPT_MENDE_KIKAKUI,           ///< Mende_Kikakui
  U_SCRIPT_MODI,                    ///< Modi
  U_SCRIPT_MRO,                     ///< Mro
  U_SCRIPT_OLD_NORTH_ARABIAN,       ///< Old_North_Arabian
  U_SCRIPT_NABATAEAN,               ///< Nabataean
  U_SCRIPT_PALMYRENE,               ///< Palmyrene
  U_SCRIPT_PAU_CIN_HAU,             ///< Pau_Cin_Hau
  U_SCRIPT_OLD_PERMIC,              ///< Old_Permic
  U_SCRIPT_PSALTER_PAHLAVI,         ///< Psalter_Pahlavi
  U_SCRIPT_SIDDHAM,                 ///< Siddham
  U_SCRIPT_KHUDAWADI,               ///< Khudawadi
  U_SCRIPT_TIRHUTA,                 ///< Tirhuta
  U_SCRIPT_WARANG_CITI,             ///< Warang_Citi
  U_SCRIPT_AHOM,                    ///< Ahom
  U_SCRIPT_ANATOLIAN_HIEROGLYPHS,   ///< Anatolian_Hieroglyphs
  U_SCRIPT_HATRAN,                  ///< Hatran
  U_SCRIPT_MULTANI,                 ///< Multani
  U_SCRIPT_OLD_HUNGARIAN,           ///< Old_Hungarian
  U_SCRIPT_SIGN_WRITING,            ///< SignWriting
  U_SCRIPT_ADLAM,                   ///< Adlam
  U_SCRIPT_BHAIKSUKI,               ///< Bhaiksuki
  U_SCRIPT_MARCHEN,                 ///< Marchen
  U_SCRIPT_NEWA,                    ///< Newa
  U_SCRIPT_OSAGE,                   ///< Osage
  U_SCRIPT_TANGUT                   ///< Tangut
} u_script_t;

/**
 * Unicode bidirectional class values
 * @see http://unicode.org/reports/tr44/#Bidi_Class_Values
 */
typedef enum {
  U_BIDI_INVALID,                   ///< Invalid
  U_BIDI_LEFT_TO_RIGHT,             ///< L: Left To Right
  U_BIDI_RIGHT_TO_LEFT,             ///< R: Right To Left
  U_BIDI_ARABIC_LETTER,             ///< AL: Arabic Letter
  U_BIDI_EUROPEAN_NUMBER,           ///< EN: European Number
  U_BIDI_EUROPEAN_SEPARATOR,        ///< ES: European Separator
  U_BIDI_EUROPEAN_TERMINATOR,       ///< ET: European Terminator
  U_BIDI_ARABIC_NUMBER,             ///< AN: Arabic Number
  U_BIDI_COMMON_SEPARATOR,          ///< CS: Common Separator
  U_BIDI_NONSPACING_MARK,           ///< NSM: Nonspacing Mark
  U_BIDI_BOUNDARY_NEUTRAL,          ///< BN: Boundary Neutral
  U_BIDI_PARAGRAPH_SEPARATOR,       ///< B: Paragraph Separator
  U_BIDI_SEGMENT_SEPARATOR,         ///< S: Segment Separator
  U_BIDI_WHITE_SPACE,               ///< WS: White Space
  U_BIDI_OTHER_NEUTRAL,             ///< ON: Other Neutral
  U_BIDI_LEFT_TO_RIGHT_EMBEDDING,   ///< LRE: Left To Right Embedding
  U_BIDI_LEFT_TO_RIGHT_OVERRIDE,    ///< LRO: Left To Right Override
  U_BIDI_RIGHT_TO_LEFT_EMBEDDING,   ///< RLE: Right To Left Embedding
  U_BIDI_RIGHT_TO_LEFT_OVERRIDE,    ///< RLO: Right To Left Override
  U_BIDI_POP_DIRECTIONAL_FORMAT,    ///< PDF: Pop Directional Format
  U_BIDI_LEFT_TO_RIGHT_ISOLATE,     ///< LRI: Left To Right Isolate
  U_BIDI_RIGHT_TO_LEFT_ISOLATE,     ///< RLI: Right To Left Isolate
  U_BIDI_FIRST_STRONG_ISOLATE,      ///< FSI: First Strong Isolate
  U_BIDI_POP_DIRECTIONAL_ISOLATE    ///< PDI: Pop Directional Isolate
} u_bidi_t;

/**
 * Index usable for `variant` in `u_info_t`
 *
 * If `u_info_t.variant[c]` is `0`, that specific variant does not exist.
 * If one of the EXPANDS flags is set in `u_flag`,  `c` will expands into
 * multiple characters, in this case `u_info_t.variant[c]` is an index usable
 * within the array `kUSpecialVariants`. The value that is pointed by
 * kUSpecialVariants[index] indicates the length of the expanded sequence,
 * i.e. if kUSpecialVariants[index] is equal to n, the expanded sequence will
 * be located between kUSpecialVariants[index+1] and kUSpecialVariants[index+n]
 */
typedef enum {
  U_CASE_UPPER    = 0,  ///< value to be added to `c` to get its uppercase variant
  U_CASE_LOWER    = 1,  ///< value to be added to `c` to get its lowercase variant
  U_CASE_TITLE    = 2,  ///< value to be added to `c` to get its titlecase variant
  U_CASE_FOLD     = 3,  ///< value to be added to `c` to get its foldcase variant
  U_CHAR_UNACCENT = 4,  ///< unaccent variant of  `c` (no arithmetic performed)
} u_variant_t;

/**
 * Character info
 */
typedef struct {
  u_flags_int_t    flags;       ///< flags: 0 or more flags listed in `u_flag`
  u_category_int_t category;    ///< category: One of the categories listed in `u_category_t`
  u_script_int_t   script;      ///< script: One of the scripts listed in `u_script_t`
  u_bidi_int_t     bidi;        ///< bidi class: one of the classes listed in `u_bidi_t`
  u_variants_int_t variant[5];  ///< indexable with `u_variant_t`
} u_info_t;

/**
 * Unicode code points lookup table
 */
extern const u_info_t kUInfo[];

/**
 * All Unicode pages
 * Points to `kUInfoIndex`
 */
extern const uint16_t kUPageIndex[];

/**
 * Indexes of kUInfo
 * kUInfoIndex[page index][index in page]
 */
extern const uint16_t kUInfoIndex[][256];

/**
 * Special variants characters
 */
extern const unichar kUSpecialVariants[];

/**
 * Test if the code point is valid
 */
#define u_is_valid(c) UNITEX_LIKELY(c < 0xD800 || (c > 0xDFFF &&  c <= U_MAX_VALUE))

/**
 * Test if the code point is invalid
 */
#define u_is_invalid(c) UNITEX_UNLIKELY(c >= 0xD800 && c  <= 0xDFFF )

/**
 * If the code point is invalid replace it by U_REPLACEMENT_CHAR
 */
#define u_replace_if_invalid(c) u_is_valid(c) ? c : U_REPLACEMENT_CHAR

/**
 * Lookup character
 */
#define u_info(c) (&kUInfo[kUInfoIndex[kUPageIndex[c >> 8u]][c & 0xFF]])
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
// anonymous namespaces in C++ are more versatile and superior to static.
/* ************************************************************************** */
/**
 * Lookup the character info by its code point
 *
 * @param  c The character to lookup
 * @return If a character with the given code point is found, return the
 *         corresponding u_info_t entry, If not found kUInfo[0] is returned
 * @see    `unichar_info`
 */
UNITEX_FORCE_INLINE
const u_info_t* u_lookup(const unichar c) {
  uint16_t page;
  uint16_t offset;

  if (u_is_invalid(c)) {
    return &kUInfo[0];
  }

  page = kUPageIndex[c >> 8u];
  offset = kUInfoIndex[page][c & 0xFF];

  return &kUInfo[offset];
}
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_TABLE_H_                             // NOLINT
