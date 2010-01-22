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
#ifndef JamoCodageH
#define JamoCodageH

#include "Unicode.h"
/* This include is only required by Jamo2Syl */
#include "codeForKorean.h"

#define HANGUL_SYL_START	0xAC00     // GA
#define HANGUL_SYL_END		0xD7A3      // HIH
#define UNI_HJA   0x4E00 /* Start code of CJK Unified Ideagraphs */
#define UNI_HANJAE 0x9FFF /* End code of CJK Unified Ideagraphs */
#define UNI_CHANJAS 0xF900 /* Start code of CJK Compatiblilty  Ideagraphs */
#define UNI_CHANJAE 0xFA2D /* End code of CJK Compatiblilty  Ideagraphs */
#define UNI_HJAMO_CSTART  0x3130 /* Start code of Hangul Compatibility Jamo */
#define UNI_HJAMO_CEND  0x318E   /* End code of Hangul Compatibility Jamo */
//#define UNI_CHG_JAMO_CE  0x318E /* End code of Hangul Compatibility Jamo  */
//#define UNI_CHG_CS  0x3130 /* Start code of Hangul Jamo Initial consonants:19 */
//#define UNI_CHG_CE  0x314E /* End code of Hangul Jamo Initial consonants */
//#define UNI_CHG_VS  0x314F /* Start code of Hangul Jamo Final consonants:28 */

#define JAMO_SIZE 68
#define MAX_ORDER_JAMO_SIZE 8
#define KR_SYLLAB_BOUND_v2 0x318D

#define N_VOWELS 21
#define N_FINAL_CONSONANTS 28

#define INDEX_FIRST_VOWEL 19
#define INDEX_FIRST_FINAL_CONSONANT 40

extern unichar sylMarkStr[5];

/* This extent is only required by Jamo2Syl */
class jamoCodage : public convert_windows949kr_uni {

   /* This array contains the lines of the Jamo configuration file. Index are
    * line numbers */
	unichar* orderTableJamo[256];

	/* This array is another indirection for converting Unicode Jamos to Jamos.
	 * If we have a Unicode Jamo #Z, CjamoUjamoTable[Z-0x3130] will point
	 * to a unichar array containing equivalent Jamos */
	unichar* CjamoUjamoTable[256];

public:

	jamoCodage() {
		for(int i=0;i<256;i++) {
		   orderTableJamo[i]=NULL;
		   CjamoUjamoTable[i]=NULL;
		}
		initJamoMap();
	};

	~jamoCodage() {
		for(int i=0;i<JAMO_SIZE;i++) {
			if (orderTableJamo[i]!=NULL) {
				delete [] orderTableJamo[i];
			}
		}
	};

	int sylToJamo(unichar syl,unichar* output,int pos);
	int jamoToSJamo(unichar jamo,unichar* output,int pos);
	int convertSylToJamo(unichar* input,unichar* output,int size,int limit);
	int convertSyletCjamoToJamo(unichar* input,unichar* output,int size,int limit);
	void jamoMapOut();
	
private:
	void initJamoMap();



};
extern const char *defaultSylToJamoMap;


#endif
