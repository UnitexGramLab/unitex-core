 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include <stdio.h>
#include <stdlib.h>
#include "HTMLCharacters.h"
#include "Error.h"

/**
 * This library provides facilities to replace HTML characters encoding like
 * &#eacute; by their unicode numbers. All the codes present here have been
 * taken from the DTD of HTML as released by the W3C.
 */


struct search_tree_node* init_control_characters();
struct search_tree_node* init_normal_characters();

struct search_tree_node* control_characters=init_control_characters();
struct search_tree_node* normal_characters=init_normal_characters();


/**
 * This function creates and returns a tree that associates unicode numbers
 * to HTML control characters.
 */
struct search_tree_node* init_control_characters() {
struct search_tree_node* root=NULL;
insert_string(&root,"quot",34);
insert_string(&root,"amp",38);
insert_string(&root,"lt",60);
insert_string(&root,"gt",62);
return root;
}


/**
 * This function creates and returns a tree that associates unicode numbers
 * to HTML non control characters.
 */
struct search_tree_node* init_normal_characters() {
struct search_tree_node* root=NULL;
/* Spacing Modifier Letters */
insert_string(&root,"circ",710);
insert_string(&root,"tilde",732);
/* General Punctuation */
insert_string(&root,"ensp",8194);
insert_string(&root,"emsp",8195);
insert_string(&root,"thinsp",8201);
insert_string(&root,"zwnj",8204);
insert_string(&root,"zwj",8205);
insert_string(&root,"lrm",8206);
insert_string(&root,"rlm",8207);
insert_string(&root,"ndash",8211);
insert_string(&root,"mdash",8212);
insert_string(&root,"lsquo",8216);
insert_string(&root,"rsquo",8218);
insert_string(&root,"bdquo",8222);
insert_string(&root,"dagger",8224);
insert_string(&root,"Dagger",8225);
insert_string(&root,"bull",8226);
insert_string(&root,"hellip",8230);
insert_string(&root,"permil",8240);
insert_string(&root,"prime",8242);
insert_string(&root,"Prime",8243);
insert_string(&root,"lsaquo",8249);
insert_string(&root,"rsaquo",8250);
insert_string(&root,"oline",8254);
insert_string(&root,"frasl",8260);
insert_string(&root,"euro",8364);
/* Latin 1 */
insert_string(&root,"nbsp",160);
insert_string(&root,"iexcl",161);
insert_string(&root,"cent",162);
insert_string(&root,"pound",163);
insert_string(&root,"curren",164);
insert_string(&root,"yen",165);
insert_string(&root,"brvbar",166);
insert_string(&root,"sect",167);
insert_string(&root,"uml",168);
insert_string(&root,"copy",169);
insert_string(&root,"ordf",170);
insert_string(&root,"laquo",171);
insert_string(&root,"not",172);
insert_string(&root,"shy",173);
insert_string(&root,"reg",174);
insert_string(&root,"macr",175);
insert_string(&root,"deg",176);
insert_string(&root,"plusmn",177);
insert_string(&root,"sup2",178);
insert_string(&root,"sup3",179);
insert_string(&root,"acute",180);
insert_string(&root,"micro",181);
insert_string(&root,"para",182);
insert_string(&root,"middot",183);
insert_string(&root,"cedil",184);
insert_string(&root,"sup1",185);
insert_string(&root,"ordm",186);
insert_string(&root,"raquo",187);
insert_string(&root,"frac14",188);
insert_string(&root,"frac12",189);
insert_string(&root,"frac34",190);
insert_string(&root,"iquest",191);
insert_string(&root,"Agrave",192);
insert_string(&root,"Aacute",193);
insert_string(&root,"Acirc",194);
insert_string(&root,"Atilde",195);
insert_string(&root,"Auml",196);
insert_string(&root,"Aring",197);
insert_string(&root,"AElig",198);
insert_string(&root,"Ccedil",199);
insert_string(&root,"Egrave",200);
insert_string(&root,"Eacute",201);
insert_string(&root,"Ecirc",202);
insert_string(&root,"Euml",203);
insert_string(&root,"Igrave",204);
insert_string(&root,"Iacute",205);
insert_string(&root,"Icirc",206);
insert_string(&root,"Iuml",207);
insert_string(&root,"ETH",208);
insert_string(&root,"Ntilde",209);
insert_string(&root,"Ograve",210);
insert_string(&root,"Oacute",211);
insert_string(&root,"Ocirc",212);
insert_string(&root,"Otilde",213);
insert_string(&root,"Ouml",214);
insert_string(&root,"times",215);
insert_string(&root,"Oslash",216);
insert_string(&root,"Ugrave",217);
insert_string(&root,"Uacute",218);
insert_string(&root,"Ucirc",219);
insert_string(&root,"Uuml",220);
insert_string(&root,"Yacute",221);
insert_string(&root,"THORN",222);
insert_string(&root,"szlig",223);
insert_string(&root,"agrave",224);
insert_string(&root,"aacute",225);
insert_string(&root,"acirc",226);
insert_string(&root,"atilde",227);
insert_string(&root,"auml",228);
insert_string(&root,"aring",229);
insert_string(&root,"aelig",230);
insert_string(&root,"ccedil",231);
insert_string(&root,"egrave",232);
insert_string(&root,"eacute",233);
insert_string(&root,"ecirc",234);
insert_string(&root,"euml",235);
insert_string(&root,"igrave",236);
insert_string(&root,"iacute",237);
insert_string(&root,"icirc",238);
insert_string(&root,"iuml",239);
insert_string(&root,"eth",240);
insert_string(&root,"ntilde",241);
insert_string(&root,"ograve",242);
insert_string(&root,"oacute",243);
insert_string(&root,"ocirc",244);
insert_string(&root,"otilde",245);
insert_string(&root,"ouml",246);
insert_string(&root,"divide",247);
insert_string(&root,"oslash",248);
insert_string(&root,"ugrave",249);
insert_string(&root,"uacute",250);
insert_string(&root,"ucirc",251);
insert_string(&root,"uuml",252);
insert_string(&root,"yacute",253);
insert_string(&root,"thorn",254);
insert_string(&root,"yuml",255);
/* Latin Extended-A */
insert_string(&root,"OElig",338);
insert_string(&root,"oelig",339);
insert_string(&root,"Scaron",352);
insert_string(&root,"scaron",353);
insert_string(&root,"Yuml",376);
/* Latin Extended-B */
insert_string(&root,"fnof",402);
/* Greek */
insert_string(&root,"Alpha",913);
insert_string(&root,"Beta",914);
insert_string(&root,"Gamma",915);
insert_string(&root,"Delta",916);
insert_string(&root,"Epsilon",917);
insert_string(&root,"Zeta",918);
insert_string(&root,"Eta",919);
insert_string(&root,"Theta",920);
insert_string(&root,"Iota",921);
insert_string(&root,"Kappa",922);
insert_string(&root,"Lambda",923);
insert_string(&root,"Mu",924);
insert_string(&root,"Nu",925);
insert_string(&root,"Xi",926);
insert_string(&root,"Omicron",927);
insert_string(&root,"Pi",928);
insert_string(&root,"Rho",929);
insert_string(&root,"Sigma",931); /* There is no unicode character 930 */
insert_string(&root,"Tau",932);
insert_string(&root,"Upsilon",933);
insert_string(&root,"Phi",934);
insert_string(&root,"Chi",935);
insert_string(&root,"Psi",936);
insert_string(&root,"Omega",937);
insert_string(&root,"alpha",945);
insert_string(&root,"beta",946);
insert_string(&root,"gamma",947);
insert_string(&root,"delta",948);
insert_string(&root,"epsilon",949);
insert_string(&root,"zeta",950);
insert_string(&root,"eta",951);
insert_string(&root,"theta",952);
insert_string(&root,"iota",953);
insert_string(&root,"kappa",954);
insert_string(&root,"lambda",955);
insert_string(&root,"mu",956);
insert_string(&root,"nu",957);
insert_string(&root,"xi",958);
insert_string(&root,"omicron",959);
insert_string(&root,"pi",960);
insert_string(&root,"rho",961);
insert_string(&root,"sigmaf",962);
insert_string(&root,"sigma",963);
insert_string(&root,"tau",964);
insert_string(&root,"upsilon",965);
insert_string(&root,"phi",966);
insert_string(&root,"chi",967);
insert_string(&root,"psi",968);
insert_string(&root,"omega",969);
insert_string(&root,"thetasym",977);
insert_string(&root,"upsih",978);
insert_string(&root,"piv",982);
/* Letterlike Symbols */
insert_string(&root,"weierp",8472);
insert_string(&root,"image",8465);
insert_string(&root,"real",8476);
insert_string(&root,"trade",8482);
insert_string(&root,"alefsym",8501);
/* Arrows */
insert_string(&root,"larr",8592);
insert_string(&root,"uarr",8593);
insert_string(&root,"rarr",8594);
insert_string(&root,"darr",8595);
insert_string(&root,"harr",8596);
insert_string(&root,"crarr",8629);
insert_string(&root,"lArr",8656);
insert_string(&root,"uArr",8657);
insert_string(&root,"rArr",8658);
insert_string(&root,"dArr",8659);
insert_string(&root,"hArr",8660);
/* Mathematical Operators */
insert_string(&root,"forall",8704);
insert_string(&root,"part",8706);
insert_string(&root,"exist",8707);
insert_string(&root,"empty",8709);
insert_string(&root,"nabla",8711);
insert_string(&root,"isin",8712);
insert_string(&root,"notin",8713);
insert_string(&root,"ni",8715);
insert_string(&root,"prod",8719);
insert_string(&root,"sum",8721);
insert_string(&root,"minus",8722);
insert_string(&root,"lowast",8727);
insert_string(&root,"radic",8730);
insert_string(&root,"prop",8733);
insert_string(&root,"infin",8734);
insert_string(&root,"ang",8736);
insert_string(&root,"and",8743);
insert_string(&root,"or",8744);
insert_string(&root,"cap",8745);
insert_string(&root,"cup",8746);
insert_string(&root,"int",8747);
insert_string(&root,"there4",8756);
insert_string(&root,"sim",8764);
insert_string(&root,"cong",8773);
insert_string(&root,"asymp",8776);
insert_string(&root,"ne",8800);
insert_string(&root,"equiv",8801);
insert_string(&root,"le",8804);
insert_string(&root,"ge",8805);
insert_string(&root,"sub",8834);
insert_string(&root,"sup",8835);
insert_string(&root,"nsub",8836);
insert_string(&root,"sube",8838);
insert_string(&root,"supe",8839);
insert_string(&root,"oplus",8853);
insert_string(&root,"otimes",8855);
insert_string(&root,"perp",8869);
insert_string(&root,"sdot",8901);
/* Miscellaneous Technical */
insert_string(&root,"lceil",8968);
insert_string(&root,"rceil",8969);
insert_string(&root,"lfloor",8970);
insert_string(&root,"rfloor",8911);
insert_string(&root,"lang",9001);
insert_string(&root,"rang",9002);
/* Geometric Shapes */
insert_string(&root,"loz",9674);
/* Miscellaneous Symbols */
insert_string(&root,"spades",9824);
insert_string(&root,"clubs",9827);
insert_string(&root,"hearts",9829);
insert_string(&root,"diams",9830);
return root;
}


