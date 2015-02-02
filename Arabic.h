/*
 * Unitex
 *
 * Copyright (C) 2001-2015 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef ARABIC_H
#define ARABIC_H

#include "Unicode.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

/* Here are the absolute definition of the arabic letters */

/* Plain letters */
#define ABS_AR_HAMZA 						0x0621
#define ABS_AR_ALEF_WITH_MADDA_ABOVE 		0x0622
#define ABS_AR_ALEF_WITH_HAMZA_ABOVE 		0x0623
#define ABS_AR_WAW_WITH_HAMZA_ABOVE 		0x0624
#define ABS_AR_ALEF_WITH_HAMZA_BELOW 		0x0625
#define ABS_AR_YEH_WITH_HAMZA_ABOVE 		0x0626
#define ABS_AR_ALEF 						0x0627
#define ABS_AR_BEH 							0x0628
#define ABS_AR_TEH_MARBUTA 					0x0629
#define ABS_AR_TEH 							0x062A
#define ABS_AR_THEH 						0x062B
#define ABS_AR_JEEM 						0x062C
#define ABS_AR_HAH 							0x062D
#define ABS_AR_KHAH 						0x062E
#define ABS_AR_DAL 							0x062F
#define ABS_AR_THAL							0x0630
#define ABS_AR_REH 							0x0631
#define ABS_AR_ZAIN 						0x0632
#define ABS_AR_SEEN 						0x0633
#define ABS_AR_SHEEN 						0x0634
#define ABS_AR_SAD	 						0x0635
#define ABS_AR_DAD	 						0x0636
#define ABS_AR_TAH	 						0x0637
#define ABS_AR_ZAH	 						0x0638
#define ABS_AR_AIN	 						0x0639
#define ABS_AR_GHAIN						0x063A
#define ABS_AR_TATWEEL 						0x0640
#define ABS_AR_FEH	 						0x0641
#define ABS_AR_QAF	 						0x0642
#define ABS_AR_KAF	 						0x0643
#define ABS_AR_LAM	 						0x0644
#define ABS_AR_MEEM	 						0x0645
#define ABS_AR_NOON	 						0x0646
#define ABS_AR_HEH	 						0x0647
#define ABS_AR_WAW	 						0x0648
#define ABS_AR_ALEF_MAQSURA					0x0649
#define ABS_AR_YEH	 						0x064A
/* Diacritics */
#define ABS_AR_FATHATAN						0x064B
#define ABS_AR_DAMMATAN						0x064C
#define ABS_AR_KASRATAN						0x064D
#define ABS_AR_FATHA						0x064E
#define ABS_AR_DAMMA						0x064F
#define ABS_AR_KASRA						0x0650
#define ABS_AR_SHADDA						0x0651
#define ABS_AR_SUKUN						0x0652
#define ABS_AR_SUPERSCRIPT_ALEF				0x0670
/* Hamza variant */
#define ABS_AR_ALEF_WASLA					0x0671

