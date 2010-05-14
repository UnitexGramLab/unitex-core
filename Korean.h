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

#ifndef KoreanH
#define KoreanH

/**
 * This is a library to manipulate Korean text from Hanguls to Jamos and
 * from Jamos to Hanguls. When we convert to Jamos, we always convert to
 * single letters, i.e. we don't produce SSANG XXX but two XXX (the same
 * for compound vowels and compound consonants). Moreover, we always produce
 * standard initial Jamo consonants in order to avoid computation difficulties.
 */

#include "Unicode.h"
#include "Alphabet.h"
#include "Error.h"
#include "HashTable.h"


#define JAMO_SIZE 68
#define MAX_ORDER_JAMO_SIZE 8

#define MAX_LETTERS_IN_A_SYLLAB 5
#define KR_SYLLABLE_BOUND 0x318D

#define N_FINAL_CONSONANTS 28
#define N_VOWELS 21
#define INDEX_FIRST_VOWEL 19
#define INDEX_FIRST_FINAL_CONSONANT 40

#define KR_HANGUL_SYL_START   0xAC00
#define KR_HANGUL_SYL_END     0xD7A3

#define KR_HCJ_START		0x3130
#define KR_HCJ_END		0x318E


/**
 * Here we define the Hangul Compatibility Jamos used in the library.
 * In the Hangul Compatibility Jamo charset, no difference is made between
 * initial and final consonant.
 */
/* Consonants */
#define HCJ_KIYEOK 			0x3131
#define HCJ_SSANGKIYEOK 	0x3132
#define HCJ_KIYEOK_SIOS		0x3133
#define HCJ_NIEUN 			0x3134
#define HCJ_NIEUN_CIEUC		0x3135
#define HCJ_NIEUN_HIEUH		0x3136
#define HCJ_TIKEUT 			0x3137
#define HCJ_SSANGTIKEUT 	0x3138
#define HCJ_RIEUL 			0x3139
#define HCJ_RIEUL_KIYEOK	0x313A
#define HCJ_RIEUL_MIEUM 	0x313B
#define HCJ_RIEUL_PIEUP		0x313C
#define HCJ_RIEUL_SIOS		0x313D
#define HCJ_RIEUL_THIEUTH	0x313E
#define HCJ_RIEUL_PHIEUPH	0x313F
#define HCJ_RIEUL_HIEUH		0x3140
#define HCJ_MIEUM 			0x3141
#define HCJ_PIEUP 			0x3142
#define HCJ_SSANGPIEUP 		0x3143
#define HCJ_PIEUP_SIOS 		0x3144
#define HCJ_SIOS 			0x3145
#define HCJ_SSANGSIOS 		0x3146
#define HCJ_IEUNG 			0x3147
#define HCJ_CIEUC 			0x3148
#define HCJ_SSANGCIEUC 		0x3149
#define HCJ_CHIEUCH 		0x314A
#define HCJ_KHIEUKH 		0x314B
#define HCJ_THIEUTH 		0x314C
#define HCJ_PHIEUPH 		0x314D
#define HCJ_HIEUH 			0x314E
/* Vowels */
#define HCJ_A 				0x314F
#define HCJ_AE 				0x3150
#define HCJ_YA 				0x3151
#define HCJ_YAE 			0x3152
#define HCJ_EO 				0x3153
#define HCJ_E 				0x3154
#define HCJ_YEO 			0x3155
#define HCJ_YE 				0x3156
#define HCJ_O 				0x3157
#define HCJ_WA 				0x3158
#define HCJ_WAE 			0x3159
#define HCJ_OE 				0x315A
#define HCJ_YO 				0x315B
#define HCJ_U 				0x315C
#define HCJ_WEO 			0x315D
#define HCJ_WE 				0x315E
#define HCJ_WI 				0x315F
#define HCJ_YU 				0x3160
#define HCJ_EU 				0x3161
#define HCJ_YI 				0x3162
#define HCJ_I 				0x3163


/**
 * Here we define the Standard Jamos used in the library
 */
