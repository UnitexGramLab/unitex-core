/*
 * Unitex
 *
 * Copyright (C) 2001-2016 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

unichar to_buckwalter(unichar c) {
switch (c) {
/* Plain letters */
case ABS_AR_HAMZA: 						return '\'';
case ABS_AR_ALEF_WITH_MADDA_ABOVE: 		return '|';
case ABS_AR_ALEF_WITH_HAMZA_ABOVE: 		return '>';
case ABS_AR_WAW_WITH_HAMZA_ABOVE: 		return '&';
case ABS_AR_ALEF_WITH_HAMZA_BELOW: 		return '<';
case ABS_AR_YEH_WITH_HAMZA_ABOVE: 		return '}';
case ABS_AR_ALEF: 						return 'A';
case ABS_AR_BEH: 						return 'b';
case ABS_AR_TEH_MARBUTA: 				return 'p';
case ABS_AR_TEH: 						return 't';
case ABS_AR_THEH: 						return 'v';
case ABS_AR_JEEM: 						return 'j';
case ABS_AR_HAH: 						return 'H';
case ABS_AR_KHAH: 						return 'x';
case ABS_AR_DAL: 						return 'd';
case ABS_AR_THAL:						return '*';
case ABS_AR_REH: 						return 'r';
case ABS_AR_ZAIN:						return 'z';
case ABS_AR_SEEN: 						return 's';
case ABS_AR_SHEEN: 						return '$';
case ABS_AR_SAD:	 					return 'S';
case ABS_AR_DAD:	 					return 'D';
case ABS_AR_TAH:	 					return 'T';
case ABS_AR_ZAH:	 					return 'Z';
case ABS_AR_AIN:	 					return 'E';
case ABS_AR_GHAIN:						return 'g';
case ABS_AR_TATWEEL: 					return '_';
case ABS_AR_FEH:	 					return 'f';
case ABS_AR_QAF:	 					return 'q';
case ABS_AR_KAF:	 					return 'k';
case ABS_AR_LAM:	 					return 'l';
case ABS_AR_MEEM:	 					return 'm';
case ABS_AR_NOON:	 					return 'n';
case ABS_AR_HEH:	 					return 'h';
case ABS_AR_WAW:	 					return 'w';
case ABS_AR_ALEF_MAQSURA:				return 'Y';
case ABS_AR_YEH:	 					return 'y';
/* Diacritics */
case ABS_AR_FATHATAN:					return 'F';
case ABS_AR_DAMMATAN:					return 'N';
case ABS_AR_KASRATAN:					return 'K';
case ABS_AR_FATHA:						return 'a';
case ABS_AR_DAMMA:						return 'u';
case ABS_AR_KASRA:						return 'i';
case ABS_AR_SHADDA:						return '~';
case ABS_AR_SUKUN:						return 'o';
case ABS_AR_SUPERSCRIPT_ALEF:			return '`';
/* Hamza variant */
case ABS_AR_ALEF_WASLA:					return '{';
default: return c;
}
}


/**
 * Buckwalter transliteration modified for Unitex
 */
unichar to_buckwalter_plusplus(unichar c) {
unichar c2=to_buckwalter(c);
if (c==c2) {
	/* If the character wasn't converted by to_buckwalter, it was not
	 * an Arabic char, so we return it. Doing so avoid problems when using
	 * to_buckwalter_plus_plus to convert an Arabic text containing {, } or any
	 * character using in this transliteration */
	return c;
}
switch(c2) {
case '\'': 	return 'c';
case '|': 	return 'C';
case '>': 	return 'O';
case '&': 	return 'W';
case '<': 	return 'I';
case '}': 	return 'e';
case '*': 	return 'J';
case '$': 	return 'M';
case '~': 	return 'G';
case '`': 	return 'R';
case '{': 	return 'L';
default: 	return c2;
}
}


unichar buckwalter_plusplus_to_buckwalter(unichar c) {
switch(c) {
case 'c': 	return '\'';
case 'C': 	return '|';
case 'O': 	return '>';
case 'W': 	return '&';
case 'I': 	return '<';
case 'e': 	return '}';
case 'J': 	return '*';
case 'M': 	return '$';
case 'G': 	return '~';
case 'R': 	return '`';
case 'L': 	return '{';
default: 	return c;
}
}