/* Here are the absolute definition of the Buckwalter transliterations */
#define BW_AR_HAMZA 					'c'
#define BW_AR_ALEF_WITH_MADDA_ABOVE 	'C'
#define BW_AR_ALEF_WITH_HAMZA_ABOVE 	'O'
#define BW_AR_WAW_WITH_HAMZA_ABOVE 		'W'
#define BW_AR_ALEF_WITH_HAMZA_BELOW 	'I'
#define BW_AR_YEH_WITH_HAMZA_ABOVE 		'e'
#define BW_AR_ALEF 						'A'
#define BW_AR_BEH 					 	'b'
#define BW_AR_TEH_MARBUTA 				'p'
#define BW_AR_TEH 						't'
#define BW_AR_THEH 						'v'
#define BW_AR_JEEM 						'j'
#define BW_AR_HAH 						'H'
#define BW_AR_KHAH 						'x'
#define BW_AR_DAL 						'd'
#define BW_AR_THAL						'J'
#define BW_AR_REH 						'r'
#define BW_AR_ZAIN						'z'
#define BW_AR_SEEN 						's'
#define BW_AR_SHEEN 					'M'
#define BW_AR_SAD	 					'S'
#define BW_AR_DAD	 					'D'
#define BW_AR_TAH	 					'T'
#define BW_AR_ZAH	 					'Z'
#define BW_AR_AIN	 					'E'
#define BW_AR_GHAIN						'g'
#define BW_AR_TATWEEL 					'_'
#define BW_AR_FEH	 					'f'
#define BW_AR_QAF	 					'q'
#define BW_AR_KAF	 					'k'
#define BW_AR_LAM	 					'l'
#define BW_AR_MEEM	 					'm'
#define BW_AR_NOON	 					'n'
#define BW_AR_HEH	 					'h'
#define BW_AR_WAW	 					'w'
#define BW_AR_ALEF_MAQSURA				'Y'
#define BW_AR_YEH	 					'y'
/* Diacritics */
#define BW_AR_FATHATAN					'F'
#define BW_AR_DAMMATAN					'N'
#define BW_AR_KASRATAN					'K'
#define BW_AR_FATHA						'a'
#define BW_AR_DAMMA						'u'
#define BW_AR_KASRA						'i'
#define BW_AR_SHADDA					'G'
#define BW_AR_SUKUN						'o'
#define BW_AR_SUPERSCRIPT_ALEF			'R'
/* Hamza variant */
#define BW_AR_ALEF_WASLA					'|'


/* This macro is for debug only. You should not use it unless you are me. SP */
//#define AR_WORK_ON_TRANSLITERATED_FORMS

/* Here are the relative definition of arabic letters, depending of the
 * AR_WORK_ON_TRANSLITERATED_FORMS macro.
 */
#ifndef AR_WORK_ON_TRANSLITERATED_FORMS

/* Plain letters */
#define AR_HAMZA 						0x0621
#define AR_ALEF_WITH_MADDA_ABOVE 		0x0622
#define AR_ALEF_WITH_HAMZA_ABOVE 		0x0623
#define AR_WAW_WITH_HAMZA_ABOVE 		0x0624
#define AR_ALEF_WITH_HAMZA_BELOW 		0x0625
#define AR_YEH_WITH_HAMZA_ABOVE 		0x0626
#define AR_ALEF 						0x0627
#define AR_BEH 							0x0628
#define AR_TEH_MARBUTA 					0x0629
#define AR_TEH 							0x062A
#define AR_THEH 						0x062B
#define AR_JEEM 						0x062C
#define AR_HAH 							0x062D
#define AR_KHAH 						0x062E
#define AR_DAL 							0x062F
#define AR_THAL							0x0630
#define AR_REH 							0x0631
#define AR_ZAIN 						0x0632
#define AR_SEEN 						0x0633
#define AR_SHEEN 						0x0634
#define AR_SAD	 						0x0635
#define AR_DAD	 						0x0636
#define AR_TAH	 						0x0637
#define AR_ZAH	 						0x0638
#define AR_AIN	 						0x0639
#define AR_GHAIN						0x063A
#define AR_TATWEEL 						0x0640
#define AR_FEH	 						0x0641
#define AR_QAF	 						0x0642
#define AR_KAF	 						0x0643
#define AR_LAM	 						0x0644
#define AR_MEEM	 						0x0645
#define AR_NOON	 						0x0646
#define AR_HEH	 						0x0647
#define AR_WAW	 						0x0648
#define AR_ALEF_MAQSURA					0x0649
#define AR_YEH	 						0x064A
/* Diacritics */
#define AR_FATHATAN						0x064B
#define AR_DAMMATAN						0x064C
#define AR_KASRATAN						0x064D
#define AR_FATHA						0x064E
#define AR_DAMMA						0x064F
#define AR_KASRA						0x0650
#define AR_SHADDA						0x0651
#define AR_SUKUN						0x0652
#define AR_SUPERSCRIPT_ALEF				0x0670
/* Hamza variant */
#define AR_ALEF_WASLA					0x0671

