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
