/*
 * Unitex
 *
 * Copyright (C) 2001-2017 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
 * @file      test.h
 * @brief     Functions to test Unicode v9.0.0 code points
 *
 * @author    cristian.martinez@univ-paris-est.fr (martinec)
 *
 * @attention Do not include this file directly, rather include the
 *            base/common.h header file to gain this file's functionality
 *
 * @warning   These functions don't handle code points greater than 0xFFFF
 *
 * @note      Use cpplint.py tool to detect style errors:
 *            `cpplint.py --linelength=120 test.h`
 *
 * @date      September 2016
 */
/* ************************************************************************** */
#ifndef UNITEX_BASE_UNICODE_TEST_H_                                 // NOLINT
#define UNITEX_BASE_UNICODE_TEST_H_                                 // NOLINT
/* ************************************************************************** */
#include "base/unicode/table.h"
#include "base/preprocessor/punctuation.h"
#include "base/preprocessor/util.h"
#include "base/preprocessor/variadic.h"
/* ************************************************************************** */
namespace unitex {
/* ************************************************************************** */
namespace {   // namespace unitex::{unnamed}, enforce one-definition-rule
/* ************************************************************************** */
// all U__* macros must be undefined at the end of this file
/* ************************************************************************** */
#define U__PARAMS(...)        U__PARAMS_(UNITEX_PP_VA_NARGS(__VA_ARGS__),__VA_ARGS__)
#define U__PARAMS_(n, ...)    UNITEX_PP_TOKEN_CAT_(U__PARAMS__, n)(__VA_ARGS__)
#define U__PARAMS__0()        UNITEX_PP_EMPTY()
#define U__PARAMS__1(_1)      UNITEX_PP_COMMA() u_##_1##_int_t _1
#define U__PARAMS__2(_1,...)  U__PARAMS__1(_1) U__PARAMS__1(__VA_ARGS__)
#define U__PARAMS__3(_1,...)  U__PARAMS__1(_1) U__PARAMS__2(__VA_ARGS__)
#define U__PARAMS__4(_1,...)  U__PARAMS__1(_1) U__PARAMS__3(__VA_ARGS__)
#define U__PARAMS__5(_1,...)  U__PARAMS__1(_1) U__PARAMS__4(__VA_ARGS__)
#define U__PARAMS__6(_1,...)  U__PARAMS__1(_1) U__PARAMS__5(__VA_ARGS__)
#define U__PARAMS__7(_1,...)  U__PARAMS__1(_1) U__PARAMS__6(__VA_ARGS__)
#define U__PARAMS__8(_1,...)  U__PARAMS__1(_1) U__PARAMS__7(__VA_ARGS__)
/* ************************************************************************** */
#define U__DECLARE__FUNCTION__TEST__(_name,_prepare,_test,...)                   \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar c  U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL;       \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const u_info_t* u_info U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL; \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar* s U__PARAMS(__VA_ARGS__)) UNITEX_PARAMS_NON_NULL;       \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar c U__PARAMS(__VA_ARGS__)) {                              \
  if(u_is_invalid(c)) return 0;                                                  \
  const u_info_t* u_info = u_info(c);                                            \
  _prepare                                                                       \
  return _test;                                                                  \
}                                                                                \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const u_info_t* u_info U__PARAMS(__VA_ARGS__)) {                       \
  _prepare                                                                       \
  return _test;                                                                  \
}                                                                                \
                                                                                 \
UNITEX_FORCE_INLINE                                                              \
int _name(const unichar* s U__PARAMS(__VA_ARGS__))  {                            \
  register const unichar* it = s;                                                \
  while (*it != '\0') {                                                          \
    if (!_name(*it UNITEX_PP_IF(UNITEX_PP_VA_NARGS(__VA_ARGS__))(                \
                UNITEX_PP_COMMA() UNITEX_PP_EXPAND(__VA_ARGS__),                 \
                UNITEX_PP_EMPTY())))  {                                          \
      return -(it - s);                                                          \
    }                                                                            \
    ++it;                                                                        \
  }                                                                              \
  return (it - s);                                                               \
}
/* ************************************************************************** */
U__DECLARE__FUNCTION__TEST__(u_test_flag,,
                            ((u_info->flags & flags) != 0),
                            flags)
U__DECLARE__FUNCTION__TEST__(u_test_category,,
                            ((u_info->category & category) != 0),
                            category)
