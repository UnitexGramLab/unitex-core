 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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
//
//	segtype.cpp
//

#include <stdio.h>
#include <stdlib.h>
#include "Unicode.h"
#include "unimap.h"
#include "segtype.h"

unsigned char wideCharTable::charMapBelow256[256] = {
// 0x00
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,RET_SYL,NUL_SYL,NUL_SYL,RET_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
// 0x20
	SPA_SYL,TPH_SYL,TPH_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,TPH_SYL,
	TPS_SYL,TPE_SYL,SYM_SYL,SYM_SYL,TPH_SYL,TPH_SYL,TPH_SYL,TPH_SYL,
	NUM_SYL,NUM_SYL,NUM_SYL,NUM_SYL,NUM_SYL,NUM_SYL,NUM_SYL,NUM_SYL,
	NUM_SYL,NUM_SYL,TPH_SYL,TPH_SYL,TPH_SYL,SYM_SYL,TPH_SYL,TPH_SYL,
//0x40
	SYM_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,TPS_SYL,TPH_SYL,TPE_SYL,TPH_SYL,TPH_SYL,
//0x60
	TPH_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,ASC_SYL,
	ASC_SYL,ASC_SYL,ASC_SYL,TPS_SYL,TPH_SYL,TPE_SYL,TPH_SYL,NUL_SYL,
// 0x80
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
	NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,NUL_SYL,
// 0xa0
	NUL_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,
	SYM_SYL,SYM_SYL,SYM_SYL,TPS_SYL,SYM_SYL,NUL_SYL,SYM_SYL,SYM_SYL,
	SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,TPH_SYL,
	SYM_SYL,SYM_SYL,SYM_SYL,TPE_SYL,SYM_SYL,SYM_SYL,SYM_SYL,SYM_SYL,
// 0xc0 - 0x24f latin/latin extended-A,B //
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,TPH_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,
// 0xe0
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,TPH_SYL,
	ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL,ESC_SYL
};


unsigned char wideCharTable::spacingModifierTable[0x50] // 
= { 
// 0x02b0
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, TPH_SYL, TPH_SYL, TPH_SYL, TPH_SYL, TPH_SYL, SYM_SYL, SYM_SYL, 
// 0x02c0
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
// 0x02d0
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
// 0x02e0
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
// 0x02f0
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL
};
unsigned char wideCharTable::symbolsPunctuationTable[0x40]= {
// 0x3000
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	TPS_SYL, TPE_SYL, TPS_SYL, TPE_SYL, TPS_SYL, TPE_SYL, TPS_SYL, TPE_SYL, 
// 0x3010
	TPS_SYL, TPE_SYL, SYM_SYL, SYM_SYL, TPS_SYL, TPE_SYL, TPS_SYL, TPE_SYL, 
	TPS_SYL, TPE_SYL, TPS_SYL, TPE_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
// 0x3020
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
// 0x3030
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, 
	SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL, SYM_SYL
};


unsigned char wideCharTable::unicode_bankmap[256]={
//00
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	HAN_SYL,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x01,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
// 0x40
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
//0x80	
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,HJA_SYL,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
	HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
	HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
// 0xc0
	HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
	HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
	HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,HAN_SYL,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
	0x00,HJA_SYL,HJA_SYL,0x00,0x00,0x00,0x00,0x00
};

wideCharTable::wideCharTable()
{
}
wideCharTable::~wideCharTable()
{
}
int 
wideCharTable::check_range_character(unichar compareValue)
{
	if(compareValue < UNIZONE_IPA_Extensions) {// zone de latins
		return((int)(charMapBelow256[compareValue]&0xffff));
	} else if (compareValue < UNIZONE_Spacing_Modifier_Letters ){// zone de latins
		return((int)(SYM_SYL&0xffff));
	} else if (compareValue < UNIZONE_Combining_Diacritical_Marks ){// zone de latins
		return((int)((spacingModifierTable[compareValue 
			- UNIZONE_Spacing_Modifier_Letters] ) & 0xffff));
	} else if (compareValue < UNIZONE_Hangul_Jamo) {// zone de latins
		return((int)(SYM_SYL));
	} else if (compareValue < UNIZONE_Ethiopic) {// zone de latins
		return((int)(HAN_SYL));
	} else if (compareValue  < UNIZONE_CJK_Symbols_and_Punctuation ) { // zone de hangul jamo pour
		return((int)(SYM_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Hiragana) { // zone de hangul jamo pour
		return((int)(symbolsPunctuationTable[(unsigned int)compareValue
			- (unsigned int)UNIZONE_CJK_Symbols_and_Punctuation ])& 0xffff);// le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Bopomofo) { // zone de hangul jamo pour
		return((int)(ESC_SYL));	  // le Hiragana/katagana avec MS dbcs
	} else if (compareValue  < UNIZONE_Hangul_Compatiblity_Jamo) { // zone de hangul jamo pour
		return((int)(SYM_SYL));	  
	} else if (compareValue  < UNIZONE_Kanbun) { // zone de hangul jamo pour  
		return((int)(SYM_SYL));	  // hangeul compatible region
	} else if (compareValue  < UNIZONE_CJK_Unified_Ideographs_Externsion_A ) { // zone de hangul jamo pour  
		return((int)(SYM_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Yi_Syllables ) { // zone de ideogramme  
		return((int)(HJA_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Hangul_Syllables ) {  
		return((int)(SYM_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_High_Surrogates ) { // zone de syllabe coreen  
		return((int)(HAN_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_CJK_Compatibility_Ideographs ) {  
		return((int)(SYM_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Alphabet_Presentation_Forms) { 
		return((int)(HJA_SYL));	 // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_CJK_Compatibility_Forms ) { 
		return((int)(SYM_SYL));	  // le compatibie avec MS dbcs
	} else if (compareValue  < UNIZONE_Small_Form_Variants) { // zone de ideogrammes 
		return((int)(HJA_SYL));	 // le compatibie avec MS dbcs
	}
	return((int)(SYM_SYL));
}