/* Initial consonants */
#define SJ_IC_KIYEOK 		0x1100
#define SJ_IC_SSANGKIYEOK 	0x1101
#define SJ_IC_NIEUN 		0x1102
#define SJ_IC_TIKEUT 		0x1103
#define SJ_IC_SSANGTIKEUT 	0x1104
#define SJ_IC_RIEUL 		0x1105
#define SJ_IC_MIEUM 		0x1106
#define SJ_IC_PIEUP 		0x1107
#define SJ_IC_SSANGPIEUP 	0x1108
#define SJ_IC_SIOS 			0x1109
#define SJ_IC_SSANGSIOS 	0x110A
#define SJ_IC_IEUNG 		0x110B
#define SJ_IC_CIEUC 		0x110C
#define SJ_IC_SSANGCIEUC 	0x110D
#define SJ_IC_CHIEUCH 		0x110E
#define SJ_IC_KHIEUKH 		0x110F
#define SJ_IC_THIEUTH 		0x1110
#define SJ_IC_PHIEUPH 		0x1111
#define SJ_IC_HIEUH 		0x1112
/* Vowels */
#define SJ_A 				0x1161
#define SJ_AE 				0x1162
#define SJ_YA 				0x1163
#define SJ_YAE 				0x1164
#define SJ_EO 				0x1165
#define SJ_E 				0x1166
#define SJ_YEO 				0x1167
#define SJ_YE 				0x1168
#define SJ_O 				0x1169
#define SJ_WA 				0x116A
#define SJ_WAE 				0x116B
#define SJ_OE 				0x116C
#define SJ_YO 				0x116D
#define SJ_U 				0x116E
#define SJ_WEO 				0x116F
#define SJ_WE 				0x1170
#define SJ_WI 				0x1171
#define SJ_YU 				0x1172
#define SJ_EU 				0x1173
#define SJ_YI 				0x1174
#define SJ_I 				0x1175
/* Final consonants */
#define SJ_FC_KIYEOK		0x11A8
#define SJ_FC_SSANGKIYEOK	0x11A9
#define SJ_FC_SSANGKIYEOK	0x11A9
#define SJ_FC_KIYEOK_SIOS	0x11AA
#define SJ_FC_NIEUN			0x11AB
#define SJ_FC_NIEUN_CIEUC	0x11AC
#define SJ_FC_NIEUN_HIEUH	0x11AD
#define SJ_FC_TIKEUT		0x11AE
#define SJ_FC_RIEUL			0x11AF
#define SJ_FC_RIEUL_KIYEOK	0x11B0
#define SJ_FC_RIEUL_MIEUM 	0x11B1
#define SJ_FC_RIEUL_PIEUP	0x11B2
#define SJ_FC_RIEUL_SIOS	0x11B3
#define SJ_FC_RIEUL_THIEUTH	0x11B4
#define SJ_FC_RIEUL_PHIEUPH	0x11B5
#define SJ_FC_RIEUL_HIEUH	0x11B6
#define SJ_FC_MIEUM			0x11B7
#define SJ_FC_PIEUP			0x11B8
#define SJ_FC_PIEUP_SIOS	0x11B9
#define SJ_FC_SIOS			0x11BA
#define SJ_FC_SSANGSIOS		0x11BB
#define SJ_FC_IEUNG			0x11BC
#define SJ_FC_CIEUC			0x11BD
#define SJ_FC_CHIEUCH		0x11BE
#define SJ_FC_KHIEUKH		0x11BF
#define SJ_FC_THIEUTH		0x11C0
#define SJ_FC_PHIEUPH		0x11C1
#define SJ_FC_HIEUH			0x11C2


#define SJ_RIEUL_HIEUH		0x111A
#define SJ_PIEUP_SIOS 		0x1121




class Korean {

   /* This array contains the lines of the Jamo configuration file. Index are
    * line numbers. Each line is made as follows:
    *
    * AAA BBB CCC....
    *
    * AAA		= standard Jamo
    * BBB		= HCJ equivalent
    * CCC....	= single Jamo letter equivalent sequence, if AAA is a compound letter
    */
   unichar* jamo_table[256];

   /* This array is an indirection for converting Hangul compatibility Jamos to Jamos.
    * If we have a HCJ #Z, HCJ_to_SJ[Z-0x3130] will point
    * to a unichar array containing equivalent Jamos.
    *
    * NOTE: HCJ consonant are always turned to initial SJ consonants
    */
   unichar* HCJ_to_SJ_table[256];

   Alphabet* alphabet;


public:

   /* This is used to optimize Hangul->Jamo conversions */
   struct hash_table* table;

   Korean(Alphabet* alph) {
	   if (alph==NULL) {
		   fatal_error("Unexpected NULL alphabet in Korean()\n");
	   }
	   if (alph->korean_equivalent_syllable==NULL) {
	   	   fatal_error("Unexpected NULL Chinese to Hangul map in Korean()\n");
	   }
      for(int i=0;i<256;i++) {
         jamo_table[i]=NULL;
         HCJ_to_SJ_table[i]=NULL;
      }
      initJamoMap();
      alphabet=alph;
      table=new_hash_table(1024,0.75f,(HASH_FUNCTION)hash_unichar,(EQUAL_FUNCTION)u_equal,
      	      (FREE_FUNCTION)free,(KEYCOPY_FUNCTION)keycopy);
   };

   ~Korean() {
      for(int i=0;i<JAMO_SIZE;i++) {
         if (jamo_table[i]!=NULL) {
            delete [] jamo_table[i];
         }
      }
      free_hash_table(table);
   };

   int single_Hangul_to_Jamos(unichar syl,unichar* output,int pos);
   int single_HCJ_to_Jamos(unichar jamo,unichar* output,int pos);
   int Jamos_to_Hangul(unichar* input,unichar* output);
   void Hanguls_to_Jamos(unichar* src,unichar* dest,int only_syllables);

private:
   void initJamoMap();
   int Hanguls_to_Jamos_internal(unichar* input,unichar* output,int only_syllables);
};

int single_HGJ_to_Jamos(unichar c,unichar* dest,Korean* korean);
int convert_jamo_to_hangul(unichar* src,unichar* dest,Korean* korean);
void Hanguls_to_Jamos(unichar* src,unichar* dest,Korean* korean,int only_syllables);
int get_length_in_jamo(unichar hangul,Korean* korean);

#endif
