/*
 * Unitex
 *
 * Copyright (C) 2001-2013 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
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

#include "Korean.h"

#ifndef HAS_UNITEX_NAMESPACE
#define HAS_UNITEX_NAMESPACE 1
#endif

namespace unitex {

static int test_letter(unichar c,Alphabet* alphabet) {
if (alphabet==NULL) return 0;
return is_letter(c,alphabet);
}


/**
 * Converts a Chinese char by its Hangul equivalent, if any;
 * returns the char unmodified otherwise. 'map' is not supposed to be NULL;
 */
unichar Chinese_to_Hangul(unichar src,unichar* map) {
unichar c=map[src];
if (c!='\0') {
	return c;
}
return src;
}


/**
 * Converts a syllable text into a Jamo one, including Chinese -> Hangul conversion
 * and Jamo compatible -> Jamo conversion. If 'only_syllables' is non zero,
 * characters that are neither Chinese chars nor Hanguls are left untouched.
 * This is useful in MultiFlex, when a string may already have been turned into
 * Jamos. For all other use cases, this value should be 0.
 */
void Hanguls_to_Jamos(unichar* src,unichar* dest,Korean* korean,int only_syllables) {
if (korean==NULL) {
   /* No Korean data? A simple copy will do. */
	u_strcpy(dest,src);
   return;
}
int ret;
struct any* value=get_value(korean->table,src,HT_INSERT_IF_NEEDED,&ret);
if (ret==HT_KEY_ADDED) {
   /* If we had not already this tag, we insert it */
   korean->Hanguls_to_Jamos(src,dest,only_syllables);
   value->_ptr=u_strdup(dest);
   return;
}
u_strcpy(dest,(unichar*)(value->_ptr));
}



/**
 * Converts a syllable text into a Jamo one, including Chinese -> Hangul conversion
 * and Jamo compatible -> Jamo conversion. If 'only_syllables' is non zero,
 * characters that are neither Chinese chars nor Hanguls are left untouched.
 */
void Korean::Hanguls_to_Jamos(unichar* src,unichar* dest,int only_syllables) {
unichar temp[1024];
/* Then, we put a syllable bound before every character that is
 * 1) not a syllable one but in the alphabet (i.e. latin letters)
 * 2) a Jamo letter in order to avoid ambiguity when the syllable on its left
 *    is turned into Jamo letters */
int j=0;
unichar c;
unichar* map=alphabet->korean_equivalent_syllable;
for (int i=0;src[i]!='\0';i++) {
   c=Chinese_to_Hangul(src[i],map);
   if (!only_syllables && ((test_letter(c,alphabet) && !u_is_Hangul(c))
         || (u_is_Hangul_Compatility_Jamo(c) && c!=KR_SYLLABLE_BOUND)
         || u_is_Hangul_Jamo(c)
          /* This test is in comment because we want to distinguish the usages
          * of the two kinds of Jamos:
          *
          * - Hangul Jamos are used as letters to be combined to form Hangul
          * - Hangul Compatibility Jamos should only be used when someone wants
          *   to quote a Jamo in a text as a character. So, HCJ should never be used
          *   in grammars and dictionaries, unless we really want to match a Jamo as
          *   a whole character, not to be combined with other Jamos.
          *
          */)){
      temp[j++]=KR_SYLLABLE_BOUND;
   }
   temp[j++]=c;
}
temp[j]='\0';
if (j>1024) {
	fatal_error("Token too long in Korean::Hanguls_to_Jamos\n");
}
/* Then, we perform the syllable -> Jamo conversion */
Hanguls_to_Jamos_internal(temp,dest,only_syllables);
}


/**
 * Turns a Hangul compatibility jamo to a standard jamo sequence.
 * If the input is not a compatibility jamo letter, it is just copied to
 * the output string. Returns the length of the output string.
 */
int single_HGJ_to_Jamos(unichar c,unichar* dest,Korean* korean) {
if (u_is_Hangul_Compatility_Jamo(c) && korean!=NULL) {
	int pos=korean->single_HCJ_to_Jamos(c,dest,0);
	dest[pos]='\0';
	return pos;
} else {
	dest[0]=c;
	dest[1]='\0';
	return 1;
}
}


void kprintf(unichar* s) {
error("s=_%S_\n",s);
for (int i=0;s[i];i++) {
   error("%C (%X) ",s[i],s[i]);
}
error("\n");
}

/**
 * Turns a standard Jamo letter sequence into a Hangul syllable sequence.
 *
 * for debug only: returns 1 if old and new versions of Jamo->Hangul conversions
 *                 provide the same results; 0 otherwise.
 */
int convert_jamo_to_hangul(unichar* src,unichar* dest,Korean* korean) {
unichar t[1024];
/* We make a copy without empty initial consonants, if any */
int j=0;
for (int i=0;src[i]!='\0';i++) {
   if (src[i]==SJ_IC_IEUNG) {
      if (i>0 && src[i-1]==KR_SYLLABLE_BOUND) {
         if (src[i+1]==KR_SYLLABLE_BOUND) {
            t[j++]=src[i];
         }
      } else {
         t[j++]=src[i];
      }
   } else {
      t[j++]=src[i];
   }

   /*if (src[i]!=SJ_IC_IEUNG || (i>0 && src[i-1]!=KR_SYLLABLE_BOUND)) {
      t[j++]=src[i];
   }*/
}
t[j]='\0';
korean->Jamos_to_Hangul(src,dest);
return 1;
}


/**
 * Returns the number of Jamo letters contained in the given Hangul.
 * If the given character is neither a Hangul nor a Chinese character,
 * the function should return 1.
 */