/**
 * This function takes a sequence representing an HTML normal character
 * name like "eacute" and returns its unicode number, or -1 if not found.
 */
int get_normal_character_number(char* sequence) {
int value;
if (get_string_number(normal_characters,sequence,&value)) return value;
return -1;
}


/**
 * This function takes a sequence representing an HTML control character
 * name like "gt" and returns its unicode number, or -1 if not found.
 */
int get_control_character_number(char* sequence) {
int value;
if (get_string_number(control_characters,sequence,&value)) return value;
return -1;
}


/**
 * This function takes a string that is supposed to contain an hexadecimal
 * integer value, and returns its value, or MALFORMED_HTML_CODE if
 * the string is not a valid hexadecimal integer.
 */
int analyse_hexadecimal_code(char* sequence) {
int value=0;
for (int i=0;sequence[i]!='\0';i++) {
	if (sequence[i]>='0' && sequence[i]<='9') {
		value=value*16+(sequence[i]-'0');
	} else if (sequence[i]>='a' && sequence[i]<='f') {
		value=value*16+(sequence[i]-'a'+10);
	} else if (sequence[i]>='A' && sequence[i]<='F') {
		value=value*16+(sequence[i]-'A'+10);
	} else return MALFORMED_HTML_CODE;
}
return value;
}


