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

#ifndef CODE_PAGE_KR
#define CODE_PAGE_KR
#include "unimap.h"

unsigned short orgUniMbcsMap[0x20000]
={
#include "KSC5601.txt"
0,0
};
class convert_windows949kr_uni  {

    int ready_uniMbcs;
public:
    wchar_t *loadHJAConvMap;
    short int *mbcsUni949Table;
    unsigned char *uniMbcs949Table;

    convert_windows949kr_uni(){
        loadHJAConvMap = 0;
        mbcsUni949Table = new short int[256*128];
        uniMbcs949Table = new unsigned char[0x20000];
        strToMapKr();
        ready_uniMbcs = 1;
    };
    ~convert_windows949kr_uni(){
    delete mbcsUni949Table;
    delete uniMbcs949Table;
    if(loadHJAConvMap) unLoadHJAMap();
    };

    void mbcsToUniWithSz(unsigned char mbcs[],wchar_t *des,int mbcs_byte_cnt)
    {
        unsigned char page;
        unsigned char off;

        for( int i = 0; i < mbcs_byte_cnt;i++){
                if(mbcs[i] & 0x80){
                    page = mbcs[i] & 0x7f ;
                    i++;
                } else {
                     page = 0;
                }
                off = mbcs[i];
                *des++ =  (wchar_t)mbcsUni949Table[page*256 + off];
         }
    };
    void uniToMbcsWithSz(wchar_t *uchars,unsigned char *mbcs,int uchar_cnt){
        for( int i = 0; i < uchar_cnt;i++){
        *mbcs++ = uniMbcs949Table[*uchars++ * 2];
        *mbcs++=  uniMbcs949Table[*uchars++ * 2 +1];
        }
    } ;
    void mbcsToUniByStr(unsigned char *mbcs,wchar_t *des){
    unsigned char page;
    unsigned char off;
        while(*mbcs){
                if(*mbcs & 0x80){
                    page = (*mbcs++ & 0x7f);
                    if(*mbcs == 0) break; // abnormal case
                } else {
                     page = 0;
                }
                off = *mbcs++;
               *des = (wchar_t)mbcsUni949Table[page*256 + off];
               ++des;
         }
         *des++ = 0;
     };
    void UniToMbcsByStr(wchar_t *up,unsigned char *mbcs){
        while(*up){
            if( *up < 128){
                    *mbcs++ = uniMbcs949Table[*up++ * 2];
            } else {
                    *mbcs++ = uniMbcs949Table[*up++ * 2];
                    *mbcs++=  uniMbcs949Table[*up++ * 2+1];
            }
        }
    };
    int mbcs949clen(unsigned char *mbcs){
    int ret = 0;
        while(*mbcs){
                if(*mbcs & 0x80){
                    mbcs++;
                    if(*mbcs == 0) break; // abnormal case
                }
                mbcs++;
               ++ret;
         }
         return(ret);
     };

    void strToMapKr() {
    unsigned short *wp;

    unsigned char page, off;
    int i;
        wp = orgUniMbcsMap;
        for( i = 0 ; i < 128 ;i++)
                mbcsUni949Table[i] = i;
        for( i = 128; i < 128 * 256; i++)
             mbcsUni949Table[i] = '?';
        for( i = 0; i < 256;i++){
                uniMbcs949Table[i*2] = i;
        }
        for( i = 256; i < 0x10000;i++){
                uniMbcs949Table[i*2] = '?';
                uniMbcs949Table[i*2 + 1] = '?';
        }
        while(*wp){
          if(*wp & 0x8000){
              page = (*wp & 0xff00 ) >> 8;
              off = *wp & 0xff;
              wp++;
              mbcsUni949Table[(page & 0x7f)* 256 +off] = *wp;
              // uni
              if(page){
                  uniMbcs949Table[*wp*2] = page;
                  uniMbcs949Table[*wp*2 + 1] = off;
              }
              wp++;
          }
        }
    };

	int convHJAtoHAN(wchar_t *src,wchar_t *des){
	int csz = 0;
	wchar_t c;
	int index = 0;

    	if(loadHJAConvMap){
    		while(src[index]){
    			c = loadHJAConvMap[src[index]];
    			if(des) {
                   if(c){
    			        des[index] =  c;
    				    csz++;
    				} else
    				    des[index] = src[index];
    			} else {
    			    if(c){
    				  src[index] = c;
    				  csz++;
    	            }
    			}
    		    index++;
    		}
    		if(des) des[index] = 0;
    	}
    	return(csz);
	};
	wchar_t get4HexVal(wchar_t *l)
{
	wchar_t su = 0;
	wchar_t c;
	for (int i = 0; i < 4; i++){
		c = *l++;

		if(  ( c >= (unichar)'0' ) && (c <=(unichar)'9'))
			su = su * 16 + c- L'0';
		else if(  ( c >= (unichar)'a' ) && (c <=(unichar)'f'))
			su = su * 16 + c- L'a'+10;
		else if(  ( c >= (unichar)'A' ) && (c <=(unichar)'9'))
			su = su * 16 +  c- (unichar)'A'+10;
		else
			fatal_error("illegal value in conv map\n");
	}
	return(su);
};
wchar_t getValueIdx(wchar_t *s,int &idx)
{
	if(s[idx] == (unichar)'0'){
		idx ++;
		if((s[idx] == (unichar)'x') ||( s[idx] == (unichar)'X')){
			idx += 4;
			return(get4HexVal(s));
		}
		fatal_error("illegal value\n");
	}
	return(s[idx++]);
}
    void loadHJAMap(char *f)
    {
    	U_FILE* lf = u_fopen(UTF16_LE,f,U_READ);
    	int idx;
    	wchar_t srcIdx;
    	wchar_t desIdx;
    	wchar_t UtempLine[256];

    	loadHJAConvMap = (wchar_t *)malloc( sizeof(wchar_t)*0x10000);
    	for(idx = 0; idx < 0x10000;idx++) loadHJAConvMap[idx] = 0;
    	while(EOF!=u_fgets((unichar *)UtempLine,lf)){
    		idx = 0;
    		if(UtempLine[idx] == (unichar)' ') continue;
    		srcIdx = getValueIdx(UtempLine,idx);
            if(
               ((srcIdx >= UNIZONE_CJK_Unified_Ideographs) &&
                (srcIdx < UNIZONE_Yi_Syllables)) ||
               ((srcIdx >= UNIZONE_CJK_Compatibility_Ideographs) &&
                (srcIdx < UNIZONE_Alphabet_Presentation_Forms)) ||
               ((srcIdx >= UNIZONE_CJK_Compatibility_Forms) &&
                (srcIdx < UNIZONE_Small_Form_Variants))
            ){
               		idx++;
                    desIdx = getValueIdx(UtempLine,idx);
                    if((desIdx >= UNIZONE_Hangul_Syllables) &&
                    (desIdx < UNIZONE_High_Surrogates) )
                    {

                    loadHJAConvMap[srcIdx] = desIdx;
                    continue;
                    }
            }
            fatal_error("illegal value in HanMap\n");
    	}

	};
	void unLoadHJAMap()
    {
	 free(loadHJAConvMap);
	};

};
#endif //CODE_PAGE_KR

