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

#include <stdlib.h>
#include <stdarg.h>
#include "Unicode.h"
#include "etc.h"
#include "jamoCodage.h"
#include "Error.h"

unichar sylMarkStr[5]={'<','S','S','>',0};


const char *defaultSylToJamoMap = ""
   "# map of  korean alphabet in the UNICODE\n"
   "#		\n"
   "#		\n"
   "		\n"
   "<SS>	0x318D			# bounce of syllabe\n"
   "Initial_Consonants				\n"
   "0x1100	0x3131	0x1100				#0x3131\n"
   "0x1101 0x3132	0x1100	0x1100 	 	#0x3132\n"
   "0x1102	0x3134	0x1102			 	#0x3134\n"
   "0x1103	0x3137	0x1103				#0x3137\n"
   "0x1104	0x3138	0x1103	0x1103		#0x3138\n"
   "0x1105	0x3139	0x1105				#0x3139\n"
   "0x1106	0x3141	0x1106				#0x3141\n"
   "0x1107	0x3142	0x1107			 	#0x3142\n"
   "0x1108	0x3143	0x1107 0x1107		#0x3143\n"
   "0x1109	0x3145	0x1109			 	#0x3145\n"
   "0x110A	0x3146	0x1109 0x1109		#0x3146\n"
   "0x110B	0x3147						#0x3147\n"
   "0x110C	0x3148	0x110C				#0x3148\n"
   "0x110D	0x3149	0x110C 0x110C		#0x3149\n"
   "0x110E	0x314A	0x110E       		#0x314A\n"
   "0x110F	0x314B	0x110F				#0x314B\n"
   "0x1110	0x314c	0x1110				#0x314C\n"
   "0x1111	0x314D	0x1111				#0x314D\n"
   "0x1112	0x314E	0x1112   			#0x314E\n"
   "Vowels				\n"
   "0x1161	0x314F	0x1161				#0x314F\n"
   "0x1162	0x3150	0x1161 0x1175		#0x3150\n"
   "0x1163	0x3151	0x1175 0x1161		#0x3151\n"
   "0x1164	0x3152	0x1175 0x1161 0x1175#0x3152\n"
   "0x1165	0x3153	0x1165				#0x3153\n"
   "0x1166	0x3154	0x1165 0x1175		#0x3154\n"
   "0x1167	0x3155	0x1175 0x1165		#0x3155\n"
   "0x1168	0x3156	0x1175 0x1165 0x1175#0x3156\n"
   "0x1169	0x3157	0x1169				#0x3157\n"
   "0x116A	0x3158	0x1169 0x1161		#0x3158\n"
   "0x116B	0x3159	0x1169 0x1175 0x1165#0x3159\n"
   "0x116C	0x315A	0x1169 0x1175		#0x315A\n"
   "0x116D	0x315B	0x1175 0x1169		#0x315B\n"
   "0x116E	0x315c	0x116E				#0x315C\n"
   "0x116F	0x315d	0x116E 0x1165		#0x315D\n"
   "0x1170	0x315E	0x116E 0x1165 0x1175#0x315E\n"
   "0x1171	0x315F	0x116E 0x1175		#0x315F\n"
   "0x1172	0x3160	0x1175 0x116E		#0x3160\n"
   "0x1173	0x3161	0x1173				#0x3161\n"
   "0x1174	0x3162	0x1173 0x1175		#0x3162\n"
   "0x1175	0x3163	0x1175				#0x3163\n"
   "Final_Consonants\n"
   "FCNULL	-	-	-	#final consonant is vide\n"
   "0x11A8	0x3131	0x1100				#0x3131\n"
   "0x11A9	0x3132	0x1100 0x1100		#0x3132\n"
   "0x11AA	0x3133	0x1100 0x1109		#0x3133\n"
   "0x11AB	0x3134	0x1102				#0x3134\n"
   "0x11AC	0x3135	0x1102 0x110C		#0x3135\n"
   "0x11AD	0x3136	0x1102 0x1112		#0x3136\n"
   "0x11AE	0x3137	0x1103				#0x3137\n"
   "0x11AF	0x3139	0x1105				#0x3139\n"
   "0x11B0	0x313a	0x1105 0x1100		#0x313A\n"
   "0x11B1	0x313b	0x1105 0x1106		#0x313B\n"
   "0x11B2	0x313c	0x1105 0x1107		#0x313C\n"
   "0x11B3	0x313d	0x1105 0x1109		#0x313D\n"
   "0x11B4	0x313e	0x1105 0x1110		#0x313E\n"
   "0x11B5	0x313f	0x1105 0x1111		#0x313F\n"
   "0x11B6	0x3140	0x1105 0x1112		#0x3140\n"
   "0x11B7	0x3141	0x1106				#0x3141\n"
   "0x11B8	0x3142	0x1107				#0x3142\n"
   "0x11B9	0x3144	0x1107 0x1109		#0x3144\n"
   "0x11BA	0x3145	0x1109				#0x3145\n"
   "0x11BB	0x3146	0x1109 0x1109		#0x3146\n"
   "0x11BC	0x3147	0x110B				#0x3147\n"
   "0x11BD	0x3148	0x110C				#0x3148\n"
   "0x11BE	0x314a	0x110E				#0x314A\n"
   "0x11BF	0x314B	0x110F				#0x314B\n"
   "0x11C0	0x314c	0x1110				#0x314C\n"
   "0x11C1	0x314d	0x1111				#0x314D\n"
   "0x11C2	0x314e	0x1112				#0x314E\n"\