int get_length_in_jamo(unichar hangul,Korean* korean) {
unichar t[2];
t[0]=hangul;
t[1]='\0';
unichar t2[16];
Hanguls_to_Jamos(t,t2,korean,0);
int pos=0;
if (t2[0]==KR_SYLLABLE_BOUND) {
   pos=1;
}
return u_strlen(t2+pos);
}



/**
 * A function to initialize arrays easily. The last value must be 0.
 */
static unichar* kdup(unichar first,...) {
unichar* result=new unichar[MAX_ORDER_JAMO_SIZE];
result[0]=first;
//error("[  %C  ] (%x) => ",first,first);
int d,n=1;
if (first!=0) {
   va_list args;
   va_start(args,first);
   while ((d=va_arg(args,int))!=0) {
      result[n++]=(unichar)d;
	  //if (n>2) error("[  %C  ] (%x)   ",d,d);
   }
   //error("\n");
   va_end(args);
}
while (n!=MAX_ORDER_JAMO_SIZE) {
   result[n++]=0;
}
return result;
}


/**
 * New simpler version by Sébastien Paumier.
 */
void Korean::initJamoMap() {
/* First, we initialize the initial consonants */
//error("----initial consonants----------\n");
jamo_table[ 0]=kdup(0x1100, 0x3131, 0x1100, 0);
jamo_table[ 1]=kdup(0x1101, 0x3132, 0x1100, 0x1100, 0);
jamo_table[ 2]=kdup(0x1102, 0x3134, 0x1102, 0);
jamo_table[ 3]=kdup(0x1103, 0x3137, 0x1103, 0);
jamo_table[ 4]=kdup(0x1104, 0x3138, 0x1103, 0x1103, 0);
jamo_table[ 5]=kdup(0x1105, 0x3139, 0x1105, 0);
jamo_table[ 6]=kdup(0x1106, 0x3141, 0x1106, 0);
jamo_table[ 7]=kdup(0x1107, 0x3142, 0x1107, 0);
jamo_table[ 8]=kdup(0x1108, 0x3143, 0x1107, 0x1107, 0);
jamo_table[ 9]=kdup(0x1109, 0x3145, 0x1109, 0);
jamo_table[10]=kdup(0x110A, 0x3146, 0x1109, 0x1109, 0);
jamo_table[11]=kdup(0x110B, 0x3147, 0x110B, 0);
jamo_table[12]=kdup(0x110C, 0x3148, 0x110C, 0);
jamo_table[13]=kdup(0x110D, 0x3149, 0x110C, 0x110C, 0);
/* SP commented the following transcription rules since they
 * corresponded to historical things */
jamo_table[14]=kdup(0x110E, 0x314A, 0x110E, 0/*, 0x1112, 0x110C, 0*/);
jamo_table[15]=kdup(0x110F, 0x314B, 0x110F, 0/*, 0x1112, 0x1100, 0*/);
jamo_table[16]=kdup(0x1110, 0x314C, 0x1110, 0/*, 0x1112, 0x1103, 0*/);
jamo_table[17]=kdup(0x1111, 0x314D, 0x1111, 0);
jamo_table[18]=kdup(0x1112, 0x314E, 0x1112, 0);
/* Then, we initialize the 21 vowels */
//error("----vowels----------\n");
jamo_table[19]=kdup(0x1161, 0x314F, 0x1161, 0);
jamo_table[20]=kdup(0x1162, 0x3150, 0x1161, 0x1175, 0);
jamo_table[21]=kdup(0x1163, 0x3151, 0x1175, 0x1161, 0);
jamo_table[22]=kdup(0x1164, 0x3152, 0x1175, 0x1161, 0x1175, 0);
jamo_table[23]=kdup(0x1165, 0x3153, 0x1165, 0);
jamo_table[24]=kdup(0x1166, 0x3154, 0x1165, 0x1175, 0);
jamo_table[25]=kdup(0x1167, 0x3155, 0x1175, 0x1165, 0);
jamo_table[26]=kdup(0x1168, 0x3156, 0x1175, 0x1165, 0x1175, 0);
jamo_table[27]=kdup(0x1169, 0x3157, 0x1169, 0);
jamo_table[28]=kdup(0x116A, 0x3158, 0x1169, 0x1161, 0);
jamo_table[29]=kdup(0x116B, 0x3159, 0x1169, 0x1175, 0x1165, 0);
jamo_table[30]=kdup(0x116C, 0x315A, 0x1169, 0x1175, 0);
jamo_table[31]=kdup(0x116D, 0x315B, 0x1175, 0x1169, 0);
jamo_table[32]=kdup(0x116E, 0x315C, 0x116E, 0);
jamo_table[33]=kdup(0x116F, 0x315D, 0x116E, 0x1165, 0);
jamo_table[34]=kdup(0x1170, 0x315E, 0x116E, 0x1165, 0x1175, 0);
jamo_table[35]=kdup(0x1171, 0x315F, 0x116E, 0x1175, 0);
jamo_table[36]=kdup(0x1172, 0x3160, 0x1175, 0x116E, 0);
jamo_table[37]=kdup(0x1173, 0x3161, 0x1173, 0);
jamo_table[38]=kdup(0x1174, 0x3162, 0x1173, 0x1175, 0);
jamo_table[39]=kdup(0x1175, 0x3163, 0x1175, 0);
/* Finally, we initialize the 28 final consonants */
//error("----final consonants----------\n");
jamo_table[40]=kdup(0); /* Special case of the empty final consonant */
jamo_table[41]=kdup(0x11A8, 0x3131, 0x1100, 0);
jamo_table[42]=kdup(0x11A9, 0x3132, 0x1100, 0x1100, 0);
jamo_table[43]=kdup(0x11AA, 0x3133, 0x1100, 0x1109, 0);
jamo_table[44]=kdup(0x11AB, 0x3134, 0x1102, 0);
jamo_table[45]=kdup(0x11AC, 0x3135, 0x1102, 0x110C, 0);
jamo_table[46]=kdup(0x11AD, 0x3136, 0x1102, 0x1112, 0);
jamo_table[47]=kdup(0x11AE, 0x3137, 0x1103, 0);
jamo_table[48]=kdup(0x11AF, 0x3139, 0x1105, 0);
jamo_table[49]=kdup(0x11B0, 0x313A, 0x1105, 0x1100, 0);
jamo_table[50]=kdup(0x11B1, 0x313B, 0x1105, 0x1106, 0);
jamo_table[51]=kdup(0x11B2, 0x313C, 0x1105, 0x1107, 0);
jamo_table[52]=kdup(0x11B3, 0x313D, 0x1105, 0x1109, 0);
jamo_table[53]=kdup(0x11B4, 0x313E, 0x1105, 0x1110, 0);
jamo_table[54]=kdup(0x11B5, 0x313F, 0x1105, 0x1111, 0);
jamo_table[55]=kdup(0x11B6, 0x3140, 0x1105, 0x1112, 0);
jamo_table[56]=kdup(0x11B7, 0x3141, 0x1106, 0);
jamo_table[57]=kdup(0x11B8, 0x3142, 0x1107, 0);
jamo_table[58]=kdup(0x11B9, 0x3144, 0x1107, 0x1109, 0);
jamo_table[59]=kdup(0x11BA, 0x3145, 0x1109, 0);
jamo_table[60]=kdup(0x11BB, 0x3146, 0x1109, 0x1109, 0);
jamo_table[61]=kdup(0x11BC, 0x3147, 0x110B, 0);
jamo_table[62]=kdup(0x11BD, 0x3148, 0x110C, 0);
jamo_table[63]=kdup(0x11BE, 0x314A, 0x110E, 0);
jamo_table[64]=kdup(0x11BF, 0x314B, 0x110F, 0);
jamo_table[65]=kdup(0x11C0, 0x314C, 0x1110, 0);
jamo_table[66]=kdup(0x11C1, 0x314D, 0x1111, 0);
jamo_table[67]=kdup(0x11C2, 0x314E, 0x1112, 0);
for (int i=0;i<JAMO_SIZE;i++) {
   if (jamo_table[i][1]!=0) {
      HCJ_to_SJ_table[jamo_table[i][1] - 0x3130] = &(jamo_table[i][2]);
   }
}
}


