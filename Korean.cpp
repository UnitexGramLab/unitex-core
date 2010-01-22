 /*
  * Unitex
  *
  * Copyright (C) 2001-2010 Universit� Paris-Est Marne-la-Vall�e <unitex@univ-mlv.fr>
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
 * Converts the Chinese characters that may be present in 'src' into
 * their Hangul equivalent. If 'dest' is NULL, 'src' is modified.
 * Returns the number of Chinese characters that have been converted.
 */
int Korean::convHJAtoHAN(unichar* src,unichar* dest) {
unichar* loadHJAConvMap=alphabet->korean_equivalent_syllab;
if (loadHJAConvMap==NULL) {
   fatal_error("Unexpected NULL alphabet in Korean::convHJAtoHAN\n");
}
unichar c;
int n=0,index=0;
while(src[index]!='\0'){
   c=loadHJAConvMap[src[index]];
   if(dest!=NULL) {
      if (c!='\0') {
         dest[index]=c;
         n++;
      } else {
         dest[index]=src[index];
      }
   } else {
      if (c!='\0'){
         src[index]=c;
         n++;
      }
   }
   index++;
}
if (dest!=NULL) {
   dest[index]=0;
}
return n;
}



/**
 * Converts a syllab text into a Jamo one, including Chinese -> Hangul conversion
 * and Jamo compatible -> Jamo conversion.
 */
