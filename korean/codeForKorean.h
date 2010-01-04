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

#include "pro_type.h"
#include "segtype.h"
#include "orgUniMbcsMap.h"

class convert_windows949kr_uni:public pro_associ, wideCharTable  {

    int ready_uniMbcs;
public:
    unichar *loadHJAConvMap;
    short int *mbcsUni949Table;
    unsigned char *uniMbcs949Table;
	
 convert_windows949kr_uni();
 ~convert_windows949kr_uni();
    
    void mbcsToUniSz(unsigned char mbcs[],unichar *des,int mbcs_byte_cnt);
    void uniToMbcsSz(unichar *uchars,unsigned char *mbcs,int uchar_cnt);
    void mbcsToUniStr(unsigned char *mbcs,unichar *des);
    void UniToMbcsStr(unichar *up,unsigned char *mbcs);
    int mbcs949clen(unsigned char *mbcs);
    
    void loadFromFile(char *fname);
    void strToMapKr();

	int convHJAtoHAN(unichar *src,unichar *des);
   void loadHJAMap(char *f);
   void cloneHJAMap(unichar* map);
	void unLoadHJAMap();
};