#else
/* Here are the Buckwalter++ transliterated forms */
#define AR_HAMZA 						'c'
#define AR_ALEF_WITH_MADDA_ABOVE 		'C'
#define AR_ALEF_WITH_HAMZA_ABOVE 		'O'
#define AR_WAW_WITH_HAMZA_ABOVE 		'W'
#define AR_ALEF_WITH_HAMZA_BELOW 		'I'
#define AR_YEH_WITH_HAMZA_ABOVE 		'e'
#define AR_ALEF 						'A'
#define AR_BEH 						 	'b'
#define AR_TEH_MARBUTA 					'p'
#define AR_TEH 							't'
#define AR_THEH 						'v'
#define AR_JEEM 						'j'
#define AR_HAH 							'H'
#define AR_KHAH 						'x'
#define AR_DAL 							'd'
#define AR_THAL							'J'
#define AR_REH 							'r'
#define AR_ZAIN							'z'
#define AR_SEEN 						's'
#define AR_SHEEN 						'M'
#define AR_SAD	 						'S'
#define AR_DAD	 						'D'
#define AR_TAH	 						'T'
#define AR_ZAH	 						'Z'
#define AR_AIN	 						'E'
#define AR_GHAIN						'g'
#define AR_TATWEEL 						'_'
#define AR_FEH	 						'f'
#define AR_QAF	 						'q'
#define AR_KAF	 						'k'
#define AR_LAM	 						'l'
#define AR_MEEM	 						'm'
#define AR_NOON	 						'n'
#define AR_HEH	 						'h'
#define AR_WAW	 						'w'
#define AR_ALEF_MAQSURA					'Y'
#define AR_YEH	 						'y'
/* Diacritics */
#define AR_FATHATAN						'F'
#define AR_DAMMATAN						'N'
#define AR_KASRATAN						'K'
#define AR_FATHA						'a'
#define AR_DAMMA						'u'
#define AR_KASRA						'i'
#define AR_SHADDA						'G'
#define AR_SUKUN						'o'
#define AR_SUPERSCRIPT_ALEF				'R'
/* Hamza variant */
#define AR_ALEF_WASLA					'|'

#endif



/* Replacement rules for one diacritic that can be omitted regardless
 * its position in the token */
#define FATHA_OMISSION "fatha omission"
#define DAMMA_OMISSION "damma omission"
#define KASRA_OMISSION "kasra omission"
#define SUKUN_OMISSION "sukun omission"
#define SUPERSCRIPT_ALEF_OMISSION "superscript alef omission"
/* Replacement rules for one diacritic that can be omitted but
 * only at the end of the token */
#define FATHATAN_OMISSION_AT_END "fathatan omission at end"
#define DAMMATAN_OMISSION_AT_END "dammatan omission at end"
#define KASRATAN_OMISSION_AT_END "kasratan omission at end"
/* Replacement rules for two diacritics at the end of the token.
 * For "shadda X", any or both diacritics can be omitted */
#define SHADDA_FATHA_OMISSION_AT_END "shadda fatha omission at end"
#define SHADDA_DAMMA_OMISSION_AT_END "shadda damma omission at end"
#define SHADDA_KASRA_OMISSION_AT_END "shadda kasra omission at end"
#define SHADDA_FATHATAN_OMISSION_AT_END "shadda fathatan omission at end"
#define SHADDA_DAMMATAN_OMISSION_AT_END "shadda dammatan omission at end"
#define SHADDA_KASRATAN_OMISSION_AT_END "shadda kasratan omission at end"
/* Replacement rules for two diacritics not at the end of the token.
 * For "shadda X", X or both diacritics can be omitted */
#define SHADDA_FATHA_OMISSION "shadda fatha omission"
#define SHADDA_DAMMA_OMISSION "shadda damma omission"
#define SHADDA_KASRA_OMISSION "shadda kasra omission"
#define SHADDA_SUPERSCRIPT_ALEF_OMISSION "shadda superscript alef omission"
/* Solar assimilation rule for Al: shadda can appear after the first letter following
 * 'Al' if this letter is a solar one. Example: Al + raeiysu => AlrUaeiysu */