/**
 * Converts the given Hangul syllable into its Jamo string equivalent.
 * Returns the updated position in the output string.
 *
 * NOTE: this function inserts the syllable bound before the Jamos
 */
int Korean::single_Hangul_to_Jamos(unichar syl,unichar* output,int pos) {
if (syl<KR_HANGUL_SYL_START || syl>KR_HANGUL_SYL_END) {
   fatal_error("Invalid Hangul %X in single_Hangul_to_Jamos\n",syl);
}
output[pos++]=KR_SYLLABLE_BOUND;
unichar tmp=syl-KR_HANGUL_SYL_START;
int final_consonant=tmp%N_FINAL_CONSONANTS;
tmp=tmp/N_FINAL_CONSONANTS;
int vowel=tmp%N_VOWELS;
int initial_consonant=tmp/N_VOWELS;
unichar* ptr=&jamo_table[initial_consonant][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
ptr=&jamo_table[vowel+INDEX_FIRST_VOWEL][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
ptr=&jamo_table[final_consonant+INDEX_FIRST_FINAL_CONSONANT][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
return pos;
}


/**
 * Converts the given Hangul Compatibility Jamo into its Jamo string equivalent.
 * Returns the updated position in the output string.
 *
 * NOTE: this function DOES NOT insert the syllable bound before the Jamos
 */
int Korean::single_HCJ_to_Jamos(unichar jamo,unichar* output,int pos) {
if (jamo<KR_HCJ_START || jamo>KR_HCJ_END) {
   fatal_error("Invalid Hangul Compatibility Jamo %X in single_HCJ_to_Jamos\n",jamo);
}
unichar* ptr=HCJ_to_SJ_table[jamo-KR_HCJ_START];
if (ptr!=NULL) {
   while (*ptr!='\0') {
      output[pos++]=*ptr++;
   }
} else {
   output[pos++]=jamo;
}
return pos;
}


/**
 * Converts a Hangul string into its Jamo equivalent. Returns the length of the output.
 * Note that it [ignores backslashes and that it] converts Hangul Compatibility Jamos
 * into standard Jamos, but only if 'only_syllables' is set to zero.
 */
int Korean::Hanguls_to_Jamos_internal(unichar* input,unichar* output,int only_syllables) {
int n=0;
unichar c;
for (int i=0;input[i]!='\0';i++) {
   c=Chinese_to_Hangul(input[i],alphabet->korean_equivalent_syllable);
   /*if (c=='\\') {
      i++;
      continue;
   }*/
   if (c>=KR_HANGUL_SYL_START && c<=KR_HANGUL_SYL_END) {
	   n=single_Hangul_to_Jamos(c,output,n);
   }
   else if (!only_syllables && c>=KR_HCJ_START && c<=KR_HCJ_END) {
      n=single_HCJ_to_Jamos(c,output,n);
   }
   else {
      output[n++]=c;
   }
}
output[n]='\0';
/*error("zzz depuis <%S> vers <",input);
for (int i=0;output[i]!='\0';i++) {
	error("%X ",output[i]);
}
error(">\n");*/
return n;
}


#define KR_END_OF_STRING -1
#define KR_END_OF_SYLLABLE -2
#define KR_JAMO_ERROR -3
#define KR_NO_MATCH -4
#define KR_MATCH -5


/**
 * Tries to read a sequence of initial Jamo consonants. In case of success, it
 * returns KR_MATCH and stores in '*initial_consonant' a value that will be used
 * to obtain the Hangul syllable code when combined with information about
 * vowel and final consonants. If the end of string or the syllable bound is found,
 * the result is converted into a single HCJ and stored into '*hcj'
 * and KR_SYLLABLE is returned. If no initial
 * consonant is found, it returns KR_NO_MATCH.
 *
 * NOTE: this function considers that the input should be made of
 *       standard Jamo initial consonants.
 */
int read_initial_consonants(unichar* input,int *pos_input,int *initial_consonant,unichar *hcj) {
switch(input[*pos_input]) {
/*case HCJ_KIYEOK:*/
case SJ_IC_KIYEOK: {
	*initial_consonant=0;
	*hcj=HCJ_KIYEOK;
	if (/*input[(*pos_input)+1]==HCJ_KIYEOK ||*/ input[(*pos_input)+1]==SJ_IC_KIYEOK) {
		*initial_consonant=1;
		*hcj=HCJ_SSANGKIYEOK;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGKIYEOK:*/
case SJ_IC_SSANGKIYEOK: {
	*initial_consonant=1;
	*hcj=HCJ_SSANGKIYEOK;
	break;
}
/*case HCJ_NIEUN:*/
case SJ_IC_NIEUN: {
	*initial_consonant=2;
	*hcj=HCJ_NIEUN;
	break;
}
/*case HCJ_TIKEUT:*/
case SJ_IC_TIKEUT: {
	*initial_consonant=3;
	*hcj=HCJ_TIKEUT;
	if (/*input[(*pos_input)+1]==HCJ_TIKEUT ||*/ input[(*pos_input)+1]==SJ_IC_TIKEUT) {
		*initial_consonant=4;
		*hcj=HCJ_SSANGTIKEUT;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGTIKEUT:*/
case SJ_IC_SSANGTIKEUT: {
	*initial_consonant=4;
	*hcj=HCJ_SSANGTIKEUT;
	break;
}
/*case HCJ_RIEUL:*/
case SJ_IC_RIEUL: {
	*initial_consonant=5;
	*hcj=HCJ_RIEUL;
	break;
}
/*case HCJ_MIEUM:*/
case SJ_IC_MIEUM: {
	*initial_consonant=6;
	*hcj=HCJ_MIEUM;
	break;
}
/*case HCJ_PIEUP:*/
case SJ_IC_PIEUP: {
	*initial_consonant=7;
	*hcj=HCJ_PIEUP;
	if (/*input[(*pos_input)+1]==HCJ_PIEUP ||*/ input[(*pos_input)+1]==SJ_IC_PIEUP) {
		*initial_consonant=8;
		*hcj=HCJ_SSANGPIEUP;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGPIEUP:*/
case SJ_IC_SSANGPIEUP: {
	*initial_consonant=8;
	*hcj=HCJ_SSANGPIEUP;
	break;
}
/*case HCJ_SIOS:*/
case SJ_IC_SIOS: {
	*initial_consonant=9;
	*hcj=HCJ_SIOS;
	if (/*input[(*pos_input)+1]==HCJ_SIOS ||*/ input[(*pos_input)+1]==SJ_IC_SIOS) {
		*initial_consonant=10;
		*hcj=HCJ_SSANGSIOS;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGSIOS:*/
case SJ_IC_SSANGSIOS: {
	*initial_consonant=10;
	*hcj=HCJ_SSANGSIOS;
	break;
}
/*case HCJ_IEUNG:*/
case SJ_IC_IEUNG: {
	*initial_consonant=11;
	*hcj=HCJ_IEUNG;
	break;
}
/*case HCJ_CIEUC:*/
case SJ_IC_CIEUC: {
	*initial_consonant=12;
	*hcj=HCJ_CIEUC;
	if (/*input[(*pos_input)+1]==HCJ_CIEUC ||*/ input[(*pos_input)+1]==SJ_IC_CIEUC) {
		*initial_consonant=13;
		*hcj=HCJ_SSANGCIEUC;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGCIEUC:*/
case SJ_IC_SSANGCIEUC: {
	*initial_consonant=13;
	*hcj=HCJ_SSANGCIEUC;
	break;
}
/*case HCJ_CHIEUCH:*/
case SJ_IC_CHIEUCH: {
	*initial_consonant=14;
	*hcj=HCJ_CHIEUCH;
	break;
}
/*case HCJ_KHIEUKH:*/
case SJ_IC_KHIEUKH: {
	*initial_consonant=15;
	*hcj=HCJ_KHIEUKH;
	break;
}
/*case HCJ_THIEUTH:*/
case SJ_IC_THIEUTH: {
	*initial_consonant=16;
	*hcj=HCJ_THIEUTH;
	break;
}
/*case HCJ_PHIEUPH:*/
case SJ_IC_PHIEUPH: {
	*initial_consonant=17;
	*hcj=HCJ_PHIEUPH;
	break;
}
/*case HCJ_HIEUH:*/
case SJ_IC_HIEUH: {
	*initial_consonant=18;
	*hcj=HCJ_HIEUH;
	if (/*input[(*pos_input)+1]==HCJ_KIYEOK ||*/ input[(*pos_input)+1]==SJ_IC_KIYEOK) {
	   *initial_consonant=15;
	   *hcj=HCJ_KHIEUKH;
	   (*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_CIEUC ||*/ input[(*pos_input)+1]==SJ_IC_CIEUC) {
      *initial_consonant=14;
      *hcj=HCJ_CHIEUCH;
      (*pos_input)++;
   } else if (/*input[(*pos_input)+1]==HCJ_TIKEUT ||*/ input[(*pos_input)+1]==SJ_IC_TIKEUT) {
      *initial_consonant=16;
      *hcj=HCJ_THIEUTH;
      (*pos_input)++;
   } else if (/*input[(*pos_input)+1]==HCJ_PIEUP ||*/ input[(*pos_input)+1]==SJ_IC_PIEUP) {
      *initial_consonant=17;
      *hcj=HCJ_PHIEUPH;
      (*pos_input)++;
   }
	break;
}
default: {
	return KR_NO_MATCH;
}
} /* End of switch */
/* Now we check whether we are at the end of the  or at the end of the string */
(*pos_input)++;
if (input[*pos_input]=='\0') {
	return KR_END_OF_STRING;
}
if (input[*pos_input]==KR_SYLLABLE_BOUND) {
	return KR_END_OF_SYLLABLE;
}
/* If we have something else, we have a normal match */
return KR_MATCH;
}


/**
 * Tries to read a sequence of Jamo vowels. In case of success, it
 * returns KR_MATCH and stores in '*vowel' a value that will be used
 * to obtain the Hangul code when combined with information about
 * initial and final consonants. The result is also converted to a HCJ
 * and stored in '*hcj'. If no vowel is found, it returns KR_JAMO_ERROR.
 *
 * NOTE: this function considers that the input should be made of
 *       standard Jamo vowels.
 */
int read_vowels(unichar* input,int *pos_input,int *vowel,unichar *hcj) {
switch(input[*pos_input]) {
/*case HCJ_A:*/
case SJ_A: {
	*vowel=0;
	*hcj=HCJ_A;
	if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
		*vowel=1;
		*hcj=HCJ_AE;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_AE:*/
case SJ_AE: {
	*vowel=1;
	*hcj=HCJ_AE;
	break;
}
/*case HCJ_EO:*/
case SJ_EO: {
	*vowel=4;
	*hcj=HCJ_EO;
	if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
		*vowel=5;
		*hcj=HCJ_E;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_E:*/
case SJ_E: {
	*vowel=5;
	*hcj=HCJ_E;
	break;
}
/*case HCJ_O:*/
case SJ_O: {
	*vowel=8;
	*hcj=HCJ_O;
	if (/*input[(*pos_input)+1]==HCJ_A ||*/ input[(*pos_input)+1]==SJ_A) {
		*vowel=9;
		*hcj=HCJ_WA;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
		*vowel=11;
		*hcj=HCJ_OE;
		(*pos_input)++;
		if (/*input[(*pos_input)+1]==HCJ_EO ||*/ input[(*pos_input)+1]==SJ_EO) {
			*vowel=10;
			*hcj=HCJ_WAE;
			(*pos_input)++;
		}
	}
	break;
}
/*case HCJ_WA:*/
case SJ_WA: {
	*vowel=9;
	*hcj=HCJ_WA;
	break;
}
/*case HCJ_WAE:*/
case SJ_WAE: {
	*vowel=10;
	*hcj=HCJ_WAE;
	break;
}
/*case HCJ_OE:*/
case SJ_OE: {
	*vowel=11;
	*hcj=HCJ_OE;
	break;
}
/*case HCJ_U:*/
case SJ_U: {
	*vowel=13;
	*hcj=HCJ_U;
	if (/*input[(*pos_input)+1]==HCJ_EO ||*/ input[(*pos_input)+1]==SJ_EO) {
		*vowel=14;
		*hcj=HCJ_WEO;
		(*pos_input)++;
		if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
			*vowel=15;
			*hcj=HCJ_WE;
			(*pos_input)++;
		}
	} else if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
		*vowel=16;
		*hcj=HCJ_WI;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_WEO:*/
case SJ_WEO: {
	*vowel=14;
	*hcj=HCJ_WEO;
	break;
}
/*case HCJ_WE:*/
case SJ_WE: {
	*vowel=15;
	*hcj=HCJ_WE;
	break;
}
/*case HCJ_WI:*/
case SJ_WI: {
	*vowel=16;
	*hcj=HCJ_WI;
	break;
}
/*case HCJ_EU:*/
case SJ_EU: {
	*vowel=18;
	*hcj=HCJ_EU;
	if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
		*vowel=19;
		*hcj=HCJ_YI;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_YI:*/
case SJ_YI: {
	*vowel=19;
	*hcj=HCJ_YI;
	break;
}
/*case HCJ_I:*/
case SJ_I: {
	*vowel=20;
	*hcj=HCJ_I;
	if (/*input[(*pos_input)+1]==HCJ_A ||*/ input[(*pos_input)+1]==SJ_A) {
		*vowel=2;
		*hcj=HCJ_YA;
		(*pos_input)++;
		if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
			*vowel=3;
			*hcj=HCJ_YAE;
			(*pos_input)++;
		}
	} else if (/*input[(*pos_input)+1]==HCJ_EO ||*/ input[(*pos_input)+1]==SJ_EO) {
		*vowel=6;
		*hcj=HCJ_YEO;
		(*pos_input)++;
		if (/*input[(*pos_input)+1]==HCJ_I ||*/ input[(*pos_input)+1]==SJ_I) {
			*vowel=7;
			*hcj=HCJ_YE;
			(*pos_input)++;
		}
	} else if (/*input[(*pos_input)+1]==HCJ_O ||*/ input[(*pos_input)+1]==SJ_O) {
		*vowel=12;
		*hcj=HCJ_YO;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_U ||*/ input[(*pos_input)+1]==SJ_U) {
		*vowel=17;
		*hcj=HCJ_YU;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_YA:*/
case SJ_YA: {
	*vowel=2;
	*hcj=HCJ_YA;
	break;
}
/*case HCJ_YAE:*/
case SJ_YAE: {
	*vowel=3;
	*hcj=HCJ_YAE;
	break;
}
/*case HCJ_YEO:*/
case SJ_YEO: {
	*vowel=6;
	*hcj=HCJ_YEO;
	break;
}
/*case HCJ_YE:*/
case SJ_YE: {
	*vowel=7;
	*hcj=HCJ_YE;
	break;
}
/*case HCJ_YO:*/
case SJ_YO: {
	*vowel=12;
	*hcj=HCJ_YO;
	break;
}
/*case HCJ_YU:*/
case SJ_YU: {
	*vowel=17;
	*hcj=HCJ_YU;
	break;
}
default: {
	return KR_JAMO_ERROR;
}
} /* End of switch */
/* Now we check whether we are at the end of the syllable or at the end of the string */
(*pos_input)++;
if (input[*pos_input]=='\0') {
	return KR_END_OF_STRING;
}
if (input[*pos_input]==KR_SYLLABLE_BOUND) {
	return KR_END_OF_SYLLABLE;
}
/* If we have something else, we have a normal match */
return KR_MATCH;
}


/**
 * Tries to read a sequence of Jamo final consonants. In case of success, it
 * returns KR_MATCH and stores in '*final_consonants' a value that will be used
 * to obtain the Hangul code when combined with information about
 * vowel and initial consonants. The result is also converted to a HCJ
 * and stored in '*hcj'. If no final consonant is found, it returns KR_NO_MATCH.
 *
 * If 'all_sequences' is non zero, then the function tries to match final
 * sequences that have a HCJ equivalent but that should not appear in a
 * normal syllable's Jamo represention, such as HCJ_SSANGPIEUP.
 *
 * NOTE: this function considers that the input should be made of
 *       standard Jamo initial consonants, but it also tolerates
 *       standard Jamo final consonants, even if those are not
 *       produced by the library's Hangul->Jamo function
 */
int read_final_consonants(unichar* input,int *pos_input,int *final_consonant, unichar *hcj,
		                  int all_sequences) {
switch(input[*pos_input]) {
/*case HCJ_KIYEOK:*/
case SJ_FC_KIYEOK:
case SJ_IC_KIYEOK: {
	*final_consonant=1;
	*hcj=HCJ_KIYEOK;
	if (/*input[(*pos_input)+1]==HCJ_KIYEOK ||*/ input[(*pos_input)+1]==SJ_FC_KIYEOK ||
		input[(*pos_input)+1]==SJ_IC_KIYEOK) {
		*final_consonant=2;
		*hcj=HCJ_SSANGKIYEOK;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_SIOS ||*/ input[(*pos_input)+1]==SJ_FC_SIOS ||
			input[(*pos_input)+1]==SJ_IC_SIOS) {
		*final_consonant=3;
		*hcj=HCJ_KIYEOK_SIOS;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGKIYEOK:*/
case SJ_FC_SSANGKIYEOK:
case SJ_IC_SSANGKIYEOK: {
	*final_consonant=2;
	*hcj=HCJ_SSANGKIYEOK;
	break;
}
/*case HCJ_KIYEOK_SIOS:*/
case SJ_FC_KIYEOK_SIOS: {
	*final_consonant=3;
	*hcj=HCJ_KIYEOK_SIOS;
	break;
}
/*case HCJ_NIEUN:*/
case SJ_FC_NIEUN:
case SJ_IC_NIEUN: {
	*final_consonant=4;
	if (/*input[(*pos_input)+1]==HCJ_CIEUC ||*/ input[(*pos_input)+1]==SJ_FC_CIEUC ||
		input[(*pos_input)+1]==SJ_IC_CIEUC) {
		*final_consonant=5;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_HIEUH ||*/ input[(*pos_input)+1]==SJ_FC_HIEUH ||
		input[(*pos_input)+1]==SJ_IC_HIEUH) {
		*final_consonant=6;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_NIEUN_CIEUC:*/
case SJ_FC_NIEUN_CIEUC: {
	*final_consonant=5;
	*hcj=HCJ_NIEUN_CIEUC;
	break;
}
/*case HCJ_NIEUN_HIEUH:*/
case SJ_FC_NIEUN_HIEUH: {
	*final_consonant=6;
	*hcj=HCJ_NIEUN_HIEUH;
	break;
}
/*case HCJ_TIKEUT:*/
case SJ_FC_TIKEUT:
case SJ_IC_TIKEUT: {
	*final_consonant=7;
	*hcj=HCJ_TIKEUT;
	if (all_sequences && /*input[(*pos_input)+1]==HCJ_TIKEUT ||*/ (input[(*pos_input)+1]==SJ_FC_TIKEUT ||
		input[(*pos_input)+1]==SJ_IC_TIKEUT)) {
		(*pos_input)++;
		*hcj=HCJ_SSANGTIKEUT;
	}
	break;
}
/*case HCJ_SSANGTIKEUT:*/
case SJ_IC_SSANGTIKEUT: {
	if (all_sequences) {
		*hcj=HCJ_SSANGTIKEUT;
	}
	break;
}
/*case HCJ_RIEUL:*/
case SJ_FC_RIEUL:
case SJ_IC_RIEUL:{
	*final_consonant=8;
	*hcj=HCJ_RIEUL;
	if (/*input[(*pos_input)+1]==HCJ_KIYEOK ||*/ input[(*pos_input)+1]==SJ_FC_KIYEOK ||
		input[(*pos_input)+1]==SJ_IC_KIYEOK) {
		*final_consonant=9;
		*hcj=HCJ_RIEUL_KIYEOK;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_MIEUM ||*/ input[(*pos_input)+1]==SJ_FC_MIEUM ||
		input[(*pos_input)+1]==SJ_IC_MIEUM) {
		*final_consonant=10;
		*hcj=HCJ_RIEUL_MIEUM;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_PIEUP ||*/ input[(*pos_input)+1]==SJ_FC_PIEUP ||
		input[(*pos_input)+1]==SJ_IC_PIEUP) {
		*final_consonant=11;
		*hcj=HCJ_RIEUL_PIEUP;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_SIOS ||*/ input[(*pos_input)+1]==SJ_FC_SIOS ||
		input[(*pos_input)+1]==SJ_IC_SIOS) {
		*final_consonant=12;
		*hcj=HCJ_RIEUL_SIOS;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_THIEUTH ||*/ input[(*pos_input)+1]==SJ_FC_THIEUTH ||
		input[(*pos_input)+1]==SJ_IC_THIEUTH) {
		*final_consonant=13;
		*hcj=HCJ_RIEUL_THIEUTH;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_PHIEUPH ||*/ input[(*pos_input)+1]==SJ_FC_PHIEUPH ||
		input[(*pos_input)+1]==SJ_IC_PHIEUPH) {
		*final_consonant=14;
		*hcj=HCJ_RIEUL_PHIEUPH;
		(*pos_input)++;
	} else if (/*input[(*pos_input)+1]==HCJ_HIEUH ||*/ input[(*pos_input)+1]==SJ_FC_HIEUH ||
		input[(*pos_input)+1]==SJ_IC_HIEUH) {
		*final_consonant=15;
		*hcj=HCJ_RIEUL_HIEUH;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_RIEUL_KIYEOK:*/
case SJ_FC_RIEUL_KIYEOK: {
	*final_consonant=9;
	*hcj=HCJ_RIEUL_KIYEOK;
	break;
}
/*case HCJ_RIEUL_MIEUM:*/
case SJ_FC_RIEUL_MIEUM: {
	*final_consonant=10;
	*hcj=HCJ_RIEUL_MIEUM;
	break;
}
/*case HCJ_RIEUL_PIEUP:*/
case SJ_FC_RIEUL_PIEUP: {
	*final_consonant=11;
	*hcj=HCJ_RIEUL_PIEUP;
	break;
}
/*case HCJ_RIEUL_SIOS:*/
case SJ_FC_RIEUL_SIOS: {
	*final_consonant=12;
	*hcj=HCJ_RIEUL_SIOS;
	break;
}
/*case HCJ_RIEUL_THIEUTH:*/
case SJ_FC_RIEUL_THIEUTH: {
	*final_consonant=13;
	*hcj=HCJ_RIEUL_THIEUTH;
	break;
}
/*case HCJ_RIEUL_PHIEUPH:*/
case SJ_FC_RIEUL_PHIEUPH: {
	*final_consonant=14;
	*hcj=HCJ_RIEUL_PHIEUPH;
	break;
}
/*case HCJ_RIEUL_HIEUH:*/
case SJ_FC_RIEUL_HIEUH: {
	*final_consonant=15;
	*hcj=HCJ_RIEUL_HIEUH;
	break;
}
/*case HCJ_MIEUM:*/
case SJ_FC_MIEUM:
case SJ_IC_MIEUM: {
	*final_consonant=16;
	*hcj=HCJ_MIEUM;
	break;
}
/*case HCJ_PIEUP:*/
case SJ_FC_PIEUP:
case SJ_IC_PIEUP: {
	*final_consonant=17;
	*hcj=HCJ_PIEUP;
	if (/*input[(*pos_input)+1]==HCJ_SIOS ||*/ input[(*pos_input)+1]==SJ_FC_SIOS ||
		input[(*pos_input)+1]==SJ_IC_SIOS) {
		*final_consonant=18;
		*hcj=HCJ_PIEUP_SIOS;
		(*pos_input)++;
	} else if (all_sequences && /*input[(*pos_input)+1]==HCJ_PIEUP ||*/ (input[(*pos_input)+1]==SJ_FC_PIEUP ||
		input[(*pos_input)+1]==SJ_IC_PIEUP)) {
		*hcj=HCJ_SSANGPIEUP;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_PIEUP_SIOS:*/
case SJ_FC_PIEUP_SIOS: {
	*final_consonant=18;
	*hcj=HCJ_PIEUP_SIOS;
	break;
}
/*case HCJ_SIOS:*/
case SJ_FC_SIOS:
case SJ_IC_SIOS: {
	*final_consonant=19;
	*hcj=HCJ_SIOS;
	if (/*input[(*pos_input)+1]==HCJ_SIOS ||*/ input[(*pos_input)+1]==SJ_FC_SIOS ||
		input[(*pos_input)+1]==SJ_IC_SIOS) {
		*final_consonant=20;
		*hcj=HCJ_SSANGSIOS;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGSIOS:*/
case SJ_FC_SSANGSIOS:
case SJ_IC_SSANGSIOS: {
	*final_consonant=20;
	*hcj=HCJ_SSANGSIOS;
	break;
}
/*case HCJ_IEUNG:*/
case SJ_FC_IEUNG:
case SJ_IC_IEUNG: {
	*final_consonant=21;
	*hcj=HCJ_IEUNG;
	break;
}
/*case HCJ_CIEUC:*/
case SJ_FC_CIEUC:
case SJ_IC_CIEUC: {
	*final_consonant=22;
	*hcj=HCJ_CIEUC;
	if (all_sequences && /*input[(*pos_input)+1]==HCJ_CIEUC ||*/ (input[(*pos_input)+1]==SJ_FC_CIEUC ||
		input[(*pos_input)+1]==SJ_IC_CIEUC)) {
		*hcj=HCJ_SSANGCIEUC;
		(*pos_input)++;
	}
	break;
}
/*case HCJ_SSANGCIEUC:*/
case SJ_IC_SSANGCIEUC: {
	if (all_sequences) {
		*hcj=HCJ_SSANGCIEUC;
	}
	break;
}
/*case HCJ_CHIEUCH:*/
case SJ_FC_CHIEUCH:
case SJ_IC_CHIEUCH: {
	*final_consonant=23;
	*hcj=HCJ_CHIEUCH;
	break;
}
/*case HCJ_KHIEUKH:*/
case SJ_FC_KHIEUKH:
case SJ_IC_KHIEUKH: {
	*final_consonant=24;
	*hcj=HCJ_KHIEUKH;
	break;
}
/*case HCJ_THIEUTH:*/
case SJ_FC_THIEUTH:
case SJ_IC_THIEUTH: {
	*final_consonant=25;
	*hcj=HCJ_THIEUTH;
	break;
}
/*case HCJ_PHIEUPH:*/
case SJ_FC_PHIEUPH:
case SJ_IC_PHIEUPH: {
	*final_consonant=26;
	*hcj=HCJ_PHIEUPH;
	break;
}
/*case HCJ_HIEUH:*/
case SJ_FC_HIEUH:
case SJ_IC_HIEUH: {
	*final_consonant=27;
	*hcj=HCJ_HIEUH;
	break;
}
default: {
	return KR_NO_MATCH;
}
} /* End of switch */
/* Now we check whether we are at the end of the syllable or at the end of the string */
(*pos_input)++;
if (input[*pos_input]=='\0') {
	return KR_END_OF_STRING;
}
if (input[*pos_input]==KR_SYLLABLE_BOUND) {
	return KR_END_OF_SYLLABLE;
}
/* If we have something else, we have a normal match. This may indicate that, after
 * the Jamo sequence, we have a non Korean character */
return KR_MATCH;
}


/**
 * Tries to read a Jamo sequence that is not part of a syllable. In case
 * of success, returns 1 and stores the corresponding HCJ in '*hcj';
 * returns 0 otherwise.
 */
int read_jamos(unichar* input,int *pos_input,unichar *hcj) {
int foo;
/* We try to read a vowel sequence */
int ret=read_vowels(input,pos_input,&foo,hcj);
if (ret!=KR_JAMO_ERROR) {
	/* If a vowel sequence was found, we won */
	return 1;
}
/* If not, we try to read a consonant sequence */
ret=read_final_consonants(input,pos_input,&foo,hcj,1);
if (ret!=KR_NO_MATCH) {
	/* If a consonant sequence was found, we won */
	return 1;
}
return 0;
}


/**
 * Tries to decode one Hangul from the given Jamo input. Returns KR_END_OF_SYLLAB in case of
 * success, KR_END_OF_STRING if the end of the input has been reached, or
 * KR_JAMO_ERROR if the string is not a valid Jamo sequence. If the input contains
 * Jamo that do not belong to a syllable, they are converted to HCJ. If the input
 * is neither a valid Jamo syllable nor a HCJ, its current char is copied as is to
 * the output.
 */
int decode_one_Jamo_syllab(unichar* input,unichar* output,int *pos_input,int *pos_output) {
if (input[*pos_input]=='\0') {
	output[*pos_output]='\0';
	return KR_END_OF_STRING;
}
int old_pos_input=*pos_input;
int old_pos_output=*pos_output;
int result,ret,initial_consonant,vowel,final_consonant;
unichar hcj;
if (input[*pos_input]==KR_SYLLABLE_BOUND) {
	result=0;
	initial_consonant=11;
	vowel=0;
	final_consonant=0;
	(*pos_input)++;
	ret=read_initial_consonants(input,pos_input,&initial_consonant,&hcj);
	if (ret==KR_END_OF_SYLLABLE || ret==KR_END_OF_STRING) {
		/* If we have matched and finished, we return */
		output[(*pos_output)++]=hcj;
		return ret;
	}
	ret=read_vowels(input,pos_input,&vowel,&hcj);
	if (ret==KR_JAMO_ERROR) {
		/* If we can't read a vowel, then it means that the input Jamo syllable is
		 * not correct. In such a case, we skip the syllable bound, so that
		 * the next call to this function will read the input char by char */
		*pos_input=old_pos_input+1;
		*pos_output=old_pos_output;
		return KR_END_OF_SYLLABLE;
	}
	if (ret!=KR_END_OF_SYLLABLE && ret != KR_END_OF_STRING) {
		/* If we are not at the end of the sequence, we try to read
		 * a final consonant sequence */
		ret=read_final_consonants(input,pos_input,&final_consonant,&hcj,0);
	}
	int syllable=(initial_consonant*21+vowel)*28+final_consonant+0xAC00;
	output[(*pos_output)++]=(unichar)syllable;
	return KR_END_OF_SYLLABLE;
}
/* We try to read a Jamo sequence that is not part of a syllable */
if (read_jamos(input,pos_input,&hcj)) {
	output[(*pos_output)++]=hcj;
	if (input[*pos_input]=='\0') {
		return KR_END_OF_STRING;
	}
	/* We return KR_END_OF_SYLLABLE either if there is actually one
	 * or if there is something else after the Jamo sequence in the input, like a
	 * latin character */
	return KR_END_OF_SYLLABLE;
}
/* If we have a non Jamo character, we just output it */
output[(*pos_output)++]=input[(*pos_input)++];
return KR_END_OF_SYLLABLE;
}


/**
 * Converts a Jamo string to a Hangul one. Returns the length of the output or
 * KR_JAMO_ERROR in case of error.
 */
int Korean::Jamos_to_Hangul(unichar* input,unichar* output) {
int pos_input=0;
int pos_output=0;
int res;
while ((res=decode_one_Jamo_syllab(input,output,&pos_input,&pos_output))==KR_END_OF_SYLLABLE) {}
output[pos_output]='\0';
if (res==KR_JAMO_ERROR) {
	return KR_JAMO_ERROR;
}
return pos_output;
}

} // namespace unitex


