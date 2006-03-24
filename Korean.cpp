 /*
  * Unitex
  *
  * Copyright (C) 2001-2006 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

unichar first_letter_tab[]={0x3131,0x3132,0x3134,0x3137,0x3138,
                            0x3139,0x3141,0x3142,0x3143,0x3145,
                            0x3146,0x3147,0x3148,0x3149,0x314A,
                            0x314B,0x314C,0x314D,0x314E};

unichar second_letter_tab_A[]={0x314F,0x3150,0x3151,0x3152,0x3153,
                             0x3154,0x3155,0x3156};

unichar second_letter_tab_B[]={0x3157,0x315B,0x315C,0x3160,0x3161};

unichar second_letter_tab_C[]={0x3163};

unichar last_part_A[28][3];

unichar last_part_B[4][2];

unichar last_part_C[2][2];

/**
 * Converts a Korean syllab S into a letter sequence stored in res. Returns 0
 * if S is not a Korean syllab, 1 otherwise.
 */
int syllabToLetters(unichar S,unichar* res) {
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


int __korean_bidon=initKoreanArrays();