unichar from_buckwalter(unichar c) {
switch (c) {
/* Plain letters */
case '\'':	return ABS_AR_HAMZA;
case '|':	return ABS_AR_ALEF_WITH_MADDA_ABOVE;
case '>':	return ABS_AR_ALEF_WITH_HAMZA_ABOVE;
case '&':	return ABS_AR_WAW_WITH_HAMZA_ABOVE;
case '<':	return ABS_AR_ALEF_WITH_HAMZA_BELOW;
case '}':	return ABS_AR_YEH_WITH_HAMZA_ABOVE;
case 'A':	return ABS_AR_ALEF;
case 'b':	return ABS_AR_BEH;
case 'p':	return ABS_AR_TEH_MARBUTA;
case 't':	return ABS_AR_TEH;
case 'v':	return ABS_AR_THEH;
case 'j':	return ABS_AR_JEEM;
case 'H':	return ABS_AR_HAH;
case 'x':	return ABS_AR_KHAH;
case 'd':	return ABS_AR_DAL;
case '*':	return ABS_AR_THAL;
case 'r':	return ABS_AR_REH;
case 'z':	return ABS_AR_ZAIN;
case 's':	return ABS_AR_SEEN;
case '$':	return ABS_AR_SHEEN;
case 'S':	return ABS_AR_SAD;
case 'D':	return ABS_AR_DAD;
case 'T':	return ABS_AR_TAH;
case 'Z':	return ABS_AR_ZAH;
case 'E':	return ABS_AR_AIN;
case 'g':	return ABS_AR_GHAIN;
case '_':	return ABS_AR_TATWEEL;
case 'f':	return ABS_AR_FEH;
case 'q':	return ABS_AR_QAF;
case 'k':	return ABS_AR_KAF;
case 'l':	return ABS_AR_LAM;
case 'm':	return ABS_AR_MEEM;
case 'n':	return ABS_AR_NOON;
case 'h':	return ABS_AR_HEH;
case 'w':	return ABS_AR_WAW;
case 'Y':	return ABS_AR_ALEF_MAQSURA;
case 'y':	return ABS_AR_YEH;
/* Diacritics */
case 'F':	return ABS_AR_FATHATAN;
case 'N':	return ABS_AR_DAMMATAN;
case 'K':	return ABS_AR_KASRATAN;
case 'a':	return ABS_AR_FATHA;
case 'u':	return ABS_AR_DAMMA;
case 'i':	return ABS_AR_KASRA;
case '~':	return ABS_AR_SHADDA;
case 'o':	return ABS_AR_SUKUN;
case '`':	return ABS_AR_SUPERSCRIPT_ALEF;
/* Hamza variant */
case '{':	return ABS_AR_ALEF_WASLA;
default: return c;
}
}


