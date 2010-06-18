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

#include "Arabic.h"
#include "String_hash.h"
#include "Error.h"


unichar to_buckwalter(unichar c) {
switch (c) {
/* Plain letters */
case AR_HAMZA: 						return '\'';
case AR_ALEF_WITH_MADDA_ABOVE: 		return '|';
case AR_ALEF_WITH_HAMZA_ABOVE: 		return '>';
case AR_WAW_WITH_HAMZA_ABOVE: 		return '&';
case AR_ALEF_WITH_HAMZA_BELOW: 		return '<';
case AR_YEH_WITH_HAMZA_ABOVE: 		return '}';
case AR_ALEF: 						return 'A';
case AR_BEH: 						return 'b';
case AR_TEH_MARBUTA: 				return 'p';
case AR_TEH: 						return 't';
case AR_THEH: 						return 'v';
case AR_JEEM: 						return 'j';
case AR_HAH: 						return 'H';
case AR_KHAH: 						return 'x';
case AR_DAL: 						return 'd';
case AR_THAL:						return '*';
case AR_REH: 						return 'r';
case AR_ZAIN:						return 'z';
case AR_SEEN: 						return 's';
case AR_SHEEN: 						return '$';
case AR_SAD:	 					return 'S';
case AR_DAD:	 					return 'D';
case AR_TAH:	 					return 'T';
case AR_ZAH:	 					return 'Z';
case AR_AIN:	 					return 'E';
case AR_GHAIN:						return 'g';
case AR_TATWEEL: 					return '_';
case AR_FEH:	 					return 'f';
case AR_QAF:	 					return 'q';
case AR_KAF:	 					return 'k';
case AR_LAM:	 					return 'l';
case AR_MEEM:	 					return 'm';
case AR_NOON:	 					return 'n';
case AR_HEH:	 					return 'h';
case AR_WAW:	 					return 'w';
case AR_ALEF_MAQSURA:				return 'Y';
case AR_YEH:	 					return 'y';
/* Diacritics */
case AR_FATHATAN:					return 'F';
case AR_DAMMATAN:					return 'N';
case AR_KASRATAN:					return 'K';
case AR_FATHA:						return 'a';
case AR_DAMMA:						return 'u';
case AR_KASRA:						return 'i';
case AR_SHADDA:						return '~';
case AR_SUKUN:						return 'o';
case AR_SUPERSCRIPT_ALEF:			return '`';
/* Hamza variant */
case AR_ALEF_WASLA:					return '{';
default: return c;
}
}


/**
 * Buckwalter transliteration modified for Unitex
 */
unichar to_buckwalter_plusplus(unichar c) {
c=to_buckwalter(c);
switch(c) {
case '\'': 	return 'c';
case '|': 	return 'C';
case '>': 	return 'O';
case '&': 	return 'W';
case '<': 	return 'I';
case '}': 	return 'e';
case '*': 	return 'J';
case '$': 	return 'M';
case '~': 	return 'U';
case '`': 	return 'R';
case '{': 	return 'L';
default: 	return c;
}
}


int is_solar(unichar c) {
switch (c) {
case AR_TEH:
case AR_THEH:
case AR_JEEM:
case AR_DAL:
case AR_THAL:
case AR_REH:
case AR_ZAIN:
case AR_SEEN:
case AR_SHEEN:
case AR_SAD:
case AR_DAD:
case AR_TAH:
case AR_ZAH:
case AR_LAM:
case AR_NOON: return 1;
default: return 0;
}
}


int is_lunar(unichar c) {
switch(c) {
case AR_HAMZA:
case AR_ALEF_WITH_MADDA_ABOVE:
case AR_ALEF_WITH_HAMZA_ABOVE:
case AR_WAW_WITH_HAMZA_ABOVE:
case AR_ALEF_WITH_HAMZA_BELOW:
case AR_YEH_WITH_HAMZA_ABOVE:
case AR_BEH:
case AR_HAH:
case AR_KHAH:
case AR_AIN:
case AR_GHAIN:
case AR_FEH:
case AR_QAF:
case AR_KAF:
case AR_MEEM:
case AR_HEH:
case AR_WAW:
case AR_YEH:
case AR_ALEF_WASLA:	return 1;
default: return 0;
}
}


/**
 * Returns 1 if the rule name is in the hash with the value "YES"; 0 otherwise.
 */
static unsigned int test(struct string_hash* h,const char* rule_name) {
unichar key[128];
u_strcpy(key,rule_name);
int i=get_value_index(key,h,DONT_INSERT);
if (i==-1 || u_strcmp(h->value[i],"YES")) return 0;
return 1;
}


/**
 * Loads the rule configuration file. Returns 1 in case of success; 0 otherwise and *rules
 * is set to 0.
 * "YES" value means 1, any other value means 0;
 */