#define SOLAR_ASSIMILATION "solar assimilation"
/* Lunar assimilation rule for Al: sukun can appear before the first letter following
 * 'Al' if this letter is a lunar one. Example: Al + qamaru => Aloqamaru */
#define LUNAR_ASSIMILATION "lunar assimilation"
/* Al special rule: it can be written Ll (alef -> alef wasla) */
#define AL_WITH_WASLA "Al with wasla"
/* An initial alef hamza above O may be written A */
#define ALEF_HAMZA_ABOVE_O_TO_A "alef hamza above O to A"
/* An initial alef hamza below I may be written A or L */
#define ALEF_HAMZA_BELOW_I_TO_A "alef hamza below I to A"
#define ALEF_HAMZA_BELOW_I_TO_L "alef hamza below I to L"
/* Two normalization rules */
#define FATHATAN_ALEF_EQUIV_ALEF_FATHATAN "fathatan alef equiv alef fathatan"
#define FATHATAN_ALEF_MAQSURA_EQUIV_ALEF_MAQSURA_FATHATAN "fathatan alef maqsura equiv alef maqsura fathatan"
/* At the end of a word, Yc can be written e */
#define ALEF_MAQSURA_HAMZA_EQUIV_HAMZA_ABOVE_YEH "﻿﻿﻿﻿alef maqsura hamza equiv hamza above yeh"
/* There is also an extra rule that always applies: tatweel in
 * the text can be ignored if not present in the dictionary */


/* This bit-field structure is used to know which rules must be applied
 * during the dictionary lookup in morphological mode */
typedef struct {
	/* This field is used to know if we really have a rule set or not.
	 * It is used to avoid something like a test to NULL on a structure
	 * pointer before testing the structure itself
	 */
	unsigned int rules_enabled: 1;

	unsigned int fatha_omission: 1;
	unsigned int damma_omission: 1;
	unsigned int kasra_omission: 1;
	unsigned int sukun_omission: 1;
	unsigned int superscript_alef_omission: 1;
	unsigned int fathatan_omission_at_end: 1;
	unsigned int dammatan_omission_at_end: 1;
	unsigned int kasratan_omission_at_end: 1;
	unsigned int shadda_fatha_omission_at_end: 1;
	unsigned int shadda_damma_omission_at_end: 1;
	unsigned int shadda_kasra_omission_at_end: 1;
	unsigned int shadda_fathatan_omission_at_end: 1;
	unsigned int shadda_dammatan_omission_at_end: 1;
	unsigned int shadda_kasratan_omission_at_end: 1;
	unsigned int shadda_fatha_omission: 1;
	unsigned int shadda_damma_omission: 1;
	unsigned int shadda_kasra_omission: 1;
	unsigned int shadda_superscript_alef_omission: 1;
	unsigned int solar_assimilation: 1;
	unsigned int lunar_assimilation: 1;
	unsigned int al_with_wasla: 1;
	unsigned int alef_hamza_above_O_to_A: 1;
	unsigned int alef_hamza_below_I_to_A: 1;
	unsigned int alef_hamza_below_I_to_L: 1;
	unsigned int fathatan_alef_equiv_alef_fathatan: 1;
	unsigned int fathatan_alef_maqsura_equiv_alef_maqsura_fathatan: 1;
	unsigned int alef_maqsura_hamza_equiv_hamza_above_yeh: 1;
} ArabicTypoRules;


unichar to_buckwalter(unichar);
unichar to_buckwalter_plusplus(unichar);
unichar from_buckwalter(unichar);
unichar from_buckwalter_plusplus(unichar);
int is_solar(unichar);
int is_lunar(unichar);
int load_arabic_typo_rules(const VersatileEncodingConfig*,const char* f,ArabicTypoRules *rules);
int was_Al_before(const unichar* token,int pos,ArabicTypoRules rules);
int is_arabic_letter(unichar c);

} // namespace unitex

#endif
