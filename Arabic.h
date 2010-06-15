/*
 * Unitex
 *
 * Copyright (C) 2001-2010 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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


unichar to_buckwalter(unichar);
unichar to_buckwalter_plusplus(unichar);

#endif
