 /*
  * Unitex
  *
  * Copyright (C) 2001-2007 Université de Marne-la-Vallée <unitex@univ-mlv.fr>
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

#define HANGUL_SYL_START	0xac00     // GA
#define HANGUL_SYL_END		0xD7A3      // HIH
#define UNI_HJA   0x4E00 /* Start code of CJK Unified Ideagraphs */
#define UNI_HANJAE 0x9FFF /* End code of CJK Unified Ideagraphs */
#define UNI_CHANJAS 0xF900 /* Start code of CJK Compatiblilty  Ideagraphs */
#define UNI_CHANJAE 0xFA2D /* End code of CJK Compatiblilty  Ideagraphs */
#define UNI_HJAMO_CSTART  0x3130 /* Start code of Hangul Compatibility Jamo */
//#define UNI_CHG_JAMO_CE  0x318E /* End code of Hangul Compatibility Jamo  */
//#define UNI_CHG_CS  0x3130 /* Start code of Hangul Jamo Initial consonants:19 */
//#define UNI_CHG_CE  0x314E /* End code of Hangul Jamo Initial consonants */
//#define UNI_CHG_VS  0x314F /* Start code of Hangul Jamo Final consonants:28 */
#define UNI_HJAMO_CEND  0x3163 /* End code of Hangul Jamo Final consonants */
#include "codeForKorean.h"

class jamoCodage : public convert_windows949kr_uni  {

	unichar *orderTableJamo[256];
	unichar *mapJamoTable[256];
	unichar *CjamoUjamoTable[256];
	int jamoSize;
public:
	unichar sylMark;
	unichar *sylMarkStr;
	jamoCodage(){

		sylMark = 0;
		sylMarkStr = 0;
		for(int i = 0; i < 256; i++) CjamoUjamoTable[i] = 0;
		loadJamoMap(0);
	};
	~jamoCodage(){

		if(sylMarkStr) delete sylMarkStr;
	};
	int loadJamoMap(char *fname);
	int sylToJamo(unichar syl,unichar *obuff,int o_off);
	int jamoToSJamo(unichar jamo,unichar *obuff,int o_off);
	int convertSylToJamo(unichar *ibuff,unichar *obuff,int sz,int limit);
	int convertSyletCjamoToJamo(unichar *ibuff,unichar *obuff,int sz,int limit);
	void jamoMapOut();
	


};
extern char *defaultSylToJamoMap;


