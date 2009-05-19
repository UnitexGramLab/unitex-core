 /*
  * Unitex
  *
  * Copyright (C) 2001-2009 Université Paris-Est Marne-la-Vallée <unitex@univ-mlv.fr>
  *
  * This library is free LEintRead2software; you can redistribute it and/or
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
#include "Unicode.h"
#include "codeForKorean.h"
#include "etc.h"
#include "Error.h"

// using namespace std;


convert_windows949kr_uni::convert_windows949kr_uni()
{
    loadHJAConvMap = 0;
    mbcsUni949Table = new short int[256*128];
    uniMbcs949Table = new unsigned char[0x20000];
    strToMapKr();
    ready_uniMbcs = 1;
}
convert_windows949kr_uni::~convert_windows949kr_uni(){
delete [] mbcsUni949Table;
delete [] uniMbcs949Table;
if(loadHJAConvMap) unLoadHJAMap();
}
    
void 
convert_windows949kr_uni::mbcsToUniSz(unsigned char mbcs[],unichar *des,int mbcs_byte_cnt)
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
            *des++ =  (unichar)mbcsUni949Table[page*256 + off];
     }
}
void convert_windows949kr_uni::uniToMbcsSz(unichar *uchars,unsigned char *mbcs,int uchar_cnt)
{
    for( int i = 0; i < uchar_cnt;i++){
    *mbcs++ = uniMbcs949Table[*uchars++ * 2];
    *mbcs++=  uniMbcs949Table[*uchars++ * 2 +1];
    }
}
void convert_windows949kr_uni::mbcsToUniStr(unsigned char *mbcs,unichar *des)
{
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
           *des = (unichar)mbcsUni949Table[page*256 + off]; 
           ++des;               
     }
     *des++ = 0;
}
int convert_windows949kr_uni::mbcs949clen(unsigned char *mbcs)
{
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
}
void convert_windows949kr_uni::UniToMbcsStr(unichar *up,unsigned char *mbcs)
{
    while(*up){
            if( *up < 128){
            *mbcs++ = uniMbcs949Table[*up++ * 2];
            } else {
            *mbcs++ = uniMbcs949Table[*up++ * 2];
            *mbcs++=  uniMbcs949Table[*up++ * 2+1];
            }
    }
}


void convert_windows949kr_uni::strToMapKr()
{
    unichar *wp; 
   
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
//printf("page %x off %x",page,off);
      } 
//printf("\n");
    }
 //       getchar();
}
void 
convert_windows949kr_uni::unLoadHJAMap()
{
	free(loadHJAConvMap);
}
int
convert_windows949kr_uni::convHJAtoHAN(unichar *src,unichar *des)
{
	int csz = 0;
	unichar c;
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
}

void convert_windows949kr_uni::loadHJAMap(char *f)
{
	U_FILE *lf = u_fopen(UTF16_LE,f,U_READ);
	int idx;
	unichar srcIdx;
	unichar desIdx;
	unichar UtempLine[256];
	
	loadHJAConvMap = (unichar *)malloc( sizeof(unichar)*0x10000);
	if (loadHJAConvMap==NULL) {
	   fatal_alloc_error("convert_windows949kr_uni::loadHJAMap");
	}
	for(idx = 0; idx < 0x10000;idx++) loadHJAConvMap[idx] = 0;
	while(EOF!=u_fgets(UtempLine,lf)){
		idx = 0;
		if(UtempLine[idx] == (unichar)' ') continue;
		srcIdx = getValueIdx(UtempLine,idx);
		if( (( check_range_character(srcIdx) & TYPE_MASK) != HJA_SYL) || 
			((UtempLine[idx] != (unichar)' ') && 
			 (UtempLine[idx] != '\t')) )
			fatal_error("illegal value in HanMap\n");
		idx++;
		desIdx = getValueIdx(UtempLine,idx);
		if((check_range_character(desIdx) & TYPE_MASK) != HAN_SYL) 
			fatal_error("illegal value in HanMap\n");
		
		loadHJAConvMap[srcIdx] = desIdx;

	}
}

/* The same, but cloning an existing array */
void convert_windows949kr_uni::cloneHJAMap(unichar* map) {
if (map==NULL) {
   fatal_error("NULL error in convert_windows949kr_uni::cloneHJAMap\n");
}
loadHJAConvMap=(unichar*)malloc(sizeof(unichar)*0x10000);
if (loadHJAConvMap==NULL) {
   fatal_alloc_error("convert_windows949kr_uni::cloneHJAMap");
}
for (int i=0;i<0x10000;i++) {
   loadHJAConvMap[i]=map[i];
}
}

