 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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

static const unichar first_letter_tab[]={0x3131,0x3132,0x3134,0x3137,0x3138,
                            0x3139,0x3141,0x3142,0x3143,0x3145,
                            0x3146,0x3147,0x3148,0x3149,0x314A,
                            0x314B,0x314C,0x314D,0x314E};

static const unichar second_letter_tab_A[]={0x314F,0x3150,0x3151,0x3152,0x3153,
                             0x3154,0x3155,0x3156};

static const unichar second_letter_tab_B[]={0x3157,0x315B,0x315C,0x3160,0x3161};

static const unichar second_letter_tab_C[]={0x3163};

static unichar last_part_A[28][3];

static unichar last_part_B[4][2];

static unichar last_part_C[2][2];



/**
 * Converts a Korean syllab S into a letter sequence stored in res. Returns 0
 * if S is not a Korean syllab, 1 otherwise.
 *
 * Here we work with the Hangul Compatible Jamo alphabet.
 */
int syllabToLetters_HCJ(unichar S,unichar* res) {
if (S<0xAC00 || S>0xD7A3) return 0;
res[0]=first_letter_tab[(S-0xAC00)/0x24C];
int pos=(S-0xAC00)%0x24C;

if (pos<=0xDF) {
   // case of vertical vowels
   res[1]=second_letter_tab_A[pos/0x1C];
   pos=pos%0x1C;
   res[2]='\0';
   u_strcat(res,last_part_A[pos]);
   return 1;
}
else {
   res[2]='\0';
   pos=pos-0xE0;
   if (pos<=0x6F) {res[1]=second_letter_tab_B[0]; u_strcat(res,last_part_B[pos/0x1C]); u_strcat(res,last_part_A[pos%0x1C]); return 1;}
   else if (pos<=0x8B) {res[1]=second_letter_tab_B[1]; u_strcat(res,last_part_A[pos%0x1C]); return 1;}
   else if (pos<=0xFB) {res[1]=second_letter_tab_B[2]; u_strcat(res,last_part_B[(pos-0x8C)/0x1C]); u_strcat(res,last_part_A[pos%0x1C]); return 1;}
   else if (pos<=0x117) {res[1]=second_letter_tab_B[3]; u_strcat(res,last_part_A[pos%0x1C]); return 1;}
   else if (pos<=0x14F) {res[1]=second_letter_tab_B[4]; u_strcat(res,last_part_C[(pos-0x118)/0x1C]); u_strcat(res,last_part_A[pos%0x1C]); return 1;}
   else {res[1]=second_letter_tab_C[0]; u_strcat(res,last_part_A[pos%0x1C]); return 1;}
}
}


/**
 * Initializes last_part_A array
 */
void initializes_last_part_A() {
last_part_A[0][0]='\0';
last_part_A[1][0]=0x3131;   last_part_A[1][1]='\0';
last_part_A[2][0]=0x3132;   last_part_A[2][1]='\0';
last_part_A[3][0]=0x3131;   last_part_A[3][1]=0x3145;   last_part_A[3][2]='\0';
last_part_A[4][0]=0x3134;   last_part_A[4][1]='\0';
last_part_A[5][0]=0x3134;   last_part_A[5][1]=0x3148;   last_part_A[5][2]='\0';
last_part_A[6][0]=0x3134;   last_part_A[6][1]=0x314E;   last_part_A[6][2]='\0';
last_part_A[7][0]=0x3137;   last_part_A[7][1]='\0';
last_part_A[8][0]=0x3139;   last_part_A[8][1]='\0';
last_part_A[9][0]=0x3139;   last_part_A[9][1]=0x3131;   last_part_A[9][2]='\0';
last_part_A[10][0]=0x3139;  last_part_A[10][1]=0x3141;  last_part_A[10][2]='\0';
last_part_A[11][0]=0x3139;  last_part_A[11][1]=0x3142;  last_part_A[11][2]='\0';
last_part_A[12][0]=0x3139;  last_part_A[12][1]=0x3145;  last_part_A[12][2]='\0';
last_part_A[13][0]=0x3139;  last_part_A[13][1]=0x314C;  last_part_A[13][2]='\0';
last_part_A[14][0]=0x3139;  last_part_A[14][1]=0x314D;  last_part_A[14][2]='\0';
last_part_A[15][0]=0x3139;  last_part_A[15][1]=0x314E;  last_part_A[15][2]='\0';
last_part_A[16][0]=0x3141;  last_part_A[16][1]='\0';
last_part_A[17][0]=0x3142;  last_part_A[17][1]='\0';
last_part_A[18][0]=0x3142;  last_part_A[18][1]=0x3145;  last_part_A[18][2]='\0';
last_part_A[19][0]=0x3145;  last_part_A[19][1]='\0';
last_part_A[20][0]=0x3146;  last_part_A[20][1]='\0';
last_part_A[21][0]=0x3147;  last_part_A[21][1]='\0';
last_part_A[22][0]=0x3148;  last_part_A[22][1]='\0';
last_part_A[23][0]=0x314A;  last_part_A[23][1]='\0';
last_part_A[24][0]=0x314B;  last_part_A[24][1]='\0';
last_part_A[25][0]=0x314C;  last_part_A[25][1]='\0';
last_part_A[26][0]=0x314D;  last_part_A[26][1]='\0';
last_part_A[27][0]=0x314E;  last_part_A[27][1]='\0';
}