U__DECLARE__FUNCTION__TEST__(u_test_script,,
                            ((u_info->script & script) != 0),
                            script)
U__DECLARE__FUNCTION__TEST__(u_test_bidi,,
                            ((u_info->bidi & bidi) != 0),
                            bidi)
/* *********************************************************************************************************************** */
#define u_is_digit(c)               u_test_flag(c,U_FLAG_DIGIT)  // #
#define u_is_letter(c)              u_test_flag(c,U_FLAG_LETTER) // #
/* *********************************************************************************************************************** */
#define u_has_flag_letter(c)                                           u_test_flag(c,U_FLAG_LETTER)
#define u_has_flag_uppercase(c)                                        u_test_flag(c,U_FLAG_UPPERCASE)
#define u_has_flag_lowercase(c)                                        u_test_flag(c,U_FLAG_LOWERCASE)
#define u_has_flag_titlecase(c)                                        u_test_flag(c,U_FLAG_TITLECASE)
#define u_has_flag_space(c)                                            u_test_flag(c,U_FLAG_SPACE)
#define u_has_flag_linebreak(c)                                        u_test_flag(c,U_FLAG_LINEBREAK)
#define u_has_flag_punctuation(c)                                      u_test_flag(c,U_FLAG_PUNCTUATION)
#define u_has_flag_digit(c)                                            u_test_flag(c,U_FLAG_DIGIT)
#define u_has_flag_number(c)                                           u_test_flag(c,U_FLAG_NUMBER)
#define u_has_flag_fraction(c)                                         u_test_flag(c,U_FLAG_FRACTION)
#define u_has_flag_control(c)                                          u_test_flag(c,U_FLAG_CONTROL)
#define u_has_flag_symbol(c)                                           u_test_flag(c,U_FLAG_SYMBOL)
#define u_has_flag_other(c)                                            u_test_flag(c,U_FLAG_OTHER)
#define u_has_flag_upper_expands(c)                                    u_test_flag(c,U_FLAG_UPPER_EXPANDS)
#define u_has_flag_lower_expands(c)                                    u_test_flag(c,U_FLAG_LOWER_EXPANDS)
#define u_has_flag_title_expands(c)                                    u_test_flag(c,U_FLAG_TITLE_EXPANDS)
#define u_has_flag_fold_expands(c)                                     u_test_flag(c,U_FLAG_FOLD_EXPANDS)
/* *********************************************************************************************************************** */
#define u_in_category_invalid(c)                                       u_test_category(c,U_CATEGORY_INVALID)
#define u_in_category_letter_uppercase(c)                              u_test_category(c,U_CATEGORY_LETTER_UPPERCASE)
#define u_in_category_letter_lowercase(c)                              u_test_category(c,U_CATEGORY_LETTER_LOWERCASE)
#define u_in_category_letter_titlecase(c)                              u_test_category(c,U_CATEGORY_LETTER_TITLECASE)
#define u_in_category_letter_modifier(c)                               u_test_category(c,U_CATEGORY_LETTER_MODIFIER)
#define u_in_category_letter_other(c)                                  u_test_category(c,U_CATEGORY_LETTER_OTHER)
#define u_in_category_mark_nonspacing(c)                               u_test_category(c,U_CATEGORY_MARK_NONSPACING)
#define u_in_category_mark_spacing_combining(c)                        u_test_category(c,U_CATEGORY_MARK_SPACING_COMBINING)
#define u_in_category_mark_enclosing(c)                                u_test_category(c,U_CATEGORY_MARK_ENCLOSING)
#define u_in_category_number_decimal_digit(c)                          u_test_category(c,U_CATEGORY_NUMBER_DECIMAL_DIGIT)
#define u_in_category_number_letter(c)                                 u_test_category(c,U_CATEGORY_NUMBER_LETTER)
#define u_in_category_number_other(c)                                  u_test_category(c,U_CATEGORY_NUMBER_OTHER)
#define u_in_category_punctuation_connector(c)                         u_test_category(c,U_CATEGORY_PUNCTUATION_CONNECTOR)
#define u_in_category_punctuation_dash(c)                              u_test_category(c,U_CATEGORY_PUNCTUATION_DASH)
#define u_in_category_punctuation_open(c)                              u_test_category(c,U_CATEGORY_PUNCTUATION_OPEN)
#define u_in_category_punctuation_close(c)                             u_test_category(c,U_CATEGORY_PUNCTUATION_CLOSE)
#define u_in_category_punctuation_initial_quote(c)                     u_test_category(c,U_CATEGORY_PUNCTUATION_INITIAL_QUOTE)
#define u_in_category_punctuation_final_quote(c)                       u_test_category(c,U_CATEGORY_PUNCTUATION_FINAL_QUOTE)
#define u_in_category_punctuation_other(c)                             u_test_category(c,U_CATEGORY_PUNCTUATION_OTHER)
#define u_in_category_symbol_math(c)                                   u_test_category(c,U_CATEGORY_SYMBOL_MATH)
#define u_in_category_symbol_currency(c)                               u_test_category(c,U_CATEGORY_SYMBOL_CURRENCY)
#define u_in_category_symbol_modifier(c)                               u_test_category(c,U_CATEGORY_SYMBOL_MODIFIER)
#define u_in_category_symbol_other(c)                                  u_test_category(c,U_CATEGORY_SYMBOL_OTHER)
#define u_in_category_separator_space(c)                               u_test_category(c,U_CATEGORY_SEPARATOR_SPACE)
#define u_in_category_separator_line(c)                                u_test_category(c,U_CATEGORY_SEPARATOR_LINE)
#define u_in_category_separator_paragraph(c)                           u_test_category(c,U_CATEGORY_SEPARATOR_PARAGRAPH)
#define u_in_category_other_control(c)                                 u_test_category(c,U_CATEGORY_OTHER_CONTROL)
#define u_in_category_other_format(c)                                  u_test_category(c,U_CATEGORY_OTHER_FORMAT)
#define u_in_category_other_surrogate(c)                               u_test_category(c,U_CATEGORY_OTHER_SURROGATE)
#define u_in_category_other_private_use(c)                             u_test_category(c,U_CATEGORY_OTHER_PRIVATE_USE)
#define u_in_category_other_not_assigned(c)                            u_test_category(c,U_CATEGORY_OTHER_NOT_ASSIGNED)
/* *********************************************************************************************************************** */
#define u_in_script_unknown(c)                                         u_test_script(c,U_SCRIPT_UNKNOWN)
#define u_in_script_common(c)                                          u_test_script(c,U_SCRIPT_COMMON)
#define u_in_script_latin(c)                                           u_test_script(c,U_SCRIPT_LATIN)
#define u_in_script_greek(c)                                           u_test_script(c,U_SCRIPT_GREEK)
#define u_in_script_cyrillic(c)                                        u_test_script(c,U_SCRIPT_CYRILLIC)
#define u_in_script_armenian(c)                                        u_test_script(c,U_SCRIPT_ARMENIAN)
#define u_in_script_hebrew(c)                                          u_test_script(c,U_SCRIPT_HEBREW)
#define u_in_script_arabic(c)                                          u_test_script(c,U_SCRIPT_ARABIC)
#define u_in_script_syriac(c)                                          u_test_script(c,U_SCRIPT_SYRIAC)
#define u_in_script_thaana(c)                                          u_test_script(c,U_SCRIPT_THAANA)
#define u_in_script_devanagari(c)                                      u_test_script(c,U_SCRIPT_DEVANAGARI)
#define u_in_script_bengali(c)                                         u_test_script(c,U_SCRIPT_BENGALI)
#define u_in_script_gurmukhi(c)                                        u_test_script(c,U_SCRIPT_GURMUKHI)
#define u_in_script_gujarati(c)                                        u_test_script(c,U_SCRIPT_GUJARATI)
#define u_in_script_oriya(c)                                           u_test_script(c,U_SCRIPT_ORIYA)
#define u_in_script_tamil(c)                                           u_test_script(c,U_SCRIPT_TAMIL)
#define u_in_script_telugu(c)                                          u_test_script(c,U_SCRIPT_TELUGU)
#define u_in_script_kannada(c)                                         u_test_script(c,U_SCRIPT_KANNADA)
#define u_in_script_malayalam(c)                                       u_test_script(c,U_SCRIPT_MALAYALAM)
#define u_in_script_sinhala(c)                                         u_test_script(c,U_SCRIPT_SINHALA)
#define u_in_script_thai(c)                                            u_test_script(c,U_SCRIPT_THAI)
#define u_in_script_lao(c)                                             u_test_script(c,U_SCRIPT_LAO)
#define u_in_script_tibetan(c)                                         u_test_script(c,U_SCRIPT_TIBETAN)
#define u_in_script_myanmar(c)                                         u_test_script(c,U_SCRIPT_MYANMAR)
#define u_in_script_georgian(c)                                        u_test_script(c,U_SCRIPT_GEORGIAN)
#define u_in_script_hangul(c)                                          u_test_script(c,U_SCRIPT_HANGUL)
#define u_in_script_ethiopic(c)                                        u_test_script(c,U_SCRIPT_ETHIOPIC)
#define u_in_script_cherokee(c)                                        u_test_script(c,U_SCRIPT_CHEROKEE)
#define u_in_script_canadian_aboriginal(c)                             u_test_script(c,U_SCRIPT_CANADIAN_ABORIGINAL)
#define u_in_script_ogham(c)                                           u_test_script(c,U_SCRIPT_OGHAM)
#define u_in_script_runic(c)                                           u_test_script(c,U_SCRIPT_RUNIC)
#define u_in_script_khmer(c)                                           u_test_script(c,U_SCRIPT_KHMER)
#define u_in_script_mongolian(c)                                       u_test_script(c,U_SCRIPT_MONGOLIAN)
#define u_in_script_hiragana(c)                                        u_test_script(c,U_SCRIPT_HIRAGANA)
#define u_in_script_katakana(c)                                        u_test_script(c,U_SCRIPT_KATAKANA)
#define u_in_script_bopomofo(c)                                        u_test_script(c,U_SCRIPT_BOPOMOFO)
#define u_in_script_han(c)                                             u_test_script(c,U_SCRIPT_HAN)
#define u_in_script_yi(c)                                              u_test_script(c,U_SCRIPT_YI)
#define u_in_script_old_italic(c)                                      u_test_script(c,U_SCRIPT_OLD_ITALIC)
#define u_in_script_gothic(c)                                          u_test_script(c,U_SCRIPT_GOTHIC)
#define u_in_script_deseret(c)                                         u_test_script(c,U_SCRIPT_DESERET)
#define u_in_script_inherited(c)                                       u_test_script(c,U_SCRIPT_INHERITED)
#define u_in_script_tagalog(c)                                         u_test_script(c,U_SCRIPT_TAGALOG)
#define u_in_script_hanunoo(c)                                         u_test_script(c,U_SCRIPT_HANUNOO)
#define u_in_script_buhid(c)                                           u_test_script(c,U_SCRIPT_BUHID)
#define u_in_script_tagbanwa(c)                                        u_test_script(c,U_SCRIPT_TAGBANWA)
#define u_in_script_limbu(c)                                           u_test_script(c,U_SCRIPT_LIMBU)
#define u_in_script_tai_le(c)                                          u_test_script(c,U_SCRIPT_TAI_LE)
#define u_in_script_linear_b(c)                                        u_test_script(c,U_SCRIPT_LINEAR_B)
#define u_in_script_ugaritic(c)                                        u_test_script(c,U_SCRIPT_UGARITIC)
#define u_in_script_shavian(c)                                         u_test_script(c,U_SCRIPT_SHAVIAN)
#define u_in_script_osmanya(c)                                         u_test_script(c,U_SCRIPT_OSMANYA)
#define u_in_script_cypriot(c)                                         u_test_script(c,U_SCRIPT_CYPRIOT)
#define u_in_script_braille(c)                                         u_test_script(c,U_SCRIPT_BRAILLE)
#define u_in_script_buginese(c)                                        u_test_script(c,U_SCRIPT_BUGINESE)
#define u_in_script_coptic(c)                                          u_test_script(c,U_SCRIPT_COPTIC)
#define u_in_script_new_tai_lue(c)                                     u_test_script(c,U_SCRIPT_NEW_TAI_LUE)
#define u_in_script_glagolitic(c)                                      u_test_script(c,U_SCRIPT_GLAGOLITIC)
#define u_in_script_tifinagh(c)                                        u_test_script(c,U_SCRIPT_TIFINAGH)
#define u_in_script_syloti_nagri(c)                                    u_test_script(c,U_SCRIPT_SYLOTI_NAGRI)
#define u_in_script_old_persian(c)                                     u_test_script(c,U_SCRIPT_OLD_PERSIAN)
#define u_in_script_kharoshthi(c)                                      u_test_script(c,U_SCRIPT_KHAROSHTHI)
#define u_in_script_balinese(c)                                        u_test_script(c,U_SCRIPT_BALINESE)
#define u_in_script_cuneiform(c)                                       u_test_script(c,U_SCRIPT_CUNEIFORM)
#define u_in_script_phoenician(c)                                      u_test_script(c,U_SCRIPT_PHOENICIAN)
#define u_in_script_phags_pa(c)                                        u_test_script(c,U_SCRIPT_PHAGS_PA)
#define u_in_script_nko(c)                                             u_test_script(c,U_SCRIPT_NKO)
#define u_in_script_sundanese(c)                                       u_test_script(c,U_SCRIPT_SUNDANESE)
#define u_in_script_lepcha(c)                                          u_test_script(c,U_SCRIPT_LEPCHA)
#define u_in_script_ol_chiki(c)                                        u_test_script(c,U_SCRIPT_OL_CHIKI)
#define u_in_script_vai(c)                                             u_test_script(c,U_SCRIPT_VAI)
#define u_in_script_saurashtra(c)                                      u_test_script(c,U_SCRIPT_SAURASHTRA)
#define u_in_script_kayah_li(c)                                        u_test_script(c,U_SCRIPT_KAYAH_LI)
#define u_in_script_rejang(c)                                          u_test_script(c,U_SCRIPT_REJANG)
#define u_in_script_lycian(c)                                          u_test_script(c,U_SCRIPT_LYCIAN)
#define u_in_script_carian(c)                                          u_test_script(c,U_SCRIPT_CARIAN)
#define u_in_script_lydian(c)                                          u_test_script(c,U_SCRIPT_LYDIAN)
#define u_in_script_cham(c)                                            u_test_script(c,U_SCRIPT_CHAM)
#define u_in_script_tai_tham(c)                                        u_test_script(c,U_SCRIPT_TAI_THAM)
#define u_in_script_tai_viet(c)                                        u_test_script(c,U_SCRIPT_TAI_VIET)
#define u_in_script_avestan(c)                                         u_test_script(c,U_SCRIPT_AVESTAN)
#define u_in_script_egyptian_hieroglyphs(c)                            u_test_script(c,U_SCRIPT_EGYPTIAN_HIEROGLYPHS)
#define u_in_script_samaritan(c)                                       u_test_script(c,U_SCRIPT_SAMARITAN)
#define u_in_script_lisu(c)                                            u_test_script(c,U_SCRIPT_LISU)
#define u_in_script_bamum(c)                                           u_test_script(c,U_SCRIPT_BAMUM)
#define u_in_script_javanese(c)                                        u_test_script(c,U_SCRIPT_JAVANESE)
#define u_in_script_meetei_mayek(c)                                    u_test_script(c,U_SCRIPT_MEETEI_MAYEK)
#define u_in_script_imperial_aramaic(c)                                u_test_script(c,U_SCRIPT_IMPERIAL_ARAMAIC)
#define u_in_script_old_south_arabian(c)                               u_test_script(c,U_SCRIPT_OLD_SOUTH_ARABIAN)
#define u_in_script_inscriptional_parthian(c)                          u_test_script(c,U_SCRIPT_INSCRIPTIONAL_PARTHIAN)
#define u_in_script_inscriptional_pahlavi(c)                           u_test_script(c,U_SCRIPT_INSCRIPTIONAL_PAHLAVI)
#define u_in_script_old_turkic(c)                                      u_test_script(c,U_SCRIPT_OLD_TURKIC)
#define u_in_script_kaithi(c)                                          u_test_script(c,U_SCRIPT_KAITHI)
#define u_in_script_batak(c)                                           u_test_script(c,U_SCRIPT_BATAK)
#define u_in_script_brahmi(c)                                          u_test_script(c,U_SCRIPT_BRAHMI)
#define u_in_script_mandaic(c)                                         u_test_script(c,U_SCRIPT_MANDAIC)
#define u_in_script_chakma(c)                                          u_test_script(c,U_SCRIPT_CHAKMA)
#define u_in_script_meroitic_cursive(c)                                u_test_script(c,U_SCRIPT_MEROITIC_CURSIVE)
#define u_in_script_meroitic_hieroglyphs(c)                            u_test_script(c,U_SCRIPT_MEROITIC_HIEROGLYPHS)
#define u_in_script_miao(c)                                            u_test_script(c,U_SCRIPT_MIAO)
#define u_in_script_sharada(c)                                         u_test_script(c,U_SCRIPT_SHARADA)
#define u_in_script_sora_sompeng(c)                                    u_test_script(c,U_SCRIPT_SORA_SOMPENG)
#define u_in_script_takri(c)                                           u_test_script(c,U_SCRIPT_TAKRI)
#define u_in_script_caucasian_albanian(c)                              u_test_script(c,U_SCRIPT_CAUCASIAN_ALBANIAN)
#define u_in_script_bassa_vah(c)                                       u_test_script(c,U_SCRIPT_BASSA_VAH)
#define u_in_script_duployan(c)                                        u_test_script(c,U_SCRIPT_DUPLOYAN)
#define u_in_script_elbasan(c)                                         u_test_script(c,U_SCRIPT_ELBASAN)
#define u_in_script_grantha(c)                                         u_test_script(c,U_SCRIPT_GRANTHA)
#define u_in_script_pahawh_hmong(c)                                    u_test_script(c,U_SCRIPT_PAHAWH_HMONG)
#define u_in_script_khojki(c)                                          u_test_script(c,U_SCRIPT_KHOJKI)
#define u_in_script_linear_a(c)                                        u_test_script(c,U_SCRIPT_LINEAR_A)
#define u_in_script_mahajani(c)                                        u_test_script(c,U_SCRIPT_MAHAJANI)
#define u_in_script_manichaean(c)                                      u_test_script(c,U_SCRIPT_MANICHAEAN)
#define u_in_script_mende_kikakui(c)                                   u_test_script(c,U_SCRIPT_MENDE_KIKAKUI)
#define u_in_script_modi(c)                                            u_test_script(c,U_SCRIPT_MODI)
#define u_in_script_mro(c)                                             u_test_script(c,U_SCRIPT_MRO)
#define u_in_script_old_north_arabian(c)                               u_test_script(c,U_SCRIPT_OLD_NORTH_ARABIAN)
#define u_in_script_nabataean(c)                                       u_test_script(c,U_SCRIPT_NABATAEAN)
#define u_in_script_palmyrene(c)                                       u_test_script(c,U_SCRIPT_PALMYRENE)
#define u_in_script_pau_cin_hau(c)                                     u_test_script(c,U_SCRIPT_PAU_CIN_HAU)
#define u_in_script_old_permic(c)                                      u_test_script(c,U_SCRIPT_OLD_PERMIC)
#define u_in_script_psalter_pahlavi(c)                                 u_test_script(c,U_SCRIPT_PSALTER_PAHLAVI)
#define u_in_script_siddham(c)                                         u_test_script(c,U_SCRIPT_SIDDHAM)
#define u_in_script_khudawadi(c)                                       u_test_script(c,U_SCRIPT_KHUDAWADI)
#define u_in_script_tirhuta(c)                                         u_test_script(c,U_SCRIPT_TIRHUTA)
#define u_in_script_warang_citi(c)                                     u_test_script(c,U_SCRIPT_WARANG_CITI)
#define u_in_script_ahom(c)                                            u_test_script(c,U_SCRIPT_AHOM)
#define u_in_script_anatolian_hieroglyphs(c)                           u_test_script(c,U_SCRIPT_ANATOLIAN_HIEROGLYPHS)
#define u_in_script_hatran(c)                                          u_test_script(c,U_SCRIPT_HATRAN)
#define u_in_script_multani(c)                                         u_test_script(c,U_SCRIPT_MULTANI)
#define u_in_script_old_hungarian(c)                                   u_test_script(c,U_SCRIPT_OLD_HUNGARIAN)
#define u_in_script_sign_writing(c)                                    u_test_script(c,U_SCRIPT_SIGN_WRITING)
#define u_in_script_adlam(c)                                           u_test_script(c,U_SCRIPT_ADLAM)
#define u_in_script_bhaiksuki(c)                                       u_test_script(c,U_SCRIPT_BHAIKSUKI)
#define u_in_script_marchen(c)                                         u_test_script(c,U_SCRIPT_MARCHEN)
#define u_in_script_newa(c)                                            u_test_script(c,U_SCRIPT_NEWA)
#define u_in_script_osage(c)                                           u_test_script(c,U_SCRIPT_OSAGE)
#define u_in_script_tangut(c)                                          u_test_script(c,U_SCRIPT_TANGUT)
/* *********************************************************************************************************************** */
#define u_has_bidi_invalid(c)                                          u_test_bidi(c,U_BIDI_INVALID)
#define u_has_bidi_left_to_right(c)                                    u_test_bidi(c,U_BIDI_LEFT_TO_RIGHT)
#define u_has_bidi_right_to_left(c)                                    u_test_bidi(c,U_BIDI_RIGHT_TO_LEFT)
#define u_has_bidi_arabic_letter(c)                                    u_test_bidi(c,U_BIDI_ARABIC_LETTER)
#define u_has_bidi_european_number(c)                                  u_test_bidi(c,U_BIDI_EUROPEAN_NUMBER)
#define u_has_bidi_european_separator(c)                               u_test_bidi(c,U_BIDI_EUROPEAN_SEPARATOR)
#define u_has_bidi_european_terminator(c)                              u_test_bidi(c,U_BIDI_EUROPEAN_TERMINATOR)
#define u_has_bidi_arabic_number(c)                                    u_test_bidi(c,U_BIDI_ARABIC_NUMBER)
#define u_has_bidi_common_separator(c)                                 u_test_bidi(c,U_BIDI_COMMON_SEPARATOR)
#define u_has_bidi_nonspacing_mark(c)                                  u_test_bidi(c,U_BIDI_NONSPACING_MARK)
#define u_has_bidi_boundary_neutral(c)                                 u_test_bidi(c,U_BIDI_BOUNDARY_NEUTRAL)
#define u_has_bidi_paragraph_separator(c)                              u_test_bidi(c,U_BIDI_PARAGRAPH_SEPARATOR)
#define u_has_bidi_segment_separator(c)                                u_test_bidi(c,U_BIDI_SEGMENT_SEPARATOR)
#define u_has_bidi_white_space(c)                                      u_test_bidi(c,U_BIDI_WHITE_SPACE)
#define u_has_bidi_other_neutral(c)                                    u_test_bidi(c,U_BIDI_OTHER_NEUTRAL)
#define u_has_bidi_left_to_right_embedding(c)                          u_test_bidi(c,U_BIDI_LEFT_TO_RIGHT_EMBEDDING)
#define u_has_bidi_left_to_right_override(c)                           u_test_bidi(c,U_BIDI_LEFT_TO_RIGHT_OVERRIDE)
#define u_has_bidi_right_to_left_embedding(c)                          u_test_bidi(c,U_BIDI_RIGHT_TO_LEFT_EMBEDDING)
#define u_has_bidi_right_to_left_override(c)                           u_test_bidi(c,U_BIDI_RIGHT_TO_LEFT_OVERRIDE)
#define u_has_bidi_pop_directional_format(c)                           u_test_bidi(c,U_BIDI_POP_DIRECTIONAL_FORMAT)
#define u_has_bidi_left_to_right_isolate(c)                            u_test_bidi(c,U_BIDI_LEFT_TO_RIGHT_ISOLATE)
#define u_has_bidi_right_to_left_isolate(c)                            u_test_bidi(c,U_BIDI_RIGHT_TO_LEFT_ISOLATE)
#define u_has_bidi_first_strong_isolate(c)                             u_test_bidi(c,U_BIDI_FIRST_STRONG_ISOLATE)
#define u_has_bidi_pop_directional_isolate(c)                          u_test_bidi(c,U_BIDI_POP_DIRECTIONAL_ISOLATE)
/* *********************************************************************************************************************** */
#undef U__DECLARE__FUNCTION__TEST
#undef U__PARAMS__8
#undef U__PARAMS__7
#undef U__PARAMS__6
#undef U__PARAMS__5
#undef U__PARAMS__4
#undef U__PARAMS__3
#undef U__PARAMS__2
#undef U__PARAMS__1
#undef U__PARAMS__0
#undef U__PARAMS_
#undef U__PARAMS
/* ************************************************************************** */
}  // namespace unitex::unnamed
/* ************************************************************************** */
}  // namespace unitex
/* ************************************************************************** */
#endif  // UNITEX_BASE_UNICODE_TEST_H_                              // NOLINT