void convert_Korean_text(unichar* src,unichar* dest,Korean* korean,Alphabet* alphabet) {
if (korean==NULL) {
   /* No Korean data? A simple copy will do. */
	u_strcpy(dest,src);
   return;
}
unichar temp[1024];
unichar temp2[1024];
/* First, we convert the Chinese characters, if any */
korean->convHJAtoHAN(src,dest);
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
korean->convertSyletCjamoToJamo(temp,temp2,j,1024);
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
void compatibility_jamo_to_standard_jamo(unichar c,unichar* dest,Korean* korean) {
if (u_is_Hangul_Compatility_Jamo(c)) {
   unichar tmp[32];
   dest[0]=c;
   dest[1]='\0';
   convert_Korean_text(dest,tmp,korean,NULL);
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
int get_length_in_jamo(unichar hangul,Korean* korean,Alphabet* alphabet) {
unichar t[2];
t[0]=hangul;
t[1]='\0';
unichar t2[16];
convert_Korean_text(t,t2,korean,alphabet);
int pos=0;
if (t2[0]==KR_SYLLAB_BOUND) {
   pos=1;
}
return u_strlen(t2+pos)-1;
}



/**
 * A function to initialize arrays easily. The last value must be 0.
 */
static unichar* kdup(unichar first,...) {
unichar* result=new unichar[MAX_ORDER_JAMO_SIZE];
result[0]=first;
int d,n=1;
if (first!=0) {
   va_list args;
   va_start(args,first);
   while ((d=va_arg(args,int))!=0) {
      result[n++]=d;
   }
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
orderTableJamo[ 0]=kdup(0x1100, 0x3131, 0x1100, 0);
orderTableJamo[ 1]=kdup(0x1101, 0x3132, 0x1100, 0x1100, 0);
orderTableJamo[ 2]=kdup(0x1102, 0x3134, 0x1102, 0);
orderTableJamo[ 3]=kdup(0x1103, 0x3137, 0x1103, 0);
orderTableJamo[ 4]=kdup(0x1104, 0x3138, 0x1103, 0x1103, 0);
orderTableJamo[ 5]=kdup(0x1105, 0x3139, 0x1105, 0);
orderTableJamo[ 6]=kdup(0x1106, 0x3141, 0x1106, 0);
orderTableJamo[ 7]=kdup(0x1107, 0x3142, 0x1107, 0);
orderTableJamo[ 8]=kdup(0x1108, 0x3143, 0x1107, 0x1107, 0);
orderTableJamo[ 9]=kdup(0x1109, 0x3145, 0x1109, 0);
orderTableJamo[10]=kdup(0x110A, 0x3146, 0x1109, 0x1109, 0);
orderTableJamo[11]=kdup(0x110B, 0x3147, 0);
orderTableJamo[12]=kdup(0x110C, 0x3148, 0x110C, 0);
orderTableJamo[13]=kdup(0x110D, 0x3149, 0x110C, 0x110C, 0);
orderTableJamo[14]=kdup(0x110E, 0x314A, 0x1112, 0x110C, 0);
orderTableJamo[15]=kdup(0x110F, 0x314B, 0x1112, 0x1100, 0);
orderTableJamo[16]=kdup(0x1110, 0x314C, 0x1112, 0x1103, 0);
orderTableJamo[17]=kdup(0x1111, 0x314D, 0x1111, 0);
orderTableJamo[18]=kdup(0x1112, 0x314E, 0x1112, 0);
/* Then, we initialize the 21 vowels */
orderTableJamo[19]=kdup(0x1161, 0x314F, 0x1161, 0);
orderTableJamo[20]=kdup(0x1162, 0x3150, 0x1161, 0x1175, 0);
orderTableJamo[21]=kdup(0x1163, 0x3151, 0x1175, 0x1161, 0);
orderTableJamo[22]=kdup(0x1164, 0x3152, 0x1175, 0x1161, 0x1175, 0);
orderTableJamo[23]=kdup(0x1165, 0x3153, 0x1165, 0);
orderTableJamo[24]=kdup(0x1166, 0x3154, 0x1165, 0x1175, 0);
orderTableJamo[25]=kdup(0x1167, 0x3155, 0x1175, 0x1165, 0);
orderTableJamo[26]=kdup(0x1168, 0x3156, 0x1175, 0x1165, 0x1175, 0);
orderTableJamo[27]=kdup(0x1169, 0x3157, 0x1169, 0);
orderTableJamo[28]=kdup(0x116A, 0x3158, 0x1169, 0x1161, 0);
orderTableJamo[29]=kdup(0x116B, 0x3159, 0x1169, 0x1175, 0x1165, 0);
orderTableJamo[30]=kdup(0x116C, 0x315A, 0x1169, 0x1175, 0);
orderTableJamo[31]=kdup(0x116D, 0x315B, 0x1175, 0x1169, 0);
orderTableJamo[32]=kdup(0x116E, 0x315C, 0x116E, 0);
orderTableJamo[33]=kdup(0x116F, 0x315D, 0x116E, 0x1165, 0);
orderTableJamo[34]=kdup(0x1170, 0x315E, 0x116E, 0x1165, 0x1175, 0);
orderTableJamo[35]=kdup(0x1171, 0x315F, 0x116E, 0x1175, 0);
orderTableJamo[36]=kdup(0x1172, 0x3160, 0x1175, 0x116E, 0);
orderTableJamo[37]=kdup(0x1173, 0x3161, 0x1173, 0);
orderTableJamo[38]=kdup(0x1174, 0x3162, 0x1173, 0x1175, 0);
orderTableJamo[39]=kdup(0x1175, 0x3163, 0x1175, 0);
/* Finally, we initialize the 28 final consonants */
orderTableJamo[40]=kdup(0); /* Special case of the empty final consonant */
orderTableJamo[41]=kdup(0x11A8, 0x3131, 0x1100, 0);
orderTableJamo[42]=kdup(0x11A9, 0x3132, 0x1100, 0x1100, 0);
orderTableJamo[43]=kdup(0x11AA, 0x3133, 0x1100, 0x1109, 0);
orderTableJamo[44]=kdup(0x11AB, 0x3134, 0x1102, 0);
orderTableJamo[45]=kdup(0x11AC, 0x3135, 0x1102, 0x110C, 0);
orderTableJamo[46]=kdup(0x11AD, 0x3136, 0x1102, 0x1112, 0);
orderTableJamo[47]=kdup(0x11AE, 0x3137, 0x1103, 0);
orderTableJamo[48]=kdup(0x11AF, 0x3139, 0x1105, 0);
orderTableJamo[49]=kdup(0x11B0, 0x313A, 0x1105, 0x1100, 0);
orderTableJamo[50]=kdup(0x11B1, 0x313B, 0x1105, 0x1106, 0);
orderTableJamo[51]=kdup(0x11B2, 0x313C, 0x1105, 0x1107, 0);
orderTableJamo[52]=kdup(0x11B3, 0x313D, 0x1105, 0x1109, 0);
orderTableJamo[53]=kdup(0x11B4, 0x313E, 0x1105, 0x1110, 0);
orderTableJamo[54]=kdup(0x11B5, 0x313F, 0x1105, 0x1111, 0);
orderTableJamo[55]=kdup(0x11B6, 0x3140, 0x1105, 0x1112, 0);
orderTableJamo[56]=kdup(0x11B7, 0x3141, 0x1106, 0);
orderTableJamo[57]=kdup(0x11B8, 0x3142, 0x1107, 0);
orderTableJamo[58]=kdup(0x11B9, 0x3144, 0x1107, 0x1109, 0);
orderTableJamo[59]=kdup(0x11BA, 0x3145, 0x1109, 0);
orderTableJamo[60]=kdup(0x11BB, 0x3146, 0x1109, 0x1109, 0);
orderTableJamo[61]=kdup(0x11BC, 0x3147, 0x110B, 0);
orderTableJamo[62]=kdup(0x11BD, 0x3148, 0x110C, 0);
orderTableJamo[63]=kdup(0x11BE, 0x314A, 0x110E, 0);
orderTableJamo[64]=kdup(0x11BF, 0x314B, 0x110F, 0);
orderTableJamo[65]=kdup(0x11C0, 0x314C, 0x1110, 0);
orderTableJamo[66]=kdup(0x11C1, 0x314D, 0x1111, 0);
orderTableJamo[67]=kdup(0x11C2, 0x314E, 0x1112, 0);
for (int i=0;i<JAMO_SIZE;i++) {
   if (orderTableJamo[i][1]!=0) {
      CjamoUjamoTable[orderTableJamo[i][1] - 0x3130] = &(orderTableJamo[i][2]);
   }
}
}


/**
 * Converts the given Hangul syllab into its Jamo string equivalent.
 * Returns the updated position in the output string.
 */
int Korean::sylToJamo(unichar syl,unichar* output,int pos) {
if (syl<HANGUL_SYL_START || syl>HANGUL_SYL_END) {
   fatal_error("Invalid Hangul %X in sylToJamo\n",syl);
}
output[pos++]=KR_SYLLAB_BOUND;
unichar tmp=syl-HANGUL_SYL_START;
int final_consonant=tmp%N_FINAL_CONSONANTS;
tmp=tmp/N_FINAL_CONSONANTS;
int vowel=tmp%N_VOWELS;
int initial_consonant=tmp/N_VOWELS;
unichar* ptr=&orderTableJamo[initial_consonant][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
ptr=&orderTableJamo[vowel+INDEX_FIRST_VOWEL][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
ptr=&orderTableJamo[final_consonant+INDEX_FIRST_FINAL_CONSONANT][2];
while (*ptr!='\0') {
   output[pos++]=*ptr++;
}
return pos;
}


/**
 * Converts the given extend Hangul Compatibility Jamo into its Jamo string equivalent.
 * Returns the updated position in the output string.
 */
int Korean::jamoToSJamo(unichar jamo,unichar* output,int pos) {
if (jamo<UNI_HJAMO_CSTART || jamo>UNI_HJAMO_CEND) {
   fatal_error("Invalid Hangul Compatibility Jamo %X in jamoToSJamo\n",jamo);
}
unichar* ptr=CjamoUjamoTable[jamo-UNI_HJAMO_CSTART];
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
 * Converts a Hangul string into its Jamo equivalent. 'size' is the length of
 * the input to be considered. Returns the length of the output or limit-1
 * in case of buffer overflow.
 *
 * WARNING: 'limit' is intended to avoid buffer overflow in the 'output' string,
 *          but it is bugged since the call to 'sylToJamo' may provoke the
 *          buffer overflow
 */
int Korean::convertSylToJamo(unichar* input,unichar* output,int size,int limit) {
int n=0;
for (int i=0;i<size;i++) {
   if (input[i]>=HANGUL_SYL_START && input[i]<=HANGUL_SYL_END) {
      n=sylToJamo(input[i],output,n);
   } else {
      output[n++]=input[i];
   }
   if (n>=limit) {
      return limit-1;
   }
}
output[n]='\0';
return n;
}


/**
 * The same as 'convertSylToJamo', except that it ignores backslashes and that it
 * converts Hangul Compatibility Jamos into standard Jamos.
 *
 * WARNING: suffers from the same overflow bug as 'convertSylToJamo'
 */
int Korean::convertSyletCjamoToJamo(unichar* input,unichar* output,int size,int limit) {
int n=0;
for (int i=0;i<size;i++) {
   if (input[i]=='\\') {
      i++;
      continue;
   }
   if (input[i]>=HANGUL_SYL_START && input[i]<=HANGUL_SYL_END) {
      n=sylToJamo(input[i],output,n);
   }
   else if (input[i]>=UNI_HJAMO_CSTART && input[i]<=UNI_HJAMO_CEND) {
      n=jamoToSJamo(input[i],output,n);
   }
   else {
      output[n++] = input[i];
   }
   if (n>=limit) {
      n=limit;
      break;
   }
}
output[n]='\0';
return n;
}