/**
 * Initializes last_part_B array
 */
void initializes_last_part_B() {
last_part_B[0][0]='\0';
last_part_B[1][0]=0x314F;  last_part_B[1][1]='\0';
last_part_B[2][0]=0x3150;  last_part_B[2][1]='\0';
last_part_B[3][0]=0x3163;  last_part_B[3][1]='\0';
}


/**
 * Initializes last_part_C array
 */
void initializes_last_part_C() {
last_part_C[0][0]='\0';
last_part_C[1][0]=0x3163;  last_part_C[1][1]='\0';
}


/**
 * Initializes arrays
 */
int initKoreanArrays() {
initializes_last_part_A();
initializes_last_part_B();
initializes_last_part_C();
return 0;
}


static int __korean_foo=initKoreanArrays();

static int test_letter(unichar c,Alphabet* alphabet) {
if (alphabet==NULL) return 0;
return is_letter(c,alphabet);
}

/**
 * Converts a syllab text into a Jamo one, including Chinese -> Hangul conversion
 * and Jamo compatible -> Jamo conversion.
 */
void convert_Korean_text(unichar* src,unichar* dest,jamoCodage* jamo,Alphabet* alphabet) {
if (jamo==NULL) {
   /* No Korean data? A simple copy will do. */
	u_strcpy(dest,src);
   return;
}
unichar temp[1024];
unichar temp2[1024];
/* First, we convert the Chinese characters, if any */
jamo->convHJAtoHAN(src,dest);
/* Then, we put a syllab bound before every character that is
 * 1) not a syllab one but in the alphabet (i.e. latin letters)
 * 2) a Jamo letter in order to avoid ambiguity when the syllab on its left
 *    is turned into Jamo letters */
int j=0;
for (int i=0;dest[i]!='\0';i++) {
   if ((test_letter(dest[i],alphabet) && !u_is_korea_syllabe_letter(dest[i]))
         || (u_is_Hangul_Compatility_Jamo(dest[i]) && dest[i]!=KR_SYLLAB_BOUND)
         || u_is_Hangul_Jamo(dest[i])){
      /* We do not put yet the real syllab bound character, because we will
       * have to use it as a marker in order to know where the empty consonant
       * should be inserted */
      temp[j++]=0x0001;
   }
   temp[j++]=dest[i];
}
temp[j]='\0';
/* Then, we perform the syllab -> Jamo conversion */
jamo->convertSyletCjamoToJamo(temp,temp2,j,1024);
/* Finally, we insert the empty consonant where it is necessary and we replace
 * our 0x0001 marker by the syllab bound */
j=0;
for (int i=0;temp2[i]!='\0';i++) {
   if (temp2[i]==0x0001) {
      dest[j++]=KR_SYLLAB_BOUND;
   } else if (temp2[i]==KR_SYLLAB_BOUND) {
      dest[j++]=KR_SYLLAB_BOUND;
      if (!u_is_Hangul_Jamo_initial_consonant(temp2[i+1])) {
         /* If there is no initial consonant, we insert the empty one */
         dest[j++]=KR_EMPTY_INITIAL_CONSONANT;
      }
   } else {
      dest[j++]=temp2[i];
   }
}
dest[j]='\0';
}


/**
 * Turns a Hangul compatibility jamo to a standard jamo sequence.
 * If the input is not a compatibility jamo letter, it is just copied to
 * the output string.
 */
void compatibility_jamo_to_standard_jamo(unichar c,unichar* dest,jamoCodage* jamo) {
if (u_is_Hangul_Compatility_Jamo(c)) {
   unichar tmp[32];
   dest[0]=c;
   dest[1]='\0';
   convert_Korean_text(dest,tmp,jamo,NULL);
   int start=0;
   if (tmp[0]==KR_SYLLAB_BOUND) {
	   start++;
   }
   u_strcpy(dest,tmp+start);
} else {
	dest[0]=c;
	dest[1]='\0';
}
}


/**
 * Turns a standard Jamo letter sequence into a Hangul syllab sequence.
 */
void convert_jamo_to_hangul(unichar* src,unichar* dest,Jamo2Syl* jamo2syl) {
jamo2syl->cleanMachine();
unichar t[128];
/* We make a copy without empty initial consonants, if any */
int j=0;
for (int i=0;src[i]!='\0';i++) {
   if (src[i]!=KR_EMPTY_INITIAL_CONSONANT) {
      t[j++]=src[i];
   }
}
t[j]='\0';
jamo2syl->convStr(t,j+1,dest);
}


/**
 * Returns the number of Jamo letters contained in the given Hangul.
 * If the given character is neither a Hangul nor a Chinese character,
 * the function should return 1.
 */
int get_length_in_jamo(unichar hangul,jamoCodage* jamo,Alphabet* alphabet) {
unichar t[2];
t[0]=hangul;
t[1]='\0';
unichar t2[16];
convert_Korean_text(t,t2,jamo,alphabet);
int pos=0;
if (t2[0]==KR_SYLLAB_BOUND) {
   pos=1;
}
return u_strlen(t2+pos)-1;
}