;

#if 0
int jamoCodage::loadJamoMap(char *mfName) {
   U_FILE *f = 0;
   unsigned short *wp; // token buff
   int i;
   int svFlag;
   char line[256];
   unichar buff[256];

   int rsz;
   const char *rp = NULL;
   unichar roffset;

   int JamoIndex, JamoCnt;
   unsigned short *segPtr[16];
   int segCnt;
   unsigned short *swp;

   if (!mfName) {
      rp = defaultSylToJamoMap;
      roffset = 0;
   } else {
      f = u_fopen(UTF16_LE, mfName, U_READ);
      if (!f)
         fatal_error("Cannot open %s\n", mfName);
   }

   if (jamoSize > 0) {
      for (int i = 0; i < jamoSize; i++)
         if (orderTableJamo[i])
            delete[] orderTableJamo[i];
   }

   jamoSize = 0;
   int jamoOffset = 0;
   do {
      rsz = 0;

      if (f) {
         if (EOF == u_fgets_limit2(buff, 256, f))
            break;
      } else {
         if (*rp == 0)
            break;
         while (*rp != '\n')
            line[rsz++] = *rp++;
         rp++;
         line[rsz++] = 0;
         mbcsToUniStr((unsigned char *) line, (unichar *) buff);
      }

      segCnt = 0;
      svFlag = 1;
      swp = buff;
      while (*swp) {
         switch (*swp) {
         case '#':
            *swp = 0;
            break;
         case ' ':
         case '\t':
            svFlag = 1;
            *swp++ = 0;
            break;
         default:
            if (svFlag) {
               segPtr[segCnt++] = swp;
               svFlag = 0;
            }
            swp++;
         }
      }
      if (!segCnt)
         continue;
      JamoCnt = 0;
      wp = segPtr[0];
      if (*wp == '0') { // hex value
         JamoIndex = uniToInt((unichar *) wp);
         orderTableJamo[jamoOffset] = new unichar[8];
         mapJamoTable[JamoIndex - 0x1100] = orderTableJamo[jamoOffset];
         for (int i = 0; i < 8; i++)
            orderTableJamo[jamoOffset][i] = 0;
         orderTableJamo[jamoOffset][JamoCnt++] = (unichar) JamoIndex;
      } else if (*wp == '<') { // symbol for syllabe mark
         if (sylMarkStr != NULL)
            delete[] sylMarkStr;
         sylMarkStr = new unichar[u_strlen((unsigned short *) wp) + 1];
         u_strcpy((unsigned short *) sylMarkStr, (unsigned short *) wp);
         if (segCnt == 2) {
            wp = segPtr[1];
            if (orderTableJamo[0xff])
               delete[] orderTableJamo[0xff];
            orderTableJamo[0xff] = new unichar[8];
            for (int i = 0; i < 8; i++)
               orderTableJamo[0xff][i] = 0;
            if (*wp == '0')
               orderTableJamo[0xff][0] = (unichar) uniToInt((unichar *) wp);
            else
               orderTableJamo[0xff][0] = *wp;
            sylMark = orderTableJamo[0xff][0];
         }
         continue;
      } else if (!u_strcmp(wp, "Initial_Consonants")) {
         jamoOffset = 0;
         continue;
      } else if (!u_strcmp(wp, "Vowels")) {
         if (jamoOffset != 19)
            fatal_error("table error\n");
         continue;
      } else if (!u_strcmp(wp, "Final_Consonants")) {
         if (jamoOffset != 40)
            fatal_error("table error\n");
         continue;
      } else if (!u_strcmp(wp, "FCNULL")) {
         orderTableJamo[jamoOffset] = new unichar[8];
         for (int i = 0; i < 8; i++)
            orderTableJamo[jamoOffset][i] = 0;
         jamoOffset++;
         continue;
      }
      if (segCnt < 2)
         fatal_error("Line format :error\n");
      wp = segPtr[1];

      if (wp[1] == '\0')
         orderTableJamo[jamoOffset][JamoCnt++] = *wp;
      else
         orderTableJamo[jamoOffset][JamoCnt++] = (unichar) uniToInt(
               (unichar *) wp);

      for (i = 2; i < segCnt; i++) {
         wp = segPtr[i];
         if (wp[1] == '\0')
            orderTableJamo[jamoOffset][JamoCnt++] = *wp;
         else
            orderTableJamo[jamoOffset][JamoCnt++] = (unichar) uniToInt(
                  (unichar *) wp);
      }
      jamoOffset++;
   } while (true);
   jamoSize = jamoOffset;

   for (int i = 0; i < jamoSize; i++)
      if (orderTableJamo[i][1])
         CjamoUjamoTable[orderTableJamo[i][1] - 0x3130] = &orderTableJamo[i][2];

   if (f)
      u_fclose(f);
   return (1);
}
#endif





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
      result[n++]=(unichar)d;
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
void jamoCodage::initJamoMap() {
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
int jamoCodage::sylToJamo(unichar syl,unichar* output,int pos) {
if (syl<HANGUL_SYL_START || syl>HANGUL_SYL_END) {
   fatal_error("Invalid Hangul %X in sylToJamo\n",syl);
}
output[pos++]=KR_SYLLAB_BOUND_v2;
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
int jamoCodage::jamoToSJamo(unichar jamo,unichar* output,int pos) {
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
int jamoCodage::convertSylToJamo(unichar* input,unichar* output,int size,int limit) {
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
int jamoCodage::convertSyletCjamoToJamo(unichar* input,unichar* output,int size,int limit) {
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


void jamoCodage::jamoMapOut() {
   unichar markReturn = '\n';

   U_FILE *f;
   int i, j;
   f = u_fopen(UTF16_LE, "jamoCTable.txt", U_WRITE);
   if (!f)
      fopenErrMessage("jamoCTable.txt");

   u_fprintf(f,
         "Unicode Jamos, pair of Hangul Jamo et Hangul Compatiblity Jamo\n");
   for (i = 0; i < JAMO_SIZE; i++) {
      if (orderTableJamo[i][0]) { // detect the condition of which the final consonrant is vide
         u_fputc(orderTableJamo[i][0], f);
         u_fputc(orderTableJamo[i][1], f);//			fwrite(&orderTableJamo[i][0],2,2,f);
      }
      u_fputc(markReturn, f);//		fwrite(&markReturn,2,1,f);
   }
   u_fprintf(f, "Using Basic Jamo:");
   unichar basicJamo[128];
   int cntBasicJamo = 0;

   for (i = 0; i < JAMO_SIZE; i++) {
      if (orderTableJamo[i][0]) {
         if (orderTableJamo[i][2] && !orderTableJamo[i][3]) // detect a character
         {
            for (j = 0; j < cntBasicJamo; j++) {
               if (basicJamo[j] == orderTableJamo[i][2])
                  break;
            }
            if (j == cntBasicJamo) {
               basicJamo[cntBasicJamo++] = orderTableJamo[i][2];
               if (cntBasicJamo == 127)
                  fatal_error("too long Jamos\n");
            }
         }
      }
   }
   for (i = 0; i < cntBasicJamo; i++) {
      for (j = i; j < cntBasicJamo; j++) {
         if (basicJamo[i] > basicJamo[j]) {
            basicJamo[127] = basicJamo[i];
            basicJamo[i] = basicJamo[j];
            basicJamo[j] = basicJamo[127];
         }
      }
   }
   basicJamo[cntBasicJamo++] = orderTableJamo[0xff][0];
   for (i = 0; i < cntBasicJamo; i++) {
      u_fputc(basicJamo[i], f); // fwrite(&basicJamo[i],2,1,f);
      u_fputc(markReturn, f); //fwrite(&markReturn,2,1,f);
   }
   u_fclose(f);
}