unichar from_buckwalter_plusplus(unichar c) {
return from_buckwalter(buckwalter_plusplus_to_buckwalter(c));
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
int load_arabic_typo_rules(const VersatileEncodingConfig* vec,const char* name,ArabicTypoRules *rules) {
memset(rules,0,sizeof(ArabicTypoRules));
if (name==NULL) {
	fatal_error("NULL arabic rule configuration file name!\n");
}
if (rules==NULL) {
	fatal_error("Unexpected NULL pointer in load_arabic_typo_rules\n");
}
struct string_hash* h=load_key_value_list(name,vec,'=');
if (h==NULL) {
	error("Cannot load %s\n",name);
	return 0;
}
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
(*rules).alef_hamza_above_O_to_A=test(h,ALEF_HAMZA_ABOVE_O_TO_A);
(*rules).alef_hamza_below_I_to_A=test(h,ALEF_HAMZA_BELOW_I_TO_A);
(*rules).alef_hamza_below_I_to_L=test(h,ALEF_HAMZA_BELOW_I_TO_L);
(*rules).fathatan_alef_equiv_alef_fathatan=test(h,FATHATAN_ALEF_EQUIV_ALEF_FATHATAN);
(*rules).fathatan_alef_maqsura_equiv_alef_maqsura_fathatan=test(h,FATHATAN_ALEF_MAQSURA_EQUIV_ALEF_MAQSURA_FATHATAN);
(*rules).alef_maqsura_hamza_equiv_hamza_above_yeh=test(h,ALEF_MAQSURA_HAMZA_EQUIV_HAMZA_ABOVE_YEH);
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
int was_Al_before(const unichar* token,int pos,ArabicTypoRules rules) {
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


/**
 * Returns 1 if c is either an Arabic letter according to AR_XXX definitions
 * or an absolute arabic letter like ABS_AR_XXX; 0 otherwise.
 */
int is_arabic_letter(unichar c) {
switch(c) {
/* Standard Arabic */
case ABS_AR_HAMZA:
case ABS_AR_ALEF_WITH_MADDA_ABOVE:
case ABS_AR_ALEF_WITH_HAMZA_ABOVE:
case ABS_AR_WAW_WITH_HAMZA_ABOVE:
case ABS_AR_ALEF_WITH_HAMZA_BELOW:
case ABS_AR_YEH_WITH_HAMZA_ABOVE:
case ABS_AR_ALEF:
case ABS_AR_BEH:
case ABS_AR_TEH_MARBUTA:
case ABS_AR_TEH:
case ABS_AR_THEH:
case ABS_AR_JEEM:
case ABS_AR_HAH:
case ABS_AR_KHAH:
case ABS_AR_DAL:
case ABS_AR_THAL:
case ABS_AR_REH:
case ABS_AR_ZAIN:
case ABS_AR_SEEN:
case ABS_AR_SHEEN:
case ABS_AR_SAD:
case ABS_AR_DAD:
case ABS_AR_TAH:
case ABS_AR_ZAH:
case ABS_AR_AIN:
case ABS_AR_GHAIN:
case ABS_AR_TATWEEL:
case ABS_AR_FEH:
case ABS_AR_QAF:
case ABS_AR_KAF:
case ABS_AR_LAM:
case ABS_AR_MEEM:
case ABS_AR_NOON:
case ABS_AR_HEH:
case ABS_AR_WAW:
case ABS_AR_ALEF_MAQSURA:
case ABS_AR_YEH:
case ABS_AR_FATHATAN:
case ABS_AR_DAMMATAN:
case ABS_AR_KASRATAN:
case ABS_AR_FATHA:
case ABS_AR_DAMMA:
case ABS_AR_KASRA:
case ABS_AR_SHADDA:
case ABS_AR_SUKUN:
case ABS_AR_SUPERSCRIPT_ALEF:
case ABS_AR_ALEF_WASLA:
/* Buckwalter */
case BW_AR_HAMZA:
case BW_AR_ALEF_WITH_MADDA_ABOVE:
case BW_AR_ALEF_WITH_HAMZA_ABOVE:
case BW_AR_WAW_WITH_HAMZA_ABOVE:
case BW_AR_ALEF_WITH_HAMZA_BELOW:
case BW_AR_YEH_WITH_HAMZA_ABOVE:
case BW_AR_ALEF:
case BW_AR_BEH:
case BW_AR_TEH_MARBUTA:
case BW_AR_TEH:
case BW_AR_THEH:
case BW_AR_JEEM:
case BW_AR_HAH:
case BW_AR_KHAH:
case BW_AR_DAL:
case BW_AR_THAL:
case BW_AR_REH:
case BW_AR_ZAIN:
case BW_AR_SEEN:
case BW_AR_SHEEN:
case BW_AR_SAD:
case BW_AR_DAD:
case BW_AR_TAH:
case BW_AR_ZAH:
case BW_AR_AIN:
case BW_AR_GHAIN:
case BW_AR_TATWEEL:
case BW_AR_FEH:
case BW_AR_QAF:
case BW_AR_KAF:
case BW_AR_LAM:
case BW_AR_MEEM:
case BW_AR_NOON:
case BW_AR_HEH:
case BW_AR_WAW:
case BW_AR_ALEF_MAQSURA:
case BW_AR_YEH:
case BW_AR_FATHATAN:
case BW_AR_DAMMATAN:
case BW_AR_KASRATAN:
case BW_AR_FATHA:
case BW_AR_DAMMA:
case BW_AR_KASRA:
case BW_AR_SHADDA:
case BW_AR_SUKUN:
case BW_AR_SUPERSCRIPT_ALEF:
case BW_AR_ALEF_WASLA:
return 1;
default: return 0;
}
}

} // namespace unitex