int load_arabic_typo_rules(char* name,ArabicTypoRules *rules) {
memset(rules,0,sizeof(ArabicTypoRules));
if (name==NULL) {
	fatal_error("NULL arabic rule configuration file name!\n");
}
if (rules==NULL) {
	fatal_error("Unexpected NULL pointer in load_arabic_typo_rules\n");
}
struct string_hash* h=load_key_value_list(name,DEFAULT_MASK_ENCODING_COMPATIBILITY_INPUT,'=');
if (h==NULL) return 0;
(*rules).rules_enabled=1;
(*rules).fatha_omission=test(h,FATHA_OMISSION);
(*rules).damma_omission=test(h,DAMMA_OMISSION);
(*rules).kasra_omission=test(h,KASRA_OMISSION);
(*rules).sukun_omission=test(h,SUKUN_OMISSION);
(*rules).superscript_alef_omission=test(h,SUPERSCRIPT_ALEF_OMISSION);
(*rules).fathatan_omission_at_end=test(h,FATHATAN_OMISSION_AT_END);
(*rules).dammatan_omission_at_end=test(h,DAMMATAN_OMISSION_AT_END);
(*rules).kasratan_omission_at_end=test(h,KASRATAN_OMISSION_AT_END);
(*rules).shadda_fatha_omission_at_end=test(h,SHADDA_FATHA_OMISSION_AT_END);
(*rules).shadda_damma_omission_at_end=test(h,SHADDA_DAMMA_OMISSION_AT_END);
(*rules).shadda_kasra_omission_at_end=test(h,SHADDA_KASRA_OMISSION_AT_END);
(*rules).shadda_fathatan_omission_at_end=test(h,SHADDA_FATHATAN_OMISSION_AT_END);
(*rules).shadda_dammatan_omission_at_end=test(h,SHADDA_DAMMATAN_OMISSION_AT_END);
(*rules).shadda_kasratan_omission_at_end=test(h,SHADDA_KASRATAN_OMISSION_AT_END);
(*rules).shadda_fatha_omission=test(h,SHADDA_FATHA_OMISSION);
(*rules).shadda_damma_omission=test(h,SHADDA_DAMMA_OMISSION);
(*rules).shadda_kasra_omission=test(h,SHADDA_KASRA_OMISSION);
(*rules).shadda_superscript_alef_omission=test(h,SHADDA_SUPERSCRIPT_ALEF_OMISSION);
(*rules).solar_assimilation=test(h,SOLAR_ASSIMILATION);
(*rules).lunar_assimilation=test(h,LUNAR_ASSIMILATION);
(*rules).al_with_wasla=test(h,AL_WITH_WASLA);
(*rules).alef_hamza_above_O=test(h,ALEF_HAMZA_ABOVE_O);
(*rules).alef_hamza_below_I_to_A=test(h,ALEF_HAMZA_BELOW_I_TO_A);
(*rules).alef_hamza_below_I_to_L=test(h,ALEF_HAMZA_BELOW_I_TO_L);
(*rules).fathatan_alef_equiv_alef_fathatan=test(h,FATHATAN_ALEF_EQUIV_ALEF_FATHATAN);
(*rules).fathatan_alef_maqsura_equiv_alef_maqsura_fathatan=test(h,FATHATAN_ALEF_MAQSURA_EQUIV_ALEF_MAQSURA_FATHATAN);
free_string_hash(h);
return 1;
}


/**
 * This function takes a token and a position and checks whether there is
 * a valid sequence with Al before the given position. This function considers
 * the optional prepositions b(i) and k(a), as well as the optional
 * conjunctions w(a) and f(a).
 *
 * Examples:
 * 		Alraeiysu + pos=2 => OK
 * 		lilraeiysu + pos=3 => OK
 * 		llraeiysu + pos=2 => OK, but only if the i omission rule is active
 */
int was_Al_before(unichar* token,int pos,ArabicTypoRules rules) {
if (pos<=1) return 0;
pos--;
if (token[pos--]!=AR_LAM) return 0;
int ok=0;
int lil=0;
if (token[pos]==AR_ALEF) { ok=1; pos--; }								/* Al */
else if (token[pos]==AR_ALEF_WASLA) { ok=rules.al_with_wasla; pos--; }	/* Ll */
else if (token[pos]==AR_LAM) { ok=rules.kasra_omission; pos--; lil=1; }	/* ll */
else if (pos>0 && token[pos]==AR_KASRA && token[pos-1]==AR_LAM) {		/* lil */
	ok=1;
	pos=pos-2;
	lil=1;
}
if (!ok) return 0;
if (pos==-1) return 1;
if (!lil) {
	/* Before Al or Ll, we can find bi or ka */
	if (token[pos]==AR_KASRA && pos>0 && token[pos-1]==AR_BEH) {	/* bi */
		pos=pos-2;
	} else if (token[pos]==AR_BEH) {								/* b */
		if (!rules.kasra_omission) return 0;
		pos--;
	} else if (token[pos]==AR_FATHA) {
		/* If we have a 'a', it can be 'ka', or 'wa'/'fa' */
		if (pos==-1) return 0;
		if (token[pos]==AR_WAW || token[pos]==AR_FEH) { 			/* wa or fa */
			/* We must be at the beginning of the word */
			return pos==0;
		} else if (token[pos]==AR_KAF) {							/* ka */
			pos--;
		} else {
			return 0;
		}
	} else if (token[pos]==AR_KAF) {								/* k */
		if (!rules.fatha_omission) return 0;
		pos--;
	} else if (token[pos]==AR_WAW) {								/* w */
		return pos==0 && rules.fatha_omission;
	} else if (token[pos]==AR_BEH) {								/* b */
		return pos==0 && rules.fatha_omission;
	} else {
		return 0;
	}
}
if (pos==-1) return 1;
if (token[pos]==AR_WAW || token[pos]==AR_FEH) { 					/* w or f */
	/* We must be at the beginning of the word */
	return pos==0 && rules.fatha_omission;
} else if (token[pos]==AR_FATHA) {									/* wa or fa */
	pos--;
	return pos==0 && (token[pos]==AR_WAW || token[pos]==AR_FEH);
} else {
	return 0;
}
}