/**
 * This function takes a string that is supposed to contain a decimal
 * integer value, and returns its value, or MALFORMED_HTML_CODE if
 * the string is not a valid decimal integer.
 */
int analyse_decimal_code(char* sequence) {
int value=0;
for (int i=0;sequence[i]!='\0';i++) {
	if (sequence[i]>='0' && sequence[i]<='9') {
		value=value*10+(sequence[i]-'0');
	}
	else return MALFORMED_HTML_CODE;
}
return value;
}


/**
 * This function takes a sequence XXX obtained as being a part of an HTML
 * character declaration of the form &XXX; and returns the associated unicode
 * character if any, UNKNOWN_CHARACTER otherwise. XXX can be of the form:
 * - decimal integer: '#220'
 * - hexadecimal integer: '#xFF4'
 * - character name: 'eacute'
 * 
 * If the parameter 'decode_control_character' is set to 0, the function
 * will return DO_NOT_DECODE_CHARACTER if the given sequence corresponds
 * to an HTML control character like 'gt'.
 * 
 * The function raises a fatal error if XXX is NULL or an empty string. It
 * returns MALFORMED_HTML_CODE if the sequence is a malformed integer code
 * like "#x42W4"
 */
int get_HTML_character(char* sequence,int decode_control_character) {
if (sequence==NULL || sequence[0]=='\0') {
	fatal_error("Internal error in get_HTML_character\n");
}
int value;
if (sequence[0]!='#') {
	/* If we have a character name */
	value=get_normal_character_number(sequence);
	if (value!=-1) return value;
	/* If the character is not in the normal character set, we 
	 * look in the control character set. */
	value=get_control_character_number(sequence);
	if (value==-1) {
		/* If the character is not a control character, then we
		 * say that it is an unknown character. */
		 return UNKNOWN_CHARACTER;
	}
	/* If the character is a control one, we return its value
	 * only if we are allowed to decode it. */
	if (decode_control_character) {
		return value;
	}
	return DO_NOT_DECODE_CHARACTER;
}
if (sequence[1]=='x') {
	/* If we have an hexadecimal integer */
	return analyse_hexadecimal_code(&(sequence[2]));
}
if (sequence[1]>='0' && sequence[1]<='9') {
	/* If we have a decimal integer */
	return analyse_decimal_code(&(sequence[1]));
}
/* If the sequence is an invalid integer declaration like '#' or '#a', it is an error */
return MALFORMED_HTML_CODE;
}


/**
 * This function returns 0 if 'c' if not an HTML control character and
 * a non null value otherwise.
 */
int is_HTML_control_character(unichar c) {
return c=='<' || c=='>' || c=='&' || c=='\'';
}

